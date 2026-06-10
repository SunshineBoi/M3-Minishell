#include "minishell.h"

void	free_app(t_app *app)
{
	if (app->tokensll)
		freetokensll(app->tokensll);
	if (app->ast)
		ast_free(app->ast);
	if (app->envp)
		freelst(app->envp);
	if (app->env_list)
		env_free(app->env_list);
	free(app);
}

t_app	*init_app(char **envp)
{
	t_app	*app;

	app = malloc(sizeof(t_app));
	if (!app)
		return (printerr_syscall(ERR_MALLOC), NULL);
	app->env_list = env_init(envp);
	if (!app->env_list)
		return (free(app), NULL); // ! should i quit if no envp path? can we guarantee to have envp at the start?
	app->envp = env_to_array(app->env_list);
	if (!app->envp)
		return (env_free(app->env_list), free(app), NULL);
	app->tokensll = NULL;
	app->ast = NULL;
	app->exitcode = EX_OK;
	return (app);
}

void	process_prompt(t_app *app, char *str)
{
	app->tokensll = build_tokensll(str);
	if (!app->tokensll)
		return ;
	if (validate_tokensll(app) == -1)
		return (freetokensll(app->tokensll));
	
	app->ast = parse_tokens(app->tokensll);
	freetokensll(app->tokensll);
	app->tokensll = NULL;
	if (!app->ast)
		return ;
	if (expand_ast(app->ast, app->envp, app->exitcode) != 0)
	{
		ast_free(app->ast);
		app->ast = NULL;
		return ;
	}
	execute_ast(app, app->ast);
	ast_free(app->ast);
	app->ast = NULL;
}

int	minishell(char **envp)
{
	t_app	*app;
	char	*prompt;
	int		exitcode;

	app = init_app(envp);
	if (!app)
		hardexit();
	while (1)
	{
		signals_at_prompt();
		g_signal = 0;
		prompt = readline("minishell$ ");
		if (g_signal == SIGINT)
		{
			setexit(app, EX_SIG_BASE + SIGINT);
			if (prompt)
				free(prompt);
			continue ;
		}
		if (!prompt) // ctrl + d
		{
			ft_putstr_fd("exit\n", 1);
			break ;
		}
		if (*prompt)
			add_history(prompt);
		process_prompt(app, prompt);
		free(prompt);
	}
	exitcode = app->exitcode;
	free_app(app);
	return (exitcode);
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
