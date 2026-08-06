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
	long		num_coders;
	long		time_to_burnout;
	long		time_to_compile;
	long		time_to_debug;
	long		time_to_refactor;
	long		compiles_required;
	long		dongle_cooldown;
	t_scheduler	scheduler;
}			t_args;

int			is_valid_number(char *str);
int			set_number(char *str, long *dest, int idx);

int			parse_scheduler(char *str, t_scheduler *dest);

int			parse_args(char **av, t_args *args);

#endif
