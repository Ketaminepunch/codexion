# **************************************************************************** #
#                                                                              #
#                                                        :::      ::::::::     #
#    Makefile                                          :+:      :+:    :+:     #
#                                                    +:+ +:+         +:+       #
#    By: vsack <vsack@student.42vienna.com>        #+#  +:+       +#+          #
#                                                +#+#+#+#+#+   +#+             #
#    Created: 2026/08/13 16:16:42 by vsack            #+#    #+#               #
#    Updated: 2026/08/13 17:11:16 by vsack           ###   ########.fr         #
#                                                                              #
# **************************************************************************** #

NAME		= codexion

CC		= cc
CFLAGS		= -Wall -Wextra -Werror
LDFLAGS		=
LDLIBS		=
DEBUG		?= 0
RM		= rm -f

# Optional libs: no configured optional library directory detected.
LIBS		=

JOBS		?= $(shell nproc)
MAKEFLAGS	+= -j $(JOBS) -l $(JOBS)

ifeq ($(DEBUG),1)
CFLAGS		+= -g
endif

SRC_DIR		= src
OBJ_DIR		= obj
SRCS		= $(SRC_DIR)/coders.c \
			  $(SRC_DIR)/dongle.c \
			  $(SRC_DIR)/heap.c \
			  $(SRC_DIR)/main.c \
			  $(SRC_DIR)/monitoring.c \
			  $(SRC_DIR)/parsing.c \
			  $(SRC_DIR)/sim.c \
			  $(SRC_DIR)/utils.c

OBJS		= $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEPS		= $(OBJS:.o=.d)

all: $(NAME)

$(NAME): $(OBJS) $(LIBS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(LIBS) $(LDLIBS) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c Makefile
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	$(RM) -r $(OBJ_DIR)

fclean: clean
	$(RM) $(NAME)

re:
	$(MAKE) fclean
	$(MAKE) all

-include $(DEPS)

.PHONY: all clean fclean re
.DEFAULT_GOAL := all

