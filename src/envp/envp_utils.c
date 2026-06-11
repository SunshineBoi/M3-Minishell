#include "minishell.h"

size_t	env_count(t_env *list)
{
	size_t	n;

	n = 0;
	while (list)
	{
		if (list->value)
			n++;
		list = list->next;
	}
	return (n);
}

void	env_free(t_env *list)
{
	t_env	*next;

	while (list)
	{
		next = list->next;
		free(list->key);
		free(list->value);
		free(list);
		list = next;
	}
}

/**
 * @brief Create a new environment node.
 * @param key Variable name (duplicated).
 * @param value Variable value (duplicated, may be NULL).
 * @return New node, or NULL on malloc failure.
 */
t_env	*env_new_node(const char *key, const char *value)
{
	t_env	*node;
	
	if (!key || !*key)
		return (NULL);
	node = malloc(sizeof(t_env));
	if (!node)
		return (perror(APP), NULL);
	node->key = ft_strdup(key);
	if (!node->key)
		return (free(node), printerr_syscall(ERR_MALLOC), NULL);
	if (value)
	{
		node->value = ft_strdup(value);
		if (!node->value)
			return (free(node->key), free(node), printerr_syscall(ERR_MALLOC), NULL);
	}
	else
		node->value = NULL;
	node->next = NULL;
	return (node);
}

static int	env_append(t_env **head, t_env **tail, char *entry)
{
	t_env	*node;
	char	*eq;

	eq = ft_strchr(entry, '=');
	if (!eq)
		return (0);
	*eq = '\0';
	node = env_new_node(entry, eq + 1);
	*eq = '=';
	if (!node)
		return (-1);
	if (!*head)
		*head = node;
	else
		(*tail)->next = node;
	*tail = node;
	return (0);
}

t_env	*env_init(char **envp)
{
	t_env	*head;
	t_env	*tail;
	size_t	i;

	head = NULL;
	tail = NULL;
	i = 0;
	while (envp && envp[i])
	{
		if (env_append(&head, &tail, envp[i]) == -1)
			return (env_free(head), NULL);
		i++;
	}
	return (head);
}
