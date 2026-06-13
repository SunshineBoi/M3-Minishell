#include "minishell.h"

/**
 * @brief pwd builtin — print working directory.
 */
int	builtin_pwd(void)
{
	char	cwd[PATH_MAX];

	if (getcwd(cwd, sizeof(cwd)))
	{
		ft_putstr_fd(cwd, 1);
		write(1, "\n", 1);
		return (0);
	}
	errmsg("pwd", NULL, strerror(errno));
	return (1);
}
