
# info
# https://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html

COMPILER=gcc
COMPILER_FLAGS=-std=c99 -pedantic -Werror -Wextra -Wall -static
CC=${COMPILER} ${COMPILER_FLAGS}

.PHONY: build
build: hots3

.PHONY: run
run: hots3
	./hots3

.PHONY: clean
clean:
	rm hots3

hots3: makefile hots3.c networking.c networking.h player.c player.h screen.c screen.h map.c map.h hero.c hero.h
	${CC} ${CC_FLAGS} $(filter-out $<,$^) -o hots3
