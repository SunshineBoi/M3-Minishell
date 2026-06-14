Show thinking

Private Council Review – src/exec/ execution engine

Focus: correctness, deadlocks, FD/memory leaks, signals, POSIX/Bash.

### 1. Confirmed defect – heredoc pipe deadlock on large input

* severity: Critical
* confidence: High
* path:line: `src/exec/exec_fd.c:261-270`, `src/exec/exec_fd.c:196-197`, `src/exec/exec_fd.c:243-244`
* reasoning: `collect_one_heredoc` creates a pipe, then `read_heredoc_loop` writes the entire heredoc to `pipe_fds[1]` in the parent *before* any child exists. No concurrent reader drains the pipe. Linux pipe buffer (∼64 KiB) fills, `write()` blocks, shell hangs. Bash avoids this by using a temp file or forking a writer.
* missing context: pipe capacity on target; whether `write` is checked for partial writes.
* validation test: `yes a | head -c 200000 > /tmp/big; cat <<EOF | wc -c < /tmp/big` – with current code the shell blocks at ∼65536 bytes. After fix it returns 200001.
* remediation: do not write heredoc in parent. Options: (a) write to `O_TMPFILE` and reopen read-only; (b) fork a short-lived writer child that writes then exits while parent continues; (c) set `O_NONBLOCK` and buffer in userspace. Always close write end before exec.


### 2. Confirmed defect – heredoc FDs leak into all pipeline children

* severity: High
* confidence: High
* path:line: `src/exec/exec_fd.c:273`, `src/exec/exec_pipeline.c:87-95`, `src/exec/exec_fd.c:68-72`
* reasoning: `redir->fd = pipe_fds[0]` is stored without `FD_CLOEXEC`. `start_pipeline` forks N children; each child only closes its *own* redirs in `do_childproc`. All other heredoc read ends remain open across `execve`. A reader waiting for EOF (e.g., `cat <<A | cat <<B`) never sees EOF because another child holds the fd open → pipeline hangs.
* missing context: definition of `t_redir`, whether any global close-all is performed.
* validation test: `cat <<A | cat <<B` with two small heredocs; observe second `cat` never terminates. `lsof -p <pid>` shows extra pipe fds.
* remediation: after `pipe()`, `fcntl(pipe_fds[0], F_SETFD, FD_CLOEXEC)`. In child before exec, iterate all AST redirs and close fds not used by this command. Parent must also close after pipeline.


### 3. Confirmed defect – `count_cmds` undercounts right-nested pipelines

* severity: Critical
* confidence: High
* path:line: `src/exec/exec_pipeline_utils.c:3-16`, `src/exec/exec_pipeline.c:25`, `src/exec/exec_pipeline.c:33`
* reasoning: `count_cmds` walks only `root = root->content.binop.left`. `_flatten_cmds` recurses both left and right. For a right-associative AST (`a | (b | c)`), `n_cmd` is 2 but 3 pointers are written into `cmds`, overflowing the malloc at `setup_cmds:29`.
* missing context: parser AST shape – evidence suggests right-nesting possible.
* validation test: Build AST manually with 4 right-nested BINOP nodes; run under ASan. Crash at `cmds[*i] = …` in `_flatten_cmds`.
* remediation: replace `count_cmds` with recursive count of both children, or allocate dynamically while flattening.


### 4. Confirmed defect – parent signals left ignored after pipeline

* severity: High
* confidence: Medium
* path:line: `src/exec/exec_dispatch.c:41`, `src/exec/exec_pipeline.c:57`
* reasoning: `signals_ignore()` is called before `start_pipeline`. Children call `signals_default()`, but parent never restores. Interactive shell then ignores SIGINT/SIGQUIT, deviating from Bash.
* missing context: implementation of `signals_ignore`.
* validation test: Run `./minishell`, execute `sleep 1 | cat`, then press Ctrl-C at prompt – shell does not terminate line. Expected: interrupt.
* remediation: Save old handlers before ignore, restore after `get_lastcmdstatus`. Use `sigaction` pair.


### 5. Confirmed defect – leak on pipeline setup failure

* severity: Medium
* confidence: High
* path:line: `src/exec/exec_dispatch.c:42-43`, `src/exec/exec_pipeline.c:85-90`
* reasoning: If `start_pipeline` fails (fork or pipe error), `exec_allcmds` returns `free(cmds), -1` without `free(pipeops)`. Already-opened pipe ends from previous iterations remain open.
* missing context: error injection path.
* validation test: `ulimit -u 1; ls | cat`; check valgrind – definite leak of `t_pipeops` and 2 fds.
* remediation: on error, call `free_pipeops(pipeops, i-1)` before return.


### 6. Likely defect – parent never closes heredoc read fds

* severity: Medium
* confidence: Medium
* path:line: `src/exec/exec_dispatch.c:54-55`, `src/exec/exec_fd.c:273`
* reasoning: `collect_heredocs` stores pipe read ends in AST. After pipeline, no `close_redirsfd` is called in parent. FDs accumulate across commands, eventually hitting `EMFILE`.
* missing context: lifetime of AST nodes.
* validation test: Loop `for i in {1..1000}; do cat <<EOF\nx\nEOF; done`; monitor `ls /proc/$$/fd | wc -l` – count grows.
* remediation: After `exec_allcmds`, walk AST and close any `redir->fd != -1`.



### 7. Likely defect – path resolution mixes errno and exitcode


* severity: Medium
* confidence: Medium
* path:line: `src/exec/exec_path.c:15-16`, `src/exec/exec_path.c:45-47`
* reasoning: `buildgoodpath` does `access(path, X_OK)` then `execve` later – TOCTOU race. `_matchcmdpath` breaks on `app->exitcode == EACCES`, comparing shell exit code to errno value. This misclassifies permission denied as success, leading to wrong error ("command not found").
* missing context: `setexecerrno` mapping.
* validation test: Create non-executable file in PATH; run it. Bash prints "Permission denied"; current code may print "command not found".
* remediation: Remove `access` check; rely on `execve` errno. Store errno separately, do not compare to `app->exitcode`.



### 8. Architectural risk – child uses `exit()` not `_exit()`


* severity: Low
* confidence: High
* path:line: `src/exec/exec_pipeline_run.c:50`, `src/exec/exec_pipeline.c:98`
* reasoning: After fork, child calls `exit()`, flushing stdio buffers duplicated from parent. Can duplicate output in pipelines, violates POSIX shell practice.
* missing context: whether parent flushes before fork.
* validation test: Parent does `printf("x")` without newline, then forks pipeline; observe "xx" in output.
* remediation: Replace with `_exit(app->exitcode)`.



### 9. Hardening recommendation – unchecked `dup2` in `restore_fd`


* severity: Low
* confidence: High
* path:line: `src/exec/exec_fd.c:92-93`, `97-98`
* reasoning: `restore_fd` does not check `dup2` return. If it fails, original stdin/stdout is lost and fd is closed anyway.
* missing context: error policy for builtins.
* validation test: Close STDIN before builtin with redirection; shell loses stdin after.
* remediation: Check return, on failure `dup2` again from `/dev/tty` or abort restore.



### 10. Hardening recommendation – redirections lack `O_CLOEXEC`


* severity: Low
* confidence: Medium
* path:line: `src/exec/exec_fd.c:23`
* reasoning: `open(redir->target, flag, 0644)` leaves fd open across exec for unrelated children. Combined with finding #2, leaks are amplified.
* missing context: platform support.
* validation test: `exec 3> /tmp/x; ls -l /proc/self/fd` in child shows fd 3.
* remediation: open with `O_CLOEXEC` or `fcntl(FD_CLOEXEC)` immediately after open.




Overall remediation strategy


* Replace heredoc pipe with temp file or forked writer; always set `FD_CLOEXEC`.
* Fix `count_cmds` to be recursive.
* Ensure parent restores signals and closes all temporary fds after each AST execution.
* Use `_exit` in children, check all syscalls (`dup2`, `write`, `pipe`, `fork`).
* Remove `access()` pre-check; base errors on `execve` errno.



These changes address the deadlock, FD leaks, signal, and POSIX deviations evident in the supplied sources.*