#include "minishell.h"

int	main(int ac, char **av, char **envp)
{
	int		exitcode;
	t_app	*app;

	(void)av;
	if (ac != 1)
	{
		ft_putstr_fd("minishell: invalid number of arguments\n", 2);
		return (EXIT_FAILURE);
	}
	app = init_app(envp);
	if (!app)
		hardexit();
	exitcode = minishell(app);
	free_app(app);
	return (exitcode);
}
