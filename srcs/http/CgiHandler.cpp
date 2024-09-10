#include "../../include/http/CgiHandler.hpp"
#include "../../include/http/Request.hpp"
#include "../../include/server/Core.hpp"

CgiHandler::CgiHandler() {}

CgiHandler::CgiHandler(CgiHandler& src)
    :  _path(src._path), _env(src._env) {}

CgiHandler::~CgiHandler() {}

CgiHandler& CgiHandler::operator=(const CgiHandler& rhs) {
    if (this != &rhs) {
        _env = rhs._env;
        _path = rhs._path;
    }
    return *this;
}

/***** PROCESS *****/
std::string CgiHandler::exec_cgi_script(std::string script_path, Server* cluster, Request* req) {
    (void)cluster;
    (void)req;
    std::string result;

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        std::cerr << "pipe(): error detected" << '\n';
        return "";
    }

    pid_t pid = fork();
    if (pid == -1) {
        std::cerr << "fork(): error detected" << '\n';
        return "";
    }
    
    if (pid == 0) {
        close(pipefd[0]);

        std::map<std::string, std::string>::iterator it;
        std::string result;
        std::vector<char*> args;
        std::vector<char*> env;

        args.push_back(strdup(script_path.c_str()));
        args.push_back(NULL);

        for (it = this->_env.begin(); it != _env.end(); it++) {
            result = it->first + '=' + it->second;
            env.push_back(strdup(result.c_str()));
        }
        env.push_back(NULL);

        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        execve(script_path.c_str(), &args[0], &env[0]);
        _exit(1);
    } else {
        close(pipefd[1]);

        int status;
        waitpid(pid, &status, 0);

        char buffer[1024];
        std::string tmp;
        ssize_t bytes_read;
        while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
            result.append(buffer, bytes_read);
        }
        close(pipefd[0]);
    }
    return result;
}

void CgiHandler::init_env(Server* cluster, Request* req, bool client_has_session) {
    (void) cluster;
    if (client_has_session) {
        setenv("AUTHENTICATED", "TRUE", 1);
        this->_env["AUTHENTICATED"] = "TRUE";
    } else {
        setenv("AUTHENTICATED", "FALSE", 1);
        this->_env["AUTHENTICATED"] = "FALSE";    
    }
    setenv("REQUEST_METHOD", req->get_method().c_str(), 1);
    this->_env["REQUEST_METHOD"] = req->get_method();
    setenv("QUERY_STRING", req->get_query_string().c_str(), 1);
    this->_env["QUERY_STRING"] = req->get_query_string();
    setenv("CONTENT_TYPE", req->get_header("Content-Type").c_str(), 1);
    this->_env["CONTENT_TYPE"] = req->get_header("Content-Type");
    setenv("CONTENT_LENGTH", req->get_header("Content-Length").c_str(), 1);
    this->_env["CONTENT_LENGTH"] = req->get_header("Content-Length");
    setenv("SERVER_PROTOCOL", req->get_http_protocol().c_str(), 1);
    this->_env["SERVER_PROTOCOL"] = req->get_http_protocol();
    setenv("HTTP_COOKIE", req->get_header("Cookie").c_str(), 1);
    this->_env["HTTP_COOKIE"] = req->get_header("Cookie");
    setenv("HTTP_USER_AGENT", req->get_header("User-Agent").c_str(), 1);
    this->_env["HTTP_USER_AGENT"] = req->get_header("User-Agent");
    setenv("HTTP_REFERER", req->get_header("Referer").c_str(), 1);
    this->_env["HTTP_REFERER"] = req->get_header("Referer");
}

void        CgiHandler::exec_cgi(std::string script_path, Location *config, Response *res) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        res->req_return(config, "500");
        throw CgiLogicError();
    }

    pid_t pid = fork();
    if (pid == -1) {
        res->req_return(config, "500");
        throw CgiLogicError();
    }
    
    if (pid == 0) {
        std::map<std::string, std::string>::iterator it;
        std::string result;
        std::vector<char*> args;
        std::vector<char*> env;

        close(pipefd[0]);
        args.push_back(strdup(script_path.c_str()));
        args.push_back(NULL);

        for (it = this->_env.begin(); it != _env.end(); it++) {
            result = it->first + '=' + it->second;
            env.push_back(strdup(result.c_str()));
        }
        env.push_back(NULL);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        execve(script_path.c_str(), &args[0], &env[0]);
        exit(1);
    } else {
        int         status;
        char        buffer[1024];
        std::string tmp;
        ssize_t     bytes_read;

        close(pipefd[1]);
        waitpid(pid, &status, 0);
        while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer))) > 0)
            this->_script_res.append(buffer, bytes_read);
        close(pipefd[0]);
    }
}

bool        CgiHandler::parse_cgi(Location *config, Request *req, Response *res) {
    std::istringstream  is(this->_script_res);
    std::string         line;
    std::string         filename;
    std::string         file_content;
    bool                login = false;

    if (this->_script_res.find("You have successfully logged in") != std::string::npos) {
        login = true;
    }

    while (std::getline(is, line)) {
        size_t      pos = line.find("=");
        std::string key;
        std::string val;

        if (line.empty() || pos == std::string::npos)
            break;
        key = line.substr(0, pos);
        val = line.substr(pos + 1, line.length() + pos + 1);
        this->_res_headers[key] = val;
    }

    if (!config->get_ret_path().empty() && this->_res_headers.count("status") > 0 
        && config->get_ret_path() == this->_res_headers["status"]) {
        res->req_return(config, config->get_ret_path());
        return false;
    } else {
        if (this->_res_headers.count("file") > 0) {
            filename = this->_res_headers["file"];

            if (!req_exists(config, filename)) {
                res->req_return(config, "404");
            } else {
                if (is_empty(config, filename))
                    res->req_return(config, "204");
                else if (bad_type(req, filename))
                    res->req_return(config, "406");
                else {
                    file_content = get_file_content(config->get_root() + filename);
                    res->set_file(filename);
                    res->set_body(file_content);
                    res->set_status_code("200");
                    res->set_status_msg(res->get_status_msg("200"));
                    return false;
                }
            }
        } else {
            bool    has_head = false;
            is.clear();
            is.str(this->_script_res);

            while (std::getline(is, line)) {
                if (line.find("status=") != std::string::npos || line.find("file=") != std::string::npos) {
                    has_head = true;
                    continue;
                }
                
                if (line.empty()) {
                    if (has_head == true) {
                        has_head = false;
                        continue;
                    } else
                        break;
                }
                file_content += line + "\n";
            }

            if (file_content.empty()) {
                if (this->_res_headers.count("status") > 0 && req_exists(config, "/status/" + this->_res_headers["status"] + ".html"))
                    res->req_return(config, this->_res_headers["status"]);
                else
                    res->req_return(config, "500");
                return false;
            } else {
                if (this->_res_headers.count("type") > 0)
                    res->set_file(this->_res_headers["type"]);
                else
                    res->set_file(".html");

                if (this->_res_headers.count("status") > 0) {
                    res->set_file(this->_res_headers["status"]);
                    res->set_status_msg(this->_res_headers["status"]);
                } else {
                    res->set_status_code("200");
                    res->set_status_msg(res->get_status_msg("200"));
                }
                res->set_body(file_content);
            }
        }
    }
    return login;
}

/***** EXCEPTIONS *****/
const char *CgiHandler::CgiLogicError::what() const throw() {
    return ("\033[31mERROR\033[0m: Can't launch CGI script");
}
