GRAY 		= \033[90m
GREEN   	= \033[32m
RED			= \033[31m
MAGENTA 	= \033[35m
YELLOW  	= \033[33m
BLUE		= \033[34m
BOLD		= \033[1m
RESET   	= \033[0m

NAME		= webserv

CXX			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++98 -fsanitize=address
SRCDIR		= srcs
SRC 		= $(wildcard $(SRCDIR)/*.cpp) $(wildcard $(SRCDIR)/*/*.cpp)
OBJDIR		= .obj
OBJ 		= $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRC))
HEADER		= -I ./include

all: $(NAME)

$(NAME): $(OBJ)
	@printf "$(GRAY) - $(NAME)$(RESET)\t"
	@$(CXX) $(CXXFLAGS) $(HEADER) $(OBJ) -o $(NAME)
	@printf "$(GREEN)[OK]$(RESET)\n"

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@printf "$(GRAY) - objects$(RESET)\t"
	@rm -rf $(OBJDIR)
	@printf "$(YELLOW)[REMOVED]$(RESET)\n"

fclean: clean
	@printf "$(GRAY) - $(NAME)$(RESET)\t"
	@rm -f $(NAME)
	@printf "$(YELLOW)[REMOVED]$(RESET)\n"

re: fclean all

debug: fclean $(OBJ)
	@printf "$(GRAY) - $(NAME)$(RESET)\t"
	@$(CXX) -g3 $(CXXFLAGS) $(HEADER) $(OBJ) -o $(NAME) -fsanitize=address
	@printf "$(BOLD)$(MAGENTA)[DEBUG]$(RESET)\n"

.PHONY: all, clean, fclean, re, debug