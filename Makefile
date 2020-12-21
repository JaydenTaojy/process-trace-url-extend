CC=gcc
CFLAGS=-Wall -g
urlextend: urlextend.c 
	${CC} ${CFLAGS} -o urlextend urlextend.c
clean: 
	-rm $(objects) urlextend