/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   scheduling.c                                      :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: vsack <vsack@student.42vienna.com>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/10 18:42:45 by vsack            #+#    #+#              */
/*   Updated: 2026/08/10 19:36:48 by vsack           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/codexion.h"
#include <stdint.h>

int	compare_requests(t_request *request1, t_request *request2, t_args *args)
{
	if (args->scheduler == FIFO)
	{
		if (request1->arrival_time < request2->arrival_time)
			return (1);
		return (0);
	}
	else
	{
		if (request1->deadline == request2->deadline)
		{
			if (request1->arrival_time < request2->arrival_time)
				return (1);
			return (0);
		}
		else
		{
			if (request1->deadline < request2->deadline)
				return (1);
			return (0);
		}
	}
}

void	heap_swap(t_heap *heap, uint64_t i, uint64_t j)
{
	t_request	tmp;

	tmp = heap->items[i];
	heap->items[i] = heap->items[j];
	heap->items[j] = tmp;
}

void	heap_push(t_heap *heap, t_request *request, t_args *args)
{
	uint64_t	i;

	i = heap->count;
	heap->items[heap->count] = *request;
	heap->count++;
	while (i > 0 && compare_requests(&heap->items[i], &heap->items[(i - 1) / 2],
			args))
	{
		heap_swap(heap, i, (i - 1) / 2);
		i = (i - 1) / 2;
	}
}

uint64_t	most_urgent_child(t_heap *heap, uint64_t i, t_args *args)
{
	uint64_t	best;
	uint64_t	left;
	uint64_t	right;

	best = i;
	left = 2 * i + 1;
	right = left + 1;
	if (left < heap->count && compare_requests(&heap->items[left],
			&heap->items[best], args))
		best = left;
	if (right < heap->count && compare_requests(&heap->items[right],
			&heap->items[best], args))
		best = right;
	return (best);
}

t_request	heap_pop(t_heap *heap, t_args *args)
{
	t_request	root_val;
	uint64_t	i;
	uint64_t	best;

	i = 0;
	root_val = heap->items[0];
	heap->items[0] = heap->items[heap->count - 1];
	heap->count--;
	best = most_urgent_child(heap, i, args);
	while (best != i)
	{
		heap_swap(heap, i, best);
		i = best;
		best = most_urgent_child(heap, i, args);
	}
	return (root_val);
}
