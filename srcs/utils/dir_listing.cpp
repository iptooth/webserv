/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dir_listing.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jlecorne <jlecorne@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/16 13:48:13 by jlecorne          #+#    #+#             */
/*   Updated: 2024/08/16 12:06:28 by jlecorne         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server/Core.hpp"

std::string generate_directory_listing(const std::string& directory_path, std::string curr_path) {
    DIR                 *dir;
    struct dirent       *entry;
    struct stat         file_stat;
    std::stringstream   ss;
    
    ss << "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n"
       << "<meta charset=\"UTF-8\">\n"
       << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
       << "<title>Directory Listing</title>\n"
       << "<link rel=\"stylesheet\" href=\"styles.css\">\n</head>\n<body>\n"
       << "<h1>Index of " << directory_path << "</h1>\n"
       << "<table>\n"
       << "<tr><th>Name</th><th>Last Modified</th><th>Size</th></tr>\n";

    if ((dir = opendir(directory_path.c_str())) != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            std::string list_path;
            std::string fullPath = directory_path + "/" + entry->d_name;
            stat(fullPath.c_str(), &file_stat);

            ss << "<tr><td";
            
            if (!curr_path.empty() && curr_path[curr_path.size() - 1] == '/') {
                list_path = curr_path + entry->d_name;
            }
            else
                list_path = curr_path + '/' + entry->d_name;
                
            if (entry->d_type == DT_DIR)
                ss << " class=\"directory\"><a href=\"" << list_path << "/\">" << entry->d_name << "/</a></td><td>";
            else
                ss << "><a href=\"" << list_path << "\">" << entry->d_name << "</a></td><td>";

            char timeBuffer[80];
            strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", localtime(&file_stat.st_mtime));
            ss << timeBuffer << "</td><td>";

            if (entry->d_type == DT_DIR) {
                ss << "-";
            } else {
                ss << file_stat.st_size;
            }
            
            ss << "</td></tr>\n";
        }
        closedir(dir);
    } else {
        std::cerr << "Error: Could not open directory " << directory_path << std::endl;
        return "";
    }
    ss << "</table>\n</body>\n</html>";

    return ss.str();
}

void        directory_listing_handler(Location* config, Request* req, Response* res) {
    std::string body = generate_directory_listing(config->get_root() + req->get_path(), req->get_path());

    if (!body.empty()) {
        res->set_file(".html");
        res->set_body(body);
        res->set_status_code("200");
        res->set_status_msg(" OK");
    } else {
        res->req_return(config, "403");
    }
}