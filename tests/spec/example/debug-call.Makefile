first: second
	$(debugger 2) 
	@echo $@ done
second: third
	@echo second done
third: 
	echo third done
