all: foo
	$(warning "done")

foo: 
	$(warning "hi")
	$(debugger )
	$(warning "there")
