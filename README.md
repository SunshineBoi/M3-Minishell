*This project has been created as part of the 42 curriculum by lkai-yua, kong.*

# minishell

## Description

`minishell` is a simplified re-implementation of a POSIX-like shell (bash-inspired),
built from scratch in C as part of the 42 core curriculum. The goal of the project is
to understand, at a low level, how a shell actually works: reading a line of input,
turning it into tokens, parsing it into a command structure, expanding variables and
quotes, and finally executing the result as one or more processes connected by pipes
and redirections.

The shell supports:

- An interactive prompt built on [GNU Readline](https://tiswww.case.edu/php/chet/readline/rltop.html) (with history navigation).
- Simple commands and pipelines (`cmd1 | cmd2 | cmd3`).
- Redirections: `<`, `>`, `>>`, and heredocs (`<<`).
- Single (`'...'`) and double (`"..."`) quoting, with correct expansion rules for each.
- Environment variable expansion (`$VAR`, `$?`) and unquoted word splitting.
- Built-in commands implemented internally: `echo`, `cd`, `pwd`, `export`, `unset`,
  `env`, `exit`.
- External commands resolved and executed via `PATH`, with proper `fork`/`execve` handling.
- Signal handling that mimics bash's behaviour for `Ctrl-C`, `Ctrl-\`, and `Ctrl-D`,
  both at the prompt and while a child process is running.

`&&`, `||`, `;` and subshells `( )` are intentionally out of scope for this project.

## Instructions

### Requirements

- A C compiler (`cc`/`gcc`/`clang`).
- GNU `make`.
- The `readline` development library (`libreadline-dev` on Debian/Ubuntu,
  `readline` via Homebrew on macOS).
- Optional, for the test suite: [Criterion](https://github.com/Snaipe/Criterion).

### Build

```sh
make        # builds the ./minishell executable
make re     # full rebuild
make clean  # remove object files
make fclean # remove object files and the executable
```

### Run

```sh
./minishell
```

This opens an interactive prompt. Examples:

```sh
minishell$ echo "Hello, $USER" | wc -c
minishell$ cat < input.txt | grep foo >> output.txt
minishell$ export MY_VAR=42 && echo $MY_VAR   # note: && is not supported, use two lines
```

Exit the shell with `exit`, or `Ctrl-D` on an empty line.

### Tests

Unit tests (built with [Criterion](https://github.com/Snaipe/Criterion)) cover the
lexer, parser, AST, expander, environment handling, builtins, and redirections:

```sh
make test
```

An interactive signal-behaviour test (spawns the shell via a pty and asserts on
`Ctrl-C`/`Ctrl-\` behaviour) is also available:

```sh
python3 test/test_signals_interactive.py
```

## Technical Overview

Every line of input flows through the same pipeline: **Lexer → Validator → Parser →
Expander → Executor**. See [`_resource/ARCHITECTURE.md`](_resource/ARCHITECTURE.md)
for a deep dive with diagrams; the summary below is organized by module.

### Lexer

Converts the raw input line into a singly-linked list of tokens (`t_tokensll`),
character by character. `build_tokensll` (`lexer.c`) walks the string, skips
whitespace, and calls `build_token` once per token, which dispatches into
`utils_lexer.c`:

- **Words** (`TOK_STR`) — `string_build` glues runs of quoted and unquoted
  characters into a single token, stopping only at whitespace or an unquoted
  operator character. A quoted span is consumed in one shot by
  `quotes_build`/`_quotes_build_helper` (`utils_lexer_build.c`), which scan only
  for the matching closing quote — anything in between, including whitespace or
  `|`/`<`/`>`, is swallowed as literal content rather than treated as a token
  boundary (e.g. `echo "a|b"` lexes to two tokens, not three, and produces no
  `TOK_PIPE`). An unclosed quote is reported as a syntax error (`ERR_QUOTE`).
- **Operators** (`utils_lexer_token.c`) — `|`, `<`, `>`, `>>`, `<<`, recognized by
  `special_build` even when not separated by whitespace (e.g. `echo hi>out`).
- **Escapes** — an unquoted `\` (`backslash_build`) consumes itself and appends the
  following character literally, stripping its special meaning, matching bash for
  the unquoted case. A trailing `\` with nothing after it is instead kept as a
  literal backslash, a simplification — real bash would prompt for a continuation
  line here. Backslash gets no special handling inside quotes at the lexer stage.

Quotes and `$` are kept **raw** in each token's value at this stage — the quote
characters themselves are copied verbatim into `token->val` — nothing is expanded
or stripped yet; that happens later in the Expander (`handle_quotes` strips the
quote characters while `expand_word` interprets `$`). A malloc failure anywhere in
the pipeline frees what's been built so far and calls `hardexit`; a quote error
instead frees and returns `NULL`, leaving the syntax-error reporting to the caller.
List bookkeeping — `init_token`, `freetoken`, `freetokensll`, and the `t_sll_ops`
traversal helpers used while linking tokens together — lives in `utils_sll.c`.

A dedicated validation pass then walks the finished token list and rejects illegal
grammar before any parsing happens: `is_valid_pipe`/`is_valid_redir`
(`utils_validator_tokens.c`) check leading/trailing/doubled pipes, redirections
with no target, and a redirection immediately followed by a pipe or another
redirection; `validate_tokensll` (`validator_tokens.c`) drives the walk and also
rejects unsupported operators (`;`, `&`, `&&`, `(`, `)`) that got lexed as plain
strings, since this shell doesn't implement them. Any failure sets the shell's exit
code to `2`, matching bash.

### AST

The parser output is a small binary tree of two node kinds:

- `NODE_CMD` — one command: a `NULL`-terminated `argv`, plus a singly-linked list of
  `t_redir` (type, target, resolved fd) in encounter order.
- `NODE_BINOP` — currently only `BIN_PIPE` is produced by the parser (`BIN_AND`,
  `BIN_OR`, `BIN_SEQ` exist in the enum for future extension but are not wired up),
  connecting a `left`/`right` subtree.

Each node carries a `t_span {start, end}` — token indices, not byte offsets — for
error reporting. It's populated as a byproduct of how far the parser's cursor moved
while building the node, not computed separately. The whole tree is freed recursively
via `ast_free`.

### Parser

A small recursive-descent parser (`t_parser` cursor over the token list) turns the
validated token stream into an AST:

```
parse_tokens()
  └─ parse_pipeline()
       └─ parse_simple_command()   → one NODE_CMD (argv + redirs)
       while next token is '|': wrap left/right in a NODE_BINOP(BIN_PIPE)
```

**Building a command node.** `parse_simple_command` keeps consuming tokens as long as
the current one is a word (`TOK_STR`) or one of the four redirection operators — that
whitelist, not a specific check for `|`, is what ends the loop; `|` (or end of input)
simply isn't in it. Each iteration re-checks the current token and dispatches:

- a word → pushed onto a growable `argv` array (`t_argv_builder`: starts at capacity
  4, doubles on demand, always keeps a trailing `NULL` so it's a valid
  `execve`-ready array at every point, not just at the end);
- a redirection operator → its type is captured, the cursor advances, and the *next*
  token must be a word (its value becomes the redirect's target) or parsing fails;
  the resulting `t_redir` is appended to the tail of the command's redir list.

Because both cases live in the same loop, arguments and redirections can interleave
freely in the source (`cat < in out > final` is legal), rather than needing separate
argument/redirection phases.

**Two distinct ways this returns nothing.** A single failed word or redirect mid-loop
aborts immediately. But even a loop that completes normally produces `NULL` if it
collected zero words and zero redirects — this is what turns `cmd |` (or `| cmd`)
into a syntax error: after consuming the pipe, the next call to
`parse_simple_command` sees no valid tokens to consume, returns `NULL`, and the
pipeline builder above propagates that failure instead of accepting an empty side.

**Folding the pipeline, left to right.** `parse_pipeline` builds the first command as
`left`, then loops while it sees `TOK_PIPE`: advance past it, parse the next command
as `right`, and immediately wrap `(left, right)` in a new `BIN_PIPE` node that
becomes `left` for the next iteration. This is iterative rather than recursive
specifically so `a | b | c` folds as `(a | b) | c` — left-associative, matching how a
pipeline is conceptually a flat left-to-right chain rather than right-nested.

**Redirections as a plain linked list, not a growable array like argv.**
Linked list also preserves encounter order for free, which matters because later redirects override earlier ones on the same fd (`cmd > a > b` — `b` wins), and the same `t_redir` node carries the `fd` field used later at execution time.

### Expander

_`src/expander/`_

Walks the AST and rewrites every raw word into its final form. `expand_ast` recurses
through `NODE_BINOP`s the same way the AST itself is shaped, and for each `NODE_CMD`
calls `expand_cmd_node`, which runs three passes over the command, in this fixed
order:

1. `process_assignments` (`expand_assignment.c`) — peels leading `NAME=value` tokens
   off `argv` and applies them straight to the shell's environment.
2. `expand_argv` — expands and field-splits every remaining argv word.
3. `expand_redirs` — expands every redirection target, collapsing it back down to
   exactly one word.

**Leading assignments (`expand_assignment.c`).** `is_valid_assignment` recognizes
`NAME=value` (POSIX identifier rules via `is_name_start`) and splits it into
`name`/raw value; `process_assignments` loops as long as `argv[0]` matches: the
value is expanded and set into the environment immediately (`apply_assignment` →
`env_set`/`update_env_array`), then the word is shifted out of `argv`, and the loop
re-checks the new `argv[0]`. Because each assignment updates `app->envp` right away,
later assignments in the same prefix see earlier ones (`A=1 B=$A cmd` resolves `B` to
`1`). One quirk: a raw value ending in a literal, unquoted `;` has that trailing `;`
silently stripped (`FOO=bar;` sets `FOO` to `bar`, not `bar;`) — a workaround for this
shell not lexing `;` as a statement separator at all, so a semicolon typed at the end
of a bash-style line doesn't leak into the value. Another deviation from bash: these
assignments are **not** scoped to the one command — nothing restores the environment
afterward, so `FOO=bar echo hi` leaves `FOO=bar` set in the shell for good, unlike
bash's temporary-environment behaviour for that single command.

**Core primitive: `expand_word`.** Everything above funnels through here — it's
called once per remaining argv word, once per redirect target, and once per
assignment's raw value. It runs a character-by-character state machine
(`t_expand_ctx`, `expand_step`) over the raw token, tracking quote state (`t_qstate`:
`Q_NONE` / `Q_SQUOTE` / `Q_DQUOTE`), an accumulating string buffer (`t_strbuf`) for
the word currently being built, and a `t_wordlist` for words already completed. Each
`expand_step` call tries, in order: `handle_whitespace` → `handle_quotes` →
`handle_backslash` → `handle_variable` → fall through to a literal copy of the
current byte. These checks are mutually exclusive on the byte itself (whitespace vs.
quote char vs. `\` vs. `$`), so the ordering is really about layering — boundary
detection, then quote-mode toggles, then escapes, then substitution, with literal
copy as the catch-all.

- **Quote toggling** (`handle_quotes`) — `'`/`"` are never copied, they flip
  `ctx->state` (guarded so a quote char of the *other* kind is ignored while already
  inside a quote, e.g. `"it's"` stays intact). Single quotes make everything until
  the closing `'` fully literal — no `$`, no backslash escapes. Double quotes still
  expand `$`, and additionally recognize backslash escapes for `\`, `"`, `$` only
  (`handle_backslash`); any other backslash is just a literal character.
- **Whitespace** (`handle_whitespace`) only splits a word when `ctx->state ==
  Q_NONE`. In practice this branch never fires during the raw token scan: the lexer
  already splits on every unquoted whitespace when building tokens, so any
  whitespace still present inside a single token got there *because* it was inside
  quotes — meaning `ctx->state` is never `Q_NONE` at that point. The only place
  unquoted-whitespace splitting actually happens is a second, independent path — see
  field splitting below.
- **Variable expansion** (`handle_variable`) — dispatch depends on both what follows
  the `$` and the current quote state:

  | Input pattern (at `ctx->i`) | Guard | Action | Unquoted (`Q_NONE`) | Double-quoted (`Q_DQUOTE`) | Single-quoted (`Q_SQUOTE`) |
  |---|---|---|---|---|---|
  | `$?` | `state != Q_SQUOTE` | `handle_status` — renders `last_status` as digits | Splits like any expanded text (status is always digits, so effectively still one word) | Appended literally via `sb_push_str`, no split | Never reached — literal `$?` chars copied as-is |
  | `$0`–`$9` | `state != Q_SQUOTE` | Consumes `$` + digit, appends **nothing** | Silently disappears (no positional params) | Same — silently disappears | Never reached — literal chars |
  | `$NAME` (valid identifier start) | `state != Q_SQUOTE` | `handle_env_var` — `env_lookup`, empty string `""` if unset | Value is field-split via `append_unquoted_text` (spaces in value → multiple words; unset → contributes nothing) | Value appended literally via `sb_push_str`; unset still forces `word_in_progress = 1` (empty-string arg, matches bash's `"$UNSET"` → empty arg) | Never reached — literal `$NAME` chars |
  | `$` followed by anything else (e.g. `$$`, `$ `, `$@`) | `state != Q_SQUOTE` | Not `?`, not digit, not name-start → falls to the last branch inside `handle_variable` | Literal `$` char pushed, next char handled on the following iteration | Same — literal `$` | Never reached — literal `$` |
  | Any `$...` | `state == Q_SQUOTE` | Guard fails entirely, `handle_variable` returns 0 immediately | — | — | Falls through to raw-copy default: `$` copied as a plain character, no special meaning at all |

- **Field splitting has two separate mechanisms, not one:**
  - Ordinary/escaped/quoted characters accumulate into the string buffer one at a
    time and only get flushed to the `t_wordlist` at the end of the token (or when
    whitespace is hit in `Q_NONE`, which — per above — doesn't actually happen
    mid-token on raw input).
  - An **expanded variable's value**, when substituted in `Q_NONE`, is instead
    pushed through `append_unquoted_text`, which re-scans *that value* for
    whitespace and flushes a completed word to the `t_wordlist` immediately at each
    space — this is what lets `echo $VAR` (where `$VAR="a b"`) become two argv
    entries. Whitespace found this way is judged purely on the substituted text; if
    the value happens to contain a literal quote character, that's just data to this
    function, not a delimiter. Inside double quotes, the same value is instead
    appended as one literal blob (`sb_push_str`, no splitting) — even an
    unset/empty `"$VAR"` still counts as one empty-string word, matching bash.

`expand_word` reports its result in a single `t_wordlist *out` (`{items, count}`),
which is structurally identical to the parser's `t_argv_builder` — kept as a separate
type purely to keep "fragments from splitting one word" (the expander's concern)
distinct from "the whole command's final argv" (the parser/exec's concern), not
because the growth logic differs.

`expand_argv` calls `expand_word` once per remaining argv word and flattens every
resulting `t_wordlist` into the command's new argv via `push_words_to_builder`.
`expand_redirs` calls it once per redirection target and additionally enforces that
the result collapses to **exactly one**, non-empty word — anything else is reported
as an ambiguous redirect (`ERR_CMDNEXEC`), matching bash.

### Exec

_`src/exec/`_

Turns the expanded AST into real processes:

- `exec_pipeline*.c` — counts the commands in a pipeline, opens the pipes, `fork`s
  one child per command, and wires `stdin`/`stdout` through `dup2` before `execve`.
- `exec_fd.c` — opens/duplicates redirection file descriptors per command
  (`<`, `>`, `>>`), including `collect_heredocs`, which reads all `<<` delimiters
  **before** the pipeline is spawned to avoid a deadlock between parent and child
  writing/reading on the heredoc pipe.
- `exec_path*.c` — resolves a command name against `PATH` (splitting it, joining each
  entry with the command, checking executability).
- `exec_dispatch.c` — the top-level entry point (`execute_ast`): runs a single builtin
  directly in the parent process when the AST is one bare `NODE_CMD` and the command is
  a builtin (so state like `cd`/`export` persists in the shell), otherwise spawns the
  full pipeline via `fork`.

Exit statuses follow POSIX conventions (`t_exitcode` in `include/minishell.h`):
`126` for found-but-not-executable, `127` for command-not-found, `128 + signum` for a
child killed by a signal.

### Builtins

Builtins are dispatched by `exec_builtin`/`is_builtin` (`builtin_dispatch.c`) and each
lives in its own file:

| Builtin  | Notes |
|----------|-------|
| `echo`   | <ul><li>Prints its arguments space-separated with a trailing newline.</li><li>Accepts a leading run of valid `-n` flags (`-n`, `-nn`, `-nnn`, ...) to suppress that newline.</li><li>`-na` and similar are *not* recognized as flags — printed as a literal argument instead, matching bash.</li></ul> |
| `cd`     | <ul><li>No args → `$HOME` (error if unset).</li><li>`-` → `$OLDPWD` (error if unset); like bash, the resolved path is printed to stdout.</li><li>Otherwise `chdir`s straight to the given argument.</li><li>Rejects `cd a b` with "too many arguments" and leaves the directory unchanged.</li><li>After a successful `chdir`, sets `OLDPWD` to the previous `getcwd()` result and `PWD` to the new one.</li></ul> |
| `pwd`    | <ul><li>Prints `getcwd()` followed by a newline.</li><li>On failure (e.g. the cwd was deleted out from under the shell), reports `strerror(errno)` and returns an error status instead.</li></ul> |
| `export` | <ul><li>No args → prints every variable, sorted alphabetically, as `declare -x NAME="value"` (or bare `declare -x NAME` if it has no value).</li><li>`NAME=value` sets/updates the variable via `env_set`.</li><li>`NAME` alone marks it exported without a value, but only if it doesn't already exist — an existing value is left untouched.</li><li>Identifiers are validated via `is_valid_identifier` (letter/`_` first, then alnum/`_`); invalid ones report an error and are skipped, but the remaining arguments are still processed.</li></ul> |
| `unset`  | <ul><li>Removes each named variable via `env_unset`.</li><li>Invalid identifiers report an error and set the exit status to `1`, but don't stop the rest of the arguments from being processed.</li><li>Unsetting a variable that doesn't exist is a silent no-op, matching bash.</li></ul> |
| `env`    | <ul><li>Prints every variable that currently has a (non-`NULL`) value as `KEY=value`.</li><li>Variables created via a valueless `export NAME` are excluded, mirroring real env's "exported but unset" behaviour.</li></ul> |
| `exit`   | <ul><li>Always prints `exit` to stderr first, like bash.</li><li>No args → exits with the shell's last recorded exit status.</li><li>`exit N M` → "too many arguments", returns `1` *without* exiting.</li><li>A non-numeric argument → "numeric argument required", exits with status `2`.</li><li>A numeric argument exits with `(unsigned char)value`, i.e. wraps mod 256 (`exit 300` → status `44`, `exit -1` → status `255`).</li></ul> |

The environment itself is kept as a linked list (`t_env` in `src/envp/envp.c`), rebuilt
into a flat `char **envp` array (`update_env_array`) whenever it changes, so that both
`execve` and the builtins always see a consistent view of the environment.

### Signals

_`src/signals.c`_

Signal handling switches disposition depending on what the shell is currently doing,
mirroring bash's own behaviour:

| Context                              | `Ctrl-C` (SIGINT)                                   | `Ctrl-\` (SIGQUIT) |
|---------------------------------------|------------------------------------------------------|--------------------|
| At the prompt (`signals_at_prompt`)   | Cancel the current line, print a newline, redraw a fresh prompt (`handlesig_prompt` uses `rl_replace_line`/`rl_on_new_line`/`rl_redisplay`) | Ignored |
| Executing a pipeline (`signals_ignore` in parent, `signals_default` in each child) | Parent ignores it; the child (which has default disposition) is killed by the kernel, and the parent reports `128 + SIGINT` via `waitpid` | Child dumps core and prints `Quit (core dumped)`; parent is unaffected |
| Reading a heredoc                     | Interrupts collection, discards what was read, returns to a fresh prompt | Ignored |

`g_signal` is the single `volatile sig_atomic_t` global allowed by the subject; it only
records *that* a signal was caught so the main loop can react (`handle_sigint_in_main`)
— all actual state mutation happens outside the handler.

## Resources

### Classic references

- [Bash Reference Manual](https://www.gnu.org/software/bash/manual/bash.html) — ground truth for quoting, expansion, and exit-status rules.
- [GNU Readline Library manual](https://tiswww.case.edu/php/chet/readline/readline.html) — prompt/history API used for the REPL.
- [POSIX Shell & Utilities specification](https://pubs.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html) — the formal grammar minishell approximates.
- `man 2 fork`, `man 2 execve`, `man 2 pipe`, `man 2 dup2`, `man 2 waitpid`, `man 3 readline`, `man 7 signal` — the syscalls/library calls the executor and signal handling are built on.
- [Writing a simple shell in C](https://brennan.io/2015/01/16/write-a-shell-in-c/) — a widely-referenced introductory walkthrough of the fork/exec/wait shell loop.

### Use of AI

Claude (Anthropic) was used throughout this project as a learning and review tool. Specific uses:

- **Conceptual explanation** — understanding the overall scope of the project (what a POSIX shell is actually expected to do end to end: tokenizing, parsing, expansion, process/pipeline management, signal handling) and the theory behind unfamiliar mechanics — signal disposition across `fork`, why heredocs need to be collected before the pipeline starts to avoid a deadlock, quoting/expansion/field-splitting rules — before writing any code against them.
- **Architecture review** — discussing how to split the project into modules (lexer/parser/AST/expander/exec/builtins/signals), sanity-checking the data flow between them, and reviewing whether a given design decision (e.g. running builtins in-process vs. forking, where to keep environment state) fit the project's scope and constraints.
- **Debugging assistance** — helping narrow down the root cause of crashes, hangs, and incorrect exit codes during development.

All core logic, implementation, and final decisions were written and verified manually. Claude was used to ask questions, review drafts, and reason through edge cases.