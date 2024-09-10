/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Part.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jlecorne <jlecorne@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/30 17:57:21 by jlecorne          #+#    #+#             */
/*   Updated: 2024/08/21 15:16:51 by jlecorne         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef     PART_HPP
# define    PART_HPP

#include <iostream>
#include <map>

class Part
{
private:
    bool                                _file;
    std::string                         _type;
    std::string                         _body;
    std::map<std::string, std::string>  _disposition;
public:
    Part();
    ~Part();

    /***** GETTERS *****/
    bool                                get_isfile(void);
    std::string                         get_dispo(std::string key);
    std::string                         get_body(void);

    /***** SETTERS *****/
    void                                set_isfile();
    void                                set_type(std::string type);
    void                                set_body(std::string body);
    void                                set_dispo(std::string key, std::string val);

    /***** UTILS *****/
    bool                                is_file();
};

#endif