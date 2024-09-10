#include "../../include/server/Core.hpp"

void        part_headers(std::string header, Part *part) {
    std::istringstream  stream(header);
    std::string         line;
    
    while (std::getline(stream, line)) {
        if (line.empty()) {
            break;
        } else if (line.find("Content-Disposition:") != std::string::npos) {
            size_t      ddot = line.find(";");
            std::string new_line;
            
            if (ddot == std::string::npos) {
                new_line = line.substr(21, line.length() - 21);
                part->set_dispo("dispo", new_line);             
            } else {
                size_t      spos = 21;
                size_t      epos = 0;
                std::string key;
                std::string val;

                while (spos < line.length()) {
                    if (ddot != std::string::npos)
                        new_line = line.substr(spos, ddot - spos);
                    else
                        new_line = line.substr(spos, line.length() - spos);
                    epos = new_line.find("=");
                    
                    if (epos == std::string::npos) {
                        part->set_dispo("dispo", new_line);
                    } else {
                        size_t qpos;
                        key = new_line.substr(0, epos);

                        if (key.find("file") != std::string::npos)
                            part->set_isfile();
                        qpos = new_line.find("\"", epos + 2);
                        val = new_line.substr(epos + 2, qpos - (epos + 2));
                        part->set_dispo(key, val);
                    }
                    if (ddot != std::string::npos) {
                        spos = ddot + 2;
                    } else {
                        while (spos < line.size() && line[spos] != ' ')
                            spos++;
                        while (spos < line.size() && line[spos] == ' ')
                            spos++;
                    }
                    ddot = line.find(";", spos);
                }
            }
        } else if (line.find("Content-Type:") != std::string::npos)
            part->set_type(line.substr(14, line.length() - 14));
    }
}

void        recv_multipart(int socket, Request *req, Response *res, Location *config) {
    std::string         del = "--" + req->get_bound();
    std::string         end_del = del + "--";
    std::string         data;
    char                buff[CHUNK_SIZE];
    ssize_t             bytes;
    size_t              total_recv = 0;
    
    while (true) {
        bytes = recv(socket, &buff, CHUNK_SIZE, 0);

        if (bytes <= 0) {
            if (bytes < 0) {
                res->req_return(config, "500");
                throw Core::InterServError();
            } else
                break;
        }
        data.append(buff, bytes);
        total_recv += bytes;

        if (config->get_max_body_size() > 0 && total_recv > config->get_max_body_size()) {
            res->req_return(config, "413");
            throw Core::NotMatchingConf();
        }

        if (data.find(end_del) != std::string::npos)
            break;
    }

    while (true) {
        size_t  pos = data.find(del);
        size_t  end_pos = data.find(end_del);

        if (pos != std::string::npos && pos != end_pos) {
            size_t  next_pos = data.find(del, pos + del.length());
            
            if (next_pos != std::string::npos) {
                Part        *part = new Part;
                std::string part_h;
                std::string part_b;
                std::string part_data = data.substr(pos + del.length(), (next_pos - (pos + del.length())) - 1);
                size_t      spos = part_data.find("\r\n\r\n");

                if (spos == std::string::npos) {
                    delete part;
                    res->req_return(config, "400");
                    throw Core::BodyRecvError();
                }
                part_h = part_data.substr(0, spos);
                part_b = part_data.substr(spos + 4, std::string::npos);
                part_headers(part_h, part);
                part->set_body(part_b);
                req->add_part(part);
                data.erase(0, next_pos);
            } else
                break;
        } else
            break;
    }
}

void        recv_chunked(int socket, Request *req, Response *res, Location *config) {
    size_t  total_recv = 0;

    while (true) {
        std::string chunk_head = read_line(socket, res, config);
        size_t      chunk_size = hex_string_to_size_t(chunk_head);
        char        buffer[CHUNK_SIZE];

        if (chunk_size == 0)
            break;
        
        while (chunk_size > 0) {
            size_t  to_read = std::min(chunk_size, (size_t)CHUNK_SIZE);
            ssize_t received = recv(socket, buffer, to_read, 0);
            
            if (received <= 0) {
                if (received == 0)
                    res->req_return(config, "400");
                else
                    res->req_return(config, "500");
                throw Core::BodyRecvError();
            }
            req->add_chunk(std::string(buffer, received));
            total_recv += received;
            chunk_size -= received;

            if (config->get_max_body_size() > 0 && total_recv > config->get_max_body_size()) {
                res->req_return(config, "413");
                throw Core::NotMatchingConf();
            }
        }
        read_line(socket, res, config);
    }
}

void        recv_contLen(int socket, Request *req, Response *res, Location *config) {
    size_t  ctl = string_to_size_t(req->get_header("Content-Length"));
    size_t  total_recv = 0;
    char    buffer[CHUNK_SIZE];

    if (config->get_max_body_size() > 0 && ctl > config->get_max_body_size()) {
        res->req_return(config, "413");
        throw Core::NotMatchingConf();
    }
    while (total_recv < ctl) {
        size_t to_read = std::min(ctl - total_recv, (size_t)CHUNK_SIZE);
        ssize_t received = recv(socket, buffer, to_read, 0);
        
        if (received <= 0) {
            if (received == 0)
                res->req_return(config, "400");
            else
                res->req_return(config, "500");
            throw Core::BodyRecvError();
        }
        req->add_chunk(std::string(buffer, received));
        total_recv += received;

        if (config->get_max_body_size() > 0 && total_recv > config->get_max_body_size()) {
            res->req_return(config, "413");
            throw Core::NotMatchingConf();
        }
        req->set_body();
    }
}
