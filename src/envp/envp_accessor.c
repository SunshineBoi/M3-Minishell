/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   envp_accessor.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kong <kong@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/04 12:51:52 by kong              #+#    #+#             */
/*   Updated: 2026/07/04 13:11:21 by kong             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * @brief Look up a variable by key.
 * @param list Head of the environment list.
 * @param key Variable name to find.
 * @return The value string (may be NULL for export-only), or NULL if not found.
 */
char	*env_get(t_env *list, const char *key)
{
	if (!key)
		return (NULL);
	while (list)
	{
		if (ft_strcmp(list->key, key) == 0)
			return (list->value);
		list = list->next;
	}
	return (NULL);
}

/**
 * @brief Overwrite an existing node's value in place.
 * @param node Node to update.
 * @param value New value (NULL is a no-op, keeps the current value).
 * @return 0 on success, ERR_MALLOC on failure.
 */
static int	env_update_value(t_env *node, const char *value)
{
	char	*new_val;

	if (!value)
		return (0);
	new_val = ft_strdup(value);
	if (!new_val)
		return (ERR_MALLOC);
	free(node->value);
	node->value = new_val;
	return (0);
}

/**
 * @brief Set or create an environment variable.
 * @param list Pointer to head pointer (may insert at head).
 * @param key Variable name.
 * @param value Variable value (NULL to mark export-only).
 * @return 0 on success, ERR_MALLOC on failure.
 */
int	env_set(t_env **list, const char *key, const char *value)
{
	t_env	*cur;
	t_env	*node;

	if (!key || !*key)
		return (0);
	cur = *list;
	while (cur)
	{
		if (ft_strcmp(cur->key, key) == 0)
			return (env_update_value(cur, value));
		cur = cur->next;
	}
	node = env_new_node(key, value);
	if (!node)
		return (ERR_MALLOC);
	node->next = *list;
	*list = node;
	return (0);
}

/**
 * @brief Remove a variable by key.
 * @param list Pointer to head pointer.
 * @param key Variable name to remove.
 * @return 0 always (silent success even if not found).
 */
int	env_unset(t_env **list, const char *key)
{
	t_env	*cur;
	t_env	*prev;

	if (!key || !*key)
		return (0);
	prev = NULL;
	cur = *list;
	while (cur)
	{
		if (ft_strcmp(cur->key, key) == 0)
		{
			if (prev)
				prev->next = cur->next;
			else
				*list = cur->next;
			free(cur->key);
			free(cur->value);
			free(cur);
			return (0);
		}
		prev = cur;
		cur = cur->next;
	}
	return (0);
}
