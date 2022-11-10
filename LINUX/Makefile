build: libso_stdio.so

libso_stdio.so: so_stdio.o
	gcc -shared so_stdio.o -o libso_stdio.so
	


so_stdio.o: so_stdio.c so_stdio.h
	gcc -fPIC -c so_stdio.c -o so_stdio.o


clean:
	rm libso_stdio.so
	rm so_stdio.o