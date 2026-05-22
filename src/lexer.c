/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kong <kong@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/16 16:39:14 by kong              #+#    #+#             */
/*   Updated: 2026/05/16 16:39:14 by kong             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	_update_ops(t_sll_ops *ops)
{
	if (!ops->head)
		ops->head = ops->curr;
	if (ops->prev)
		ops->prev->next = ops->curr;
	ops->prev = ops->curr;
}

static int	build_token(t_sll_ops *ops, char *str)
{
	int	toklen;

	ops->curr = init_token();
	if (!ops->curr)
		return (ERR_MALLOC);
	// process special char
	if (isspecialsym(*str))
	{
		toklen = special_build(str, ops->curr);
		if (toklen == ERR_MALLOC)
			return (freetoken(ops->curr), ERR_MALLOC);
	}
	else
	{
		// process quotes and text
		toklen = string_build(ops->curr, str);
		if (toklen == ERR_QUOTE)
			return (freetoken(ops->curr), ERR_QUOTE);
		else if (toklen == ERR_MALLOC)
			return (freetoken(ops->curr), ERR_MALLOC);
	}
	return (toklen);
}

t_tokensll	*build_tokensll(char *str)
{
	int	toklen;
	t_tokensll	*head;
	t_sll_ops	*ops;

	ops = init_sll_ops();
	if (!ops)
		return (hardexit(), NULL);
	while (*str)
	{
		// process whitespace
		while (*str && iswhitespace(*str))
			str++;
		if (!*str)
			break ;
		toklen = build_token(ops, str);
		if (toklen == ERR_QUOTE)
			return (freetokensll(ops->head), free(ops), NULL);
		else if (toklen == ERR_MALLOC)
			return (freetokensll(ops->head), free(ops), hardexit(), NULL);
		_update_ops(ops);		
		str += toklen;
	}
	head = ops->head;
	free(ops);
	return (head);
}
