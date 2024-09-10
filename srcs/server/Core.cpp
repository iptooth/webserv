/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Core.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jlecorne <jlecorne@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: Invalid Date        by                   #+#    #+#             */
/*   Updated: 2024/09/10 15:01:33 by jlecorne         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server/Core.hpp"
#include "../../include/http/CgiHandler.hpp"

class CgiHandler;

Core::Core() : _curr_req(NULL), _curr_conf(NULL) {}

Core::~Core() {
    close_all_sockets();

    for (std::map<int, Session*>::iterator it = _sessions.begin(); it != _sessions.end(); ++it) {
        delete it->second;
    }
    _sessions.clear();

    for (size_t i = 0; i < this->_clusters.size(); ++i)
        delete this->_clusters[i];
    
    std::map<int, Response*>::iterator it;
    for (it = this->_clients.begin(); it != this->_clients.end(); ++it)
        delete it->second;
        
    this->_clusters.clear();
    this->_clients.clear();
    this->_servers.clear();
    this->_connections.clear();
    
    if (this->_curr_req != NULL)
        delete this->_curr_req;
    if (this->_curr_conf != NULL)
        delete this->_curr_conf;
}

/***** GETTERS *****/
const   std::map<int, std::pair<std::string, int> > & Core::get_connections() const {
    return this->_connections;
}

const   std::map<int, std::pair<std::string, int> > & Core::get_servers() const {
    return this->_servers;
}

/***** PROCESS *****/
void        Core::core_parser(std::string& file) {
    std::ifstream config_file(file.c_str());
    
    if (!config_file) {
        std::cerr << "Failed to open configuration file" << '\n';
    }
    Server* tmp = NULL;
    std::string line;
    size_t pos;
    
    while (std::getline(config_file, line)) {
        pos = line.find("server {");
        if (pos != std::string::npos) {
            tmp = new Server();
            tmp->parser(config_file);
            this->_clusters.push_back(tmp);
        } else {
            continue;
        }
    }
    config_file.close();
}

void Core::remove_duplicates(std::vector<int>& vec) {
    std::vector<int> result;

    for (std::vector<int>::iterator it = vec.begin(); it != vec.end(); ++it) {
        bool is_duplicate = false;

        for (std::vector<int>::iterator jt = result.begin(); jt != result.end(); ++jt) {
            if (*it == *jt) {
                is_duplicate = true;
                break;
            }
        }

        if (!is_duplicate) {
            result.push_back(*it);
        }
    }
    vec = result;
}
int         Core::bind_ports(std::vector<pollfd>& poll_fds) {
    for (size_t i = 0; i < this->_clusters.size(); i++) {
        std::vector<int> temp;
        temp = this->_clusters[i]->get_ports();
        this->_ports.insert(this->_ports.end(), temp.begin(), temp.end());
    }
    
    remove_duplicates(this->_ports);

    for (size_t i = 0; i < this->_ports.size(); i++) {
        int server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_sockfd == -1) {
            std::cerr << "Socket error: " << strerror(errno) << '\n';
            return 2;
        }
        sockaddr_in sa;
        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = INADDR_ANY;
        sa.sin_port = htons(this->_ports[i]);

        if (bind(server_sockfd, (struct sockaddr*)&sa, sizeof(sa)) == -1) {
            std::cerr << "Bind error: " << strerror(errno) << '\n';
            close(server_sockfd);
            return 3;
        }

        if (listen(server_sockfd, 10) == -1) {
            std::cerr << "Listen error: " << strerror(errno) << '\n';
            close(server_sockfd);
            return 4;
        }
        char buffer[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(sa.sin_addr), buffer, INET_ADDRSTRLEN);
        std::string server_ip(buffer);
        this->_servers[server_sockfd] = std::make_pair(server_ip, _ports[i]);
        pollfd server_socket = { server_sockfd, POLLIN, 0 };
        poll_fds.push_back(server_socket);
        
        if (this->_ports.empty()) {
            std::cerr << "No ports to bind." << '\n';
            return 1;
        }
        std::cout << "Server listening on port " << this->_ports[i] << '\n';
    }
    return 0;
}

int         Core::webserv() {
    std::vector<pollfd>         fds;
    
    this->bind_ports(fds);

    while (true) {
        int poll_count = poll(fds.data(), fds.size(), -1);
        if (poll_count < 0) {
            std::cout << "Poll error" << std::endl;
            break;
        }
        
        for (size_t i = 0; i < fds.size(); i++) {
            if ((this->is_server_socket(fds[i].fd)) && (fds[i].revents & POLLIN)) {
                std::string requested_ip;
                int requested_port;
                struct sockaddr_in ca;
                socklen_t addr_size = sizeof(ca);
                
                int client_sockfd = accept(fds[i].fd, (struct sockaddr*)&ca, &addr_size);
                
                if (client_sockfd < 0) {
                    std::cerr << "Accept error: " << strerror(errno) << '\n';
                    return 1;
                }
                requested_ip = this->_servers[fds[i].fd].first;
                requested_port = this->_servers[fds[i].fd].second;
                pollfd client_pollfd = { client_sockfd, POLLIN, 0 };
                fds.push_back(client_pollfd);
                this->add_client(client_sockfd, requested_ip, requested_port);
                
            } else if ((this->is_client_socket(fds[i].fd)) && (fds[i].revents & POLLIN)) {
                this->_curr_req = new Request;
                this->_curr_conf = new Location;
                std::string head;
                char        buff;

                if (this->_clients[fds[i].fd])
                    delete this->_clients[fds[i].fd];
                this->_clients[fds[i].fd] = new Response;

                while (true) {
                    ssize_t bytes = recv(fds[i].fd, &buff, 1, 0);
                    head.push_back(buff);
                    size_t  head_end = head.find("\r\n\r\n");

                    if (bytes < 0 || (bytes == 0 && head_end == std::string::npos)) {
                        if (bytes < 0)
                            this->_clients[fds[i].fd]->req_return(this->_curr_conf, "500");
                        else
                            this->_clients[fds[i].fd]->req_return(this->_curr_conf, "400");
                        break;
                    } else if (head_end != std::string::npos) {
                        try {
                            this->_curr_req->http_header_parser(head);
                            this->check_http_request(this->_curr_conf, this->_curr_req, this->_clients[fds[i].fd], fds[i].fd);
                            this->req_body(this->_curr_conf, this->_curr_req, this->_clients[fds[i].fd], fds[i].fd);
                            this->handle_http_request(this->_curr_conf, this->_curr_req, this->_clients[fds[i].fd], fds[i].fd);
                        } catch (std::exception &e) {std::cout << e.what() << std::endl;}
                        break;
                    }
                }
                std::string set_cookie_header;
                if (session_exists(fds[i].fd) == true) {
                    set_cookie_header = create_set_cookie_header(this->_sessions[fds[i].fd]->session_id, "/pages/cgi/login.html");
                }

                this->_clients[fds[i].fd]->make_response(this->_curr_conf, set_cookie_header);
                
                fds[i].events = POLLOUT;
                delete this->_curr_req;
                this->_curr_req = NULL;
                delete this->_curr_conf;
                this->_curr_conf = NULL;
            }
            
            if ((this->is_client_socket(fds[i].fd)) && (fds[i].revents & POLLOUT)) {
                ssize_t bytes = send(fds[i].fd, this->_clients[fds[i].fd]->get_res().c_str() + this->_clients[fds[i].fd]->_sent_bytes, this->_clients[fds[i].fd]->get_resSize() - this->_clients[fds[i].fd]->_sent_bytes, 0);

                if (bytes < 0) {
                        continue;
                } else {
                    this->_clients[fds[i].fd]->_sent_bytes += bytes;
                    if (this->_clients[fds[i].fd]->_sent_bytes == this->_clients[fds[i].fd]->get_resSize()) {
                        std::map<int, Response*>::iterator it = this->_clients.find(fds[i].fd);
                        delete it->second;
                        this->_clients.erase(it);
                        close(fds[i].fd);
                        fds.erase(fds.begin() + i);
                        --i;
                    }
                }
            }
        }
    }
    return 0;
}

// Cookies
std::string Core::create_set_cookie_header(const std::string& session_id, const std::string& path, bool httpOnly, bool secure, int max_age) {
    std::string header = "Set-Cookie: session_id=" + session_id;
    header += "; Path=" + path;
    
    if (max_age > 0) {
        std::ostringstream oss;
        oss << max_age;
        std::string res = oss.str();
        
        header += "; Max-Age=" + res;
    }
    if (httpOnly) {
        header += "; HttpOnly";
    }
    if (secure) {
        header += "; Secure";
    }
    return header;
}

std::string Core::generate_session_id(void) {
    const std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string session_id;
    
    std::srand(static_cast<unsigned int>(time(0)));

    for (size_t i = 0; i < 16; ++i) {
        session_id += chars[std::rand() % chars.size()];
    }
    return session_id;
}

std::string Core::parse_session_id_from_response(std::string http_response) {
    std::string set_cookie_header = "Set-Cookie: ";
    std::string session_id_key = "session_id=";

    size_t pos = http_response.find(set_cookie_header);
    if (pos == std::string::npos) {
        return "";
    }

    pos += set_cookie_header.length();
    size_t end_of_header = http_response.find("\r\n", pos);
    if (end_of_header == std::string::npos) {
        end_of_header = http_response.length();
    }
    std::string cookie_header = http_response.substr(pos, end_of_header - pos);

    size_t start = cookie_header.find(session_id_key);
    if (start == std::string::npos) {
        return "";
    }

    start += session_id_key.length();
    size_t end = cookie_header.find(";", start);
    if (end == std::string::npos) {
        end = cookie_header.length();
    }

    return cookie_header.substr(start, end - start);
}

bool    Core::session_exists(int clientsockfd) {
    if (this->_sessions.count(clientsockfd)) {
        return true;
    }
    return false;
}

void        Core::get_method_handler(Server *cluster, Location* config, Request* req, Response* res) {
    if (find_cgi_ext(config, req->get_path()) && req_exists(config, req->get_path())) {
        get_config(cluster, config, config->get_cgi_path() + req->get_path());
        if (!is_method_allowed(config, req->get_method(), 1)) { 
            res->req_return(config, "405");
        } else if (!(config->get_ret_path().empty())) {
            res->req_return(config, config->get_ret_path());
        } else
            res->file_response(config, req->get_path());
    } else if (!req_exists(config, req->get_path())) {
            res->req_return(config, "404");
    } else if (!(config->get_ret_path().empty())) {
        res->req_return(config, config->get_ret_path());
    } else if (is_directory(config, req->get_path())) {
        if (config->has_index()) {
            res->index_response(config, req, res);
        } else if (config->get_autoindex() == true) {
            directory_listing_handler(config, req, res);
        } else
            res->req_return(config, "403");
    } else {
        if (is_empty(config, req->get_path())) {
            res->req_return(config, "204");
        } else if (bad_type(req, req->get_path())) {
            res->req_return(config, "406");
        } else
            res->file_response(config, req->get_path());
    }
}

void        Core::post_method_handler(int clientsockfd, Server* cluster, Location* config, Request* req, Response* res) {
    CgiHandler cgi;
        
    if (find_cgi_ext(config, req->get_path()) && req_exists(config, req->get_path()) && sent_from_site(cluster, req)) {
        if (!ref_method(cluster, config, req, res)) {
            res->req_return(config, "405");
            throw Core::NotMatchingConf();
        }
        config->route_to_cgi(req->get_path());

        bool client_has_session = false;
        if (session_exists(clientsockfd) == true) {
            client_has_session = true;
        }
        cgi.init_env(cluster, req, client_has_session);
        cgi.exec_cgi(config->get_route(), config, res);
        if (cgi.parse_cgi(config, req, res) == true) {
            std::string session_id = generate_session_id();
            Session* new_session = create_session(session_id);
            this->_sessions[clientsockfd] = new_session;
        }
    } else if (!req_exists(config, req->get_path())) {
            res->req_return(config, "404");
    } else if (!(config->get_ret_path().empty())) {
        res->req_return(config, config->get_ret_path());
    } else if (req->upload_req()) {
        uploader(config, req, res);
    } else
        res->req_return(config, "400");
}

void        Core::delete_method_handler(Location* config, Request* req, Response* res) {
    if (!req_exists(config, req->get_path())) {
            res->req_return(config, "404");
    } else if (!(config->get_ret_path().empty())) {
        res->req_return(config, config->get_ret_path());
    } else if (is_directory(config, req->get_path())) {
        res->req_return(config, "403");
    } else
        this->req_delete(config, req, res);
}

void        Core::check_http_request(Location *config, Request *req, Response *res, int client_sockfd) {
    Server*         cluster = match_cluster(req, client_sockfd);
    std::string     ret;

    get_config(cluster, config, req->get_path());
    if (!cluster) {
        res->req_return(config, "500");
        throw Core::InterServError();
    }

    if (config->has_methods()) {
        if (!is_method_allowed(config, req->get_method(), 0)) {
            res->req_return(config, "405");
            throw Core::NotMatchingConf();
        }
    }

    if ((string_to_st(req->get_header("Content-Length")) > 0 || req->chunked() == true)
        && config->get_max_body_size() > 0
        && string_to_st(req->get_header("Content-Length")) > config->get_max_body_size()) {
            res->req_return(config, "413");
            throw Core::NotMatchingConf();
    }
}

void        Core::handle_http_request(Location *config, Request *req, Response *res, int client_sockfd) {
    Server      *cluster = match_cluster(req, client_sockfd);

    get_config(cluster, config, req->get_path());
    if (!cluster) {
        res->req_return(config, "500");
        throw Core::InterServError();
    }

    if (req->get_method() == "GET")
        this->get_method_handler(cluster, config, req, res);
    else if (req->get_method() == "POST")
        this->post_method_handler(client_sockfd, cluster, config, req, res);
    else if (req->get_method() == "DELETE")
        this->delete_method_handler(config, req, res);
    else
        res->req_return(config, "405");
}

void        Core::req_body(Location *config, Request *req, Response *res, int client_sockfd) {
    Server      *cluster = match_cluster(req, client_sockfd);

    get_config(cluster, config, req->get_path());
    if (!cluster) {
        res->req_return(config, "500");
        throw Core::InterServError();
    }
    
    if (req->has_body()) {
        if (req->has_body() == 1)
            recv_chunked(client_sockfd, req, res, config);
        else if (req->has_body() == 2)
            recv_multipart(client_sockfd, req, res, config);
        else
            recv_contLen(client_sockfd, req, res, config);
    }
}

/***** UTILS *****/
void        Core::close_all_sockets() {
    std::map<int, std::pair<std::string, int> > ::iterator it;

    for (it = this->_servers.begin(); it != this->_servers.end(); ++it) {
        close(it->first);
    }
    for (it = this->_connections.begin(); it != this->_connections.end(); ++it) {
        close(it->first);
    }
}

void        Core::add_client(int client_sockfd, std::string requested_ip, int requested_port) {
    this->_connections[client_sockfd] = std::make_pair(requested_ip, requested_port);
}

bool        Core::is_server_socket(int sockfd) {
    std::map<int, std::pair<std::string, int> > ::iterator it;
    for (it = this->_servers.begin(); it != this->_servers.end(); it++) {
        if (it->first == sockfd) {
            return true;
        }
    }
    return false;
}

bool        Core::is_client_socket(int sockfd) {
    std::map<int, std::pair<std::string, int> > ::iterator it;
    for (it = this->_connections.begin(); it != this->_connections.end(); it++) {
        if (it->first == sockfd) {
            return true;
        }
    }
    return false;    
}

bool        Core::is_cgi_script(std::string filename) {
    if (((filename.find_last_of(".py") != std::string::npos) ||
        (filename.find_last_of(".php") != std::string::npos)) &&
            filename.find("/cgi-bin/") != std::string::npos) {
        return true;
    } else {
        return false;
    }
}

void        Core::req_delete(Location *config, Request *req, Response *res) {
    std::string path;

    if (content_type(config, config->get_route()) == "script/cgi")
        path = config->get_cgi_path() + req->get_path();
    else
        path = config->get_root() + req->get_path();

    if (file_exists(path)) {
        if (access(path.c_str(), W_OK) == 0) {
            if (remove(path.c_str()) == 0)
                res->req_return(config, "204");
            else
                res->req_return(config, "500");
        } else
            res->req_return(config, "401");
    } else if (!file_exists(path)) {
        res->req_return(config, "404");
    }
}

Server*     Core::match_cluster(Request *req, int client_sockfd) {
    std::vector<size_t> candidates;
    Server*             result = NULL;
    std::string         requested_ip = this->_connections[client_sockfd].first;
    int                 requested_port = this->_connections[client_sockfd].second;

    for (size_t i = 0; i < this->_clusters.size(); i++) {
        std::map<std::string, std::vector<std::string> > sockets = this->_clusters[i]->get_sockets();

        for (std::map<std::string, std::vector<std::string> >::iterator it = sockets.begin(); it != sockets.end(); it++) {
            
            std::vector<int> ports;
            int port;

            for (size_t j = 0; j < it->second.size(); j++) {
                std::istringstream iss(it->second[j]);
                if (!(iss >> port)) {
                    std::cerr << "Invalid port: " << it->second[j] << std::endl;
                    continue;
                }
                ports.push_back(port);
            }

            if (requested_ip == it->first || requested_ip == "0.0.0.0" || requested_ip == "127.0.0.1") {
                for (size_t j = 0; j < ports.size(); j++) {
                    if (requested_port == ports[j]) {
                        candidates.push_back(i);
                        break;
                    }
                }
            }
        }
    }

    if (candidates.empty()) {
        std::cerr << "No matching server block found for client: " << requested_ip << ":" << requested_port << std::endl;
        return NULL;
    }

    std::vector<std::string> server_names;
    if (candidates.size() > 1) {
        for (size_t i = 0; i < candidates.size(); i++) {
            server_names = this->_clusters[candidates[i]]->get_server_name();
            for (size_t j = 0; j < server_names.size(); j++) {
                if (req->get_header("Host").find(server_names[j]) != std::string::npos) {
                    result = this->_clusters[candidates[i]];
                    return result;
                }
            }
        }
    }

    result = this->_clusters[candidates[0]];
    return result;
}

Location*   Core::match_location(Request *req, Server* server_block) {
    std::string uri = req->get_path();
    std::string route;
    std::vector<Location*> locations;
    std::vector<Location*>::iterator it;

    locations = server_block->get_locations();
    for (it = locations.begin(); it != locations.end(); ++it) {
        if ((uri.find(route) != std::string::npos) && route != "/") {
            return *it;
        }
    }
    it = locations.begin();
    return *it;
}

/***** EXCEPTIONS *****/
const char *Core::CantFindPort::what() const throw() {
    return ("\033[31mERROR\033[0m: Can't find port for this socket");
}

const char *Core::CantCreateSocket::what() const throw() {
    return ("\033[31mERROR\033[0m: Can't create socket");
}

const char *Core::CantBindSocket::what() const throw() {
    return ("\033[31mERROR\033[0m: Can't bind the socket to port");
}

const char *Core::ListeningError::what() const throw() {
    return ("\033[31mERROR\033[0m: Failed listening on port");
}

const char *Core::NotMatchingConf::what() const throw() {
    return ("\033[31mERROR\033[0m: Can't continue request for config reason");
}

const char *Core::InterServError::what() const throw() {
    return ("\033[31mERROR\033[0m: The server encountered an internal error");
}

const char *Core::BodyRecvError::what() const throw() {
    return ("\033[31mERROR\033[0m: Error receiving request's body");
}
