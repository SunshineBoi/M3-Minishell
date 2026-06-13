#include <criterion/criterion.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include "minishell.h"

static t_redir *redir_new(t_redir_type type, const char *target)
{
	t_redir *redir;

	redir = malloc(sizeof(t_redir));
	cr_assert_not_null(redir);
	redir->type = type;
	redir->target = target ? strdup(target) : NULL;
	redir->fd = -1;
	redir->next = NULL;
	return (redir);
}

static void redir_free(t_redir *redir)
{
	t_redir *next;

	while (redir)
	{
		next = redir->next;
		free(redir->target);
		free(redir);
		redir = next;
	}
}

static void restore_fd(int saved, int target)
{
	dup2(saved, target);
	close(saved);
}

Test(redirections, output_redirection_writes_file)
{
	char	path[] = "/tmp/minishell_redir_outXXXXXX";
	int	fd;
	int	saved;
	char	buf[16] = {0};
	t_redir	*redir;

	fd = mkstemp(path);
	cr_assert(fd >= 0);
	close(fd);

	saved = dup(STDOUT_FILENO);
	redir = redir_new(REDIR_OUT, path);
	cr_assert_eq(apply_redirections(redir), 0);
	write(STDOUT_FILENO, "hi", 2);
	restore_fd(saved, STDOUT_FILENO);
	redir_free(redir);

	fd = open(path, O_RDONLY);
	cr_assert(fd >= 0);
	read(fd, buf, sizeof(buf) - 1);
	close(fd);
	cr_assert_str_eq(buf, "hi");
	unlink(path);
}

Test(redirections, input_redirection_reads_file)
{
	char	path[] = "/tmp/minishell_redir_inXXXXXX";
	int	fd;
	int	saved;
	char	buf[16] = {0};
	t_redir	*redir;

	fd = mkstemp(path);
	cr_assert(fd >= 0);
	write(fd, "data", 4);
	close(fd);

	saved = dup(STDIN_FILENO);
	redir = redir_new(REDIR_IN, path);
	cr_assert_eq(apply_redirections(redir), 0);
	read(STDIN_FILENO, buf, sizeof(buf) - 1);
	restore_fd(saved, STDIN_FILENO);
	redir_free(redir);

	cr_assert_str_eq(buf, "data");
	unlink(path);
}

Test(redirections, heredoc_fd_is_used)
{
	int	fds[2];
	int	saved;
	char	buf[16] = {0};
	t_redir	*redir;

	cr_assert_eq(pipe(fds), 0);
	write(fds[1], "doc", 3);
	close(fds[1]);

	saved = dup(STDIN_FILENO);
	redir = redir_new(REDIR_HEREDOC, NULL);
	redir->fd = fds[0];
	cr_assert_eq(apply_redirections(redir), 0);
	read(STDIN_FILENO, buf, sizeof(buf) - 1);
	restore_fd(saved, STDIN_FILENO);
	redir_free(redir);

	cr_assert_str_eq(buf, "doc");
}

Test(redirections, missing_input_returns_error)
{
	t_redir	*redir;

	redir = redir_new(REDIR_IN, "/tmp/does_not_exist_minishell");
	cr_assert_eq(apply_redirections(redir), 1);
	redir_free(redir);
}

Test(redirections, multiple_output_redirections_last_wins)
{
	char	path1[] = "/tmp/minishell_redir1XXXXXX";
	char	path2[] = "/tmp/minishell_redir2XXXXXX";
	int	fd1;
	int	fd2;
	int	saved;
	char	buf[16] = {0};
	t_redir	*r1;
	t_redir	*r2;

	fd1 = mkstemp(path1);
	fd2 = mkstemp(path2);
	cr_assert(fd1 >= 0 && fd2 >= 0);
	close(fd1);
	close(fd2);

	saved = dup(STDOUT_FILENO);
	r1 = redir_new(REDIR_OUT, path1);
	r2 = redir_new(REDIR_OUT, path2);
	r1->next = r2;
	cr_assert_eq(apply_redirections(r1), 0);
	write(STDOUT_FILENO, "final", 5);
	restore_fd(saved, STDOUT_FILENO);
	redir_free(r1);

	fd2 = open(path2, O_RDONLY);
	cr_assert(fd2 >= 0);
	read(fd2, buf, sizeof(buf) - 1);
	close(fd2);
	cr_assert_str_eq(buf, "final");

	fd1 = open(path1, O_RDONLY);
	cr_assert(fd1 >= 0);
	close(fd1);
	unlink(path1);
	unlink(path2);
}
