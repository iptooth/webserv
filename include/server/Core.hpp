/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Core.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nlence-l <nlence-l@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/13 20:52:00 by jlecorne          #+#    #+#             */
/*   Updated: 2024/09/10 14:17:24 by nlence-l         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef     Core_HPP
# define    Core_HPP

#define RED "\033[31m"
#define GREY "\033[90m"
#define BLUE "\033[34m"
#define RESET "\033[0m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define MAGENTA "\033[35m"

#define CHUNK_SIZE 4096

# include <iostream>
# include <sstream>
# include <fstream>
# include <string>
# include <vector>
# include <map>
# include <utility>
# include <poll.h>
# include <unistd.h>
# include <sys/socket.h>
# include <sys/stat.h>
# include <netinet/in.h>
# include <fcntl.h>
# include <algorithm>
# include <string.h>
# include <errno.h>
# include <arpa/inet.h>
# include <sys/types.h>
# include <dirent.h>
# include <sys/stat.h>
# include <csignal>
# include <cstdlib>
# include <cctype>

# include "../http/Request.hpp"
# include "../http/Response.hpp"
# include "Server.hpp"
# include "Location.hpp"
# include "../http/Part.hpp"

class Response;
class CGIHandler;

const std::string CGI_PATH = "./cgi-bin/";

// struct Cookie {
//     std::string name;
//     std::string value;
//     std::string path;
//     std::string domain;
//     bool secure;
//     bool http_only;
//     time_t expiry;
// };

 struct Session {
    std::string session_id;
    std::map<std::string, std::string> data;
    time_t expiry;
};

class Core
{
private:
    std::vector<int>                                    _ports;
    std::vector<Server*>                                _clusters;
    std::map<int, std::pair<std::string, int> >         _servers;
    std::map<int, std::pair<std::string, int> >         _connections;
    std::map<int, Response*>                            _clients;
    Request*                                            _curr_req;
    Location*                                           _curr_conf;
    std::map<int, Session*>                             _sessions;


public:
    Core();
    ~Core();

    /***** GETTERS *****/
    const std::map<int, std::pair<std::string, int> >&  get_connections() const;
    const std::map<int, std::pair<std::string, int> >&  get_servers() const;

    /***** PROCESS *****/
    int                                                 webserv(void);
    int                                                 bind_ports(std::vector<pollfd>& poll_fds);
    void                                                req_delete(Location *config, Request *req, Response *res);
    void                                                get_method_handler(Server *cluster, Location* config, Request* req, Response* res);
    void                                                post_method_handler(int clientsockfd , Server* cluster, Location* config, Request* req, Response* res);
    void                                                delete_method_handler(Location* config, Request* req, Response* res);
    void                                                handle_http_request(Location *config, Request *req, Response *res, int client_sockfd);
    void                                                check_http_request(Location *config, Request *req, Response *res, int client_sockfd);
    void                                                req_body(Location *config, Request *req, Response *res, int socket);
    void                                                add_client(int client_sockfd, std::string requested_ip, int requested_port);
    void                                                core_parser(std::string& file);
    void                                                remove_duplicates(std::vector<int>& vec);

    // Cookies
    // Session*                                            create_session(std::string session_id);
    bool                                                session_exists(int clientsockfd);
    std::string                                         generate_session_id(void);
    std::string                                         parse_session_id_from_response(std::string http_response);
    std::string                                         create_set_cookie_header(const std::string& session_id, const std::string& path, bool httpOnly = true, bool secure = true, int max_age = 3600);


    /***** UTILS *****/
    void                                                close_all_sockets();
    bool                                                is_server_socket(int sockfd);
    bool                                                is_client_socket(int sockfd);
    bool                                                is_cgi_script(std::string filename);
    Server*                                             match_cluster(Request *req, int client_sockfd);
    Location*                                           match_location(Request *req, Server* server_block);

    /***** EXCEPTIONS *****/
    class CantFindPort : public std::exception {
        public: virtual const char *what() const throw();
    };
    class CantCreateSocket : public std::exception {
        public: virtual const char *what() const throw();
    };
    class CantBindSocket : public std::exception {
        public: virtual const char *what() const throw();
    };
    class ListeningError : public std::exception {
        public: virtual const char *what() const throw();
    };
    class NotMatchingConf : public std::exception {
        public: virtual const char *what() const throw();
    };
    class InterServError : public std::exception {
        public: virtual const char *what() const throw();
    };
    class BodyRecvError : public std::exception {
        public: virtual const char *what() const throw();
    };
    
    
};

template<typename T>
void                                    printMsg(T msg, std::string color, int endl) {
    std::cout << color << msg << RESET;
    for (int i = 0; i < endl; i++)
        std::cout << std::endl;
}

/***** utils.cpp *****/
bool                                                    ref_method(Server *cluster, Location *config, Request *req, Response *res);
bool                                                    is_method_allowed(Location* config, std::string method, int mode);
bool                                                    sent_from_site(Server *cluster, Request *req);
bool                                                    is_url(std::string str);
void                                                    get_config(Server *cluster, Location *config, std::string req_path);
std::string                                             exec_cgi(Location *config, const std::string& script_name, int timeout_seconds);
std::string                                             get_file_content(const std::string& filename);
std::string                                             read_line(int client_socket, Response *res, Location *config);
size_t                                                  max_body_size(const Request *req, Server *cluster);
bool                                                    find_cgi_ext(Location *config, std::string uri);
bool                                                    is_directory(Location *config, std::string uri);
bool                                                    req_exists(Location *config, std::string uri);
bool                                                    is_empty(Location *config, std::string uri);
std::string                                             getMimeType(const std::string& extension);
bool                                                    bad_type(Request *req, std::string uri);
size_t                                                  hex_string_to_size_t(const std::string& hex_str);
size_t                                                  string_to_size_t(std::string string);
std::string                                             size_t_to_string(size_t n);
std::string                                             int_to_string(int n);
int                                                     string_to_int(std::string string);
size_t                                                  string_to_st(std::string string);
bool                                                    file_exists(std::string filename);
void                                                    delete_file(std::string filename, Location* config, Response* res);
bool                                                    create_uploaded_file(std::string file_path, std::string file_content);
bool                                                    config_file_checker(std::string config_file);
bool                                                    location_block_indentation_checker(const std::string& line);
bool                                                    server_block_indentation_checker(const std::string& line);
bool                                                    trailing_semicolon_checker(const std::string& line);
Session*                                                create_session(std::string session_id);

                                      
void                                                    uploader(Location *config, Request *req, Response *res);                                             


/***** recv_body.cpp *****/
void                                                    recv_multipart(int socket, Request *req, Response *res, Location *config);
void                                                    recv_contLen(int socket, Request *req, Response *res, Location *config);
void                                                    recv_chunked(int socket, Request *req, Response *res, Location *config);
void                                                    part_headers(int client_socket, Part *part, Response *res, Location *config);

/***** dir_listing.cpp *****/
void                                                    directory_listing_handler(Location* config, Request* req, Response* res);

#endif
