# Parallel Programming – MPI Assignment (Mandelbrot Set)

## 1. Problem
Compute the Mandelbrot set image (WIDTH=640, HEIGHT=480) using MPI with two scheduling methods:
- Static scheduling (block partitioning)
- Dynamic scheduling (master-worker)

## 2. How I parallelized the computation
(Describe both methods; diagrams included.)

### Static scheduling
- Partition the HEIGHT rows into roughly equal contiguous blocks.
- Each process computes its block of rows.
- Results gathered to rank 0 with MPI_Gatherv and saved to a PGM image.

Block sketch (P = 4):
```
Rank 0: rows 0..119
Rank 1: rows 120..239
Rank 2: rows 240..359
Rank 3: rows 360..479
```

### Dynamic scheduling (master-worker)
- Rank 0 is master (keeps a queue of row indices).
- Workers request a row; master sends a row index; worker computes and returns the row.
- Master writes rows into the final buffer as they arrive.

Sequence sketch:
```
Master -> W1: row 0
Master -> W2: row 1
W1 -> Master: row0 data
Master -> W1: row 3
...
Master -> Wk: DONE
```

## 3. Setup (fill with your environment details)
- MPI implementation: Open MPI / MPICH (version ...)
- OS: e.g., Ubuntu 20.04
- CPU: e.g., Intel Xeon X cores
- Memory: e.g., 32 GB
- Network: e.g., Ethernet 1Gbps / InfiniBand

## 4. How to build and run
See README.md. Example:
```
make
mpirun -np 4 ./mandelbrot_dynamic
```

## 5. Results (collect these on your machine)
Run for P = 1,2,4,8 (as available) and fill the table:

| P | T(P) (s) | Speedup S(P)=T1/Tp | Efficiency E(P)=S(P)/P |
|---:|---------:|--------------------:|-----------------------:|
| 1 |         |                     |                        |
| 2 |         |                     |                        |
| 4 |         |                     |                        |
| 8 |         |                     |                        |

## 6. Analysis to include
- Plot speedup vs P and efficiency vs P.
- Compute computation/communication ratio (instrument code if desired).
- Discuss where static or dynamic scheduling performed better and why.

## 7. Conclusions
(Write conclusions based on results collected.)

