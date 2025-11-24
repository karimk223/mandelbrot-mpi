CC=mpicc
CFLAGS=-O2

all: mandelbrot_static mandelbrot_dynamic

mandelbrot_static: mandelbrot_static.c
	$(CC) $(CFLAGS) -o mandelbrot_static mandelbrot_static.c

mandelbrot_dynamic: mandelbrot_dynamic.c
	$(CC) $(CFLAGS) -o mandelbrot_dynamic mandelbrot_dynamic.c

clean:
	rm -f mandelbrot_static mandelbrot_dynamic *.pgm
