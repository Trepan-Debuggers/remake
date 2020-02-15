PACKAGE=make

all: $(PACKAGE).txt

$(PACKAGE).txt: ../doc/remake.texi
	makeinfo --no-headers $< > $@
