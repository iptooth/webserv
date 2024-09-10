#ifndef     RESPONSE_HPP
# define    RESPONSE_HPP

#include <iostream>
#include <string>
#include "../server/Location.hpp"
#include "../server/Core.hpp"

class Location;
struct Session;

class Response
{
private:
    std::string                         _header;
    std::string                         _body;
    std::string                         _res;
    std::string                         _status;
    std::string                         _status_msg;
    std::string                         _content_type;
    std::string                         _content_dispo;
    std::string                         _file;
    std::string                         _location;
    std::map<std::string, std::string>  _status_map;
    
public:
    size_t                              _sent_bytes;

    Response();
    ~Response();

    /***** GETTERS *****/
    std::string                         get_file();
    std::string                         get_location();
    std::string                         get_status();
    std::string                         get_status_msg(std::string status);
    std::string                         get_res();
    size_t                              get_resSize();
    std::string                         get_body();

    /***** SETTERS *****/
    void                                set_body(const std::string body);
    void                                set_file(const std::string file);
    void                                set_location(const std::string loca);
    void                                set_status_code(std::string code);
    void                                set_status_msg(const std::string msg);
    void                                set_contentType(std::string type);

    /***** PROCESS *****/
    void                                file_response(Location *config, std::string path);
    void                                req_return(Location *config, std::string str);
    void                                index_response(Location *config, Request *req, Response *res);
    std::string                         generate_session_id(void);
    std::string                         create_set_cookie_header(const std::string& session_id, const std::string& path = "/", const std::string& domain = "", bool httpOnly = true, bool secure = false, int max_age = 0);
    void                                make_response(Location *config, std::string set_cookie_header);
};
std::string                             content_type(Location *config, const std::string& filename);

#endif
