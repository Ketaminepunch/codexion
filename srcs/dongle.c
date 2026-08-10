/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   dongle.c                                          :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: vsack <vsack@student.42vienna.com>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/10 17:07:04 by vsack            #+#    #+#              */
/*   Updated: 2026/08/10 20:31:55 by vsack           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/codexion.h"

uint64_t	get_time_ms(void)
{
	struct timeval	time;

	gettimeofday(&time, NULL);
	return ((time.tv_sec * 1000) + (time.tv_usec / 1000));
}

void	dongle_release(t_dongle *dongle, t_args args)
{
	pthread_mutex_lock(&dongle->lock);
	dongle->state = COOLING;
	dongle->ready_at_ms = get_time_ms() + args.dongle_cooldown;
	pthread_mutex_unlock(&dongle->lock);
	pthread_cond_broadcast(&dongle->condition);
}

void	dongle_wait_turn(t_dongle *dongle)
{
	struct timespec	wait_time;

	if (dongle->state != HELD && get_time_ms() < dongle->ready_at_ms)
	{
		wait_time.tv_sec = dongle->ready_at_ms / 1000;
		wait_time.tv_nsec = (dongle->ready_at_ms % 1000) * 1000000;
		pthread_cond_timedwait(&dongle->condition, &dongle->lock, &wait_time);
	}
	else
		pthread_cond_wait(&dongle->condition, &dongle->lock);
}

void	dongle_acquire(t_dongle *dongle, t_coder *coder, t_args *args)
{
	t_request	request;

	request.id = coder->id;
	request.arrival_time = get_time_ms();
	request.deadline = coder->last_compile_start + args->time_to_burnout;
	pthread_mutex_lock(&dongle->lock);
	heap_push(&dongle->heap, &request, args);
	while (dongle->state == HELD || get_time_ms() < dongle->ready_at_ms
		|| dongle->heap.items[0].id != coder->id)
		dongle_wait_turn(dongle);
	heap_pop(&dongle->heap, args);
	dongle->state = HELD;
	pthread_mutex_unlock(&dongle->lock);
}
