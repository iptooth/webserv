#include "../../include/http/Response.hpp"

Response::Response() : _sent_bytes(0) {
    this->_status_map["200"] = " OK";
    this->_status_map["201"] = " Created";
    this->_status_map["204"] = " No Content";
    this->_status_map["301"] = " Moved Permanently";
    this->_status_map["302"] = " Found";
    this->_status_map["303"] = " See Other";
    this->_status_map["307"] = " Temporary Redirect";
    this->_status_map["308"] = " Permanent Redirect";
    this->_status_map["400"] = " Bad Request";
    this->_status_map["401"] = " Unauthorized";
    this->_status_map["403"] = " Forbidden";
    this->_status_map["404"] = " Not Found";
    this->_status_map["405"] = " Method Not Allowed";
    this->_status_map["408"] = " Request Timeout";
    this->_status_map["413"] = " Payload Too Large";
    this->_status_map["414"] = " URI Too Long";
    this->_status_map["415"] = " Unsupported Media Type";
    this->_status_map["500"] = " Internal Server Error";
    this->_status_map["501"] = " Not Implemented";
    this->_status_map["503"] = " Service Unavailable";
    this->_status_map["505"] = " HTTP Version Not Supported";
    this->_status_map["507"] = " Inssuficient Storage";
    this->_status_map["508"] = " Loop Detected";
}

Response::~Response() {}



/***** GETTERS *****/
std::string Response::get_file() {return this->_file;}

std::string Response::get_location() {return this->_location;}

std::string Response::get_status() {return this->_status;}

std::string Response::get_status_msg(std::string status) {return this->_status_map[status];}

std::string Response::get_res() {return this->_res;}

size_t      Response::get_resSize() {return this->_res.size();}

std::string Response::get_body() {return this->_body;}



/***** SETTERS *****/
void        Response::set_body(const std::string body) {this->_body = body;}

void        Response::set_file(const std::string file) {this->_file = file;}

void        Response::set_location(const std::string loca) {this->_location = loca;}

void        Response::set_status_code(std::string code) {this->_status = code;}

void        Response::set_status_msg(const std::string msg) {this->_status_msg = msg;}

void        Response::set_contentType(std::string type) {this->_content_type = type;}



/***** PROCESS *****/
void        Response::req_return(Location *config, std::string str) {
    std::pair<std::string, std::string> ret;
    ret.first = str.substr(0, 3);

    if (str.length() > 3) {
        ret.second = str.substr(4, str.length() - 4);

        if (is_url(ret.second) || req_exists(config, ret.second)) {
            this->_location = ret.second;
        } else {
            this->_body = ret.second;
            this->_content_type = "text/plain";
        }
    } else {
        std::string filename = config->get_root() + config->get_error_page(ret.first, "");
        std::string body = get_file_content(filename);
        if (body.empty()) {
            filename = config->get_root() + "/status/" + ret.first + ".html";
            body = get_file_content(filename);
        }
        if (body.empty()) {
            if (ret.first != "404") {
                this->_body = "The response ressource could not be found.";
                filename = ".html";
            }
        } else {
            this->_body = body;
        }
        if (filename.compare(".html"))
            this->_file = filename;
    }

    this->_status = ret.first;
    this->_status_msg = this->_status_map[ret.first];
    return;
}

void        Response::index_response(Location *config, Request *req, Response *res) {
    std::vector<std::string>    indexes = config->get_index();
    std::string                 route = config->get_route();

    if (!route.empty() && route[route.size() - 1] != '/') {
        route += "/";
    }
    for (size_t i = 0; i < indexes.size(); i++) {
        if (file_exists(config->get_root() + route + indexes[i])) {
            std::string filename = config->get_root() + route + indexes[i];
            std::string body = get_file_content(filename);
            
            this->_file = indexes[i];
            this->_body = body;
            this->_status = "200";
            this->_status_msg = this->_status_map["200"];
            return;
        }
    }
    if (config->get_autoindex() == false)
        req_return(config, "403");
    else
        directory_listing_handler(config, req, res);
}

void        Response::file_response(Location *config, std::string path) {
    size_t              pos = path.rfind("/");
    std::stringstream   ss;
    std::string         filename;
    std::string         filepath = config->get_root() + path;

    if (find_cgi_ext(config, path))
        filepath = config->get_cgi_path() + path;
    std::string         file_content = get_file_content(filepath);

    if (pos != std::string::npos)
        filename = path.substr(pos + 1, path.length() - (pos + 1));
    else
        filename = path;

    if (path.find("uploads/") != std::string::npos) {
        ss << "Content-Disposition: attachment; filename=\"" << filename << "\"";
        this->_content_dispo = ss.str();
    } else if (!file_content.empty()) {
        ss << "Content-Disposition: inline";
        this->_content_dispo = ss.str();
    }
    this->_file = filename;
    this->_body = file_content;
    this->_status = "200";
    this->_status_msg = this->_status_map["200"];
    return;
}

void        Response::make_response(Location *config, std::string set_cookie_header) {
    std::stringstream head;
    
    head << "HTTP/1.1 " << this->_status << this->_status_msg << "\r\n";

    if (!this->_location.empty())
        head << "Location: " + this->_location + "\r\n";
    if (this->_body.size() > 0)
        head << "Content-Type: " + content_type(config, this->_file) + "\r\n";
    if (this->_body.size() > 0 && !this->_content_dispo.empty())
        head << this->_content_dispo + "\r\n";
    if (this->_body.size() > 0)
        head << "Content-Length: " + size_t_to_string(this->_body.size()) + "\r\n";
    if (set_cookie_header != "") {
        head << set_cookie_header + "\r\n";
    }
    
    head << "Connection: keep-alive \r\n";
    head << "\r\n";
    this->_header = head.str();
    this->_res = this->_header + this->_body;
}