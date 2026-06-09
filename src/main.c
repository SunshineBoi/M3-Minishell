#include "minishell.h"

t_app	*init_app(char **envp)
{
	t_app	*app;

	app = malloc(sizeof(t_app));
	if (!app)
		return (printerr_syscall(ERR_MALLOC), NULL);
	app->envp = envp;
	app->exitcode = EX_OK;
	app->tokensll = NULL;
	return (app);
}

void	process_prompt(t_app *app, char *str)
{
	app->tokensll = build_tokensll(str);
	print_tokensll(app->tokensll); // ! for testing use, delete later
	app->exitcode = EX_OK; // ! for time being to validate tokensll, remove it later
	validate_tokensll(app);
}

int	minishell(char **envp)
{
	t_app	*app;
	char	*prompt;

	app = init_app(envp);
	if (!app)
		hardexit();
	while (1)
	{
		prompt = readline("minishell$ ");
		if (!prompt)
			// ! how to free env copy, history, anything else?
			// ! to print correct msg - "logout"
			exit(EXIT_SUCCESS); // ctrl+d 
		if (*prompt)
			add_history(prompt);
		process_prompt(app, prompt);
		free(prompt);
	}
	free(app);
	return (EXIT_SUCCESS);
}

int	main(int ac, char **av, char **envp)
{
	(void)av;
	if (ac != 1)
	{
		ft_putstr_fd("minishell: invalid number of arguments\n", 2);
		return (EXIT_FAILURE);
	}
	return (minishell(envp));
}
