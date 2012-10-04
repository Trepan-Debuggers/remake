# Whatever it is you want to do, it should be handled by the 
# by the main (parent) Makefile. So reissue make from there.
.PHONY: all

all:
	$(MAKE) -C ../.. $@

%: all
	$(MAKE) -C ../.. $@
