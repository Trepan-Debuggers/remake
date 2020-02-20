first: second
	@echo first done

#: This is the second target
second: third
	@echo second done

#: This is the third target
third: 
	echo third done
