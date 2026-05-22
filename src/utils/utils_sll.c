/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_linkedlist.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kong <kong@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/22 17:12:42 by kong              #+#    #+#             */
/*   Updated: 2026/05/22 17:12:42 by kong             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

t_tokensll	*init_token()
{
	t_tokensll	*new;

	new = malloc(sizeof(t_tokensll));
	if (!new)
		return (printerr(ERR_MALLOC), NULL);
	new->next = NULL;
	new->val = NULL;
	new->val_size = BUFFER_SIZE;
	new->type = TOK_STR;
	return (new);
}

t_sll_ops	*init_sll_ops()
{
	t_sll_ops	*ops;

	ops = malloc(sizeof(t_sll_ops));
	if (!ops)
		return (printerr(ERR_MALLOC), NULL);
	ops->curr = NULL;
	ops->head = NULL;
	ops->prev = NULL;
	return (ops);
}

int	ft_sllsize(t_tokensll *sll)
{
	int	size;

	size = 0;
	while (sll)
	{
		size++;
		sll = sll->next;
	}
	return (size);
}

void	freetoken(t_tokensll *token)
{
	if (!token)
		return ;
	free(token->val);
	free(token);
}

void	freetokensll(t_tokensll *sll)
{
	t_tokensll	*next;

	if (!sll)
		return ;
	while (sll)
	{
		next = sll->next;
		free(sll->val);
		free(sll);
		sll = next;
	}
}
