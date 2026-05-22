/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kong <kong@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/15 20:20:01 by kong              #+#    #+#             */
/*   Updated: 2026/05/15 20:20:01 by kong             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

// t_app	*init_app();

void	process(char *str)
{
	t_tokensll	*token_sll;

	token_sll = build_tokensll(str);
	/* for testing use */
	while (token_sll)
	{
		printf("[token val]: %s$", token_sll->val);
		printf("\n\t[token type]: %d$\n", token_sll->type);
		token_sll = token_sll->next;
	}
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
