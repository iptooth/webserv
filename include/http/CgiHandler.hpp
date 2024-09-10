/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jlecorne <jlecorne@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/11 17:23:40 by nlence-l          #+#    #+#             */
/*   Updated: 2024/08/16 11:59:04 by jlecorne         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

#include <string>
#include <map>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstring>
#include <stdlib.h>

#include "../../include/server/Server.hpp"
#include "../../include/server/Location.hpp"
#include "../../include/http/Request.hpp"
#include "../../include/http/Response.hpp"

class CgiHandler
{
private:
    std::string                         _path;
    std::string                         _script_res;
    std::map<std::string, std::string>  _env;
    std::map<std::string, std::string>  _res_headers;

public:
    CgiHandler();
    CgiHandler(CgiHandler& src);
    ~CgiHandler();
    CgiHandler& operator=(const CgiHandler& rhs);

    /***** PROCESS *****/
    void                                init_env(Server* cluster, Request* req, bool client_has_session);
    std::string                         exec_cgi_script(std::string script_path, Server* cluster, Request* req);
    void                                exec_cgi(std::string script_path, Location *config, Response *res);
    bool                                parse_cgi(Location *config, Request *req, Response *res);

    /***** EXCEPTIONS *****/
    class CgiLogicError : public std::exception {
        public: virtual const char *what() const throw();
    };
};

#endif