#include <criterion/criterion.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "minishell.h"

static t_env *make_env_with_path(const char *path)
{
	t_env *list;

	list = NULL;
	if (path)
		env_set(&list, "PATH", path);
	return (list);
}

static char	*resolve_command_path(const char *cmd, t_env *env_list)
{
	t_app	app;
	char	*argv[2];
	char	*resolved;

	app.env_list = env_list;
	app.envp = env_to_array(env_list);
	app.exitcode = 0;
	argv[0] = (char *)cmd;
	argv[1] = NULL;
	resolved = resolvecmdpath(&app, argv);
	freelst(app.envp);
	return (resolved);
}


static int make_executable(const char *dir, const char *name, char *out,
		size_t out_len)
{
	int	fd;

	snprintf(out, out_len, "%s/%s", dir, name);
	fd = open(out, O_CREAT | O_WRONLY | O_TRUNC, 0700);
	if (fd < 0)
		return (-1);
	close(fd);
	return (0);
}

Test(exec_path, command_with_slash_is_returned_as_is)
{
	t_env *list;
	char *resolved;

	list = make_env_with_path("/bin");
	resolved = resolve_command_path("/bin/ls", list);
	cr_assert_not_null(resolved);
	cr_assert_str_eq(resolved, "/bin/ls");
	free(resolved);
	env_free(list);
}

Test(exec_path, missing_or_empty_path_returns_null)
{
	t_env *list;
	char *resolved;

	list = NULL;
	resolved = resolve_command_path("ls", list);
	cr_assert_null(resolved);
	env_free(list);

	list = make_env_with_path("");
	resolved = resolve_command_path("ls", list);
	cr_assert_null(resolved);
	env_free(list);
}

Test(exec_path, resolves_executable_in_path)
{
	char	cwd[PATH_MAX];
	char	tmpdir[] = "/tmp/minishell_pathXXXXXX";
	char	path[PATH_MAX];
	char	full[PATH_MAX];
	t_env	*list;
	char	*resolved;

	cr_assert_not_null(getcwd(cwd, sizeof(cwd)));
	cr_assert_not_null(mkdtemp(tmpdir));
	cr_assert_eq(make_executable(tmpdir, "mycmd", full, sizeof(full)), 0);
	snprintf(path, sizeof(path), "%s", tmpdir);
	list = make_env_with_path(path);
	resolved = resolve_command_path("mycmd", list);
	cr_assert_not_null(resolved);
	cr_assert_str_eq(resolved, full);
	free(resolved);
	env_free(list);
	chdir(cwd);
	unlink(full);
	rmdir(tmpdir);
}

Test(exec_path, empty_path_entry_uses_cwd)
{
	char	cwd[PATH_MAX];
	char	tmpdir[] = "/tmp/minishell_pathXXXXXX";
	char	full[PATH_MAX];
	t_env	*list;
	char	*resolved;

	cr_assert_not_null(getcwd(cwd, sizeof(cwd)));
	cr_assert_not_null(mkdtemp(tmpdir));
	cr_assert_eq(chdir(tmpdir), 0);
	cr_assert_eq(make_executable(".", "herecmd", full, sizeof(full)), 0);
	list = make_env_with_path(":/bin");
	resolved = resolve_command_path("herecmd", list);
	cr_assert_not_null(resolved);
	cr_assert_str_eq(resolved, "./herecmd");
	free(resolved);
	env_free(list);
	chdir(cwd);
	unlink(full);
	rmdir(tmpdir);
}

Test(exec_path, trailing_empty_path_entry_uses_cwd)
{
	char	cwd[PATH_MAX];
	char	tmpdir[] = "/tmp/minishell_pathXXXXXX";
	char	full[PATH_MAX];
	t_env	*list;
	char	*resolved;

	cr_assert_not_null(getcwd(cwd, sizeof(cwd)));
	cr_assert_not_null(mkdtemp(tmpdir));
	cr_assert_eq(chdir(tmpdir), 0);
	cr_assert_eq(make_executable(".", "herecmd", full, sizeof(full)), 0);
	list = make_env_with_path("/bin:");
	resolved = resolve_command_path("herecmd", list);
	cr_assert_not_null(resolved);
	cr_assert_str_eq(resolved, "./herecmd");
	free(resolved);
	env_free(list);
	chdir(cwd);
	unlink(full);
	rmdir(tmpdir);
}

Test(exec_path, entirely_empty_path_uses_cwd)
{
	char	cwd[PATH_MAX];
	char	tmpdir[] = "/tmp/minishell_pathXXXXXX";
	char	full[PATH_MAX];
	t_env	*list;
	char	*resolved;

	cr_assert_not_null(getcwd(cwd, sizeof(cwd)));
	cr_assert_not_null(mkdtemp(tmpdir));
	cr_assert_eq(chdir(tmpdir), 0);
	cr_assert_eq(make_executable(".", "herecmd", full, sizeof(full)), 0);
	list = make_env_with_path("");
	resolved = resolve_command_path("herecmd", list);
	cr_assert_not_null(resolved);
	cr_assert_str_eq(resolved, "./herecmd");
	free(resolved);
	env_free(list);
	chdir(cwd);
	unlink(full);
	rmdir(tmpdir);
}
