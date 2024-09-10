#include "../../include/server/Server.hpp"
#include "../../include/server/Core.hpp"

Server::Server()
    :   _max_body_size(0),
        _root(""),
        _autoindex(false),
        _ret_path(""),
        _cgi_path("")
{}

Server::Server(const Server &src) 
    : _methods(src._methods),
      _sockets(src._sockets),
      _server_name(src._server_name),
      _max_body_size(src._max_body_size),
      _index(src._index),
      _root(src._root),
      _error_pages(src._error_pages),
      _locations(),
      _autoindex(false)
{

    for (size_t i = 0; i < src._locations.size(); ++i) {
        _locations.push_back(new Location(*src._locations[i]));
    }
}

Server::~Server() {
    for (size_t i = 0; i < this->_locations.size(); i++)
        delete this->_locations[i];
    this->_locations.clear();
}

Server& Server::operator=(const Server &rhs) {
    if (this != &rhs) {
        _methods = rhs._methods;
        _sockets = rhs._sockets;
        _server_name = rhs._server_name;
        _max_body_size = rhs._max_body_size;
        _index = rhs._index;
        _root = rhs._root;
        _error_pages = rhs._error_pages;

        for (size_t i = 0; i < _locations.size(); ++i) {
            delete _locations[i];
        }
        _locations.clear();

        for (size_t i = 0; i < rhs._locations.size(); ++i) {
            _locations.push_back(new Location(*rhs._locations[i]));
        }
    }
    return *this;
}

/***** GETTERS *****/
size_t Server::get_locations_size() const {
    return this->_locations.size();
}

size_t Server::get_methods_size(void) const {
    return this->_methods.size();
}

size_t Server::get_sockets_size(void) const {
    return this->_sockets.size();
}

bool    Server::get_autoindex(void) {
    return this->_autoindex;
}

size_t  Server::get_max_body_size(void) const {
    return this->_max_body_size;
}

Location* Server::get_location(size_t index) const {
    return this->_locations[index];
}

std::string Server::get_method(size_t index) {
    if (this->get_methods_size() > index) {
        return this->_methods[index];
    } else {
        return "";
    }
}

std::string Server::get_root(void) {
    return this->_root;
}

std::string Server::get_ret_path(void) {
    return this->_ret_path;
}

std::string Server::get_cgi_path(void) {
    return this->_cgi_path;
}

std::vector<Location*>  Server::get_locations(void) const {
        return this->_locations;
}

std::vector<std::string> Server::get_index(void) {
    return this->_index;
}

std::vector<std::string> Server::get_cgi_ext(void) {
    return this->_cgi_allowed_extensions;
}

std::vector<std::string> Server::get_methods(void) {
    return this->_methods;
}

const std::vector<std::string> Server::get_server_name(void) const {
    return this->_server_name;
}

std::map<std::string, std::string>  Server::get_error_pages(void) const {
    return this->_error_pages;
}

const std::map<std::string, std::vector<std::string> > Server::get_sockets(void) const {    
    return this->_sockets;
}

/***** SETTERS *****/
void Server::set_autoindex_to_true(void) {
    this->_autoindex = true;
}

/***** PARSING *****/
std::vector<int> Server::get_ports(void) {
    std::vector<int> result;
    int temp;

    for (std::map<std::string, std::vector<std::string> >::iterator it = this->_sockets.begin(); it != this->_sockets.end(); ++it) {
        for (size_t i = 0; i < it->second.size(); ++i) {
            std::stringstream ss(it->second[i]);
            ss >> temp;
            result.push_back(temp);
        }
    }
    return result; 
}

std::string Server::parse_root_line(std::string line) {

    std::string result;

    size_t pos = line.find(" ", 4);

    result = line.substr(pos + 1, line.length() - (pos + 1) - 1);
    this->_root = result;
    return result;
}

int Server::parse_listen_line(std::string line) {
    std::string ip;
    std::string saved_ip;
    std::string port;
    std::string nbr;
    size_t point_pos;
    const char* char_ptr;
    size_t pos;
    std::string sub_str;
    int int_value;

    sub_str = line.substr(0, 11);
    if (sub_str != "    listen ") {
        std::cerr << "Syntax Error" << '\n';
        return 1;
    }
    sub_str = line.substr(11);
    pos = sub_str.find(":");
    if (pos != std::string::npos) {

        ip = sub_str.substr(0, pos);
        saved_ip = ip;

        point_pos = ip.find(".");
        if (point_pos != std::string::npos) {
            nbr = ip.substr(0, point_pos);
            for (size_t i = 0; i < nbr.length(); ++i) {
                if (!isdigit(nbr[i])) {
                    std::cerr << "[IP] Syntax Error" << '\n';
                    return 1;
                }
            }
            char_ptr = nbr.c_str();
            int_value = atoi(char_ptr);
            if (int_value < 0 || int_value > 255) {
                std::cerr << "[IP] Syntax Error" << '\n';
                return 1;
            }
        }

        ip = ip.substr(point_pos + 1);

        point_pos = ip.find(".");
        if (point_pos != std::string::npos) {
            nbr = ip.substr(0, point_pos);
            for (size_t i = 0; i < nbr.length(); ++i) {
                if (!isdigit(nbr[i])) {
                    std::cerr << "[IP] Syntax Error" << '\n';
                    return 1;
                }
            }
            char_ptr = nbr.c_str();
            int_value = atoi(char_ptr);
            if (int_value < 0 || int_value > 255) {
                std::cerr << "[IP] Syntax Error" << '\n';
                return 1;
            }
        }
        ip = ip.substr(point_pos + 1);
        point_pos = ip.find(".");
        if (point_pos != std::string::npos) {
            nbr = ip.substr(0, point_pos);
            for (size_t i = 0; i < nbr.length(); ++i) {
                if (!isdigit(nbr[i])) {
                    std::cerr << "[IP] Syntax Error" << '\n';
                    return 1;
                }
            }
            char_ptr = nbr.c_str();
            int_value = atoi(char_ptr);
            if (int_value < 0 || int_value > 255) {
                std::cerr << "[IP] Syntax Error" << '\n';
                return 1;
            }
        }
        ip = ip.substr(point_pos + 1);
        nbr = ip.substr(0);
        for (size_t i = 0; i < nbr.length(); ++i) {
            if (!isdigit(nbr[i])) {
                std::cerr << "[IP] Syntax Error" << '\n';
                return 1;
            }
        }
        char_ptr = nbr.c_str();
        int_value = atoi(char_ptr);
        if (int_value < 0 || int_value > 255) {
            std::cerr << "[IP] Syntax Error" << '\n';
            return 1;
        }
        port = sub_str.substr(pos + 1);
        for (size_t i = 0; i < port.length(); ++i) {
            if (!isdigit(port[i])) {
                std::cerr << "[PORT] Syntax Error" << '\n';
                return 1;
            }
        }
        char_ptr = port.c_str();
        int_value = atoi(char_ptr);
        if (int_value < 0 || int_value > 65535) {
            std::cerr << "[PORT] Syntax Error" << '\n';
            return 1;
        }
        this->_sockets[saved_ip].push_back(port);
        return 0;
    }
    pos = sub_str.find(".");
    if (pos != std::string::npos) {
        ip = sub_str;
        saved_ip = ip;

        point_pos = ip.find(".");
        if (point_pos != std::string::npos) {
            nbr = ip.substr(0, point_pos);
            for (size_t i = 0; i < nbr.length(); ++i) {
                if (!isdigit(nbr[i])) {
                    std::cerr << "[IP] Syntax Error" << '\n';
                    return 1;
                }
            }
            char_ptr = nbr.c_str();
            int_value = atoi(char_ptr);
            if (int_value < 0 || int_value > 255) {
                std::cerr << "[IP] Syntax Error" << '\n';
                return 1;
            }
        }
        ip = ip.substr(point_pos + 1);
        point_pos = ip.find(".");
        if (point_pos != std::string::npos) {
            nbr = ip.substr(0, point_pos);
            for (size_t i = 0; i < nbr.length(); ++i) {
                if (!isdigit(nbr[i])) {
                    std::cerr << "[IP] Syntax Error" << '\n';
                    return 1;
                }
            }
            char_ptr = nbr.c_str();
            int_value = atoi(char_ptr);
            if (int_value < 0 || int_value > 255) {
                std::cerr << "[IP] Syntax Error" << '\n';
                return 1;
            }
        }
        ip = ip.substr(point_pos + 1);
        point_pos = ip.find(".");
        if (point_pos != std::string::npos) {
            nbr = ip.substr(0, point_pos);
            for (size_t i = 0; i < nbr.length(); ++i) {
                if (!isdigit(nbr[i])) {
                    std::cerr << "[IP] Syntax Error" << '\n';
                    return 1;
                }
            }
            char_ptr = nbr.c_str();
            int_value = atoi(char_ptr);
            if (int_value < 0 || int_value > 255) {
                std::cerr << "[IP] Syntax Error" << '\n';
                return 1;
            }
        }
        ip = ip.substr(point_pos + 1);
        nbr = ip.substr(0);
        for (size_t i = 0; i < nbr.length(); ++i) {
            if (!isdigit(nbr[i])) {
                std::cerr << "[IP] Syntax Error" << '\n';
                return 1;
            }
        }
        char_ptr = nbr.c_str();
        int_value = atoi(char_ptr);
        if (int_value < 0 || int_value > 255) {
            std::cerr << "[IP] Syntax Error" << '\n';
            return 1;   
        }
        port = "80";
        this->_sockets[saved_ip].push_back(port);
        return 0;
    }
    saved_ip = "0.0.0.0";
    port = sub_str.substr(0, sub_str.length() - 1);
    for (size_t i = 0; i < port.length(); ++i) {
        if (!isdigit(port[i])) {
            std::cerr << " Only port [PORT] Syntax Error" << '\n';
            return 1;
        }
    }
    char_ptr = port.c_str();
    int_value = atoi(char_ptr);
    if (int_value < 0 || int_value > 65535) {
        std::cerr << "[PORT] Syntax Error" << '\n';
        return 1;
    }
    this->_sockets[saved_ip].push_back(port);
    return 0;
}


void Server::parser(std::ifstream& file) {
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }
        
        if (line == "}") {
            break;
        }

        size_t pos;
        
        pos = line.find("listen");
        if (pos != std::string::npos) {
            parse_listen_line(line);
            continue;
        }
    
        pos = line.find("server_name");
        if (pos != std::string::npos) {
            parse_server_name_line(line);
            continue;
        }

        pos = line.find("root");
        if (pos != std::string::npos) {
            parse_root_line(line);
            continue;
        }

        pos = line.find("return");
        if (pos != std::string::npos) {
            parse_return_line(line);
            continue;
        }

        pos = line.find("index");
        if (pos != std::string::npos) {
            parse_index_line(line);
            continue;
        }

        pos = line.find("location");
        if (pos != std::string::npos) {
            parse_location_line(file, line);
            continue;
        }

        pos = line.find("error_page");
        if (pos != std::string::npos) {
            parse_error_page_line(line);
            continue;
        }

        pos = line.find("client_max_body_size");
        if (pos != std::string::npos) {
            parse_client_max_body_size_line(line);
            continue;
        }

        pos = line.find("cgi_path");
        if (pos != std::string::npos) {
            parse_cgi_path_line(line);
            continue;
        }

        pos = line.find("cgi_ext");
        if (pos != std::string::npos) {
            parse_cgi_ext_line(line);
            continue;
        }
    }
}

void Server::parse_server_name_line(std::string line) {
    std::string result;

    size_t pos = line.find(" ", 4);
    size_t endPos = line.find(" ", pos + 1);
    if (endPos == std::string::npos) {
        result = line.substr(pos + 1, line.length() - (pos + 1) - 1);
        this->_server_name.push_back(result);
        return;
    } else {
        while (pos != std::string::npos && endPos != std::string::npos) {
            result = line.substr(pos + 1, endPos - (pos + 1));
            this->_server_name.push_back(result);
            pos = endPos;
            endPos = line.find(" ", pos + 1);
        }
        result = line.substr(pos + 1, line.length() - (pos + 1) -1);
        this->_server_name.push_back(result);
    }
}

void Server::parse_location_line(std::ifstream& file, std::string line) {
    try {
        Location* ptr = new Location();

        ptr->parse_route_line(line);

        std::string directive;
        std::string newLine;

        while (true) {
            std::getline(file, newLine);

            if (newLine == "    }") {
                break;
            }
            directive = ptr->get_directive(newLine);
            if (directive == "root") {
                ptr->parse_root_line(newLine);
            } else if (directive == "index") {
                ptr->parse_index_line(newLine);
            } else if (directive == "return") {
                ptr->parse_return_line(newLine);
            } else if (directive == "limit_except") {
                ptr->parse_allow_methods_line(newLine);
            } else if (directive == "return") {
                ptr->parse_return_line(newLine);
            } else if (directive == "autoindex on") {
                ptr->parse_autoindex_line(newLine);
            } else if (directive == "client_max_body_size") {
                ptr->parse_client_max_body_size(newLine);
            } else if (directive == "cgi_path") {
                ptr->parse_cgi_path_line(newLine);
            } else if (directive == "cgi_ext") {
                ptr->parse_cgi_ext_line(newLine);
            } else {
                continue;
            }
        }
        this->_locations.push_back(ptr);
    } catch (std::bad_alloc& ba) {
        std::cerr << "Bad allocation: " << ba.what() << '\n';
    }
}

void Server::parse_index_line(std::string line) {
    std::string result;

    size_t pos = line.find(" ", 4);
    size_t end_pos = line.find(" ", pos + 1);
    if (end_pos == std::string::npos) {
        result = line.substr(pos + 1, line.length() - (pos + 1) - 1);
        this->_index.push_back(result);
        return;
    } else {
        while (pos != std::string::npos && end_pos != std::string::npos) {
            result = line.substr(pos + 1, end_pos - (pos + 1));
            this->_index.push_back(result);
            pos = end_pos;
            end_pos = line.find(" ", pos + 1);
        }
        result = line.substr(pos + 1, line.length() - (pos + 1) - 1);
        this->_index.push_back(result);
    }
}

void Server::parse_error_page_line(std::string line) {
    std::string status_code;
    std::string file_name;

    size_t pos = line.find(" ", 4);
    size_t endPos = line.find(" ", pos + 1);
    status_code = line.substr(pos + 1, endPos - (pos + 1));

    pos = endPos + 1;
    file_name = line.substr(pos, line.length() - (pos + 1));
    this->_error_pages.insert(std::make_pair(status_code, file_name));
}

void Server::parse_client_max_body_size_line(std::string line) {
    long size = 0;
    std::string tmp;
    std::string result;

    size_t pos = line.find(" ", 4);
    size_t end_pos = line.find(";");

    if (pos != std::string::npos && end_pos != std::string::npos) {
        tmp = line.substr(pos + 1, end_pos - (pos + 1));

        for (size_t i = 0; i < tmp.length(); i++) {
            if (!isdigit(tmp[i]) && tmp[i] != 'M' && tmp[i] != 'K') {
                std::cerr << "client_max_body_size: bad argument.. setting to none" << std::endl;
                this->_max_body_size = size;
                return;
            }
        }
        
        if (tmp.find("M") != std::string::npos) {
            for (size_t i = 0; i < tmp.length(); i++) {
                if ((tmp[i] == 'M' && i != tmp.length() - 1) || tmp[i] == 'K') {
                    this->_max_body_size = size;
                    return;
                }
                size_t  mpos = tmp.find("M");
                result = tmp.substr(0, mpos);
                std::stringstream ss(result);
                ss >> size;
                size *= 1048576;
            }
        } else if (tmp.find("K") != std::string::npos) {
            for (size_t i = 0; i < tmp.length(); i++) {
                if ((tmp[i] == 'K' && i != tmp.length() - 1) || tmp[i] == 'M') {
                    std::cerr << "client_max_body_size: bad argument.. setting to none" << std::endl;
                    this->_max_body_size = size;
                    return;
                }
                size_t  mpos = tmp.find("K");
                result = tmp.substr(0, mpos);
                std::stringstream ss(result);
                ss >> size;
                size *= 1024;
            }
        } else {
            std::stringstream ss(tmp);
            ss >> size;
        }
    }
    this->_max_body_size = size;
}

void Server::parse_allow_methods_line(std::string line) {
    
    std::string temp;
    size_t pos;
    size_t end_pos;

    pos = line.find(" ", 4);
    end_pos = line.find(" ", pos + 1);
    while (end_pos != std::string::npos) {
        temp = line.substr(pos + 1, end_pos - (pos + 1));
        this->_methods.push_back(temp);
        pos = end_pos;
        end_pos = line.find(" ", pos + 1);
    }
    temp = line.substr(pos + 1, line.length() - (pos + 2));
    this->_methods.push_back(temp);
}

void Server::parse_return_line(std::string line) {
    std::string s1;

    size_t pos = line.find(" ", 10);
    size_t end_pos = line.find(";", pos + 1);
    s1 = line.substr(pos + 1, end_pos - (pos + 1));
    this->_ret_path = s1;
}

void    Server::parse_cgi_path_line(std::string line) {
    std::string res;
    size_t      pos = line.find(" ", 4);
    size_t      end_pos = line.find(";", pos + 1);

    res = line.substr(pos + 1, end_pos - (pos + 1));
    this->_cgi_path = res;
}

void    Server::parse_cgi_ext_line(std::string line) {
    std::string res;

    size_t pos = line.find(" ", 4);
    size_t end_pos = line.find(" ", pos + 1);
    if (end_pos == std::string::npos) {
        res = line.substr(pos + 1, line.length() - (pos + 1) - 1);
        this->_cgi_allowed_extensions.push_back(res);
        return;
    } else {
        while (pos != std::string::npos && end_pos != std::string::npos) {
            res = line.substr(pos + 1, end_pos - (pos + 1));
            this->_cgi_allowed_extensions.push_back(res);
            pos = end_pos;
            end_pos = line.find(" ", pos + 1);
        }
        res = line.substr(pos + 1, line.length() - (pos + 1) - 1);
        this->_cgi_allowed_extensions.push_back(res);
    }
}

/***** UTILS *****/
bool Server::has_methods(void) const {
    if (this->_methods.empty()) {
        return false;
    }
    return true;
}

std::string Server::get_error_page(std::string status, std::string actual) {
    if (!this->_error_pages.empty()) {
        for (std::map<std::string, std::string>::const_iterator it = this->_error_pages.begin(); it != this->_error_pages.end(); it++) {
            if (it->first.find(status) != std::string::npos)
                return it->second;
        }
    }
    return actual;
}

std::string Server::check_index_files(std::string root_path) const {
    for (size_t i = 0; i < this->_index.size(); i++) {
        if (file_exists(root_path + this->_index[i])) {
            return this->_index[i];
        }
    }
    return "";
}
