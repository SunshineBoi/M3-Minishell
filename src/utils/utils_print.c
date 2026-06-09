#include "minishell.h"

/**
 * @brief Write a string to a file descriptor.
 * @param s Null-terminated string to write.
 * @param fd File descriptor to write to.
 */
void	ft_putstr_fd(char *s, int fd)
{
	if (!s || fd < 0)
		return ;
	write(fd, s, ft_strlen(s));
}

void	print_tokensll(t_tokensll *tokensll)
{
	/* for testing use */
	while (tokensll)
	{
		printf("[token val]: %s$", tokensll->val);
		printf("\n\t[token type]: %d$\n", tokensll->type);
		tokensll = tokensll->next;
	}
}

/**
 * @brief Print a formatted error: minishell: prefix: msg
 *        or minishell: prefix: arg: msg
 * @param prefix Usually the command or builtin name.
 * @param arg Optional extra context (filename, variable). May be NULL.
 * @param msg The error description.
 */
void	errmsg(const char *prefix, const char *arg, const char *msg)
{
	ft_putstr_fd("minishell: ", 2);
	if (prefix)
	{
		ft_putstr_fd((char *)prefix, 2);
		ft_putstr_fd(": ", 2);
	}
	if (arg)
	{
		ft_putstr_fd((char *)arg, 2);
		ft_putstr_fd(": ", 2);
	}
	ft_putstr_fd((char *)msg, 2);
	ft_putstr_fd("\n", 2);
}

