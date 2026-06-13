#ifndef EXEC_H
# define EXEC_H

typedef struct s_fd_ops
{
	int	in_fd;
	int	out_fd;
}	t_fd_ops;

typedef struct s_pipeops
{
	int		n_cmd;
	int		prevfdin;
	int		fdout;
	int		pipebuf[2];
	pid_t	pids[256];
}	t_pipeops;

/* === exec_path_utils.c === */
int			countbydelim(char *str, char delim);
char		**ft_splitbydelim(t_app *app, char *str, char delim);
char		**getrawpathlst(t_app *app, char *match);

/* === exec_path.c === */
char		*buildgoodpath(t_app *app, char *envp, char *cmd);
char		*resolvecmdpath(t_app *app, char **argv);

/* === exec_fd.c === */
int			open_redirs(t_app *app, t_cmd_node *node);
int			do_dup2redirs(t_app *app, t_cmd_node *node);
void		close_redirsfd(t_cmd_node *node);
void		close_pipefd(int *pipebuf);
void		restore_fd(int fdin, int fdout);
int			collect_heredocs(t_app *app, t_ast_node *ast);

/* === exec_pipeline_utils.c === */
int			count_cmds(t_ast_node *root);
t_pipeops	*init_pipeops(void);
void		update_pipeops(t_pipeops *pipeops, int iter_i);
void		free_pipeops(t_pipeops *pipeops, int last_pid_id);

/* === exec_pipeline.c === */
t_cmd_node	**setup_cmds(t_app *app, t_ast_node *ast, t_pipeops *pipeops);
int			setup_pipe(t_app *app, t_pipeops *pipeops, int iter_i);
int			do_childproc(t_app *app, t_cmd_node *cmdnode, int fdin, int fdout);
int			start_pipeline(t_app *app, t_pipeops *pipeops, t_cmd_node **cmds);

/* === exec_pipeline_run.c === */
void		wait_allpids(pid_t *pids, int pid_id);
void		get_lastcmdstatus(t_app *app, pid_t *pids, int id_lastpid);
int			do_exec(t_app *app, t_cmd_node *cmdnode);

/* === exec_dispatch.c === */
int			exec_builtinproc(t_app *app, t_cmd_node *cmdnode);
int			exec_allcmds(t_app *app, t_ast_node *ast);
int			execute_ast(t_app *app, t_ast_node *ast);

#endif
