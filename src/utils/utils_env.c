#include "minishell.h"

static void	fill_buf(char *buf, int *i, long n)
{
	if (n >= 10)
		fill_buf(buf, i, n / 10);
	buf[(*i)++] = (char)('0' + (n % 10));
}

char	*itoa_status(int status)
{
	char	buf[12];
	int		i;
	long	n;
	char	*out;

	i = 0;
	if (status < 0)
		buf[i++] = '-';
	n = status;
	if (status < 0)
		n = -n;
	fill_buf(buf, &i, n);
	buf[i] = '\0';
	out = ft_strndup(buf, i);
	return (out);
}

int	is_name_start(char c)
{
	return ((c >= 'A' && c <= 'Z')
		|| (c >= 'a' && c <= 'z')
		|| c == '_');
}

size_t	var_name_len(const char *s)
{
	size_t	len;

	if (!s || !is_name_start(*s))
		return (0);
	len = 0;
	while (s[len] && (is_name_start(s[len]) || (s[len] >= '0' && s[len] <= '9')))
		len++;
	return (len);
}

const char	*env_lookup(char **envp, const char *key, size_t key_len)
{
	size_t	i;

	if (!envp || !key)
		return ("");
	i = 0;
	while (envp[i])
	{
		if (ft_strlen(envp[i]) > key_len
			&& envp[i][key_len] == '=')
		{
			size_t	j = 0;
			while (j < key_len && envp[i][j] == key[j])
				j++;
			if (j == key_len)
				return (envp[i] + key_len + 1);
		}
		i++;
	}
	return ("");
}
