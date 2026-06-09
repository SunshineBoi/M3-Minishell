#include "minishell.h"

void	printerr_syscall(t_errcode code)
{
	if (code == ERR_MALLOC || code == ERR_CMDNEXEC
		|| code == ERR_PIPE || code == ERR_FORK)
		perror("minishell");
}

void	printerr_syntax(char *tokval)
{
	ft_putstr_fd("minishell: syntax error near unexpected token `", 2);
	ft_putstr_fd(tokval, 2);
	ft_putstr_fd("'\n", 2);
}

void	printerr_quotes()
{
	ft_putstr_fd("minishell: Invalid quotes\n", 2);
}

void	printerr_redir(char *filename)
{
	ft_putstr_fd("minishell: ", 2);
	ft_putstr_fd(filename, 2);
	ft_putstr_fd(": no such file or directory\n", 2);
}

void	printerr_cmdnfound(char *cmd)
{
	ft_putstr_fd("minishell: ", 2);
	ft_putstr_fd(cmd, 2);
	ft_putstr_fd(": command not found\n", 2);
}
