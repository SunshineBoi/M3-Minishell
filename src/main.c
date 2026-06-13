#include "minishell.h"

t_app	*init_app(char **envp)
{
	t_app	*app;

	app = malloc(sizeof(t_app));
	if (!app)
		return (printerr_syscall(ERR_MALLOC), NULL);
	app->env_list = env_init(envp);
	app->envp = env_to_array(app->env_list);
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
	if (expand_ast(app, app->ast, app->exitcode) != 0)
		return (ast_free(app->ast));
	execute_ast(app, app->ast);
	ast_free(app->ast);
	app->ast = NULL;
}

static char	*get_next_line_non_interactive(void)
{
	char	*line;
	size_t	len;
	ssize_t	read;

	line = NULL;
	len = 0;
	read = getline(&line, &len, stdin);
	if (read == -1)
	{
		free(line);
		return (NULL);
	}
	if (read > 0 && line[read - 1] == '\n')
		line[read - 1] = '\0';
	return (line);
}

int	minishell(char **envp)
{
	t_app	*app;
	char	*prompt;
	char	*prompt_str;
	int		exit_code;

	app = init_app(envp);
	if (!app)
		hardexit();
	prompt_str = "minishell$ ";
	while (1)
	{
		signals_at_prompt();
		g_signal = 0;
		if (isatty(STDIN_FILENO))
			prompt = readline(prompt_str);
		else
			prompt = get_next_line_non_interactive();
		if (!prompt) // ctrl + d
		{
			if (isatty(STDIN_FILENO))
				ft_putstr_fd("exit\n", 1);
			break ;
		}
		if (g_signal == SIGINT)
		{
			setexit(app, EX_SIG_BASE + SIGINT);
			g_signal = 0;
		}
		if (*prompt)
			add_history(prompt);
		process_prompt(app, prompt);
		free(prompt);
	}
	exit_code = app->exitcode;
	env_free(app->env_list);
	if (app->envp)
		freelst(app->envp);
	free(app);
	return (exit_code);
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
