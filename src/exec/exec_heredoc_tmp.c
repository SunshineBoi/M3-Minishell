#include "minishell.h"
#include "exec_heredoc.h"

static char	*make_heredoc_path(int counter)
{
	char	*pid;
	char	*idx;
	char	*prefix_pid;
	char	*prefix_pid_sep;
	char	*path;

	pid = ft_itoa((int)getpid());
	idx = ft_itoa(counter);
	if (!pid || !idx)
		return (free(pid), free(idx), NULL);
	prefix_pid = ft_strjoin("/tmp/minishell_hd_", pid);
	prefix_pid_sep = ft_strjoin(prefix_pid, "_");
	path = ft_strjoin(prefix_pid_sep, idx);
	free(pid);
	free(idx);
	free(prefix_pid);
	free(prefix_pid_sep);
	return (path);
}

int	open_heredoc_write_file(char **path)
{
	static int	counter;
	int			tries;
	int			fd;

	tries = 0;
	while (tries < 10000)
	{
		*path = make_heredoc_path(counter++);
		if (!*path)
			return (-1);
		fd = open(*path, O_CREAT | O_EXCL | O_WRONLY, 0600);
		if (fd != -1)
			return (fd);
		free(*path);
		*path = NULL;
		if (errno != EEXIST)
			return (-1);
		tries++;
	}
	return (-1);
}

int	reopen_heredoc_read_file(char *path)
{
	int	fd;

	fd = open(path, O_RDONLY);
	unlink(path);
	return (fd);
}
