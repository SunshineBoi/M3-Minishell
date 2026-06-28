
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

void	signals_ignore(void)
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

void	handle_sigint_in_main(t_app *app)
{
	setexit(app, EX_SIG_BASE + SIGINT);
	g_signal = 0;
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
