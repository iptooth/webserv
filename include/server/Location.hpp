#ifndef     LOCATION_HPP
# define    LOCATION_HPP

# include <string>
# include <iostream>
# include <vector>
# include <map>
# include <sstream>

class Server;

class Location
{
private:
    size_t                              _max_body_size;
    std::string                         _route;
    std::string                         _root;
    std::vector<std::string>            _index;
    std::vector<std::string>            _methods;
    std::map<std::string, std::string>  _redir;
    bool                                _autoindex;
    std::map<std::string, std::string>  _error_pages;
    std::string                         _ret_path;
    std::string                         _cgi_path;
    std::vector<std::string>            _cgi_allowed_extensions;
    std::pair<std::string, std::string> _return;

public:
    Location();
    Location(const Location &src);
    ~Location();
    Location& operator=(const Location &rhs);

    /***** GETTERS *****/
    std::vector<std::string>            get_methods(void) const;
    size_t                              get_methods_size(void) const;
    size_t                              get_ext_size(void);
    bool                                get_autoindex(void);
    size_t                              get_max_body_size(void) const;
    std::string                         get_directive(std::string line);
    std::string                         get_route(void) const;
    std::string                         get_root(void) const;
    std::string                         get_ret_path(void) const;
    std::string                         get_cgi_path(void) const;
    std::string                         get_method(size_t index) const;
    std::string                         get_ext(size_t i);
    std::vector<std::string>            get_index(void) const;
    std::vector<std::string>            get_cgi_ext(void) const;
    std::map<std::string, std::string>  get_error_pages(void) const;

    /***** SETTERS *****/
    void                                set_autoindex_to_true(void);
    void                                set_autoindex_to_false(void);
    void                                set_max_body_size(double mbs);
    void                                set_error_pages(std::map<std::string, std::string> current);
    void                                set_index(std::vector<std::string> current);
    void                                set_ret_path(std::string current);
    void                                set_cgi_path(std::string current);
    void                                set_cgi_ext(std::vector<std::string> current);
    void                                set_root(std::string current);
    void                                set_error_pages(std::string error_code, std::string path);
    void                                set_route(std::string path);
    void                                set_methods(std::vector<std::string> src);

    /***** PARSING *****/
    void                                parse_return_line(std::string line);
    void                                parse_autoindex_line(std::string line);
    void                                parse_route_line(std::string line);
    void                                parse_root_line(std::string line);
    void                                parse_index_line(std::string line);
    void                                parse_allow_methods_line(std::string line);
    void                                parse_client_max_body_size(std::string line);
    void                                parse_error_page_line(std::string line);
    void                                parse_cgi_path_line(std::string line);
    void                                parse_cgi_ext_line(std::string line);

    /***** MEMBER FUNCTIONS *****/
    bool                                has_methods(void) const;
    void                                req_serv_config(Server *cluster);
    void                                clear_inheritance(Server *cluster, std::string req_path);
    void                                req_loca_config(Location *loca, Server *cluster);
    std::string                         get_error_page(std::string status, std::string actual);
    std::string                         check_index_files(std::string root_path) const;

    /***** UTILS *****/
    bool                                has_index();
    void                                route_to_cgi(std::string uri);
};

#endif