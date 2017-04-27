foo: bar | baz
	@echo '$$^ = $^'
	@echo '$$| = $|'
	touch $@

.PHONY: baz

bar baz:
	touch $@
