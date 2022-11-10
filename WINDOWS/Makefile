build: so_stdio.dll

so_stdio.lib: so_stdio.dll

so_stdio.dll: so_stdio.obj
	link /nologo /dll /out:so_stdio.dll /implib:so_stdio.lib so_stdio.obj

so_stdio.obj: so_stdio.c
	cl /c /DDLL_EXPORTS so_stdio.c

clean:
	del so_stdio.obj
	del so_stdio.lib
	del so_stdio.dll
	del so_stdio.exp