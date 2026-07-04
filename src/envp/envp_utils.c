/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   envp_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kong <kong@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/04 12:33:29 by kong              #+#    #+#             */
/*   Updated: 2026/07/04 13:11:39 by kong             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * @brief Free the entire environment list.
 * @param list Head of the list.
 */
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
 * @param value Variable value (duplicated, may be NULL - so EXPORT sees it).
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
			return (free(node->key), free(node),
				printerr_syscall(ERR_MALLOC), NULL);
	}
	else
		node->value = NULL;
	node->next = NULL;
	return (node);
}

/**
 * @brief Build a node from a "KEY=VALUE" entry, given the '=' position.
 * @param entry Raw envp entry.
 * @param eq Pointer to the '=' within entry.
 * @return New node, or NULL on malloc failure.
 */
static t_env	*env_parse_entry(char *entry, char *val)
{
	t_env	*node;
	char	*key;

	key = ft_strndup(entry, (int)(val - entry));
	if (!key)
		return (NULL);
	node = env_new_node(key, val + 1);
	free(key);
	return (node);
}

/**
 * @brief Append a node to the list, tracking head and tail.
 * @param head Pointer to head pointer (set if list was empty).
 * @param tail Pointer to tail pointer (updated to the new node).
 * @param node Node to append.
 */
static void	update_node(t_env **head, t_env **tail, t_env *node)
{
	if (!*head)
		*head = node;
	else
		(*tail)->next = node;
	*tail = node;
}

/**
 * @brief Build the environment linked list from the envp array.
 * @param envp NULL-terminated array of "KEY=VALUE" strings.
 * @return Head of the new list, or NULL on failure.
 */
t_env	*env_init(char **envp)
{
	t_env	*head;
	t_env	*tail;
	t_env	*node;
	char	*val;
	size_t	i;

	head = NULL;
	tail = NULL;
	i = 0;
	while (envp && envp[i])
	{
		val = ft_strchr(envp[i], '=');
		if (val)
		{
			node = env_parse_entry(envp[i], val);
			if (!node)
				return (env_free(head), NULL);
			update_node(&head, &tail, node);
		}
		i++;
	}
	return (head);
}
