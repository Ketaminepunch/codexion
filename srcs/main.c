/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   main.c                                            :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: vsack <vsack@student.42vienna.com>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/06 18:37:40 by vsack            #+#    #+#              */
/*   Updated: 2026/08/10 22:52:47 by vsack           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/codexion.h"

int	set_number(char *str, uint64_t *dest, int idx)
{
	if (is_valid_number(str) == 0)
	{
		fprintf(stderr, "Error: argument %i not a positive number\n", idx);
		return (1);
	}
	*dest = strtoull(str, NULL, 10);
	return (0);
}

int	parse_args(char **av, t_args *args)
{
	if (set_number(av[1], &args->num_coders, 1))
		return (1);
	if (set_number(av[2], &args->time_to_burnout, 2))
		return (1);
	if (set_number(av[3], &args->time_to_compile, 3))
		return (1);
	if (set_number(av[4], &args->time_to_debug, 4))
		return (1);
	if (set_number(av[5], &args->time_to_refactor, 5))
		return (1);
	if (set_number(av[6], &args->compiles_required, 6))
		return (1);
	if (set_number(av[7], &args->dongle_cooldown, 7))
		return (1);
	if (parse_scheduler(av[8], &args->scheduler))
		return (1);
	return (0);
}

int	main(int ac, char **av)
{
	t_args				args;
	t_simulation_state	sim;
	t_thread_arg		*thread_args;

	if (ac != 9)
	{
		return (fprintf(stderr, "Error: Not 9 arguments\n"), 1);
	}
	if (parse_args(av, &args) == 0)
	{
		if (sim_init(&sim, args))
			return (1);
		thread_args = malloc(sizeof(t_thread_arg) * args.num_coders);
		if (!thread_args)
			return (1);
		if (spawn_coders(&sim, thread_args))
			return (1);
		if (join_coders(&sim))
			return (1);
	}
	else
		return (1);
	return (0);
}

int	spawn_coders(t_simulation_state *sim, t_thread_arg *thread_args)
{
	uint64_t	i;

	i = 0;
	while (i < sim->args.num_coders)
	{
		thread_args[i].coder = &sim->coder_arr[i];
		thread_args[i].sim = sim;
		if (pthread_create(&sim->coder_arr[i].ticket, NULL, coder_thread,
				&thread_args[i]))
			return (1);
		i++;
	}
	return (0);
}

int	join_coders(t_simulation_state *sim)
{
	uint64_t	i;

	i = 0;
	while (i < sim->args.num_coders)
	{
		if (pthread_join(sim->coder_arr[i].ticket, NULL))
			return (1);
		i++;
	}
	return (0);
}
