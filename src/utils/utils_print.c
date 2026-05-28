

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

// ! for further review later
/**
 * @brief Print an error message corresponding to the given code.
 * @param code Internal error code.
 */
void	printerr(t_errcode code)
{
	if (code == ERR_MALLOC)
		perror("minishell");
	else if (code == ERR_QUOTE)
		ft_putstr_fd("minishell: Invalid quotes\n", 2);
}
