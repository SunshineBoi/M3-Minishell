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
static t_env	*env_new_node(const char *key, const char *value)
{
	t_env	*node;

	node = malloc(sizeof(t_env));
	if (!node)
		return (NULL);
	node->key = ft_strdup(key);
	if (!node->key)
		return (free(node), NULL);
	if (value)
	{
		node->value = ft_strdup(value);
		if (!node->value)
			return (free(node->key), free(node), NULL);
	}
	else
		node->value = NULL;
	node->next = NULL;
	return (node);
}

static int	_env_append(t_env **head, t_env **tail, char *entry)
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
		if (_env_append(&head, &tail, envp[i]) == -1)
			return (env_free(head), perror(APP), NULL);
		i++;
	}
	return (head);
}
