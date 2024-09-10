/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nlence-l <nlence-l@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/06 16:33:57 by jlecorne          #+#    #+#             */
/*   Updated: 2024/09/10 14:24:57 by nlence-l         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef     SERVER_HPP
# define    SERVER_HPP

# include <iostream>
# include <string>
# include <vector>
# include <map>
# include <fstream>
# include <stdlib.h>
# include "Location.hpp"
# include <cctype>

class Server
{
private:
    std::vector<std::string>                                _methods;
    std::map<std::string, std::vector<std::string> >        _sockets;
    std::vector<std::string>                                _server_name;
    size_t                                                  _max_body_size;
    std::vector<std::string>                                _index;
    std::string                                             _root;
    std::map<std::string, std::string>                      _error_pages;
    std::vector<Location*>                                  _locations;
    bool                                                    _autoindex;
    std::string                                             _ret_path;
    std::string                                             _cgi_path;
    std::vector<std::string>                                _cgi_allowed_extensions;
    std::pair<std::string, std::string>                     _return;
    
public:
    Server();
    Server(const Server &src);
    ~Server();
    Server& operator=(const Server &rhs);

    /***** GETTERS *****/
    size_t                                                  get_locations_size() const;
    size_t                                                  get_methods_size(void) const;
    size_t                                                  get_sockets_size(void) const;
    bool                                                    get_autoindex(void);
    size_t                                                  get_max_body_size(void) const;
    Location*                                               get_location(size_t index) const;
    std::string                                             get_method(size_t index);
    std::string                                             get_root(void);
    std::string                                             get_ret_path(void);
    std::string                                             get_cgi_path(void);
    std::vector<Location*>                                  get_locations(void) const;
    std::vector<std::string>                                get_index(void);
    std::vector<std::string>                                get_cgi_ext(void);
    std::vector<std::string>                                get_methods(void);
    const std::vector<std::string>                          get_server_name(void) const;
    std::map<std::string, std::string>                      get_error_pages(void) const;
    const std::map<std::string, std::vector<std::string> >  get_sockets(void) const;
    std::vector<int>                                        get_ports(void);

    /***** SETTERS *****/
    void                                                    set_autoindex_to_true(void);

    /***** PARSING *****/
    std::string                                             parse_root_line(std::string line);
    int                                                     parse_listen_line(std::string line);
    void                                                    parser(std::ifstream& file);
    void                                                    parse_server_name_line(std::string line);
    void                                                    parse_location_line(std::ifstream& file, std::string line);
    void                                                    parse_index_line(std::string line);
    void                                                    parse_error_page_line(std::string line);
    void                                                    parse_client_max_body_size_line(std::string line);
    void                                                    parse_allow_methods_line(std::string line);
    void                                                    parse_return_line(std::string line);
    void                                                    parse_cgi_path_line(std::string line);
    void                                                    parse_cgi_ext_line(std::string line);

    /***** UTILS *****/
    bool                                                    has_methods(void) const;
    std::string                                             get_error_page(std::string status, std::string actual);
    std::string                                             check_index_files(std::string root_path) const;
};

#endif