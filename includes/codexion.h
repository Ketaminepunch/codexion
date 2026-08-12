/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   codexion.h                                        :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: vsack <vsack@student.42vienna.com>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/06 18:33:52 by vsack            #+#    #+#              */
/*   Updated: 2026/08/12 19:09:42 by vsack           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <inttypes.h>
# include <pthread.h>
# include <stdint.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
# include <unistd.h>

typedef struct s_request
{
	uint64_t			id;
	uint64_t			arrival_time;
	uint64_t			deadline;
}						t_request;

typedef struct s_heap
{
	t_request			*items;
	uint64_t			count;
}						t_heap;

typedef enum e_dongle_state
{
	FREE,
	HELD,
	COOLING
}						t_dongle_state;

typedef enum e_coder_state
{
	COMPILING,
	REFACTORING,
	DEBUGGING,
	BURNOUT
}						t_coder_state;

typedef struct s_dongle
{
	pthread_mutex_t		lock;
	t_dongle_state		state;
	uint64_t			ready_at_ms;
	pthread_cond_t		condition;
	t_heap				heap;
}						t_dongle;

typedef struct s_coder
{
	pthread_t			ticket;
	pthread_mutex_t		lock;
	uint64_t			last_compile_start;
	uint64_t			compiles_finished;
	uint64_t			id;
	uint64_t			left;
	uint64_t			right;
	t_coder_state		coder_state;
}						t_coder;

typedef enum e_scheduler
{
	FIFO,
	EDF
}						t_scheduler;

typedef struct s_args
{
	uint64_t			num_coders;
	uint64_t			time_to_burnout;
	uint64_t			time_to_compile;
	uint64_t			time_to_debug;
	uint64_t			time_to_refactor;
	uint64_t			compiles_required;
	uint64_t			dongle_cooldown;
	t_scheduler			scheduler;
}						t_args;

typedef struct s_simulation_state
{
	t_dongle			*dongle_arr;
	t_coder				*coder_arr;
	t_args				args;
	pthread_mutex_t		out_lock;
	uint64_t			start_time;
	int					stop_flag;
	pthread_mutex_t		stop_lock;
	pthread_t			monitor;
}						t_simulation_state;

typedef struct s_thread_arg
{
	t_coder				*coder;
	t_simulation_state	*sim;
}						t_thread_arg;

void					dongle_release(t_dongle *dongle, t_args args);
uint64_t				get_time_ms(void);
int						is_valid_number(char *str);
int						parse_args(char **av, t_args *args);
int						parse_scheduler(char *str, t_scheduler *dest);
int						compare_requests(t_request *request1,
							t_request *request2, t_args *args);
void					heap_swap(t_heap *heap, uint64_t i, uint64_t j);
void					heap_push(t_heap *heap, t_request *request,
							t_args *args);
t_request				heap_pop(t_heap *heap, t_args *args);
uint64_t				most_urgent_child(t_heap *heap, uint64_t i,
							t_args *args);
void					log_action(t_simulation_state *sim, uint64_t id,
							char *msg);
void					*coder_thread(void *arg);
void					coder_release_dongle(t_coder *coder,
							t_simulation_state *sim);
int						coder_should_stop(t_simulation_state *sim,
							t_coder *coder);
int						set_number(char *str, uint64_t *dest, int idx);
int						sim_init(t_simulation_state *sim, t_args args);
int						spawn_coders(t_simulation_state *sim,
							t_thread_arg *thread_args);
int						join_coders(t_simulation_state *sim);

int						check_burnout(t_simulation_state *sim,
							uint64_t *burnt_id);

int						check_success(t_simulation_state *sim);

void					broadcast_stop(t_simulation_state *sim);

void					*monitor_thread(void *arg);

int						create_and_join(t_simulation_state *sim,
							t_thread_arg *thread_args);

int						dongle_acquire(t_dongle *dongle, t_coder *coder,
							t_args *args, t_simulation_state *sim);

int						coder_take_dongles(t_coder *coder,
							t_simulation_state *sim);

int						acquire_pair(t_coder *coder, t_simulation_state *sim,
							uint64_t first, uint64_t second);

void					coder_work_cycle(t_coder *coder,
							t_simulation_state *sim);

int						array_slot_init(t_simulation_state *sim, uint64_t i);

#endif
