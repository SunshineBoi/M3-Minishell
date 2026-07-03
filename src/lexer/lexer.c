/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kong <kong@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/03 13:14:36 by kong              #+#    #+#             */
/*   Updated: 2026/07/03 13:14:36 by kong             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * @brief Update list pointers after creating a new token.
 * @param ops List traversal helpers.
 */
static void	_update_ops(t_sll_ops *ops)
{
	if (!ops->head)
		ops->head = ops->curr;
	if (ops->prev)
		ops->prev->next = ops->curr;
	ops->prev = ops->curr;
}

/**
 * @brief Build a single token from the input.
 * @param ops List traversal helpers.
 * @param str Input buffer at the current position.
 * @return Characters consumed, or ERR_* on failure.
 */
static int	build_token(t_app *app, t_sll_ops *ops, char *str)
{
	int	toklen;

	ops->curr = init_token();
	if (!ops->curr)
		return (ERR_MALLOC);
	if (isspecialsym(*str))
	{
		toklen = special_build(str, ops->curr);
		if (toklen == ERR_MALLOC)
			return (freetoken(ops->curr), ERR_MALLOC);
	}
	else
	{
		toklen = string_build(ops->curr, str);
		if (toklen == ERR_QUOTE)
			return (setexit(app, EX_SYNTAX), freetoken(ops->curr), ERR_QUOTE);
		else if (toklen == ERR_MALLOC)
			return (freetoken(ops->curr), ERR_MALLOC);
	}
	return (toklen);
}

/**
 * @brief Tokenize the input string into a linked list.
 * @param str Input command line string.
 * @return Head of the token list, or NULL on failure.
 */
t_tokensll	*build_tokensll(t_app *app, char *str)
{
	int			toklen;
	t_tokensll	*head;
	t_sll_ops	*ops;

	ops = init_sll_ops();
	if (!ops)
		return (hardexit(), NULL);
	while (*str)
	{
		while (*str && iswhitespace(*str))
			str++;
		if (!*str)
			break ;
		toklen = build_token(app, ops, str);
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
