/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_validator_tokens.c                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kong <kong@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/03 15:13:26 by kong              #+#    #+#             */
/*   Updated: 2026/07/03 15:13:26 by kong             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	ispipe(t_token_type type)
{
	return (type == TOK_PIPE);
}

int	isredir(t_token_type type)
{
	return (type == TOK_DIROUT || type == TOK_DIRAPPND
		|| type == TOK_DIRIN || type == TOK_HEREDOC);
}

/**
 * @brief Check that a pipe token is followed by a valid token.
 * pipe cannot end with null (bash prompts next line); 
 * pipe cannot followed by pipe.
 * @param tokens Pipe token to validate.
 * @return 1 if valid, 0 on syntax error.
 */
int	is_valid_pipe(t_tokensll *tokens)
{
	if (!tokens->next)
		return (printerr_syntax(tokens->val), 0);
	else if (ispipe(tokens->next->type))
		return (printerr_syntax(tokens->next->val), 0);
	return (1);
}

/**
 * @brief Check that a redirection token is followed by a valid token.
 * Cannot end with null; cannot followed by pipe; cannot followed by redir;
 * @param tokens Redirection token to validate.
 * @return 1 if valid, 0 on syntax error.
 */
int	is_valid_redir(t_tokensll *tokens)
{
	if (!tokens->next)
		return (printerr_syntax("newline"), 0);
	else if (ispipe(tokens->next->type))
		return (printerr_syntax(tokens->next->val), 0);
	else if (isredir(tokens->next->type))
		return (printerr_syntax(tokens->next->val), 0);
	return (1);
}
