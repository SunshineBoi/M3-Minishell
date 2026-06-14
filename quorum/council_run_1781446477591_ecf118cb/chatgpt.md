I’ll separate source-confirmed failures from architecture-level risks, and tie every claim to exact line evidence and a concrete validation test.


## 1. Heredoc collection can deadlock before execution


**Classification:** Confirmed defect

**Severity:** Critical

**Confidence:** High

**Evidence:** `src/exec/exec_fd.c:243-244`, `src/exec/exec_fd.c:261-273`; collection is invoked before any process is spawned at `src/exec/exec_dispatch.c:54-55`.


**Reasoning:** `collect_one_heredoc()` creates a pipe and synchronously writes the entire heredoc into its write end. No process reads from the pipe until heredoc collection finishes and pipeline execution begins. Once the heredoc exceeds the kernel pipe capacity, `write()` blocks permanently. This is the classic heredoc pipe deadlock.


**Remediation options:**


* Prefer a temporary file or anonymous temporary file for heredoc storage, then rewind it.
* Alternatively, fork a dedicated writer so reading can occur concurrently.
* A dynamically growing in-memory buffer followed by a file-backed descriptor is also viable; writing the completed buffer into an unread pipe is not.



**Missing context:** None material. Pipe capacity varies by platform, affecting only the input size required to reproduce it.


**Validation test:** Run a command such as `cat <<EOF` with a generated heredoc substantially larger than the platform pipe capacity, for example several megabytes. Assert completion under a timeout and exact output preservation.



## 2. Pipeline command counting can under-allocate the flattened command array


**Classification:** Confirmed defect

**Severity:** Critical

**Confidence:** High

**Evidence:** `src/exec/exec_pipeline_utils.c:3-16`; `src/exec/exec_pipeline.c:3-18`, `src/exec/exec_pipeline.c:25-33`.


**Reasoning:** `count_cmds()` follows only the left branch of each binary node. `_flatten_cmds()` recursively traverses both branches and writes every leaf into the allocated array. A right-nested or balanced AST therefore produces more writes than allocated slots, causing heap corruption. The terminating assignment at `exec_pipeline.c:17` can also write beyond the allocation.


For example, an AST shaped as `cmd1 | (cmd2 | cmd3)` is counted as two nodes along the left path but flattened into three commands.


**Remediation options:**


* Count command leaves recursively using the same traversal semantics as `_flatten_cmds()`.
* Better, combine counting and flattening into a checked dynamic vector.
* Assert after flattening that the number written equals `pipeops->n_cmd`.



**Missing context:** Parser associativity determines whether ordinary input reaches this shape, but the execution API accepts arbitrary binary AST shapes and does not enforce left nesting.


**Validation test:** Construct left-nested, right-nested, and balanced pipeline ASTs with 3–256 leaves. Run under AddressSanitizer and assert the flattened count and order exactly match the leaves.



## 3. Parent retains heredoc descriptors after external commands and pipelines


**Classification:** Confirmed defect

**Severity:** High

**Confidence:** High

**Evidence:** Heredoc read descriptors are stored at `src/exec/exec_fd.c:270-274`. External/pipeline execution returns through `src/exec/exec_dispatch.c:30-47`, which never closes command redirection descriptors. `close_redirsfd()` is only called inside each child at `src/exec/exec_pipeline.c:68-72`.


**Reasoning:** Descriptor tables are copied by `fork()`. Closing a heredoc descriptor in a child does not close the parent’s copy. After pipeline completion, `free_pipeops()` closes pipeline descriptors only; it does not traverse command redirections. Consequently, each heredoc used by a non-parent execution path leaks a descriptor in the shell process.


Repeated heredocs can eventually exhaust `RLIMIT_NOFILE`, breaking subsequent pipes, opens, and builtins.


**Remediation options:**


* Add a parent-side traversal that closes all collected heredoc descriptors after forking, and on every pipeline setup failure.
* Establish explicit ownership: the AST owns collected heredoc descriptors until they are transferred or closed.
* Consider setting `FD_CLOEXEC` as defense in depth, though it does not replace parent cleanup.



**Missing context:** AST destruction might close these descriptors later, but no such cleanup appears in the supplied execution path. Even cleanup at the next prompt would leave descriptors open unnecessarily during execution.


**Validation test:** Record `/proc/self/fd` count or use `fcntl()` enumeration, then execute hundreds of external commands containing heredocs. The descriptor count must return to baseline after every command.



## 4. Every pipeline child inherits unrelated heredoc descriptors


**Classification:** Confirmed defect

**Severity:** High

**Confidence:** High

**Evidence:** All heredocs are collected before forking at `src/exec/exec_dispatch.c:54-55`. In a child, only the current command’s redirection list is closed at `src/exec/exec_pipeline.c:68-72`; there is no traversal over other commands’ redirection descriptors before `execve()` at `src/exec/exec_pipeline_run.c:52-55`.


**Reasoning:** Each child inherits all heredoc read descriptors collected for the entire AST. It closes only descriptors referenced by its own command. External programs therefore receive unrelated descriptors unless they happen to be marked close-on-exec. No `O_CLOEXEC`, `pipe2(O_CLOEXEC)`, or `fcntl(FD_CLOEXEC)` appears here.


This is an observable descriptor leak into executed programs and can expose resources or contribute to descriptor exhaustion.


**Remediation options:**


* In each child, close every descriptor not required by that child before `execve()`.
* Create internal descriptors with close-on-exec enabled.
* Maintain a centralized descriptor registry rather than relying on command-local cleanup.



**Missing context:** A global child cleanup routine may exist outside the supplied files, but none is called from the shown child path.


**Validation test:** Execute a helper program in a multi-command pipeline that enumerates open descriptors. Assert that only standard descriptors and intentionally inherited descriptors remain.



## 5. Empty or repeated `PATH` fields can cause an infinite loop


**Classification:** Confirmed defect

**Severity:** Critical

**Confidence:** High

**Evidence:** `src/exec/exec_path_utils.c:23-40`, especially the conditional at line 30 and return at line 40; caller loop at `src/exec/exec_path_utils.c:56-63`.


**Reasoning:** `_splithelper()` returns zero when `str[0]` is the delimiter because `if (str[lenword])` is true, the word length remains zero, but the code does increment `n_delim` at lines 34-35. Actually for a leading delimiter it returns one; however, it never assigns `*target`, leaving the pre-zeroed slot null. The caller then increments `i` and advances one byte. This avoids an infinite loop, but produces an early null entry that truncates later path entries during `_matchcmdpath()`.


For `PATH=:/valid/bin`, the list’s first slot remains null and the valid second component is stored later. `_matchcmdpath()` stops immediately at the first null. Consecutive delimiters have the same truncation problem.


**Remediation options:**


* Materialize each empty component as an allocated empty string.
* Use an index-based parser that emits exactly one entry per field, including leading, trailing, and consecutive empty fields.
* Add explicit tests for every empty-field position.



**Missing context:** `ft_calloclst()` is assumed to initialize entries to null, consistent with its name and usage. If it does not, behavior is worse and potentially undefined.


**Validation test:** Place an executable in the current directory and test `PATH=:/bin`, `PATH=/bin::/usr/bin`, and `PATH=/bin:`. Empty components must search the current directory in their specified order.



## 6. An entirely empty `PATH` does not represent the current directory


**Classification:** Confirmed defect

**Severity:** High

**Confidence:** High

**Evidence:** `src/exec/exec_path_utils.c:49-64`; command search at `src/exec/exec_path.c:40-55`.


**Reasoning:** For `PATH=` the splitter allocates a list but emits no component because `while (*str)` is false. The resulting list starts with null, so command search performs no current-directory lookup. In shell path semantics, an empty PATH component denotes the current directory; an entirely empty value therefore contains one empty component.


**Remediation idea:** Ensure the splitter emits one empty string when the source value is empty.


**Missing context:** None material.


**Validation test:** Create an executable in the working directory, set `PATH` to the empty string, and invoke it without a slash. Compare behavior with Bash and a POSIX shell.



## 7. Path search incorrectly stops at the first non-executable candidate


**Classification:** Confirmed defect

**Severity:** High

**Confidence:** High

**Evidence:** `src/exec/exec_path.c:40-49`, particularly `if (goodpath || app->exitcode == EACCES) break;` at lines 46-47.


**Reasoning:** If an earlier PATH directory contains a matching file that is not executable, search terminates immediately when `buildgoodpath()` records `EACCES`. A later PATH directory may contain a valid executable with the same name, but it is never checked.


Shell path search should continue through remaining entries, remember that at least one candidate failed due to permission or unsuitable file type, and report the appropriate failure only if no executable candidate is eventually found.


**Remediation idea:** Track a “permission-denied candidate seen” flag while continuing the search. Return the later executable when found; otherwise report the remembered error.


**Missing context:** The exact implementation of `setexecerrno()` is omitted, but the explicit `EACCES` break establishes the premature termination.


**Validation test:** Put a non-executable file named `tool` in the first PATH directory and an executable `tool` in the second. Invocation must execute the second file.



## 8. Command resolution contains a command-name-specific success exception


**Classification:** Confirmed defect

**Severity:** High

**Confidence:** High

**Evidence:** `src/exec/exec_path.c:74-80`, especially lines 77-78.


**Reasoning:** When PATH is absent, one particular command name is treated as successful and returns no executable path. `do_exec()` then returns failure at `src/exec/exec_pipeline_run.c:52-54`, but the child exits using `app->exitcode`, which was explicitly set to success. This makes a nonexistent command report status zero.


The behavior is command-name-specific, incompatible with shell semantics, and likely a leftover test accommodation.


**Remediation idea:** Remove the special case. Resolve every command through the same rules and return command-not-found status when PATH lookup is unavailable.


**Missing context:** None.


**Validation test:** Unset PATH and invoke the affected command name. Assert diagnostic output and status 127, matching invocation of any other missing command.



## 9. Pipeline signal dispositions are not restored in the parent execution path


**Classification:** Likely defect

**Severity:** High

**Confidence:** Medium-high

**Evidence:** Parent signals are ignored at `src/exec/exec_dispatch.c:41`. Both success and failure returns at `src/exec/exec_dispatch.c:42-47` contain no restoration call.


**Reasoning:** If `signals_ignore()` changes persistent process dispositions, the interactive shell remains in that state after pipeline completion. This may cause later prompts or heredoc input to ignore Ctrl-C or Ctrl-\ unexpectedly. The failure path is especially problematic because it returns immediately after partial pipeline cleanup.


**Remediation options:**


* Save and restore the exact prior dispositions around pipeline execution.
* Use a single cleanup block that restores signals on all exits.
* Avoid relying on the main loop to reset state implicitly.



**Missing context:** The definitions of `signals_ignore()` and the main-loop signal setup are omitted. The main loop may reset handlers after every `execute_ast()` call, which would reduce the impact but leave this function non-self-contained.


**Validation test:** Run a pipeline, return to the prompt, and send SIGINT while waiting for input. Repeat after forced `pipe()` and `fork()` failures using fault injection. Verify interactive handling is restored in every case.



## 10. A non-`EINTR` wait failure can be interpreted as successful child exit


**Classification:** Confirmed defect

**Severity:** High

**Confidence:** High

**Evidence:** `src/exec/exec_pipeline_run.c:5-15`.


**Reasoning:** `status` is initialized to zero. If `waitpid()` fails with an error other than `EINTR`, the loop breaks and the code still evaluates `WIFEXITED(status)`. A zero status encodes normal exit with code zero, so the shell can incorrectly overwrite its status with success despite never obtaining child status.


**Remediation idea:** Capture the `waitpid()` return value. Only inspect status when it returns the requested PID. On permanent failure, preserve or set an internal error status and report the wait error.


**Missing context:** None material.


**Validation test:** Use a test seam or wrapper to force `waitpid()` to return `-1` with `ECHILD` or another permanent error. Assert that the shell does not report success and does not run `WIF*` macros on an unacquired status.



## 11. Heredoc write results and partial writes are ignored


**Classification:** Confirmed defect

**Severity:** High

**Confidence:** High

**Evidence:** `src/exec/exec_fd.c:184-199`, particularly lines 196-197.


**Reasoning:** Both `write()` calls ignore return values. Even apart from the blocking deadlock, writes can be interrupted, partially completed, or fail. A partial write silently truncates the heredoc. A failed write still returns success from `write_expanded()`.


**Remediation idea:** Use a `write_all()` loop that handles partial writes and retries `EINTR`, while propagating other errors. Apply it to both the data and newline.


**Missing context:** None.


**Validation test:** Interpose `write()` so it performs short writes and occasionally returns `EINTR`. Assert exact heredoc contents. Also inject `EIO` and assert execution aborts with a nonzero status.



## 12. Non-signal heredoc failures are misreported as SIGINT


**Classification:** Confirmed defect

**Severity:** High

**Confidence:** High

**Evidence:** Allocation and pipe failures can return `-1` at `src/exec/exec_fd.c:258-262`; expansion failure can return `-1` through lines 243-244. Every such failure is converted to `EX_SIG_BASE + SIGINT` at `src/exec/exec_fd.c:284-290`.


**Reasoning:** The caller cannot distinguish user interruption from allocation failure, pipe failure, or expansion/write failure. All are assigned status 130. This produces incorrect shell status and suppresses appropriate diagnostics for system failures.


**Remediation idea:** Return a typed result, such as success, interrupted, and internal/system error. Set status 130 only for an observed SIGINT; set the project’s internal-error status for allocation or descriptor failures.


**Missing context:** Error constants and reporting conventions are omitted, but conflating all failures with SIGINT is independently demonstrated.


**Validation test:** Fault-inject failure into delimiter allocation, `pipe()`, expansion allocation, and write. Assert none returns 130. Separately deliver SIGINT during heredoc input and assert 130.



## 13. Previously collected heredocs leak when a later heredoc fails


**Classification:** Confirmed defect

**Severity:** High

**Confidence:** High

**Evidence:** Successful heredocs store descriptors at `src/exec/exec_fd.c:267-274`. Collection stops on the first later failure at `src/exec/exec_fd.c:281-290` and `src/exec/exec_fd.c:303-307`, with no rollback traversal.


**Reasoning:** If a command or left subtree has already collected one or more heredocs and a later heredoc is interrupted or fails, `collect_heredocs()` returns immediately. Earlier descriptors remain stored and open. This affects both multiple heredocs on one command and heredocs across AST branches.


**Remediation idea:** On any collection failure, traverse the complete AST and close all heredoc descriptors collected so far. A transactional collection context with centralized ownership would make rollback reliable.


**Missing context:** Later AST destruction might eventually close them, but no immediate cleanup is shown, and persistent AST lifetime is omitted.


**Validation test:** Use two heredocs, successfully finish the first, then interrupt the second. Compare shell descriptor count before and after. Repeat with heredocs in separate pipeline commands.



## 14. Pipeline `dup2()` failures are ignored


**Classification:** Confirmed defect

**Severity:** High

**Confidence:** High

**Evidence:** `src/exec/exec_pipeline.c:58-67`.


**Reasoning:** The child unconditionally continues after `dup2(fdin, STDIN_FILENO)` and `dup2(fdout, STDOUT_FILENO)` without checking return values. If duplication fails, the command may execute against the shell’s original standard stream or a partially configured pipeline. The original descriptor is then closed regardless, eliminating recovery.


**Remediation idea:** Check every `dup2()` result. On failure, report the error, close child-owned descriptors, and `_exit()` with the internal execution error status.


**Missing context:** Under normal descriptor validity this is uncommon, but it is reachable under descriptor exhaustion, injected failures, or corrupted pipe state.


**Validation test:** Fault-inject `dup2()` failure for each pipeline side. Verify the command is not executed and the child exits nonzero without altering unrelated streams.



## 15. Successful redirection-only commands do not reset exit status to zero


**Classification:** Confirmed defect

**Severity:** Medium

**Confidence:** High

**Evidence:** `src/exec/exec_dispatch.c:56-63`.


**Reasoning:** For a command node with no command name, redirections are opened and closed and the function returns zero, but `app->exitcode` is not set to zero. If the previous command failed, `$?` remains stale even though the redirection-only command succeeded. A successful simple command containing only redirections should produce successful status.


**Remediation idea:** Set the application exit status to zero after all redirections open successfully.


**Missing context:** It is possible the caller overwrites exit status from the return value, but the rest of this module explicitly updates `app->exitcode`, so that would be inconsistent.


**Validation test:** Run a failing command, then a successful redirection-only command, then inspect `$?`. Expected result is zero. Also verify failed redirection leaves a nonzero status.



## 16. Failure to save parent standard descriptors leaves stale status and no diagnostic


**Classification:** Confirmed defect

**Severity:** Medium

**Confidence:** High

**Evidence:** `src/exec/exec_dispatch.c:14-17`.


**Reasoning:** When either `dup()` fails, the function restores whatever was successfully duplicated and returns `-1`, but does not call `setexit()` or report the syscall failure. The shell’s observable status can remain from a previous command, and the user receives no explanation.


**Remediation idea:** Preserve `errno`, close any successfully duplicated descriptor, report the failure, and set the internal execution error status.


**Missing context:** The caller returns `-1` but does not set status at `src/exec/exec_dispatch.c:65-68`.


**Validation test:** Lower the descriptor limit or inject `dup()` failure while invoking a redirected parent builtin. Assert a diagnostic and deterministic nonzero status.



## 17. `sigaction()` failures can lead to restoration from uninitialized state


**Classification:** Likely defect

**Severity:** Medium

**Confidence:** High

**Evidence:** `src/exec/exec_fd.c:254-268`.


**Reasoning:** The first `sigaction()` return value is ignored. If it fails, `old_sa` is not initialized by the syscall, yet the function later passes it to a restoration `sigaction()`. The second call is also unchecked. This can leave signal handling in an unknown state.


**Remediation idea:** Check both calls. Restore only if installation succeeded, and preserve the original error when cleanup also fails.


**Missing context:** Actual failure is uncommon with a valid signal and action, but fault injection and platform-specific failures remain possible.


**Validation test:** Interpose `sigaction()` to fail during installation and restoration. Assert no use of uninitialized action data and deterministic error propagation.



## 18. Unquoted heredoc backslash semantics do not match shell behavior


**Classification:** Confirmed defect

**Severity:** Medium

**Confidence:** High

**Evidence:** `src/exec/exec_fd.c:163-181`, especially the unconditional expansion of every eligible `$` at lines 171-179.


**Reasoning:** The expansion loop has no backslash handling. In an unquoted heredoc, a backslash can protect expansion characters and a backslash-newline pair has special treatment. Here, an input such as `\$VAR` retains the backslash but still expands `$VAR`, producing incompatible output.


The implementation also only recognizes parameter-name expansion and `$?`; broader shell expansion support is absent from this routine.


**Remediation options:**


* Implement heredoc-specific lexical expansion with backslash rules rather than a raw character scan.
* Reuse the shell’s expansion engine in a mode appropriate for heredoc bodies.
* Keep expansion entirely disabled when the delimiter was quoted.



**Missing context:** The project may intentionally implement a limited shell subset, but the requested POSIX/Bash compatibility is not met by this routine.


**Validation test:** Compare quoted and unquoted heredocs containing escaped dollar signs, escaped backslashes, backslash-newline, `$?`, and ordinary variables against Bash.



## 19. Path probing with `access()` introduces a race and incomplete executable validation


**Classification:** Architectural risk

**Severity:** Medium

**Confidence:** High

**Evidence:** `src/exec/exec_path.c:15-17`, `src/exec/exec_path.c:25-30`; actual execution occurs later at `src/exec/exec_pipeline_run.c:55-65`.


**Reasoning:** Resolution performs `access(X_OK)` and later calls `execve()`. The filesystem object can change between those operations. `X_OK` also does not establish that the target is a regular executable file; directories and malformed executable formats are resolved only later by `execve()`.


The later error handling reduces but does not eliminate semantic inconsistencies, particularly when choosing whether to continue PATH search.


**Remediation options:**


* Attempt `execve()` directly for each PATH candidate in the child and classify `errno`.
* Continue search for errors such as missing files or unsuitable candidates, while remembering permission-denied results.
* Avoid treating `access()` as authoritative.



**Missing context:** `setexecerrno()` may classify some `execve()` errors correctly, but it cannot remove the time-of-check/time-of-use race.


**Validation test:** Replace a candidate between resolution and execution, test directories named as commands, and test invalid executable formats. Compare status and diagnostics with Bash.



## 20. Descriptor restoration errors are silently ignored


**Classification:** Hardening recommendation

**Severity:** Medium

**Confidence:** High

**Evidence:** `src/exec/exec_fd.c:88-99`.


**Reasoning:** `restore_fd()` ignores failures from both `dup2()` and `close()`. If restoring standard input or output fails, the parent shell may continue with corrupted standard descriptors while reporting the builtin’s status instead of the restoration failure.


**Remediation idea:** Return a status from `restore_fd()`, check `dup2()`, and treat restoration failure as a shell-state error. Preserve the builtin’s status separately if needed for diagnostics.


**Missing context:** Current callers assume restoration cannot fail.


**Validation test:** Fault-inject a restoration `dup2()` failure after a parent builtin. Assert that the shell detects the corrupted state rather than silently continuing.



## 21. Pipeline cleanup waits on the last process twice


**Classification:** Hardening recommendation

**Severity:** Low

**Confidence:** High

**Evidence:** The last PID is waited at `src/exec/exec_dispatch.c:44`; `free_pipeops()` then calls `wait_allpids()` through the same final index at `src/exec/exec_dispatch.c:46` and `src/exec/exec_pipeline_utils.c:56-65`.


**Reasoning:** The second wait on the final PID returns `ECHILD`, which the helper silently accepts. This is not normally a correctness failure, but it obscures genuine wait errors and complicates status accounting.


**Remediation idea:** Wait each PID exactly once. Either wait all children in one routine while separately recording the last command’s status, or exclude the already-reaped PID from cleanup.


**Missing context:** None.


**Validation test:** Instrument `waitpid()` and assert exactly one successful reap per spawned PID and no expected `ECHILD` calls.



## 22. Stale global signal state may abort a new heredoc immediately


**Classification:** Unverifiable

**Severity:** Medium

**Confidence:** Low-medium

**Evidence:** The heredoc loop checks `g_signal` at `src/exec/exec_fd.c:234-235`; the handler sets it at lines 102-105. No reset appears in `collect_one_heredoc()` at lines 249-275.


**Reasoning:** If `g_signal` remains `SIGINT` from an earlier operation, the next heredoc may abort after its first input attempt even without a new signal. Whether this occurs depends on resets elsewhere in the main loop or signal utilities, which were omitted.


**Remediation idea:** Reset the heredoc signal indicator immediately before installing the heredoc handler, preferably using `volatile sig_atomic_t` state dedicated to that operation.


**Missing context:** Declaration type of `g_signal`, and all resets outside `src/exec/`.


**Validation test:** Set the global signal state to SIGINT before entering a new heredoc without delivering a signal. Verify the heredoc proceeds normally.



## Highest-priority repair order


1. Replace pipe-backed precollection of heredocs.
2. Fix `count_cmds()` to count all leaves and add bounds assertions.
3. Introduce centralized descriptor ownership and cleanup for heredocs.
4. Correct PATH splitting and continue searching after permission failures.
5. Restore parent signal state on every pipeline exit.
6. Make wait, duplication, write, and signal-installation failures explicit.
7. Add differential tests against Bash for redirection-only commands, PATH edge cases, heredoc expansion, and pipeline signal status.