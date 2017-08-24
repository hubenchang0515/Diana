diana.so : diana_io.o fdpopen.o
	gcc -O2 -shared -W -Wall -o diana.so diana_io.o fdpopen.o
	
diana_io.o : diana_io.c diana_io.h
	gcc -O2 -W -Wall -fPIC -c diana_io.c
	
fdpopen.o : fdpopen.c fdpopen.h
	gcc -O2 -W -Wall -fPIC -c fdpopen.c

clean :
	rm *.o