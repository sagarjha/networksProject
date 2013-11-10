.phony: clean
all:	user server
user:	user.c md5c.c mddriver.c
	gcc user.c md5c.c mddriver.c -o user
server:	server.c md5c.c mddriver.c
	gcc server.c md5c.c mddriver.c -o server
doc:
	doxygen
clean:
	rm -rf user server *.o
