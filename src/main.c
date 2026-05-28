

#include "minishell.h"

// t_app	*init_app();

void	process(char *str)
{
	t_tokensll	*token_sll;

	token_sll = build_tokensll(str);
	print_tokensll(token_sll); // for testing use
	validate_tokensll(token_sll);
}

int	minishell()
{
	char	*input;

	// ! later, think of where to init t_app
	while (1)
	{
		input = readline("minishell$ ");
		if (!input)
			// ! how to free env copy, history, anything else?
			// ! to print correct msg
			exit(EXIT_SUCCESS); // ctrl+d 
		if (*input)
			add_history(input);
		process(input);
		free(input);
	}
	return (EXIT_SUCCESS);
}

int	main(void)
{
	return (minishell());
}
