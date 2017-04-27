# Makefile to show off tracing
.PHONY: all
all: foo

foo:
	@case $(MAKE) in \
	*/remake|remake) echo "Enlightened!";; \
	*/make|make) echo "This is what most folks use.";; \
	esac
	@bogus-command
