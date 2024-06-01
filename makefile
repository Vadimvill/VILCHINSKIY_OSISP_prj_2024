CC = gcc
CFLAGS = -W -Wall -Wno-unused-parameter -Wno-unused-variable -std=c11 -pedantic 

all: fileshare

fileshare: server.c
	$(CC) $(CFLAGS) server.c fileshare.c fwrapper.c globalconstants.c -o fileshare -lcrypto


clean:
	rm -f fileshare
