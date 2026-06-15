#include "minishell.h"

/**
 * @brief Update PWD and OLDPWD environment variables after chdir.
 */
static int	update_pwd(t_app *app, const char *old_pwd)
{
	char	cwd[PATH_MAX];

	if (old_pwd)
		env_set(&app->env_list, "OLDPWD", old_pwd);
	if (!getcwd(cwd, sizeof(cwd)))
		return (errmsg("cd", NULL, strerror(errno)), EX_ERR);
	env_set(&app->env_list, "PWD", cwd);
	return (EX_OK);
}

/**
 * @brief Resolve the target directory from argv.
 */
static char	*resolve_target(char **argv, t_app *app)
{
	char	*target;

	if (!argv[1])
	{
		target = env_get(app->env_list, "HOME");
		if (!target)
			errmsg("cd", NULL, "HOME not set");
		return (target);
	}
	if (ft_strcmp(argv[1], "-") == 0)
	{
		target = env_get(app->env_list, "OLDPWD");
		if (!target)
			errmsg("cd", NULL, "OLDPWD not set");
		return (target);
	}
	return (argv[1]);
}

/**
 * @brief cd builtin — change directory.
 *
 * - No args → cd to $HOME
 * - "-" → cd to $OLDPWD, print new path
 * - Otherwise → cd to given path
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
		return (errmsg("cd", target, strerror(errno)), EX_ERR);
	if (argv[1] && ft_strcmp(argv[1], "-") == 0)
	{
		ft_putstr_fd(target, 1);
		write(1, "\n", 1);
	}
	return (update_pwd(app, old_pwd));
}
