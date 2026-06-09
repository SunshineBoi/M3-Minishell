#include "minishell.h"

void	printerr_cd(char *filename)
{
	ft_putstr_fd("minishell: cd: ", 2);
	ft_putstr_fd(filename, 2);
	ft_putstr_fd(": no such file or directory\n", 2);
}
