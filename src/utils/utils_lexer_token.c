/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_lexer_token.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kong <kong@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/22 17:19:08 by kong              #+#    #+#             */
/*   Updated: 2026/05/22 17:19:08 by kong             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	build_ops_pipe(t_tokensll *token)
{
	token->type = TOK_PIPE;
	token->val = ft_strndup("|", 1);
	if (!token->val)
		return (ERR_MALLOC);
	return (1);
}

int	build_ops_dirout(t_tokensll *token)
{
	token->type = TOK_DIROUT;
	token->val = ft_strndup(">", 1);
	if (!token->val)
		return (ERR_MALLOC);
	return (1);
}

int	build_ops_dirappnd(t_tokensll *token)
{
	token->type = TOK_DIRAPPND;
	token->val = ft_strndup(">>", 2);
	if (!token->val)
		return (ERR_MALLOC);
	return (2);
}

int	build_ops_dirin(t_tokensll *token)
{
	token->type = TOK_DIRIN;
	token->val = ft_strndup("<", 1);
	if (!token->val)
		return (ERR_MALLOC);
	return (1);
}

int	build_ops_heredoc(t_tokensll *token)
{
	token->type = TOK_HEREDOC;
	token->val = ft_strndup("<<", 2);
	if (!token->val)
		return (ERR_MALLOC);
	return (2);
}
