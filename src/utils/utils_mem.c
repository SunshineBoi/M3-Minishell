/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_mem.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kong <kong@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/22 17:15:55 by kong              #+#    #+#             */
/*   Updated: 2026/05/22 17:15:55 by kong             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"


void	*ft_memcpy(void *dest, const void *src, size_t nbyte)
{
	unsigned char		*dest_p;
	const unsigned char	*src_p;

	if (!dest || !src)
		return (NULL);
	dest_p = dest;
	src_p = src;
	while (nbyte--)
		*dest_p++ = *src_p++;
	return (dest);
}

char	*ft_realloc(char *old, size_t old_size, size_t new_size)
{
	char	*new;

	new = malloc(new_size + 1);
	if (!new)
		return (NULL);
	if (old)
	{
		new = (char *)ft_memcpy(new, old, old_size);
		new[old_size] = '\0';
		// on success, free old string
		free(old);
	}
	return (new);
}