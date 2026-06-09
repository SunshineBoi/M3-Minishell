#include "minishell.h"

t_app	*init_app(char **envp)
{
	t_app	*app;

	app = malloc(sizeof(t_app));
	if (!app)
		return (printerr_syscall(ERR_MALLOC), NULL);
	app->envp = envp;
	app->env_list = env_init(envp);
	app->exitcode = EX_OK;
	app->tokensll = NULL;
	app->ast = NULL;
	return (app);
}

void	process_prompt(t_app *app, char *str)
{
	app->tokensll = build_tokensll(str);
	if (validate_tokensll(app) == -1)
		return (freetokensll(app->tokensll));
	
	app->ast = parse_tokens(app->tokensll);
	freetokensll(app->tokensll);
	app->tokensll = NULL;
	if (!app->ast)
		return ;

	// ! todo: standardize update envp into app
	// ! todo: standardize update exit code into app
	if (expand_ast(app->ast, app->envp, app->exitcode) != 0)
		return (ast_free(app->ast));

	execute_ast(app, app->ast);
	ast_free(app->ast);
	app->ast = NULL;
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
		signals_at_prompt();
		g_signal = 0;
		prompt = readline("minishell$ ");
		if (!prompt) // ctrl + d
		{
			ft_putstr_fd("exit\n", 1);
			break ;
		}
		if (g_signal == SIGINT)
		{
			setexit(app, EX_SIG_BASE + SIGINT);
			free(prompt);
			continue ;
		}
		if (*prompt)
			add_history(prompt);
		process_prompt(app, prompt);
		free(prompt);
	}
	env_free(app->env_list);
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
