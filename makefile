GXX := gcc -w -ansi -pedantic -Wall
GXXDEBUG := gcc -DDEBUG -w -ansi -pedantic -Wall

%: %.c
	$(GXX) $< -o $@.out
%-debug: %.c
	$(GXXDEBUG) $< -o $@.out

clean: *.out
	rm *.out
	rm core