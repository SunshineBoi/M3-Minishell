#include "minishell.h"

/**
 * @brief Validate an identifier name.
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
 * @brief unset builtin — remove environment variables.
 *
 * Supports multiple arguments. Invalid identifiers print an error
 * and set status to 1, but processing continues.
 * Unsetting a non-existent variable silently succeeds.
 */
int	builtin_unset(char **argv, t_app *app)
{
	int	i;
	int	ret;

	ret = 0;
	i = 1;
	while (argv[i])
	{
		if (!is_valid_identifier(argv[i]))
		{
			errmsg("unset", argv[i], "not a valid identifier");
			ret = 1;
		}
		else
			env_unset(&app->env_list, argv[i]);
		i++;
	}
	update_env_array(app);
	return (ret);
}
