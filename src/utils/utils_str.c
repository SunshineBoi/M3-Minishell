#include "minishell.h"

/**
 * @brief Compute the length of a null-terminated string.
 * @param str Input string.
 * @return Length excluding the null terminator.
 */
size_t	ft_strlen(const char *str)
{
	size_t	count;

	count = 0;
	while (str[count])
		count++;
	return (count);
}

/**
 * @brief Duplicate up to len characters into a new string.
 * @param str Source string.
 * @param len Maximum number of characters to copy.
 * @return Newly allocated string, or NULL on failure.
 */
char	*ft_strndup(char *str, int len)
{
	char	*new;
	int		i;

	new = malloc((len + 1) * sizeof(char));
	if (!new)
		return (NULL);
	i = 0;
	while (i < len && str[i])
	{
		new[i] = str[i];
		i++;
	}
	new[i] = '\0';
	return (new);
}

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
 * @brief Duplicate a null-terminated string.
 * @param s Source string.
 * @return Newly allocated copy, or NULL on failure.
 */
char	*ft_strdup(const char *s)
{
	size_t	len;
	char	*dup;
	size_t	i;

	if (!s)
		return (NULL);
	len = ft_strlen(s);
	dup = malloc(len + 1);
	if (!dup)
		return (NULL);
	i = 0;
	while (i < len)
	{
		dup[i] = s[i];
		i++;
	}
	dup[i] = '\0';
	return (dup);
}

/**
 * @brief Concatenate two strings into a newly allocated string.
 * @param s1 First string.
 * @param s2 Second string.
 * @return New string s1+s2, or NULL on failure.
 */
char	*ft_strjoin(const char *s1, const char *s2)
{
	char	*result;
	size_t	len1;
	size_t	len2;

	if (!s1 && !s2)
		return (NULL);
	if (!s1)
		return (ft_strdup(s2));
	if (!s2)
		return (ft_strdup(s1));
	len1 = ft_strlen(s1);
	len2 = ft_strlen(s2);
	result = malloc(len1 + len2 + 1);
	if (!result)
		return (NULL);
	ft_memcpy(result, s1, len1);
	ft_memcpy(result + len1, s2, len2);
	result[len1 + len2] = '\0';
	return (result);
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
 * @brief Convert an integer to a null-terminated string.
 * @param n Integer to convert.
 * @return Newly allocated string, or NULL on failure.
 */
char	*ft_itoa(int n)
{
	char	buf[12];
	int		i;
	long	num;
	int		neg;
	char	*out;
	int		j;

	num = n;
	neg = (num < 0);
	if (neg)
		num = -num;
	i = 0;
	if (num == 0)
		buf[i++] = '0';
	while (num > 0)
	{
		buf[i++] = (char)('0' + (num % 10));
		num /= 10;
	}
	if (neg)
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


