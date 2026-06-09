#include "minishell.h"

volatile sig_atomic_t g_signal = 0;

void	handlesig_prompt(int sig)
{
	g_signal = sig;
	write(1, "\n", 1);
	rl_replace_line("", 0);
	rl_on_new_line();
	rl_redisplay();
}

void	signals_at_prompt(void)
{
	struct	sigaction sa;

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = handlesig_prompt;
	sigaction(SIGINT, &sa, NULL);

	// ctrl + `\`
	sa.sa_handler = SIG_IGN;
	sigaction(SIGQUIT, &sa, NULL);
}

void signals_ignore(void)
{
	struct sigaction	sa;

	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGQUIT, &sa, NULL);
}

void	signals_default(void)
{
	struct sigaction	sa;

	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGQUIT, &sa, NULL);
	sigaction(SIGPIPE, &sa, NULL);

}

void	setexecerrno(t_app *app)
{
	if (errno == EACCES || errno == EISDIR || errno == ENOEXEC)
		setexit(app, EX_CMD_NEXEC);
	else
		setexit(app, EX_CMD_NOTFOUND);  // ENOENT / ENOTDIR
}

int	count_cmds(t_ast_node *root)
{
	int	count;

	count = 0;
	while (root)
	{
		count++;
		if (root->type == NODE_BINOP)
			root = root->content.binop.left;
		else
			break ;
	}
	return (count);
}

int	ft_strhaschr(char *str, char ch)
{
	int	i;

	i = 0;
	while (str[i])
	{
		if (str[i] == ch)
			return (1);
		i++;
	}
	return (0);
}

void	freelst(char **lst)
{
	int	i;

	if (lst)
	{
		i = 0;
		while (lst[i])
		{
			free(lst[i]);
			i++;
		}
		free(lst);
	}
}

char	**ft_calloclst(int size)
{
	char	**lst;
	int		i;
	int		total;

	total = size + 1;
	lst = malloc(sizeof(char *) * total);
	if (!lst)
		return (NULL);
	i = 0;
	while (i < total)
	{
		lst[i] = NULL;
		i++;
	}
	return (lst);
}

int	countbydelim(char *str, char delim)
{
	int	count;

	if (!str)
		return (0);
	count = 1;
	while (*str)
	{
		while (*str && *str != delim)
			str++;
		if (*str == delim)
		{
			str++;
			count++;
		}
	}
	return (count);
}

int	_splithelper(char **target, char *str, char delim)
{
	int	lenword;
	int	n_delim;

	lenword = 0;
	n_delim = 0;
	if (str[lenword])
	{
		while (str[lenword] && str[lenword] != delim)
			lenword++;
		if (str[lenword] == delim)
			n_delim += 1;
		*target = ft_strndup(str, lenword);
		if (!*target)
			return (-1);
	}
	return (lenword + n_delim);
}

char	**ft_splitbydelim(t_app *app, char *str, char delim)
{
	char	**lst;
	int		size;
	int		i;

	size = countbydelim(str, delim);
	if (size == 0)
		return (NULL);
	lst = ft_calloclst(size);
	if (!lst)
		return (setexit(app, EX_ERR), perror(APP), NULL);
	i = 0;
	while (*str)
	{
		size = _splithelper(&(lst[i]), str, delim);
		if (size == -1)
			return (setexit(app, EX_ERR), perror(APP), freelst(lst), NULL);
		i++;
		str += size;
	}
	return (lst);
}

int	ft_strncmp(const char *s1, const char *s2, size_t nbyte)
{
	while (nbyte > 0 && *s1 && *s1 == *s2)
	{
		s1++;
		s2++;
		nbyte--;
	}
	if (nbyte == 0)
		return (0);
	return ((unsigned char)*s1 - (unsigned char)*s2);
}

char	**getrawpathlst(t_app *app, char *match)
{
	int		i;
	char	**rawpath;
	char	**envp;

	// need to extract just PATH= variables
	envp = app->envp;
	if (!envp)
		return (NULL);
	i = 0;
	rawpath = NULL;
	while (envp[i])
	{
		if (ft_strncmp(envp[i], match, ft_strlen(match)) == 0)
		{
			rawpath = ft_splitbydelim(app, envp[i] + ft_strlen(match), ':');
			if (!rawpath)
				return (NULL);
			break ;
		}
		i++;
	}
	return (rawpath);
}

char	*ft_strjoin(char *sa, char *sb)
{
	char	*new;
	int		sa_size;
	int		sb_size;
	int		i;

	sa_size = ft_strlen(sa);
	sb_size = ft_strlen(sb);
	new = malloc((sa_size + sb_size + 1));
	if (!new)
		return (NULL);
	i = 0;
	while (*sa)
		new[i++] = *sa++;
	while (*sb)
		new[i++] = *sb++;
	new[i] = '\0';
	return (new);
}

char	*buildgoodpath(t_app *app, char *envp, char *cmd)
{
	char	*temp;
	char	*path;

	temp = ft_strjoin(envp, "/");
	if (!temp)
		return (setexit(app, EX_ERR), perror(APP), NULL);
	path = ft_strjoin(temp, cmd);
	free(temp);
	if (!path)
		return (setexit(app, EX_ERR), perror(APP), NULL);
	// check if any issue with given path
	if (access(path, X_OK) != 0)
		return (setexecerrno(app), free(path), NULL);
	return (path);
}

/*
* only info that gets returned from child to parent:
exit status    → via waitpid
signals        → kernel delivers them
written files  → kernel persists them
pipe data      → kernel buffers it

* a distinction to make:
if command is passed as `cat` without path, it returns:
- 126: (errno) permission denied
- 127: command not found

* However, when passing '/cat' directly to execve (syscall), it returns:
- 126: (errno) permission denied
- 127: (errno) No such file or directory
*/

char	*_cmdwithpath(t_app *app, char *cmd)
{
	char	*goodpath;

	goodpath = NULL;
	if (access(cmd, X_OK) != 0)
		return (setexecerrno(app), ft_perror(cmd), NULL);
	goodpath = ft_strndup(cmd, ft_strlen(cmd));
	if (!goodpath)
		return (setexit(app, EX_ERR), perror(APP), NULL);
	return (goodpath);
}

char	*_matchcmdpath(t_app *app, char **pathlst, char *cmd)
{
	int		i;
	char	*goodpath;

	goodpath = NULL;
	i = 0;
	while (pathlst[i])
	{
		if (!pathlst[i][0])  // empty string, test with relative path
			goodpath = buildgoodpath(app, ".", cmd);
		else
			// each path[i] is raw path like 'bin'
			goodpath = buildgoodpath(app, pathlst[i], cmd);
		if (goodpath || app->exitcode == EACCES) // immediate exit on no permission errno
			break ;
		i++;
	}
	if (!goodpath)
	{
		if (app->exitcode == EX_CMD_NOTFOUND)
			return (printerr_cmdnfound(cmd), NULL);
		return (ft_perror(cmd), NULL);
	}
	return (goodpath);
}

char	*resolvecmdpath(t_app *app, char **argv)
{
	char 	*cmd;
	char	**pathlst;
	char	*goodpath;

	goodpath = NULL;
	cmd = argv[0];
	if (ft_strhaschr(cmd, '/'))
	{	// option 1: contains / path
		goodpath = _cmdwithpath(app, cmd);
		if (!goodpath)
			return (NULL);
		return (goodpath);
	}
	pathlst = getrawpathlst(app, "PATH="); // option 2: no path and no env
	if (!pathlst)
		return (setexit(app, EX_CMD_NOTFOUND), printerr_cmdnfound(cmd), NULL);
	goodpath = _matchcmdpath(app, pathlst, cmd); // option 3: test all paths
	if (!goodpath)
		return (freelst(pathlst), NULL);
	freelst(pathlst);
	setexit(app, EX_OK); // reset exit code if found a valid path
	return (goodpath);
}

int	open_redirs(t_app *app, t_cmd_node *node)
{
	t_redir	*redir;
	int		flag;

	redir = node->redirs;
	flag = 0;
	while (redir)
	{
		if (redir->type == REDIR_IN)
			flag = O_RDONLY;
		else if (redir->type == REDIR_OUT)
			flag = O_WRONLY | O_CREAT | O_TRUNC;
		else if (redir->type == REDIR_APPEND)
			flag = O_WRONLY | O_CREAT | O_APPEND;
		else if (redir->type == REDIR_HEREDOC)
		{
			redir = redir->next;
			continue ;
		}
		redir->fd = open(redir->target, flag, 0644);
		if (redir->fd == -1)
			return (setexit(app, EX_ERR), ft_perror(redir->target), -1);
		redir = redir->next;
	}
	return (0);
}

int	do_dup2redirs(t_app *app, t_cmd_node *node)
{
	t_redir	*redir;

	redir = node->redirs;
	while (redir)
	{
		if (redir->fd != -1)
		{
			if (redir->type == REDIR_IN || redir->type == REDIR_HEREDOC)
			{
				if (dup2(redir->fd, STDIN_FILENO) == -1)
					return (setexit(app, EX_ERR), perror(APP), -1);
			}
			else if (redir->type == REDIR_APPEND || redir->type == REDIR_OUT)
			{
				if (dup2(redir->fd, STDOUT_FILENO) == -1)
					return (setexit(app, EX_ERR), perror(APP), -1);
			}
		}
		redir = redir->next;
	}
	return (0);
}

void	close_redirsfd(t_cmd_node *node)
{
	t_redir	*redir;

	redir = node->redirs;
	while (redir)
	{
		if (redir->fd != -1)
		{
			close(redir->fd);
			redir->fd = -1;
		}
		redir = redir->next;
	}
}

static void	_flatten_cmds(t_ast_node *ast, t_cmd_node **cmds, int *i)
{
	if (!ast)
		return ;
	if (ast->type == NODE_BINOP)
	{
		_flatten_cmds(ast->content.binop.left, cmds, i);
		_flatten_cmds(ast->content.binop.right, cmds, i);
	}
	else
	{
		cmds[*i] = &(ast->content.cmd);
		(*i)++;
	}
	cmds[*i] = NULL;
}

int	do_exec(t_app *app, t_cmd_node *cmdnode)
{
	char	*cmdpath;
	int		status;
	
	if (is_builtin(cmdnode->argv[0]))
	{
		status = exec_builtin(cmdnode->argv, app);
		return (setexit(app, status), exit(app->exitcode), -1);
	}
	cmdpath = resolvecmdpath(app, cmdnode->argv);
	if (!cmdpath)
		return (-1);
	if (execve(cmdpath, cmdnode->argv, app->envp) == -1)
	{
		free(cmdpath);
		if (!ft_strhaschr(cmdnode->argv[0], '/'))
		{
			setexecerrno(app);
			if (app->exitcode == EX_CMD_NOTFOUND)
				return (printerr_cmdnfound(cmdnode->argv[0]), -1);
			return (ft_perror(cmdnode->argv[0]), -1);
		}
		return (setexecerrno(app), ft_perror(cmdnode->argv[0]), -1);
	}
	return (0);
}

void	close_pipefd(int *pipebuf)
{
	if (!pipebuf)
		return ;
	if (pipebuf[0] != -1)
	{
		close(pipebuf[0]);
		pipebuf[0] = -1;
	}
	if (pipebuf[1] != -1)
	{
		close(pipebuf[1]);
		pipebuf[1] = -1;
	}
}

void	wait_allpids(pid_t *pids, int pid_id)
{
	int	i;

	i = 0;
	while (i <= pid_id)
	{
		while (waitpid(pids[i], NULL, 0) == -1)
		{
			if (errno != EINTR)
				break ;
		}
		i++;
	}
}

void	get_lastcmdstatus(t_app *app, pid_t *pids, int id_lastpid)
{
	int	status;

	status = 0;
	// when waitpid is blocking, a signal can interrupt and cause it to return -1, and we need it to keep waiting
	while (waitpid(pids[id_lastpid], &status, 0) == -1)
	{
		if (errno != EINTR)
			break ;
	}
	if (WIFEXITED(status))
		app->exitcode = WEXITSTATUS(status);
	else if (WIFSIGNALED(status))
	{	// parent always live to receive forked's status
		if (WTERMSIG(status) == SIGQUIT)
			ft_putstr_fd("Quit (core dumped)\n", 2);
		else if (WTERMSIG(status) == SIGINT)
			write(2, "\n", 1);
		app->exitcode = EX_SIG_BASE + WTERMSIG(status);
	}
}

// after fork, called inside child
int	do_childproc(t_app *app, t_cmd_node *cmdnode, int fdin, int fdout)
{
	signals_default();
	// -1 means we are not in child, dont dup2
	if (fdin != -1)
	{
		dup2(fdin, STDIN_FILENO);
		close(fdin);
	}
	if (fdout != -1)
	{
		dup2(fdout, STDOUT_FILENO);
		close(fdout);
	}
	if (open_redirs(app, cmdnode) == -1) // saved opened fds inside each redirect linkedlist
		return (close_redirsfd(cmdnode), -1);
	if (do_dup2redirs(app, cmdnode) == -1) // loop thru each redirects and dup2 fds.
		return (close_redirsfd(cmdnode), -1);
	close_redirsfd(cmdnode);
	if (do_exec(app, cmdnode) == -1)
		return (-1);
	return (0);
}

/**
 * @brief Fill the first nbyte bytes of dest with the value c.
 * @param dest Destination buffer.
 * @param c Value to set (converted to unsigned char).
 * @param nbyte Number of bytes to set.
 * @return dest, or NULL if dest is NULL.
 */
void	*ft_memset(void *dest, int c, size_t nbyte)
{
	unsigned char	*dest_p;

	if (!dest)
		return (NULL);
	dest_p = dest;
	while (nbyte--)
		*dest_p++ = (unsigned char)c;
	return (dest);
}

typedef struct s_pipeops
{
	int		n_cmd;
	int		prevfdin;
	int		fdout;
	int		pipebuf[2];
	pid_t	pids[256];
}	t_pipeops;

t_pipeops	*init_pipeops()
{
	t_pipeops	*pipeops;

	pipeops = malloc(sizeof(t_pipeops));
	if (!pipeops)
		return (NULL);
	pipeops->n_cmd = 0;
	pipeops->prevfdin = -1;
	pipeops->fdout = -1;
	pipeops->pipebuf[0] = -1;
	pipeops->pipebuf[1] = -1;
	ft_memset(pipeops->pids, -1, sizeof(pipeops->pids));
	return (pipeops);
}

t_cmd_node	**_setup_cmds(t_app *app, t_ast_node *ast, t_pipeops *pipeops)
{
	t_cmd_node	**cmds;
	int			i;

	pipeops->n_cmd = count_cmds(ast);
	if (pipeops->n_cmd > 256)
		return (setexit(app, EX_ERR), 
			ft_putstr_fd("minishell: pipeline too long\n", 2), NULL);
	cmds = malloc(sizeof(t_cmd_node *) * (pipeops->n_cmd + 1));
	if (!cmds)
		return (setexit(app, EX_ERR), perror(APP), NULL);
	i = 0;
	_flatten_cmds(ast, cmds, &i);
	return (cmds);
}

void	free_pipeops(t_pipeops *pipeops, int last_pid_id)
{
	/*
	previous buffer has previous process written into it
	if nvr close the read end, the write will nvr receives EOF
	this causes the child process itself blocked -> waitpid hangs (deadlock)
	*/
	if (pipeops->prevfdin != -1)
		close(pipeops->prevfdin);
	if (pipeops->pipebuf[0] != -1)
		close(pipeops->pipebuf[0]);
	if (pipeops->pipebuf[1] != -1)
		close(pipeops->pipebuf[1]);
	wait_allpids(pipeops->pids, last_pid_id);
	free(pipeops);
}

int	setup_pipe(t_app *app, t_pipeops *pipeops, int iter_i)
{
	int	n_pipes;

	n_pipes = pipeops->n_cmd - 1;
	pipeops->pipebuf[0] = -1;
	pipeops->pipebuf[1] = -1;
	pipeops->fdout = -1; // out starts with -1
	if (n_pipes > 0 && iter_i < n_pipes)
	{
		if (pipe(pipeops->pipebuf) == -1)
			return (setexit(app, EX_ERR), perror(APP), -1);
		// subsequent fdout updates with current buffer writend
		pipeops->fdout = pipeops->pipebuf[1];
	}
	return (0);
}

void	update_pipeops(t_pipeops *pipeops, int iter_i)
{
	if (pipeops->prevfdin != -1)
	{
		close(pipeops->prevfdin);
		pipeops->prevfdin = -1;
	}
	if (pipeops->pipebuf[1] != -1)
	{
		close(pipeops->pipebuf[1]);
		pipeops->pipebuf[1] = -1;
	}
	if (iter_i < pipeops->n_cmd - 1)
		pipeops->prevfdin = pipeops->pipebuf[0]; // subsequent in starts with previous buffer readend
	else if (pipeops->pipebuf[0] != -1)
	{
		close(pipeops->pipebuf[0]);
		pipeops->pipebuf[0] = -1;
	}
}

int	start_pipeline(t_app *app, t_pipeops *pipeops, t_cmd_node **cmds)
{
	int	i;

	i = 0;
	while (i < pipeops->n_cmd)
	{
		if (setup_pipe(app, pipeops, i) == -1)
			return (free_pipeops(pipeops, i - 1), -1);
		pipeops->pids[i] = fork(); // create child
		if (pipeops->pids[i] == -1)
			return (free_pipeops(pipeops, i - 1), 
				setexit(app, EX_ERR), perror(APP), -1);
		if (pipeops->pids[i] == 0)
		{
			if (pipeops->pipebuf[0] != -1)
				close(pipeops->pipebuf[0]);
			do_childproc(app, cmds[i], pipeops->prevfdin, pipeops->fdout);
			free(cmds);
			free(pipeops);
			exit(app->exitcode);
		}
		update_pipeops(pipeops, i);
		i++;
	}
	return (0);
}

int	exec_allcmds(t_app *app, t_ast_node *ast)
{	// given list of commands, execute one by one
	t_pipeops	*pipeops;
	t_cmd_node	**cmds;

	pipeops = init_pipeops();
	if (!pipeops)
		return (setexit(app, EX_ERR), perror(APP), -1);
	cmds = _setup_cmds(app, ast, pipeops);
	if (!cmds)
		return (free(pipeops), -1);
	signals_ignore();
	if (start_pipeline(app, pipeops, cmds) == -1)
		return (free(cmds), -1);
	get_lastcmdstatus(app, pipeops->pids, pipeops->n_cmd - 1);
	free(cmds);
	free_pipeops(pipeops, pipeops->n_cmd - 2);  
	return (0);
}

/**
 * @brief Check if a command name is a shell builtin.
 * @param cmd Command name (argv[0]).
 * @return 1 if builtin, 0 otherwise.
 */
int	is_builtin(const char *cmd)
{
	if (!cmd)
		return (0);
	if (ft_strcmp(cmd, "echo") == 0)
		return (1);
	if (ft_strcmp(cmd, "cd") == 0)
		return (1);
	if (ft_strcmp(cmd, "pwd") == 0)
		return (1);
	if (ft_strcmp(cmd, "export") == 0)
		return (1);
	if (ft_strcmp(cmd, "unset") == 0)
		return (1);
	if (ft_strcmp(cmd, "env") == 0)
		return (1);
	if (ft_strcmp(cmd, "exit") == 0)
		return (1);
	return (0);
}

/**
 * @brief Dispatch to the appropriate builtin implementation.
 * @param argv NULL-terminated argument array.
 * @param app Shell context.
 * @return Exit status of the builtin.
 */
int	exec_builtin(char **argv, t_app *app)
{
	if (ft_strcmp(argv[0], "echo") == 0)
		return (builtin_echo(argv));
	if (ft_strcmp(argv[0], "cd") == 0)
		return (builtin_cd(argv, app));
	if (ft_strcmp(argv[0], "pwd") == 0)
		return (builtin_pwd());
	if (ft_strcmp(argv[0], "export") == 0)
		return (builtin_export(argv, app));
	if (ft_strcmp(argv[0], "unset") == 0)
		return (builtin_unset(argv, app));
	if (ft_strcmp(argv[0], "env") == 0)
		return (builtin_env(app));
	if (ft_strcmp(argv[0], "exit") == 0)
		return (builtin_exit(argv, app));
	return (1);
}

void	restore_fd(int fdin, int fdout)
{
	if (fdin != -1)
	{
		dup2(fdin, STDIN_FILENO);
		close(fdin);
	}
	if (fdout != -1)
	{
		dup2(fdout, STDOUT_FILENO);
		close(fdout);
	}
}

int	exec_builtinproc(t_app *app, t_cmd_node *cmdnode)
{
	int		temp_in;
	int		temp_out;
	int		status;

	// builtin to be ran at parent particularly for 
	// cd, export, exit, unset
	temp_in = -1;
	temp_out = -1;
	if (cmdnode->redirs)
	{
		temp_in = dup(STDIN_FILENO);
		temp_out = dup(STDOUT_FILENO);
		if (temp_in == -1 || temp_out == -1)
			return (restore_fd(temp_in, temp_out), -1);
		if (open_redirs(app, cmdnode) == -1) // saved opened fds inside each redirect linkedlist
			return (restore_fd(temp_in, temp_out), close_redirsfd(cmdnode), -1);
		if (do_dup2redirs(app, cmdnode) == -1) // loop thru each redirects and dup2 fds.
			return (restore_fd(temp_in, temp_out), close_redirsfd(cmdnode), -1);
		close_redirsfd(cmdnode);
	}
	status = exec_builtin(cmdnode->argv, app);
	restore_fd(temp_in, temp_out);
	setexit(app, status);
	return (status);
}

int	execute_ast(t_app *app, t_ast_node *ast)
{
	if (!ast)
		return (0);
	if (ast->type == NODE_CMD)
	{	// opt1: no cmd but has redirects
		if (!ast->content.cmd.argv)
		{	// Bash creates/truncates the file and returns 0
			if (open_redirs(app, &ast->content.cmd) == -1)
				return (close_redirsfd(&ast->content.cmd), -1);
			close_redirsfd(&ast->content.cmd);
			return (0);
		}
		else if (is_builtin(ast->content.cmd.argv[0])) // opt2: has 1 command and is a builtin
		{
			if (exec_builtinproc(app, &ast->content.cmd) == -1)
				return (-1);
			return (0);
		}
	}
	// opt3: normal path
	if (exec_allcmds(app, ast) == -1)
		return (-1);
	return (0);
}

/*
signal(received, action)

received event : disposition (response)
- SIGINT : terminate process 
- SIGQUIT: terminate + core dumped
- SIGPIPE: terminate process

Context 1: AT PROMPT (readline waiting for input)
    Ctrl+C  → cancel the line, print newline, show fresh prompt
    Ctrl+\  → ignore
    Ctrl+D  → EOF → exit shell
    Need: custom SIGINT handler that calls rl_replace_line + rl_redisplay

Context 2: DURING COMMAND EXECUTION (waiting for child)
    Ctrl+C  → kill the child (default), parent catches via waitpid WIFSIGNALED. Parent has to start with SIG_IGN.
    Ctrl+\ (use SIG_DFL) → quit the child, parent prints "Quit (core dumped)"
    Parent must NOT die
    Need: parent has SIG_IGN, child has SIG_DFL

Context 3: DURING HEREDOC (reading delimiter)
    Ctrl+C  → interrupt heredoc collection, don't execute, reset prompt
    Need: similar to context 1 but discard collected content

* at prompt: SIGINT interrupts readline
sa.sa_handler = handle_sigint_prompt;
sa.sa_flags = 0;               // NO SA_RESTART
sigaction(SIGINT, &sa, NULL);

* before fork: parent ignores
sa.sa_handler = SIG_IGN;
sa.sa_flags = 0;
sigaction(SIGINT, &sa, NULL);

* in child after fork: restore default
sa.sa_handler = SIG_DFL;
sa.sa_flags = 0;
sigaction(SIGINT, &sa, NULL);

*/