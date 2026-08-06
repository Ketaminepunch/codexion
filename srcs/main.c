/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   main.c                                            :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: vsack <vsack@student.42vienna.com>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/06 18:37:40 by vsack            #+#    #+#              */
/*   Updated: 2026/08/06 19:39:26 by vsack           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/codexion.h"

int	set_number(char *str, long *dest, int idx)
{
	if (is_valid_number(str) == 0)
	{
		fprintf(stderr, "Error: argument %i not a positive number\n", idx);
		return (1);
	}
	*dest = atoi(str);
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
	t_args	args;

	if (ac != 9)
	{
		fprintf(stderr, "Error: argument count not 9\n");
		return (1);
	}
	if (parse_args(av, &args) == 1)
		return (1);
	return (0);
}
