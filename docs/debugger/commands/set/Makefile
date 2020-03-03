# Whatever it is you want to do, it should be handled by the
# by the main (parent) Makefile. So reissue make from there.
.PHONY: html

html:
	$(MAKE) -C ../.. $@

%: html
	$(MAKE) -C ../.. $@
