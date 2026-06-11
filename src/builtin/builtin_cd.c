#include "minishell.h"

/**
 * @brief Update PWD and OLDPWD environment variables after chdir.
 */
static int	update_pwd(t_app *app, const char *old_pwd)
{
	char	cwd[PATH_MAX];

	if (old_pwd)
	{
		if (env_set(&app->env_list, "OLDPWD", old_pwd) == ERR_MALLOC)
			return (EX_ERR);
	}
	if (getcwd(cwd, sizeof(cwd)))
	{
		if (env_set(&app->env_list, "PWD", cwd) == ERR_MALLOC) // reset with current directory
			return (EX_ERR);
	}
	return (EX_OK);
}

static char	*resolve_target(char **argv, t_app *app)
{
	char	*target;

	if (!argv[1])
	{
		target = env_get(app->env_list, "HOME");
		if (!target)
			return (errmsg("cd", NULL, "HOME not set"), NULL);
		return (target);
	}
	if (ft_strcmp(argv[1], "-") == 0)
	{
		target = env_get(app->env_list, "OLDPWD");
		if (!target)
			return (errmsg("cd", NULL, "OLDPWD not set"), NULL);
		printf("%s\n", target);
		return (target);
	}
	return (argv[1]);
}

/**
 * @brief cd builtin — change directory.
 *
 * - Too many args
 * - No args → cd to $HOME
 * - "-" → cd to $OLDPWD, print new path
 * - Otherwise → cd to given path
 * 
 */
int	builtin_cd(char **argv, t_app *app)
{
	char	*target;
	char	*old_pwd;
	char	cwd[PATH_MAX];

	if (argv[1] && argv[2])
		return (errmsg("cd", NULL, "too many arguments"), EX_ERR);
	old_pwd = NULL;
	if (getcwd(cwd, sizeof(cwd)))
		old_pwd = cwd;
	target = resolve_target(argv, app);
	if (!target)
		return (EX_ERR);
	if (chdir(target) != 0)
	{
		errmsg("cd", target, strerror(errno));
		return (EX_ERR);
	}
	if (update_pwd(app, old_pwd) != EX_OK)
		return (EX_ERR);
	return (EX_OK);
}
