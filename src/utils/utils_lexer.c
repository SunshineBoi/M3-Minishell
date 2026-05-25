/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_lexer.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kong <kong@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/16 16:37:18 by kong              #+#    #+#             */
/*   Updated: 2026/05/16 16:37:18 by kong             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	_resizebuffer(t_tokensll *token, int old_size, int new_size)
{
	if (new_size >= token->val_size)
	{
		token->val_size = (new_size) * 2;
		token->val = ft_realloc(token->val, old_size, token->val_size);
		if (!token->val)
			return (ERR_MALLOC);
	}
	return (1);
}

static int	_quotes_build(char *str, t_tokensll *token, char quote)
{
	int	src_i;
	int	dest_i;

	dest_i = 0;
	if (token && token->val == NULL)
	{
		token->val = malloc(token->val_size);
		if (!token->val)
			return (printerr(ERR_MALLOC), ERR_MALLOC);
	}
	else
		dest_i = ft_strlen(token->val);
	src_i = 1;
	while (str[src_i] && str[src_i] != quote)
		src_i++;
	if (str[src_i] == '\0')
		return (printerr(ERR_QUOTE), ERR_QUOTE);
	else if (str[src_i] == quote)
		src_i += 1;
	// adjust buffer size if needed
	if (_resizebuffer(token, dest_i, src_i + dest_i) == ERR_MALLOC)
		return (printerr(ERR_MALLOC), ERR_MALLOC);
	// starts appending
	ft_memcpy((token->val) + dest_i, str, src_i);
	token->val[src_i + dest_i] = '\0';
	return (src_i);
}

static int	_char_build(char ch, t_tokensll *token)
{
	int	dest_i;

	dest_i = 0;
	if (token && token->val == NULL)
	{
		token->val = malloc(token->val_size);
		if (!token->val)
			return (printerr(ERR_MALLOC), ERR_MALLOC);
	}
	else
		dest_i = ft_strlen(token->val);
	// starts appending
	if (dest_i + 1 >= token->val_size)
	{
		token->val_size = (dest_i + 1) * 2;
		token->val = ft_realloc(token->val, dest_i, token->val_size);
		if (!token->val)
			return (printerr(ERR_MALLOC), ERR_MALLOC);
	}
	token->val[dest_i] = ch;
	token->val[dest_i + 1] = '\0';
	return (1);
}

int	string_build(t_tokensll *token, char *str)
{
	int	len;
	int	built_skipped;

	len = 0;
	while (str[len] && !iswhitespace(str[len]) && !isspecialsym(str[len]))
	{
		built_skipped = 0;
		// ! to confirm if need to implement backslash
		if (str[len] == '\'' || str[len] == '"')
			built_skipped = _quotes_build(str + len, token, str[len]);
		else
			built_skipped = _char_build(str[len], token);
		if (built_skipped == ERR_QUOTE)
			return (ERR_QUOTE);
		else if (built_skipped == ERR_MALLOC)
			return (ERR_MALLOC);
		len += built_skipped;
	}
	return (len);
}

int	special_build(char *str, t_tokensll *token)
{
	int	build_flag;

	build_flag = 0;
	if (*str == '|')
		build_flag = build_ops_pipe(token);
	else if (*str == '>')
	{	
		if (*(str + 1) && *(str + 1) == '>')
			build_flag = build_ops_dirappnd(token);
		else
			build_flag = build_ops_dirout(token);
	}
	else if (*str == '<')
	{
		if (*(str + 1) && *(str + 1) == '<')
			build_flag = build_ops_heredoc(token);
		else
			build_flag = build_ops_dirin(token);
	}
	if (build_flag == ERR_MALLOC)
		return (printerr(ERR_MALLOC), ERR_MALLOC);
	return (build_flag);
}
