# `remake --tasks`

Here we show how you can document your targets using lines that start with `#:`

This is the `Makefile` used in this demo:

```Makefile
# Makefile to show off tracing
.PHONY: all

#: This is the default target
all: test-make

#: Tests which version of make you are running
test-make:
	@case $(MAKE) in \
	*/remake|remake) echo "Enlightended!";; \
	*/make|make) echo "This is what most folks use.";; \
	esac

: 	@bogus-command
```

![remake-tasks](remake-tasks.gif)

# `remake --trace`

Here we show POSIX shell `set -x`-like tracing. The Makefile is the same as in the previous demo.

![remake-trace](remake-trace.gif)

# `remake --search-parent`

Here we show how in contrast to GNU `make`, `remake` will look in parent directories for a `Makefile` when there is none found in the current directory.

![remake-search-parent](remake-search-parent.gif)

# How these were made:

The "cast" screenshots were made with `asciinema`. For example:

```
$ asciinema rec remake-tasks.cast
```

and then running through `asciicast2gif`.

You can edit the `.cast` files. The specific commands used after this were:

```console
$ asciicast2gif -w 60 -h 16 remake-session.cast  remake-tasks.gif
$ asciicast2gif -w 57 -h 20 remake-search-parent.cast remake-search-parent.gif
$ asciicast2gif -w 57 -h 10 remake-trace.cast remake-trace.gif
```
