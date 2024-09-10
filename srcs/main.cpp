#include "../include/server/Core.hpp"

Core *core = NULL;

void signal_handler(int signum) {
    (void)signum;
    std::cout << std::endl << "Exiting Webserv..";
    if (core) {
        delete core;
        core = NULL;
    }
    exit(0);
}

int     main(int ac, char **av) {
    if (ac == 2) {
        if (config_file_checker(av[1]) == false) {
            return 1;
        }
        try {
            core = new Core;
            std::string config_file(av[1]);

            signal(SIGINT, signal_handler);

            core->core_parser(config_file);
            core->webserv();
        } catch (std::exception &e) {
            std::cerr << e.what() << '\n';
            return 2;
        }
    } else {
        std::cerr << "Error: Usage: ./webserv <config file>" << '\n';
        return 3;
    }
    return 0;
}
