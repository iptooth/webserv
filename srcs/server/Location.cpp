#include "../../include/server/Location.hpp"
#include "../../include/server/Server.hpp"
#include "../../include/server/Core.hpp"


Location::Location()
    :   _max_body_size(0),
        _route(""),
        _root(""),
        _autoindex(false),
        _ret_path(""),
        _cgi_path("")
{}

Location::Location(const Location &src)
    : _max_body_size(src._max_body_size),
      _route(src._route),
      _root(src._root),
      _index(src._index),
      _methods(src._methods),
      _redir(src._redir),
      _autoindex(src._autoindex),
      _error_pages(src._error_pages)
{}

Location::~Location() {}

Location& Location::operator=(const Location &rhs) {
    if (this != &rhs) {
        _max_body_size = rhs._max_body_size;
        _route = rhs._route;
        _root = rhs._root;
        _index = rhs._index;
        _methods = rhs._methods;
        _redir = rhs._redir;
        _autoindex = rhs._autoindex;
        _error_pages = rhs._error_pages;
    }
    return *this;
}

/***** GETTERS *****/
size_t      Location::get_ext_size(void) {
    return this->_cgi_allowed_extensions.size();
}

bool        Location::get_autoindex(void) {
    return this->_autoindex;
}

size_t      Location::get_max_body_size(void) const {
    return this->_max_body_size;
}

std::string Location::get_directive(std::string line) {
    std::string result;
    size_t start_index = 8;

    if (line.length() < 8)
        return "";
    size_t end_pos = line.find(" ", start_index);
    result = line.substr(8, end_pos - 8);

    if (result.compare("autoindex") == 0) {
        std::string state;
        size_t      pos = end_pos;
        
        end_pos = line.find(";", end_pos);
        state = line.substr(pos, line.length() - (end_pos - 2));
        result += state;
    }
    return result;
}

std::string Location::get_route(void) const {
    return this->_route;
}

std::string Location::get_root(void) const {
    return this->_root;
}

std::string Location::get_ret_path(void) const {
    return this->_ret_path;
}

std::string Location::get_cgi_path(void) const {
    return this->_cgi_path;
}

std::string Location::get_ext(size_t i) {
    return this->_cgi_allowed_extensions[i];
}

size_t Location::get_methods_size(void) const {
    return this->_methods.size();
}

std::vector<std::string> Location::get_index(void) const {
    return this->_index;
}

std::vector<std::string> Location::get_methods(void) const {
    return this->_methods;
}

std::vector<std::string> Location::get_cgi_ext(void) const {
    return this->_cgi_allowed_extensions;
}

std::map<std::string, std::string> Location::get_error_pages(void) const {
    return this->_error_pages;
}

std::string Location::get_method(size_t index) const {
    return this->_methods[index];
}

/***** SETTERS *****/
void    Location::set_autoindex_to_true(void) {
    this->_autoindex = true;
}

void    Location::set_autoindex_to_false(void) {
    this->_autoindex = false;
}

void    Location::set_max_body_size(double mbs) {
    this->_max_body_size = mbs;
}

void    Location::set_error_pages(std::map<std::string, std::string> current) {
    this->_error_pages = current;
}

void    Location::set_index(std::vector<std::string> current) {
    this->_index = current;
}

void    Location::set_methods(std::vector<std::string> current) {
    this->_methods = current;
}

void    Location::set_ret_path(std::string current) {
    this->_ret_path = current;
}

void    Location::set_cgi_path(std::string current) {
    this->_cgi_path = current;
}

void    Location::set_cgi_ext(std::vector<std::string> current) {
    this->_cgi_allowed_extensions = current;
}

void    Location::set_root(std::string current) {
    this->_root = current;
}

void    Location::set_error_pages(std::string error_code, std::string path) {
    this->_error_pages[error_code] = path;
}

void    Location::set_route(std::string path) {
    this->_route = path;
}

/***** PARSING *****/
void        Location::parse_return_line(std::string line) {
    std::string s1;

    size_t pos = line.find(" ", 14);
    size_t end_pos = line.find(";", pos + 1);
    s1 = line.substr(pos + 1, end_pos - (pos + 1));
    this->_ret_path = s1;
}

void        Location::parse_autoindex_line(std::string line) {
    std::string result;

    size_t pos = line.find(" ", 8);

    result = line.substr(pos + 1, line.length() - (pos + 1) - 1);
    if (result == "on") {
       this->_autoindex = true;
    }  
}

void        Location::parse_route_line(std::string line) {

        std::string result;

        size_t pos = line.find("/");
        size_t endPos = line.find_last_of(" ");
        if (pos != std::string::npos) {
            result = line.substr(pos, endPos - pos);
        }
        this->_route = result;
}

void        Location::parse_root_line(std::string line) {

    std::string result;

    size_t pos = line.find(" ", 8);

    result = line.substr(pos + 1, line.length() - (pos + 1) - 1);
    this->_root = result;
}

void        Location::parse_index_line(std::string line) {
    std::string result;

    size_t pos = line.find("index");
    line  = line.substr(pos, line.length() - pos);
    pos = line.find(" ");
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

void        Location::parse_allow_methods_line(std::string line) {

    std::string result;

    size_t pos = line.find(" ", 8);
    size_t endPos = line.find(" ", pos + 1);
    if (endPos == std::string::npos) {
        result = line.substr(pos + 1, line.length() - (pos + 1) - 1);
        this->_methods.push_back(result);
        return;
    } else {
        while (pos != std::string::npos && endPos != std::string::npos) {
            result = line.substr(pos + 1, endPos - (pos + 1));
            this->_methods.push_back(result);
            pos = endPos;
            endPos = line.find(" ", pos + 1);
        }
        result = line.substr(pos + 1, line.length() - (pos + 1) -1);
        this->_methods.push_back(result);
    }
}

void        Location::parse_client_max_body_size(std::string line) {
    long size = 0;
    std::string tmp;
    std::string result;

    size_t pos = line.find(" ", 8);
    size_t end_pos = line.find(";");

    if (pos != std::string::npos && end_pos != std::string::npos) {
        tmp = line.substr(pos + 1, end_pos - (pos + 1));

        for (size_t i = 0; i < tmp.length(); i++) {
            if ((!isdigit(tmp[i]) && tmp[i] != 'M' && tmp[i] != 'K')) {
                std::cerr << "client_max_body_size: bad argument.. setting to none" << std::endl;
                this->_max_body_size = size;
                return;
            }
        }
        
        if (tmp.find("M") != std::string::npos) {
            for (size_t i = 0; i < tmp.length(); i++) {
                if ((tmp[i] == 'M' && i != tmp.length() - 1) || tmp[i] == 'K') {
                    std::cerr << "client_max_body_size: bad argument.. setting to none" << std::endl;
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

void        Location::parse_error_page_line(std::string line) {

    std::string status_code;
    std::string file_name;

    size_t pos = line.find(" ", 8);
    size_t endPos = line.find(" ", pos + 1);
    status_code = line.substr(pos + 1, endPos - (pos + 1));

    pos = endPos + 1;
    file_name = line.substr(pos, line.length() - pos - 1);
    this->_error_pages.insert(std::make_pair(status_code, file_name));
}

void        Location::parse_cgi_path_line(std::string line) {
    std::string res;
    size_t      pos = line.find(" ", 8);
    size_t      endPos = line.find(";", pos + 1);

    res = line.substr(pos + 1, endPos - (pos + 1));
    this->_cgi_path = res;
}

void        Location::parse_cgi_ext_line(std::string line) {
    std::string res;

    size_t pos = line.find(" ", 8);
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

/***** MEMBER FUNCTIONS *****/
bool        Location::has_methods(void) const {
    if (this->_methods.empty()) {
        return false;
    }
    return true;
}

void        Location::req_serv_config(Server *cluster) {
    this->set_root(cluster->get_root());
    this->set_methods(cluster->get_methods());
    this->set_max_body_size(cluster->get_max_body_size());
    this->set_error_pages(cluster->get_error_pages());
    this->set_index(cluster->get_index());
    this->set_ret_path(cluster->get_ret_path());
    if (cluster->get_autoindex() == true)
        this->set_autoindex_to_true();
}

void        Location::clear_inheritance(Server *cluster, std::string req_path) {
    std::string path = req_path;
    std::string loca;
    size_t      pos;
    size_t      rpos;

    if (req_path[req_path.length() - 1] == '/') {
        if (req_path.compare("/") != 0) {
            path = req_path.substr(0, req_path.length() - 1);
            pos = path.rfind("/");

            if (pos != std::string::npos)
                path = path.substr(pos + 1, path.length() - (pos + 1));
        }
    } else {
        size_t ppos = req_path.find(".");
        rpos = req_path.rfind("/");
        pos = req_path.find("/");

        if (ppos != std::string::npos || !is_directory(this, req_path)) {
            path = req_path.substr(0, rpos + 1);
            rpos = req_path.rfind("/");
            pos = req_path.find("/");
        }
        while (pos != rpos) {
            path = path.substr(pos + 1, rpos);
            rpos = path.rfind("/");
            pos = path.find("/");
        }
    }

    Location    *tmp = NULL;

    for (size_t i = 0; i < cluster->get_locations_size(); i++) {
        loca = cluster->get_location(i)->get_route();

        if (loca[loca.length() - 1] != '/')
            loca += "/";

        if ((loca == "/" && path == "/") || (loca.find(path) != std::string::npos && path != "/"))
            tmp = cluster->get_location(i);
    }

    if (cluster->get_methods().empty() && !this->get_methods().empty() && ((tmp != NULL && tmp->get_methods().empty()) || tmp == NULL))
        this->_methods.clear();
        
    if (cluster->get_ret_path().empty() && !this->get_ret_path().empty() && ((tmp != NULL && tmp->get_ret_path().empty()) || tmp == NULL))
        this->set_ret_path("");
}

void        Location::req_loca_config(Location *loca, Server *cluster) {
    std::vector<std::string> null;
    std::map<std::string, std::string> error_pages = this->get_error_pages();
    std::map<std::string, std::string>::iterator it;

    if (!loca || !cluster) {
        return;
    }
    
    for (it = error_pages.begin(); it != error_pages.end(); ++it) {
        this->set_error_pages(it->first, it->second);
    } 

    if (loca->get_autoindex() == true || (loca->get_autoindex() == false && cluster->get_autoindex() == true)) {
        this->set_autoindex_to_true();
    } else
        this->set_autoindex_to_false();

    if (!(loca->get_index().empty())) {
        this->_index.clear();
        this->set_index(loca->get_index());
    } else if (loca->get_index().empty() && !(cluster->get_index().empty())) {
        this->_index.clear();
        this->set_index(cluster->get_index());
    } else
        this->_index.clear();

    if (!loca->get_methods().empty()) {
        this->_methods.clear();
        this->set_methods(loca->get_methods());
    } else if (loca->get_methods().empty() && !cluster->get_methods().empty()) {
        this->_methods.clear();
        this->set_methods(cluster->get_methods());
    } else
        this->_methods.clear();

    if (!(loca->get_ret_path().empty())) {
        this->set_ret_path(loca->get_ret_path());
    } else if (loca->get_ret_path().empty() && !(cluster->get_ret_path().empty()))
        this->set_ret_path(cluster->get_ret_path());
    else
        this->set_ret_path("");

    if (loca->get_max_body_size() > 0)
        this->set_max_body_size(loca->get_max_body_size());

    if (!(loca->get_root().empty()))
        this->set_root(loca->get_root());
}

std::string Location::get_error_page(std::string status, std::string actual) {
    if (!this->_error_pages.empty()) {
        for (std::map<std::string, std::string>::const_iterator it = this->_error_pages.begin(); it != this->_error_pages.end(); it++) {
            if (it->first.find(status) != std::string::npos) {
                return it->second;
            }
        }
    }
    return actual;
}

std::string Location::check_index_files(std::string root_path) const {
    for (size_t i = 0; i < this->_index.size(); i++) {
        if (file_exists(root_path + this->_index[i])) {
            return this->_index[i];
        }
    }
    return "";
}

/***** UTILS *****/
bool    Location::has_index() {
    if (this->_index.size() > 0)
        return true;
    return false;
}

void    Location::route_to_cgi(std::string uri) {
    size_t      pos = uri.rfind("/");
    std::string req = uri;

    if (pos != std::string::npos)
        req = uri.substr(pos, uri.length() - pos);
    else
        req = "/" + uri;
    this->_route = this->_cgi_path + req;
}