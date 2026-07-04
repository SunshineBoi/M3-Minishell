/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand_helpers_1.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kong <kong@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/03 22:02:33 by kong              #+#    #+#             */
/*   Updated: 2026/07/03 22:02:34 by kong             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	init_ctx(t_expand_ctx *ctx, const char *input, char **envp, int last_status)
{
	ctx->input = input;
	ctx->envp = envp;
	ctx->last_status = last_status;
	if (sb_init(&ctx->sb) != 0)
		return (ERR_MALLOC);
	if (wl_init(&ctx->wl) != 0)
	{
		sb_free(&ctx->sb);
		return (ERR_MALLOC);
	}
	ctx->state = Q_NONE;
	ctx->word_in_progress = 0;
	ctx->i = 0;
	return (0);
}

int	flush_word(t_wordlist *wl, t_strbuf *sb, int force_empty)
{
	char	*word;

	if (sb->len == 0 && !force_empty)
		return (0);
	if (sb->len == 0)
		word = ft_strndup("", 0);
	else
		word = ft_strndup(sb->buf, (int)sb->len);
	if (!word)
		return (ERR_MALLOC);
	if (wl_push(wl, word) != 0)
	{
		free(word);
		return (ERR_MALLOC);
	}
	sb->len = 0;
	if (sb->buf)
		sb->buf[0] = '\0';
	return (0);
}

int	append_unquoted_text(t_strbuf *sb, t_wordlist *wl,
		const char *text, int *word_in_progress)
{
	size_t	i;

	if (!text)
		return (0);
	i = 0;
	while (text[i])
	{
		if (iswhitespace(text[i]))
		{
			if (*word_in_progress)
			{
				if (flush_word(wl, sb, 1) != 0)
					return (ERR_MALLOC);
				*word_in_progress = 0;
			}
			i++;
			continue ;
		}
		*word_in_progress = 1;
		if (sb_push_char(sb, text[i]) != 0)
			return (ERR_MALLOC);
		i++;
	}
	return (0);
}

int	handle_whitespace(t_expand_ctx *ctx)
{
	if (ctx->state == Q_NONE && iswhitespace(ctx->input[ctx->i]))
	{
		if (ctx->word_in_progress)
		{
			if (flush_word(&ctx->wl, &ctx->sb, 1) != 0)
				return (ERR_MALLOC);
			ctx->word_in_progress = 0;
		}
		ctx->i++;
		return (1);
	}
	return (0);
}

int	handle_quotes(t_expand_ctx *ctx)
{
	if (ctx->input[ctx->i] == '\'' && ctx->state != Q_DQUOTE)
	{
		if (ctx->state == Q_SQUOTE)
			ctx->state = Q_NONE;
		else
			ctx->state = Q_SQUOTE;
		ctx->word_in_progress = 1;
		ctx->i++;
		return (1);
	}
	if (ctx->input[ctx->i] == '"' && ctx->state != Q_SQUOTE)
	{
		if (ctx->state == Q_DQUOTE)
			ctx->state = Q_NONE;
		else
			ctx->state = Q_DQUOTE;
		ctx->word_in_progress = 1;
		ctx->i++;
		return (1);
	}
	return (0);
}
