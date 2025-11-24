# Mandelbrot MPI Assignment

Files:
- mandelbrot_static.c : static scheduling MPI implementation
- mandelbrot_dynamic.c: dynamic (master-worker) MPI implementation
- Makefile


Build:
```
make
```

Run examples:
```
mpirun -np 1 ./mandelbrot_static
mpirun -np 2 ./mandelbrot_static
mpirun -np 4 ./mandelbrot_static

mpirun -np 4 ./mandelbrot_dynamic
```

Outputs:
- mandelbrot_static.pgm
- mandelbrot_dynamic.pgm

Notes:
- If `mpicc`/`mpirun` are not available on your machine, install Open MPI or MPICH.
- For performance measurement, run several trials and record the printed max runtime.
