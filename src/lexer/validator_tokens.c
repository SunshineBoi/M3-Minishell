/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   validator_tokens.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kong <kong@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/03 15:10:05 by kong              #+#    #+#             */
/*   Updated: 2026/07/03 15:10:20 by kong             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	has_unsupported_ops(const char *val)
{
	if (!val)
		return (0);
	if (ft_strcmp(val, ";") == 0 || ft_strcmp(val, "&") == 0
		|| ft_strcmp(val, "&&") == 0 || ft_strcmp(val, "(") == 0
		|| ft_strcmp(val, ")") == 0)
		return (1);
	return (0);
}

static int	is_leading_pipe(t_app *app, t_tokensll *tokensll)
{
	if (ispipe(tokensll->type))
	{
		app->exitcode = EX_SYNTAX;
		printerr_syntax(tokensll->val);
		return (-1);
	}
	return (0);
}

static int	check_token_syntax(t_app *app, t_tokensll *tokensll)
{
	int	syntax_error;

	syntax_error = 0;
	if (ispipe(tokensll->type) && !is_valid_pipe(tokensll))
		syntax_error = 1;
	else if (isredir(tokensll->type) && !is_valid_redir(tokensll))
		syntax_error = 1;
	else if (tokensll->type == TOK_STR && has_unsupported_ops(tokensll->val))
	{
		syntax_error = 1;
		printerr_syntax(tokensll->val);
	}
	if (!syntax_error)
		return (0);
	app->exitcode = EX_SYNTAX;
	return (-1);
}

int	validate_tokensll(t_app *app)
{
	t_tokensll	*tokensll;

	tokensll = app->tokensll;
	if (!tokensll)
		return (0);
	if (is_leading_pipe(app, tokensll) == -1)
		return (-1);
	while (tokensll)
	{
		if (check_token_syntax(app, tokensll) == -1)
			return (-1);
		tokensll = tokensll->next;
	}
	return (0);
}
