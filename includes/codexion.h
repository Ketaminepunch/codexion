/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   codexion.h                                        :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: vsack <vsack@student.42vienna.com>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/06 18:33:52 by vsack            #+#    #+#              */
/*   Updated: 2026/08/06 19:34:17 by vsack           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <stdint.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
# include <unistd.h>

typedef enum e_scheduler
{
	FIFO,
	EDF
}			t_scheduler;

typedef struct s_args
{
	uint64_t	num_coders;
	uint64_t	time_to_burnout;
	uint64_t	time_to_compile;
	uint64_t	time_to_debug;
	uint64_t	time_to_refactor;
	uint64_t	compiles_required;
	uint64_t	dongle_cooldown;
	t_scheduler	scheduler;
}			t_args;

int			is_valid_number(char *str);
int			set_number(char *str, uint64_t *dest, int idx);

int			parse_scheduler(char *str, t_scheduler *dest);

int			parse_args(char **av, t_args *args);

#endif
