/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_str_1.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kong <kong@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/03 15:39:46 by kong              #+#    #+#             */
/*   Updated: 2026/07/03 15:39:46 by kong             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * @brief Compare two null-terminated strings.
 * @param s1 First string.
 * @param s2 Second string.
 * @return Differences between first differing characters.
 */
int	ft_strcmp(const char *s1, const char *s2)
{
	size_t	i;

	i = 0;
	while (s1[i] && s1[i] == s2[i])
		i++;
	return ((unsigned char)s1[i] - (unsigned char)s2[i]);
}

/**
 * @brief Compare at most n bytes of two strings.
 */
int	ft_strncmp(const char *s1, const char *s2, size_t n)
{
	size_t	i;

	i = 0;
	while (i < n && s1[i] && s1[i] == s2[i])
		i++;
	if (i == n)
		return (0);
	return ((unsigned char)s1[i] - (unsigned char)s2[i]);
}

/**
 * @brief Locate the first occurrence of c in s.
 * @return Pointer to the character, or NULL if not found.
 */
char	*ft_strchr(const char *s, int c)
{
	while (*s)
	{
		if (*s == (char)c)
			return ((char *)s);
		s++;
	}
	if ((char)c == '\0')
		return ((char *)s);
	return (NULL);
}

/**
 * @brief Fill buf with the decimal digits of num, least significant first.
 * @param num Non-negative value to convert.
 * @param buf Destination buffer.
 * @return Number of digits written.
 */
static int	_itoa_digits(long num, char *buf)
{
	int	i;

	i = 0;
	if (num == 0)
		buf[i++] = '0';
	while (num > 0)
	{
		buf[i++] = (char)('0' + (num % 10));
		num /= 10;
	}
	return (i);
}

/**
 * @brief Convert an integer to a null-terminated string.
 * @param n Integer to convert.
 * @return Newly allocated string, or NULL on failure.
 */
char	*ft_itoa(int n)
{
	char	buf[12];
	long	num;
	int		i;
	char	*out;
	int		j;

	num = n;
	if (n < 0)
		num = -num;
	i = _itoa_digits(num, buf);
	if (n < 0)
		buf[i++] = '-';
	out = malloc((size_t)i + 1);
	if (!out)
		return (NULL);
	out[i] = '\0';
	j = 0;
	while (i-- > 0)
		out[j++] = buf[i];
	return (out);
}
