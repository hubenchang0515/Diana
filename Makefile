CC = gcc
FLAGS = -O2 -W -Wall -fPIC

.PHONY : install clean 

diana.so : diana_io.o fdpopen.o diana_socket.o diana_signal.o diana_epoll.o
	$(CC) $(FLAGS) -shared -o diana.so diana_io.o fdpopen.o diana_socket.o diana_signal.o diana_epoll.o
	
diana_io.o : diana_io.c diana_io.h
	$(CC) $(FLAGS) -c diana_io.c
	
fdpopen.o : fdpopen.c fdpopen.h
	$(CC) $(FLAGS) -c fdpopen.c

diana_socket.o : diana_socket.c diana_io.h
	$(CC) $(FLAGS) -c diana_socket.c

diana_signal.o : diana_signal.c
	$(CC) $(FLAGS) -c diana_signal.c

diana_epoll.o : diana_epoll.c diana_io.h
	$(CC) $(FLAGS) -c diana_epoll.c

install : diana.so
	./install.lua

clean :
	rm *.o
