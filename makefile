
CC=gcc
CC_FLAGS=-std=c99 -pedantic -Werror -Wextra -Wall -static

.PHONY: build
build: hots3

.PHONY: run
run: hots3
	./hots3

hots3: hots3.c networking.c player.c screen.c
	${CC} ${CC_FLAGS} $^ -o hots3
