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
