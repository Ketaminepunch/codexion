/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   utils.c                                           :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: vsack <vsack@student.42vienna.com>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/12 18:51:19 by vsack            #+#    #+#              */
/*   Updated: 2026/08/12 19:10:47 by vsack           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/codexion.h"

int	acquire_pair(t_coder *coder, t_simulation_state *sim, uint64_t first,
		uint64_t second)
{
	t_dongle	*dongle_arr;
	int			tmp;

	dongle_arr = sim->dongle_arr;
	tmp = dongle_acquire(&dongle_arr[first], coder, &sim->args, sim);
	if (!tmp)
		return (0);
	log_action(sim, coder->id, "has taken a dongle");
	tmp = dongle_acquire(&dongle_arr[second], coder, &sim->args, sim);
	if (!tmp)
	{
		dongle_release(&dongle_arr[first], sim->args);
		return (0);
	}
	log_action(sim, coder->id, "has taken dongle");
	return (1);
}

void	coder_work_cycle(t_coder *coder, t_simulation_state *sim)
{
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
