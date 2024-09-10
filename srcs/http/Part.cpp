/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Part.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nlence-l <nlence-l@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/03 15:38:46 by jlecorne          #+#    #+#             */
/*   Updated: 2024/07/31 14:14:40 by nlence-l         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/http/Part.hpp"

Part::Part() : _file(false) {}

Part::~Part() {}

/***** GETTERS *****/
bool        Part::get_isfile(void) {return this->_file;}

std::string Part::get_dispo(std::string key) {
    if (this->_disposition.count(key) > 0) {
        return this->_disposition[key];
    } else {
        return "";
    }
}

std::string Part::get_body(void) {
    return this->_body;  
}

/***** SETTERS *****/
void        Part::set_isfile() {this->_file = true;}

void        Part::set_type(std::string type) {this->_type = type;}

void        Part::set_body(std::string body) {this->_body = body;}

void        Part::set_dispo(std::string key, std::string val) {this->_disposition[key] = val;}

/***** UTILS *****/
bool        Part::is_file() {return this->_file;}