/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   sim.c                                             :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: vsack <vsack@student.42vienna.com>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/10 22:30:00 by vsack            #+#    #+#              */
/*   Updated: 2026/08/10 22:30:00 by vsack           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/codexion.h"

int	array_slot_init(t_simulation_state *sim, uint64_t i)
{
	sim->coder_arr[i].id = i;
	sim->coder_arr[i].right = i;
	sim->coder_arr[i].left = (i - 1 + sim->args.num_coders)
		% sim->args.num_coders;
	pthread_mutex_init(&sim->coder_arr[i].lock, NULL);
	sim->coder_arr[i].last_compile_start = get_time_ms();
	sim->coder_arr[i].compiles_finished = 0;
	sim->dongle_arr[i].state = FREE;
	sim->dongle_arr[i].ready_at_ms = 0;
	pthread_mutex_init(&sim->dongle_arr[i].lock, NULL);
	pthread_cond_init(&sim->dongle_arr[i].condition, NULL);
	sim->dongle_arr[i].heap.items = malloc(sizeof(t_request) * 2);
	if (!sim->dongle_arr[i].heap.items)
		return (1);
	sim->dongle_arr[i].heap.count = 0;
	return (0);
}

int	sim_init(t_simulation_state *sim, t_args args)
{
	uint64_t	i;

	i = 0;
	sim->args = args;
	sim->start_time = get_time_ms();
	sim->stop_flag = 0;
	pthread_mutex_init(&sim->out_lock, NULL);
	pthread_mutex_init(&sim->stop_lock, NULL);
	sim->dongle_arr = malloc(sizeof(t_dongle) * args.num_coders);
	sim->coder_arr = malloc(sizeof(t_coder) * args.num_coders);
	if (!sim->dongle_arr || !sim->coder_arr)
		return (1);
	while (i < args.num_coders)
	{
		if (array_slot_init(sim, i))
			return (1);
		i++;
	}
	return (0);
}
