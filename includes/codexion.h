/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   codexion.h                                        :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: vsack <vsack@student.42vienna.com>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/06 18:33:52 by vsack            #+#    #+#              */
/*   Updated: 2026/08/10 17:03:48 by vsack           ###   ########.fr        */
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

typedef struct s_request
{
	uint64_t		id;
	uint64_t		arrival_time;
	uint64_t		deadline;
}					t_request;

typedef enum e_dongle_state
{
	FREE,
	HELD,
	COOLING
}					t_dongle_state;

typedef enum e_coder_state
{
	COMPILING,
	REFACTORING,
	DEBUGGING,
	BURNOUT
}					t_coder_state;

typedef struct s_dongle
{
	pthread_mutex_t	lock;
	t_dongle_state	state;
	uint64_t		ready_at_ms;
	pthread_cond_t	condition;
	// TODO: track who is waiting
}					t_dongle;

typedef struct s_coder
{
	pthread_t		ticket;
	pthread_mutex_t	lock;
	uint64_t		last_compile_start;
	uint64_t		compiles_finished;
	uint64_t		id;
	uint64_t		left;
	uint64_t		right;
	t_coder_state	coder_state;
}					t_coder;

typedef enum e_scheduler
{
	FIFO,
	EDF
}					t_scheduler;

typedef struct s_args
{
	uint64_t		num_coders;
	uint64_t		time_to_burnout;
	uint64_t		time_to_compile;
	uint64_t		time_to_debug;
	uint64_t		time_to_refactor;
	uint64_t		compiles_required;
	uint64_t		dongle_cooldown;
	t_scheduler		scheduler;
}					t_args;

typedef struct s_simulation_state
{
	t_dongle		*dongle_arr;
	t_coder			*coder_arr;
	t_args			args;
	pthread_mutex_t	out_lock;
	uint64_t		start_time;
	int				stop_flag;
	pthread_mutex_t	stop_lock;

}					t_simulation_state;

int					is_valid_number(char *str);
int					parse_args(char **av, t_args *args);
int					parse_scheduler(char *str, t_scheduler *dest);
int					set_number(char *str, uint64_t *dest, int idx);

#endif
