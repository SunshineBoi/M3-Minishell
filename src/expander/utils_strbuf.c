/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_strbuf.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kong <kong@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/03 22:04:22 by kong              #+#    #+#             */
/*   Updated: 2026/07/03 22:04:23 by kong             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	sb_init(t_strbuf *sb)
{
	sb->cap = 16;
	sb->len = 0;
	sb->buf = malloc(sb->cap + 1);
	if (!sb->buf)
		return (ERR_MALLOC);
	sb->buf[0] = '\0';
	return (0);
}

int	sb_reserve(t_strbuf *sb, size_t needed)
{
	char	*newbuf;
	size_t	newcap;

	if (needed + 1 <= sb->cap)
		return (0);
	newcap = (needed + 1) * 2;
	newbuf = ft_realloc(sb->buf, sb->len, newcap);
	if (!newbuf)
		return (ERR_MALLOC);
	sb->buf = newbuf;
	sb->cap = newcap;
	return (0);
}

int	sb_push_char(t_strbuf *sb, char c)
{
	if (sb_reserve(sb, sb->len + 1) != 0)
		return (ERR_MALLOC);
	sb->buf[sb->len++] = c;
	sb->buf[sb->len] = '\0';
	return (0);
}

int	sb_push_str(t_strbuf *sb, const char *s)
{
	size_t	add;

	if (!s)
		return (0);
	add = ft_strlen(s);
	if (add == 0)
		return (0);
	if (sb_reserve(sb, sb->len + add) != 0)
		return (ERR_MALLOC);
	ft_memcpy(sb->buf + sb->len, s, add);
	sb->len += add;
	sb->buf[sb->len] = '\0';
	return (0);
}

void	sb_free(t_strbuf *sb)
{
	free(sb->buf);
	sb->buf = NULL;
	sb->len = 0;
	sb->cap = 0;
}
