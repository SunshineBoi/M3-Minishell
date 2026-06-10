#include "minishell.h"

void	printerr_cd(char *filename)
{
	ft_putstr_fd("minishell: cd: ", 2);
	ft_putstr_fd(filename, 2);
	ft_putstr_fd(": no such file or directory\n", 2);
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
