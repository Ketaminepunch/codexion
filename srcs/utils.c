/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   utils.c                                           :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: vsack <vsack@student.42vienna.com>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/10 20:57:08 by vsack            #+#    #+#              */
/*   Updated: 2026/08/10 21:55:23 by vsack           ###   ########.fr        */
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

void	coder_take_dongles(t_coder *coder, t_simulation_state *sim)
{
	if (coder->left < coder->right)
	{
		dongle_acquire(&sim->dongle_arr[coder->left], coder, &sim->args);
		log_action(sim, coder->id, "has taken a dongle");
		dongle_acquire(&sim->dongle_arr[coder->right], coder, &sim->args);
		log_action(sim, coder->id, "has taken a dongle");
	}
	else
	{
		dongle_acquire(&sim->dongle_arr[coder->right], coder, &sim->args);
		log_action(sim, coder->id, "has taken a dongle");
		dongle_acquire(&sim->dongle_arr[coder->left], coder, &sim->args);
		log_action(sim, coder->id, "has taken a dongle");
	}
}

void	coder_release_dongle(t_coder *coder, t_simulation_state *sim)
{
	dongle_release(&sim->dongle_arr[coder->left], sim->args);
	dongle_release(&sim->dongle_arr[coder->right], sim->args);
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
		coder_take_dongles(coder, sim);
		pthread_mutex_lock(&coder->lock);
		coder->last_compile_start = get_time_ms();
		pthread_mutex_unlock(&coder->lock);
		log_action(sim, coder->id, "is compiling");
		usleep(sim->args.time_to_compile * 1000);
		coder_release_dongle(coder, sim);
		log_action(sim, coder->id, "is debugging");
		usleep(sim->args.time_to_debug * 1000);
		log_action(sim, coder->id, "is refactoring");
		usleep(sim->args.time_to_refactor * 1000);
		pthread_mutex_lock(&coder->lock);
		coder->compiles_finished++;
		pthread_mutex_unlock(&coder->lock);
	}
	return (NULL);
}
