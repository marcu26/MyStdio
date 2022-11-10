build: libso_stdio.so

libso_stdio.so: so_stdio.o
	sudo rm -f /lib/libso_stdio.so
	gcc -shared so_stdio.o -o libso_stdio.so
	sudo cp libso_stdio.so /lib
	


so_stdio.o: so_stdio.c so_stdio.h
	gcc -fPIC -c so_stdio.c -o so_stdio.o


clean:
	rm libso_stdio.so
	rm so_stdio.o