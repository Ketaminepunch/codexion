/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   monitoring.c                                      :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: vsack <vsack@student.42vienna.com>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/10 23:32:44 by vsack            #+#    #+#              */
/*   Updated: 2026/08/11 00:01:30 by vsack           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/codexion.h"

int	check_burnout(t_simulation_state *sim, uint64_t *burnt_id)
{
	uint64_t	i;
	uint64_t	coding_time;

	i = 0;
	while (i < sim->args.num_coders)
	{
		pthread_mutex_lock(&sim->coder_arr[i].lock);
		coding_time = get_time_ms() - sim->coder_arr[i].last_compile_start;
		if (coding_time > sim->args.time_to_burnout)
		{
			*burnt_id = sim->coder_arr[i].id;
			pthread_mutex_unlock(&sim->coder_arr[i].lock);
			return (1);
		}
		pthread_mutex_unlock(&sim->coder_arr[i].lock);
		i++;
	}
	return (0);
}

int	check_success(t_simulation_state *sim)
{
	uint64_t	i;

	i = 0;
	while (i < sim->args.num_coders)
	{
		pthread_mutex_lock(&sim->coder_arr[i].lock);
		if (sim->coder_arr[i].compiles_finished < sim->args.compiles_required)
		{
			pthread_mutex_unlock(&sim->coder_arr[i].lock);
			return (0);
		}
		pthread_mutex_unlock(&sim->coder_arr[i].lock);
		i++;
	}
	return (1);
}

void	broadcast_stop(t_simulation_state *sim)
{
	uint64_t	i;

	i = 0;
	pthread_mutex_lock(&sim->stop_lock);
	sim->stop_flag = 1;
	pthread_mutex_unlock(&sim->stop_lock);
	while (i < sim->args.num_coders)
	{
		pthread_mutex_lock(&sim->dongle_arr[i].lock);
		pthread_cond_broadcast(&sim->dongle_arr[i].condition);
		pthread_mutex_unlock(&sim->dongle_arr[i].lock);
		i++;
	}
}

void	*monitor_thread(void *arg)
{
	t_simulation_state	*sim;
	uint64_t			id;
	uint64_t			timestamp;

	sim = (t_simulation_state *)arg;
	while (1)
	{
		if (check_burnout(sim, &id))
		{
			broadcast_stop(sim);
			pthread_mutex_lock(&sim->out_lock);
			timestamp = get_time_ms() - sim->start_time;
			printf("%" PRIu64 " %" PRIu64 " %s\n", timestamp, id, "burned out");
			pthread_mutex_unlock(&sim->out_lock);
			return (NULL);
		}
		else if (check_success(sim))
		{
			broadcast_stop(sim);
			return (NULL);
		}
		usleep(1000);
	}
}
