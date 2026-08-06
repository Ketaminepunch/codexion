/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   parsing.c                                         :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: vsack <vsack@student.42vienna.com>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/06 18:23:46 by vsack            #+#    #+#              */
/*   Updated: 2026/08/06 19:39:08 by vsack           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/codexion.h"

int	is_valid_number(char *str)
{
	int	i;

	i = 0;
	if (!str || strlen(str) == 0)
		return (0);
	while (str[i] != '\0')
	{
		if (str[i] < '0' || str[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

int	parse_scheduler(char *str, t_scheduler *dest)
{
	if (strcmp(str, "fifo") == 0)
	{
		*dest = FIFO;
		return (0);
	}
	else if (strcmp(str, "edf") == 0)
	{
		*dest = EDF;
		return (0);
	}
	fprintf(stderr, "Error: Scheduler type not 'fifo' or 'edf'\n");
	return (1);
}
