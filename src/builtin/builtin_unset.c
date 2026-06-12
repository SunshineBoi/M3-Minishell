#include "minishell.h"

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
	return (ret);
}
