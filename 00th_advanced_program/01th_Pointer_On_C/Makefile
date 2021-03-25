#Author Kobey
CC	=	gcc

ifeq (&(CC), gccall)
RunDemo:Pointer_On_C.o
	gcc -Wall Pointer_On_C.o -o RunDemo
Pointer_On_C.o:Pointer_On_C.s
	gcc -Wall -c Pointer_On_C.s -o Pointer_On_C.o
Pointer_On_C.s:Pointer_On_C.i
	gcc -S -Wall Pointer_On_C.i -o Pointer_On_C.s
Pointer_On_C.i:Pointer_On_C.c
	gcc -E -Wall Pointer_On_C.c -o Pointer_On_C.i
else
RunDemo:Pointer_On_C.c
	gcc Pointer_On_C.c -o RunDemo
endif

.PHONY:clean
clean:
	rm RunDemo
