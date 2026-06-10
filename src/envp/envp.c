#include "minishell.h"

char	*env_get(t_env *list, const char *key)
{
	while (list)
	{
		if (ft_strcmp(list->key, key) == 0)
			return (list->value);
		list = list->next;
	}
	return (NULL);
}

char	**env_to_array(t_env *list)
{
	char	**arr;
	size_t	i;
	char	*tmp;

	arr = malloc(sizeof(char *) * (env_count(list) + 1));
	if (!arr)
		return (perror(APP), NULL);
	i = 0;
	while (list)
	{
		if (list->value)
		{
			tmp = ft_strjoin(list->key, "=");
			if (!tmp)
				return (freelst(arr), perror(APP), NULL);
			arr[i] = ft_strjoin(tmp, list->value);
			free(tmp);
			if (!arr[i])
				return (freelst(arr), perror(APP), NULL);
			i++;
		}
		list = list->next;
	}
	arr[i] = NULL;
	return (arr);
}

static int	match_and_set(t_env *env_node, const char *key, const char *value)
{
	char	*new_val;

	while (env_node)
	{
		if (ft_strcmp(env_node->key, key) == 0)
		{
			if (value)
			{
				new_val = ft_strdup(value);
				if (!new_val)
					return (ERR_MALLOC);
				free(env_node->value);
				env_node->value = new_val;
			}
			return (1);
		}
		env_node = env_node->next;
	}
	return (0);
}

int	env_set(t_env **head, const char *key, const char *value)
{
	t_env	*cur;
	int		found;

	cur = *head;
	found = match_and_set(cur, key, value);
	if (found == ERR_MALLOC)
		return (ERR_MALLOC);
	else if (found == 1)
		return (0);
	cur = env_new_node(key, value);
	if (!cur)
		return (ERR_MALLOC);
	cur->next = *head;
	*head = cur;
	return (0);
}

int	env_unset(t_env **list, const char *key)
{
	t_env	*cur;
	t_env	*prev;

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

