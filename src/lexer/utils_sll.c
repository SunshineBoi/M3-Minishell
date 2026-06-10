#include "minishell.h"

/**
 * @brief Allocate and initialize a token node.
 * @return New token node, or NULL on failure.
 */
t_tokensll	*init_token()
{
	t_tokensll	*new;

	new = malloc(sizeof(t_tokensll));
	if (!new)
		return (printerr_syscall(ERR_MALLOC), NULL);
	new->next = NULL;
	new->val = NULL;
	new->val_size = BUFFER_SIZE;
	new->type = TOK_STR;
	return (new);
}

/**
 * @brief Allocate and initialize list traversal helpers.
 * @return New ops structure, or NULL on failure.
 */
t_sll_ops	*init_sll_ops()
{
	t_sll_ops	*ops;

	ops = malloc(sizeof(t_sll_ops));
	if (!ops)
		return (printerr_syscall(ERR_MALLOC), NULL);
	ops->curr = NULL;
	ops->head = NULL;
	ops->prev = NULL;
	return (ops);
}

/**
 * @brief Count nodes in a token list.
 * @param sll Head of the list.
 * @return Number of nodes.
 */
int	ft_sllsize(t_tokensll *sll)
{
	int	size;

	size = 0;
	while (sll)
	{
		size++;
		sll = sll->next;
	}
	return (size);
}

/**
 * @brief Free a single token node.
 * @param token Token node to free.
 */
void	freetoken(t_tokensll *token)
{
	if (!token)
		return ;
	free(token->val);
	free(token);
}

/**
 * @brief Free a token list and its contents.
 * @param sll Head of the list.
 */
void	freetokensll(t_tokensll *sll)
{
	t_tokensll	*next;

	if (!sll)
		return ;
	while (sll)
	{
		next = sll->next;
		free(sll->val);
		free(sll);
		sll = next;
	}
}
