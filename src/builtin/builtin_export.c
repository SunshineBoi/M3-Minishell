#include "minishell.h"

/**
 * @brief Validate an identifier name (first char letter/_, rest alnum/_).
 */
static int	is_valid_identifier(const char *name)
{
	int	i;

	if (!name || !*name)
		return (0);
	if (!((*name >= 'a' && *name <= 'z') || (*name >= 'A' && *name <= 'Z')
			|| *name == '_'))
		return (0);
	i = 1;
	while (name[i])
	{
		if (!((name[i] >= 'a' && name[i] <= 'z')
				|| (name[i] >= 'A' && name[i] <= 'Z')
				|| (name[i] >= '0' && name[i] <= '9')
				|| name[i] == '_'))
			return (0);
		i++;
	}
	return (1);
}

/**
 * @brief Print a single variable in `declare -x` format.
 */
static void	print_export_var(t_env *node)
{
	ft_putstr_fd("declare -x ", 1);
	ft_putstr_fd(node->key, 1);
	if (node->value)
	{
		ft_putstr_fd("=\"", 1);
		ft_putstr_fd(node->value, 1);
		ft_putstr_fd("\"", 1);
	}
	write(1, "\n", 1);
}

/**
 * @brief Print all exported variables in sorted order (insertion sort).
 */
static int	print_sorted_exports(t_app *app)
{
	t_env	*cur;
	t_env	*min;
	t_env	*prev_min;

	/* Simple O(n²) selection — acceptable for environment size */
	prev_min = NULL;
	while (1)
	{
		min = NULL;
		cur = app->env_list;
		while (cur)
		{
			if ((!prev_min || ft_strcmp(cur->key, prev_min->key) > 0)
				&& (!min || ft_strcmp(cur->key, min->key) < 0))
				min = cur;
			cur = cur->next;
		}
		if (!min)
			break ;
		print_export_var(min);
		prev_min = min;
	}
	return (0);
}

/**
 * @brief Handle a single export argument like "VAR" or "VAR=value".
 */
static int	handle_export_arg(const char *arg, t_app *app)
{
	char	*eq;
	char	*key;
	int		ret;

	eq = ft_strchr(arg, '=');
	if (eq)
	{
		key = ft_strndup((char *)arg, (int)(eq - arg));
		if (!key)
			return (1);
		if (!is_valid_identifier(key))
		{
			errmsg("export", arg, "not a valid identifier");
			free(key);
			return (1);
		}
		ret = env_set(&app->env_list, key, eq + 1);
		free(key);
		return (ret != 0);
	}
	if (!is_valid_identifier(arg))
	{
		errmsg("export", arg, "not a valid identifier");
		return (1);
	}
	/* export VAR without value — only mark as exported if not existing */
	if (!env_get(app->env_list, arg))
		env_set(&app->env_list, arg, NULL);
	return (0);
}

/**
 * @brief export builtin.
 *
 * No args → print all exported variables (sorted, declare -x format).
 * With args → set/export each variable.
 */
int	builtin_export(char **argv, t_app *app)
{
	int	i;
	int	ret;

	if (!argv[1])
		return (print_sorted_exports(app));
	ret = 0;
	i = 1;
	while (argv[i])
	{
		if (handle_export_arg(argv[i], app) != 0)
			ret = 1;
		i++;
	}
	return (ret);
}
