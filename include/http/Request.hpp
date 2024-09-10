#ifndef     REQUEST_HPP
# define    REQUEST_HPP

#include <string>
#include <sstream>
#include <vector>
#include "Part.hpp"

class Request
{
private:
    int                                 _port;
    bool                                _chunked;
    std::string                         _header;
    std::string                         _method;
    std::string                         _path;
    std::string                         _protocol;
    std::string                         _body;
    std::string                         _fragment;
    std::string                         _boundary;
    std::string                         _ip_addr;
    std::string                         _referer;
    std::vector<std::string>            _chunks;
    std::vector<Part*>                  _parts;
    std::string                         _query_string;
    std::map<std::string, std::string>  _parameters;
    std::map<std::string, std::string>  _headers;
    std::map<std::string, std::string>  _cookies;

public:
    Request();
    ~Request();

    /***** GETTERS *****/
    std::string                         get_full_header(void) const;
    std::string                         get_header(std::string header_name);
    int                                 get_port(void) const;
    std::string                         get_path(void) const;
    std::string                         get_bound(void);
    std::string                         get_ip_addr(void) const;
    std::string                         get_query_string(void) const;
    std::string                         get_method(void) const;
    std::string                         get_http_protocol(void) const;
    std::string                         get_body(void) const;
    std::vector<Part*>                  get_parts(void);
    std::string                         get_session_id(void);

    /***** SETTERS *****/
    void                                add_chunk(std::string chunk);
    void                                add_part(Part* part);
    void                                set_body(void);

    /***** PARSING *****/
    void                                body_type();
    void                                parse_request_line(std::string header);
    void                                parse_headers(std::string header);
    void                                http_header_parser(std::string header);
    void                                parse_key_value_pairs();
    std::pair<std::string, std::string> parameters_parser(std::string string);
    std::map<std::string, std::string>  parse_cookie_header(std::string cookie_header);

    /***** UTILS *****/
    int                                 has_body(void);
    bool                                upload_req(void);
    bool                                chunked() const;
    void                                print_infos(void) const;
};

#endif