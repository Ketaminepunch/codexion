/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   coders.c                                          :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: vsack <vsack@student.42vienna.com>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/10 20:57:08 by vsack            #+#    #+#              */
/*   Updated: 2026/08/12 19:10:30 by vsack           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/codexion.h"

void	log_action(t_simulation_state *sim, uint64_t id, char *msg)
{
	uint64_t	timestamp;

	pthread_mutex_lock(&sim->out_lock);
	timestamp = get_time_ms() - sim->start_time;
	printf("%" PRIu64 " %" PRIu64 " %s\n", timestamp, id, msg);
	pthread_mutex_unlock(&sim->out_lock);
}

int	coder_should_stop(t_simulation_state *sim, t_coder *coder)
{
	uint64_t	local;

	pthread_mutex_lock(&sim->stop_lock);
	local = sim->stop_flag;
	pthread_mutex_unlock(&sim->stop_lock);
	if (local || coder->compiles_finished >= sim->args.compiles_required)
		return (1);
	return (0);
}

int	coder_take_dongles(t_coder *coder, t_simulation_state *sim)
{
	uint64_t	first;
	uint64_t	second;

	if (coder->left == coder->right)
	{
		if (!dongle_acquire(&sim->dongle_arr[coder->left], coder, &sim->args,
				sim))
			return (0);
		log_action(sim, coder->id, "has taken a dongle");
		return (1);
	}
	first = coder->left;
	second = coder->right;
	if (first > second)
	{
		first = coder->right;
		second = coder->left;
	}
	return (acquire_pair(coder, sim, first, second));
}

void	coder_release_dongle(t_coder *coder, t_simulation_state *sim)
{
	if (coder->left == coder->right)
		dongle_release(&sim->dongle_arr[coder->left], sim->args);
	else
	{
		dongle_release(&sim->dongle_arr[coder->left], sim->args);
		dongle_release(&sim->dongle_arr[coder->right], sim->args);
	}
}

void	*coder_thread(void *arg)
{
	t_thread_arg		*ta;
	t_coder				*coder;
	t_simulation_state	*sim;

	ta = (t_thread_arg *)arg;
	coder = ta->coder;
	sim = ta->sim;
	while (!coder_should_stop(sim, coder))
	{
		if (!coder_take_dongles(coder, sim))
			break ;
		coder_work_cycle(coder, sim);
	}
	return (NULL);
}
