/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_lexer_token.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kong <kong@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/03 13:50:24 by kong              #+#    #+#             */
/*   Updated: 2026/07/03 13:50:24 by kong             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * @brief Build a pipe token (|).
 * @param token Token node to populate.
 * @return Number of characters consumed, or ERR_MALLOC on failure.
 */
int	build_ops_pipe(t_tokensll *token)
{
	token->type = TOK_PIPE;
	token->val = ft_strndup("|", 1);
	if (!token->val)
		return (ERR_MALLOC);
	return (1);
}

/**
 * @brief Build a redirection output token (>).
 * @param token Token node to populate.
 * @return Number of characters consumed, or ERR_MALLOC on failure.
 */
int	build_ops_dirout(t_tokensll *token)
{
	token->type = TOK_DIROUT;
	token->val = ft_strndup(">", 1);
	if (!token->val)
		return (ERR_MALLOC);
	return (1);
}

/**
 * @brief Build a redirection append token (>>).
 * @param token Token node to populate.
 * @return Number of characters consumed, or ERR_MALLOC on failure.
 */
int	build_ops_dirappnd(t_tokensll *token)
{
	token->type = TOK_DIRAPPND;
	token->val = ft_strndup(">>", 2);
	if (!token->val)
		return (ERR_MALLOC);
	return (2);
}

/**
 * @brief Build a redirection input token (<).
 * @param token Token node to populate.
 * @return Number of characters consumed, or ERR_MALLOC on failure.
 */
int	build_ops_dirin(t_tokensll *token)
{
	token->type = TOK_DIRIN;
	token->val = ft_strndup("<", 1);
	if (!token->val)
		return (ERR_MALLOC);
	return (1);
}

/**
 * @brief Build a heredoc token (<<).
 * @param token Token node to populate.
 * @return Number of characters consumed, or ERR_MALLOC on failure.
 */
int	build_ops_heredoc(t_tokensll *token)
{
	token->type = TOK_HEREDOC;
	token->val = ft_strndup("<<", 2);
	if (!token->val)
		return (ERR_MALLOC);
	return (2);
}
