.SECONDEXPANSION:
foo.x:

foo.%: $$(%_a) $$(%_b) bar
	@:

foo.x: x_a := bar

%.x: x_b := baz

bar baz: ; @echo '$@'
