#include "minishell.h"

/**
 * @brief Check if a character is whitespace.
 * @param ch Character code to test.
 * @return Nonzero if whitespace, zero otherwise.
 */
int	iswhitespace(int ch)
{
	return (ch == ' ' || ch == '\n' || ch == '\t'
		|| ch == '\f' || ch == '\v' || ch == '\r');
}

/**
 * @brief Check if a character is a special shell symbol.
 * @param ch Character code to test.
 * @return Nonzero if special, zero otherwise.
 */
int	isspecialsym(int ch)
{
	return (ch == '|' || ch == '>' || ch == '<');
}

int	ft_strhaschr(char *str, char ch)
{
	int	i;

	i = 0;
	while (str[i])
	{
		if (str[i] == ch)
			return (1);
		i++;
	}
	return (0);
}

static void	update_num_sign(const char *s, int *sign, int *pos)
{
	int	i;

	i = (*pos);
	if (s[i] == '+' || s[i] == '-')
	{
		if (s[i] == '-')
			(*sign) = -1;
		(*pos)++;
	}
}

static int	is_whitespaceonly(char *s)
{
	while (*s)
	{
		if (!iswhitespace(*s))
			return (0);
		s++;
	}
	return (1);
}

int	is_numeric(const char *s, long long *val)
{
	int			i;
	int			sign;
	long long	result;

	i = 0;
	sign = 1;
	result = 0;
	while (s[i] == ' ' || (s[i] >= '\t' && s[i] <= '\r'))
		i++;
	update_num_sign(s, &sign, &i);
	if (!s[i])
		return (0);
	while (s[i])
	{
		if (s[i] < '0' || s[i] > '9')
			return (is_whitespaceonly(s + i));
		if (sign == 1 && (result > (LLONG_MAX - (s[i] - '0')) / 10))
			return (0);
		if (sign == -1 && (-result < (LLONG_MIN + (s[i] - '0')) / 10))
			return (0);
		result = result * 10 + (s[i] - '0');
		i++;
	}
	*val = result * sign;
	return (1);
}
