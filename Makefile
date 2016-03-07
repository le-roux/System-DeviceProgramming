CFLAGS := -std=gnu99 -lpthread
PHONY := clean

global : Lab1.1 Lab1.2 Lab1.3

clean : 
	rm -f Lab1.1
	rm -f Lab1.2
	rm -f Lab1.3
	rm -f fv*
