/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jlecorne <jlecorne@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/23 15:41:53 by jlecorne          #+#    #+#             */
/*   Updated: 2024/08/16 18:23:02 by jlecorne         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/http/Request.hpp"
#include "../../include/server/Core.hpp"

Request::Request() : _port(0), _chunked(false) {}

Request::~Request() {
    for (size_t i = 0; i < this->_parts.size(); i++)
        delete _parts[i];
    this->_parts.clear();
}

/***** GETTERS *****/
std::string Request::get_full_header(void) const {
    return this->_header;
}

std::string Request::get_header(std::string header_name) {
    if (this->_headers.count(header_name) > 0) {
        return this->_headers[header_name];
    } else {
        return "";
    }
}

int         Request::get_port(void) const {
    return this->_port;
}

std::string Request::get_path(void) const {
    return this->_path;
}

std::string Request::get_bound(void) {
    return this->_boundary;
}

std::string Request::get_ip_addr(void) const {
    return this->_ip_addr;
}

std::string Request::get_query_string(void) const {
    if (this->_method == "GET") {
        size_t pos = this->_header.find("?");
        if (pos != std::string::npos) {
            size_t end = this->_header.find(" ", pos);
            return this->_header.substr(pos + 1, end - pos - 1);
        } else {
            return "";
        }
    } else if (this->_method == "POST") {
        return this->_body;
    } else {
        return "";
    }
}

std::string Request::get_method(void) const {
    return this->_method;
}

std::string Request::get_http_protocol(void) const {
    return this->_protocol;
}

std::string Request::get_body(void) const {
    return this->_body;
}

std::vector<Part*>  Request::get_parts(void) {
    return this->_parts;
}

std::string         Request::get_session_id(void) {
    if (this->_cookies.count("session_id")) {
        return this->_cookies["session_id"];
    }
    return "";
}

/***** SETTERS *****/
void        Request::add_chunk(std::string chunk) {
    this->_chunks.push_back(chunk);
    this->_query_string += chunk;

}

void        Request::add_part(Part* part) {
    _parts.push_back(part);
}

void        Request::set_body(void) {
    for (size_t i = 0; i < this->_chunks.size(); i++) {
        this->_body += this->_chunks[i];
    }
}

/***** PARSING *****/
void        Request::body_type() {
    size_t contlen_pos = this->_header.find("Content-Length: ");
    if (contlen_pos != std::string::npos) {
        size_t endl = this->_header.find("\r\n", contlen_pos);
        if (endl != std::string::npos) {
            std::string contlen_str = this->_header.substr(contlen_pos + 16, endl - contlen_pos - 16);
            size_t contlen = string_to_int(contlen_str);
            if (contlen > 0)
                this->_headers["Content-Length"] = contlen;
        }
    }

    size_t transenco_pos = this->_header.find("Transfer-Encoding: ");
    if (transenco_pos != std::string::npos) {
        size_t endl = this->_header.find("\r\n", transenco_pos);
        if (endl != std::string::npos) {
            std::string transenco = this->_header.substr(transenco_pos + 19, endl - transenco_pos - 19);
            if (transenco == "chunked")
                this->_chunked = true;
        }
    }

    size_t bound_pos = this->_header.find("boundary=");
    if (bound_pos != std::string::npos) {
        size_t bound_end = this->_header.find("\r\n", bound_pos);
        if (bound_end != std::string::npos) {
            std::string bound_val = this->_header.substr(bound_pos + 9, bound_end - bound_pos - 9);
            if (bound_val.at(0) == '"' && bound_val.at(bound_val.length() - 1) == '"')
                bound_val = bound_val.substr(1, bound_val.size() - 2);
            this->_boundary = bound_val;
        }
    }
}

void        Request::parse_request_line(std::string header) {
    std::string line;
    std::istringstream iss(header);
    
    std::getline(iss, line, '\n');
    std::istringstream first_line_stream(line);
    first_line_stream >> this->_method >> this->_path >> this->_protocol;
    
}

void        Request::parse_headers(std::string header) {
    std::istringstream  stream(header);
    std::string         line;
    bool                has_body = false;

    while (std::getline(stream, line) && !line.empty()) {
        size_t delimiter = line.find(": ");
        
        if (delimiter != std::string::npos) {
            std::string key = line.substr(0, delimiter);
            std::string value;

            if (key.compare("Content-Length") == 0 || key.compare("Transfert-Encoding") == 0)
                has_body = true;
            if (key.compare("Transfert-Encoding") == 0 && line.find("chunked"))
                this->_chunked = true;
            if (key.compare("Content-Type") == 0) {
                size_t      pos = line.find(";");

                if (pos == std::string::npos) {
                    value = line.substr(delimiter + 2);
                } else {
                    value = line.substr(delimiter + 2, pos - (delimiter + 2));
                    
                    if (value.compare("multipart/form-data") == 0) {
                        size_t  end = line.find("\r");
                        
                        pos = line.find("boundary=");
                        this->_boundary = line.substr(pos + 9, end - (pos + 9));
                    }
                }
                has_body = true;
            } else
                value = line.substr(delimiter + 2);
            this->_headers[key] = value;
        }
    }
    if (!has_body || (has_body && this->_headers["Content-Length"].empty()))
        this->_headers["Content-Length"] = "0";
}

void        Request::http_header_parser(std::string header) {
    this->parse_request_line(header);
    this->parse_headers(header);
    if (this->_headers.count("Cookie")) {
        this->parse_cookie_header(this->get_header("Cookie"));
    }
    this->_header = header;

    std::string host_header = this->_headers["Host"];
    size_t port_pos = host_header.find(":");
    
    if (port_pos != std::string::npos) {
        std::string port = host_header.substr(port_pos + 1);
        this->_port = string_to_int(port);
    } else {
        this->_port = 80;
    }
}

void        Request::parse_key_value_pairs() {
    std::size_t start = 0;
    std::size_t ampersand_sign_pos = _body.find("&");

    if (_body.length() > 0 && ampersand_sign_pos == std::string::npos) {
        this->_parameters.insert(parameters_parser(_body));
    } else {
        while (ampersand_sign_pos != std::string::npos) {
            this->_parameters.insert(parameters_parser(_body.substr(start, ampersand_sign_pos - start)));
            start = ampersand_sign_pos + 1;
            ampersand_sign_pos = _body.find("&", start);
            if (ampersand_sign_pos == std::string::npos && start < _body.length()) {
                this->_parameters.insert(parameters_parser(_body.substr(start)));
                return;
            }
        }
    }
}

std::pair<std::string, std::string> Request::parameters_parser(std::string string) {
    std::pair<std::string, std::string> result;
    std::size_t equal_sign_pos = string.find("=");

    result.first = string.substr(0, equal_sign_pos);
    result.second = string.substr(equal_sign_pos + 1);
    return result;
}

std::map<std::string, std::string>  Request::parse_cookie_header(std::string cookie_header) {
    std::map<std::string, std::string> cookies;
    std::stringstream ss(cookie_header);
    std::string item;
    while (std::getline(ss, item, ';')) {
        size_t pos = item.find('=');
        if (pos != std::string::npos) {
            std::string name = item.substr(0, pos);
            std::string value = item.substr(pos + 1);
            cookies[name] = value;
        }
    }
    return cookies;
}

/***** UTILS *****/
int         Request::has_body(void) {
    if (this->chunked())
        return 1;
    else if (!this->_boundary.empty())
        return 2;
    else if (string_to_int(this->get_header("Content-Length")) > 0)
        return 3;
    return 0;
}

bool        Request::upload_req(void) {
    if (this->_method == "POST") {
        if (has_body() == 2) {
            for (size_t i = 0; i < this->_parts.size(); i++) {
                if (this->_parts[i]->get_isfile() == true)
                    return true;
            }
            return false;
        } else
            return false;
    } else
        return false;
}

bool        Request::chunked() const {
    return this->_chunked;
}
