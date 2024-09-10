#include "../../include/server/Core.hpp"

bool        ref_method(Server *cluster, Location *config, Request *req, Response *res) {
    size_t              pos;
    std::ostringstream  port_stream;
    std::string         ref_path;
    std::string         port;
    std::string         referer = req->get_header("Referer");
    Location            *ref_config = new Location;

    port_stream << req->get_port();
    port = port_stream.str();
    pos = referer.find(port);

    if (pos == std::string::npos) {
        res->req_return(config, "500");
        throw Core::InterServError();
    }
    ref_path = referer.substr(pos + 4, ref_path.length() - (pos + 4));
    get_config(cluster, ref_config, ref_path);

    if (is_method_allowed(ref_config, req->get_method(), 0)) {
        delete ref_config;
        return true;
    }
    else {
        delete ref_config;
        return false;
    }
}

bool    is_method_allowed(Location* config, std::string method, int mode) {
    if (config->get_methods_size() > 0) {
        for (size_t i = 0; i < config->get_methods_size(); i++) {
            if (config->get_method(i) == method) {
                return true;
            }
        }
        return false;
    } else {
        if (mode == 1)
            return false;
        else
            return true;
    }
}

bool    sent_from_site(Server *cluster, Request *req) {
    std::string ref = req->get_header("Referer");

    if (!ref.empty()) {
        size_t pos = ref.find("http://localhost:");
        
        if (pos == std::string::npos || pos != 0) {
            return false;
        } else {
            std::string sub = ref.substr(pos, 17);
            std::map<std::string, std::vector<std::string> > tmp = cluster->get_sockets();

            for (std::map<std::string, std::vector<std::string> >::iterator i = tmp.begin(); i != tmp.end(); i++) {
                std::vector<std::string> tmp1 = i->second;
                for (std::vector<std::string>::iterator j = tmp1.begin(); j != tmp1.end(); j++) {
                    if (ref.find(sub + *j) != std::string::npos)
                        return true;
                }
            }
            return false;
        }
    } else
        return false;
}

size_t hex_string_to_size_t(const std::string& hex_str) {
    std::stringstream ss;
    ss << std::hex << hex_str;
    size_t result;
    ss >> result;
    return result;
}

std::string size_t_to_string(size_t n) {
    std::stringstream ss;
    ss << n;
    std::string result = ss.str();
    return result;
}

size_t      string_to_size_t(std::string string) {
    std::stringstream ss(string);
    size_t result;
    ss >> result;
    return result;
}

int      string_to_int(std::string string) {
    if (string == "") {
        return 0;
    }
    std::stringstream ss(string);
    int  result;
    ss >> result;
    return result;
}

size_t      string_to_st(std::string string) {
    if (string == "") {
        return 0;
    }
    std::stringstream ss(string);
    size_t  result;
    ss >> result;
    return result;
}

std::string         int_to_string(int n) {
    std::stringstream ss;
    ss << n;
    std::string result = ss.str();
    return result;
    
}

bool        is_url(std::string str) {
    if (str.compare(0, 7, "http://") == 0 || str.compare(0, 8, "https://") == 0)
        return true;
    return false;
}

bool        find_cgi_ext(Location *config, std::string uri) {
    size_t      pos = uri.rfind("/");
    std::string req = uri;
    std::string ext;
    
    if (pos != std::string::npos)
        req = uri.substr(pos + 1, uri.length() - (pos + 1));
    size_t dpos = req.rfind(".");
    
    if (dpos != std::string::npos) {
        ext = req.substr(dpos, req.length() - dpos);
        for (size_t i = 0; i < config->get_ext_size(); i++)
            if (config->get_ext(i).compare(ext) == 0)
                return true;
        return false;
    } else
        return false;
}

bool        is_directory(Location *config, std::string uri) {
    std::string path = config->get_root() + uri;
    struct stat info;

    if (stat(path.c_str(), &info) != 0)
        return false;
    else if (info.st_mode & S_IFDIR)
        return true;
    else
        return false;
}

bool        req_exists(Location *config, std::string uri) {
    size_t      pos;
    struct stat info;
    std::string req = uri;

    if (!is_directory(config, uri) && !uri.empty() && uri[uri.size() - 1] == '/')
        req = uri.substr(0, uri.length() - 1);
    pos = uri.rfind("/");

    if (content_type(config, config->get_route()) == "script/cgi" && pos != std::string::npos) {
        req = uri.substr(pos, uri.length() - pos);
        return (stat((config->get_cgi_path() + req).c_str(), &info) == 0);
    } else {
        return (stat((config->get_root() + req).c_str(), &info) == 0);
    }
}

bool        is_empty(Location *config, std::string uri) {
    std::string path = config->get_root() + uri;
    std::ifstream file(path.c_str());

    return file.peek() == std::ifstream::traits_type::eof();
}

bool        file_exists(std::string filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

std::string getMimeType(const std::string& extension) {
    static std::map<std::string, std::string> mimeTypes;
    if (mimeTypes.empty()) {
        mimeTypes["html"] = "text/html";
        mimeTypes["htm"] = "text/html";
        mimeTypes["txt"] = "text/plain";
        mimeTypes["css"] = "text/css";
        mimeTypes["js"] = "application/javascript";
        mimeTypes["json"] = "application/json";
        mimeTypes["jpg"] = "image/jpeg";
        mimeTypes["jpeg"] = "image/jpeg";
        mimeTypes["png"] = "image/png";
        mimeTypes["gif"] = "image/gif";
        mimeTypes["bmp"] = "image/bmp";
        mimeTypes["ico"] = "image/x-icon";
        mimeTypes["svg"] = "image/svg+xml";
        mimeTypes["pdf"] = "application/pdf";
    }
    
    std::map<std::string, std::string>::const_iterator it = mimeTypes.find(extension);
    if (it != mimeTypes.end()) {
        return it->second;
    }
    return "application/octet-stream";
}

bool        bad_type(Request *req, std::string uri) {
    size_t      pos = uri.rfind("/");
    std::string file;
    std::string ext;
    std::string type;
    
    if (pos != std::string::npos) {
        file = uri.substr(pos + 1, uri.length());
        size_t dpos = file.rfind(".");
        if (dpos != std::string::npos) {
            ext = file.substr(dpos, file.length());
            type = getMimeType(ext);
            if (req->get_header("Accept").find(type))
                return false;
            else
                return true;
        } else
            return true;
    } else
        return true;
}

void        get_config(Server *cluster, Location *config, std::string req_path) {
    std::string path = req_path;
    std::string tmp;
    std::string loca;
    
    for (size_t i = 0; i < cluster->get_locations_size(); i++) {
        if (cluster->get_location(i)->get_route().find("cgi-bin")) {
            config->set_cgi_path(cluster->get_location(i)->get_cgi_path());
            config->set_cgi_ext(cluster->get_location(i)->get_cgi_ext());
        }
    }
    config->set_route(path);
    config->req_serv_config(cluster);

    while (true) {
        size_t  pos = path.find("/");

        if (pos != std::string::npos) {
            tmp = path.substr(0, pos + 1);
            path = path.substr(tmp.length(), (path.length() - tmp.length()));
        } else {
            tmp = path;
            path.clear();
        }

        for (size_t i = 0; i < cluster->get_locations_size(); i++) {
            loca = cluster->get_location(i)->get_route();

            if (loca[loca.length() - 1] != '/')
                loca += "/";

            if ((loca == "/" && tmp == "/") || (loca.find(tmp) != std::string::npos && tmp != "/"))
                config->req_loca_config(cluster->get_location(i), cluster);
        }

        if (path.empty())
            break;
    }
    config->clear_inheritance(cluster, req_path);
}

// void        get_config(Server *cluster, Location *config, std::string req_path) {
//     std::string path = req_path;
//     std::string tmp;
    
//     for (size_t i = 0; i < cluster->get_locations_size(); i++) {
//         if (cluster->get_location(i)->get_route().find("cgi-bin")) {
//             config->set_cgi_path(cluster->get_location(i)->get_cgi_path());
//             config->set_cgi_ext(cluster->get_location(i)->get_cgi_ext());
//         }
//     }
//     config->set_route(path);
//     config->req_serv_config(cluster);

//     for (size_t i = 0; i < cluster->get_locations_size(); i++) {
//         if (cluster->get_location(i)->get_route().compare("/") == 0) {
//             config->req_loca_config(cluster->get_location(i), cluster);
//             break;
//         }
//     }

//     while (true) {
//         size_t  pos = path.find("/", 1);
        
//         if (path.empty() || (path.compare("/") == 0 && !tmp.empty())) {
//             break;
//         }
//         tmp = path.substr(0, pos);
//         path = path.substr(tmp.length(), (path.length() - tmp.length()));

//         for (size_t i = 0; i < cluster->get_locations_size(); i++) {
//             if (tmp.compare("/") != 0 && cluster->get_location(i)->get_route()[cluster->get_location(i)->get_route().length() - 1] == '/' && tmp[tmp.length() - 1] != '/')
//                 tmp += "/";
            
//             if ((tmp.compare("/") == 0 && cluster->get_location(i)->get_route().compare("/") == 0) || (cluster->get_location(i)->get_route().compare("/") != 0 && tmp.find(cluster->get_location(i)->get_route()) != std::string::npos)) {
//                 config->req_loca_config(cluster->get_location(i), cluster);
//                 break;
//             }
//         }
//     }
// }

std::string get_file_content(const std::string& filename) {
    std::ifstream in(filename.c_str(), std::ios::in | std::ios::binary);
    if (in) {
        std::ostringstream content;
        content << in.rdbuf();
        in.close();
        return content.str();
    }
    return "";
}

std::string content_type(Location *config, const std::string& filename) {
    std::vector<std::string>    tmp = config->get_cgi_ext();

    for (size_t i = 0; i < tmp.size(); i++) {
        if (filename.find(tmp[i]) != std::string::npos)
            return "script/cgi";
    }
    
    if (filename.find(".html") != std::string::npos)
        return "text/html";
    else if (filename.find(".txt") != std::string::npos)
        return "text/plain";
    else if (filename.find(".css") != std::string::npos)
        return "text/css";
    else if (filename.find(".js") != std::string::npos)
        return "text/javascript";
    else if (filename.find(".json") != std::string::npos)
        return "application/json";
    else if (filename.find(".xml") != std::string::npos)
        return "application/xml";
    else if (filename.find(".pdf") != std::string::npos)
        return "application/pdf";
    else if (filename.find(".png") != std::string::npos)
        return "image/png";
    else if (filename.find(".jpeg") != std::string::npos || filename.find(".jpg") != std::string::npos)
        return "image/jpeg";
    else if (filename.find(".gif") != std::string::npos)
        return "image/gif";
    else if (filename.find(".svg") != std::string::npos)
        return "image/svg+xml";
    else if (filename.find(".ico") != std::string::npos)
        return "image/x-icon";
    else if (filename.find(".mpg") != std::string::npos || filename.find(".mepg") != std::string::npos)
        return "video/mpeg";
    else if (filename.find(".ogg") != std::string::npos)
        return "application/ogg";
    else if (filename.find(".webm") != std::string::npos)
        return "video/webm";
    else if (filename.find(".mp4") != std::string::npos)
        return "video/mp4";
    else if (filename.find(".cgi") || filename.find(".py") || filename.find(".php") != std::string::npos)
        return "text/html";
    else
        return "application/octet-stream";
}

std::string exec_cgi(Location *config, const std::string& script_name, int timeout_seconds) {
    std::string command = config->get_cgi_path();

    if (script_name[0] != '/')
        command += "/";
    command += script_name;

    FILE* pipe = popen(command.c_str(), "r");
    
    if (!pipe)
        return "Error executing CGI script";
    int fd = fileno(pipe);
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);

    char buffer[1024];
    std::string result;
    time_t start_time = time(NULL);
    bool timed_out = false;
    bool done = false;

    while (!done) {
        if (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            result += buffer;
        } else {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                time_t current_time = time(NULL);
                double elapsed_seconds = difftime(current_time, start_time);

                if (elapsed_seconds >= timeout_seconds) {
                    timed_out = true;
                    break;
                }
                usleep(10000);
            } else
                break;
        }

        if (feof(pipe)) {
            done = true;
        }
    }
    pclose(pipe);

    if (timed_out)
        return "Timeout while reading CGI output";
    return result;
}

std::string read_line(int client_socket, Response *res, Location *config) {
    std::string line;
    ssize_t     bytes;
    char        c;

    while (true) {
        bytes = recv(client_socket, &c, 1, 0);
        if (bytes < 0) {
            res->req_return(config, "500");
            throw Core::InterServError();
        } else if (bytes == 0 || c == '\n')
            break;
        else if (c != '\r')
            line += c;
    }
    return line;
}

size_t      max_body_size(const Request *req, Server *serv) {
    size_t ret = 0;
    std::string path = req->get_path();

    while (true) {
        size_t  pos = path.find("/", 1);
        if (pos == std::string::npos)
            break;
        std::string tmp = path.substr(0, pos);
        path = path.substr(tmp.length(), (path.length() - tmp.length()));

        for (size_t i = 0; i < serv->get_locations_size(); i++) {
            if (tmp.find(serv->get_location(i)->get_route()) != std::string::npos) {
                if (serv->get_location(i)->get_max_body_size() > 0)
                    ret = serv->get_location(i)->get_max_body_size();
            }
        }
    }
    return ret;
}

void        uploader(Location *config, Request *req, Response *res) {
    std::vector<Part*>  tmp = req->get_parts();
    std::string         path = config->get_root() + config->get_route();
    struct stat         info;

    for (std::vector<Part*>::iterator it = tmp.begin(); it != tmp.end(); it++) {
        if ((*it)->get_isfile()) {
            if (stat(path.c_str(), &info) != 0) {            
                res->req_return(config, "404");
                return;
            } else if (!S_ISDIR(info.st_mode)) {
                res->req_return(config, "400");
                return;
            } else if (access(path.c_str(), W_OK) != 0) {
                res->req_return(config, "401");
                return;
            } else {
                std::string curr_path = path;

                if (path[path.size() - 1] != '/')
                    curr_path += "/";
                curr_path += (*it)->get_dispo("filename");
                std::ofstream outfile(curr_path.c_str());
                
                if (!outfile)
                    res->req_return(config, "500");
                outfile << (*it)->get_body();

                if (!outfile)
                    res->req_return(config, "500");
                outfile.close();
            }
        }
    }
    res->req_return(config, "201");
}

bool trailing_semicolon_checker(const std::string& line) {
    if (line.find("location") != std::string::npos || line.find("server") != std::string::npos || line.find("}") != std::string::npos) {
        return true;
    }

    if (line.empty()) {
        return true;
    }
    if (line[line.size() - 1] != ';') {
        std::cerr << "Error: Semicolon missing or line is not empty" << '\n';
        return false;
    } else {
        return true;
    }
}

bool server_block_indentation_checker(const std::string& line) {
    if (!line.empty()) {
        if (line.size() < 4 || line.substr(0, 4) != "    ") {
            std::cerr << "Error: (Server) Bad indentation." << '\n';
            return false;
        }
        if (line.size() > 4 && line[4] == ' ') {
            std::cerr << "Error: (Server) Bad indentation." << '\n';
            return false;
        }
        return true;
    }
    return true;
}

bool location_block_indentation_checker(const std::string& line) {
    if (!line.empty()) {
        if (line.size() < 8 || line.substr(0, 8) != "        ") {
            std::cerr << "Error: (Location) Bad indentation." << '\n';
            return false;
        }
        if (line.size() > 8 && line[8] == ' ') {
            std::cerr << "Error: (Location) Bad indentation." << '\n';
            return false;
        }
        return true;
    }
    return true;
}

bool config_file_checker(std::string config_file) {
    std::ifstream ifs(config_file.c_str());
    std::string line;
    int cpt = 0;

    while (std::getline(ifs, line)) {
        if (line.empty()) {
            continue;
        }

        if (line != "server {") {
            std::cerr << "Error: Config file must start with a server block." << '\n';
            return false;
        } else {
            while (std::getline(ifs, line)) {
                if (line == "}" || cpt > 100) {
                    break;
                }
                if (!server_block_indentation_checker(line) || !trailing_semicolon_checker(line)) {
                    return false;
                }

                if (line.find("location") != std::string::npos) {
                    while (std::getline(ifs, line)) {
                        if (line == "    }") {
                            break;
                        } else if (line.empty()) { // Empty lines are not allowed inside location blocks
                            return false;
                        }
                        if (!location_block_indentation_checker(line) || !trailing_semicolon_checker(line)) {
                            return false;
                        }
                    }
                }
                cpt++;
            }
        }
    }
    return true;
}

Session* create_session(std::string session_id) {
    if (session_id == "") {
        return NULL;
    }
    Session* result = new Session;
    result->session_id = session_id;
    result->expiry = time(NULL) + 120;
    return result;
}
