// mandelbrot_dynamic.c
// Dynamic scheduling (master-worker): master assigns next row on demand; workers compute and return rows.
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define WIDTH 640
#define HEIGHT 480
#define MAX_ITER 255
#define TAG_TASK 1
#define TAG_RESULT 2
#define TAG_DONE 3

struct complex_t { double real, imag; };

int cal_pixel(struct complex_t c) {
    double z_real = 0, z_imag = 0;
    double z_real2, z_imag2, lengthsq;
    int iter = 0;
    do {
        z_real2 = z_real * z_real;
        z_imag2 = z_imag * z_imag;
        z_imag = 2 * z_real * z_imag + c.imag;
        z_real = z_real2 - z_imag2 + c.real;
        lengthsq = z_real2 + z_imag2;
        iter++;
    } while ((iter < MAX_ITER) && (lengthsq < 4.0));
    return iter;
}

void save_pgm(const char *filename, int image[HEIGHT][WIDTH]) {
    FILE *pgm = fopen(filename, "wb");
    if (!pgm) { perror("fopen"); return; }
    fprintf(pgm, "P2\n%d %d\n255\n", WIDTH, HEIGHT);
    for (int i=0;i<HEIGHT;i++){
        for (int j=0;j<WIDTH;j++){
            fprintf(pgm, "%d ", image[i][j]);
        }
        fprintf(pgm, "\n");
    }
    fclose(pgm);
}

int main(int argc, char **argv) {
    int rank, size;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&size);

    if (size < 2) {
        if (rank==0) fprintf(stderr,"Need at least 2 processes for dynamic scheduling (master + workers)\n");
        MPI_Finalize(); return 1;
    }

    int (*image)[WIDTH] = NULL;
    if (rank == 0) image = malloc(sizeof(int)*HEIGHT*WIDTH);

    double t0 = MPI_Wtime();

    if (rank == 0) {
        int next_row = 0;
        int active = size - 1;
        // initial dispatch
        for (int w=1; w<size && next_row < HEIGHT; ++w) {
            MPI_Send(&next_row, 1, MPI_INT, w, TAG_TASK, MPI_COMM_WORLD);
            next_row++;
        }
        // service loop
        while (active > 0) {
            MPI_Status status;
            int row_idx;
            MPI_Recv(&row_idx, 1, MPI_INT, MPI_ANY_SOURCE, TAG_RESULT, MPI_COMM_WORLD, &status);
            int src = status.MPI_SOURCE;
            int *row_buf = malloc(WIDTH * sizeof(int));
            MPI_Recv(row_buf, WIDTH, MPI_INT, src, TAG_RESULT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            // store
            if (row_idx >= 0 && row_idx < HEIGHT) {
                for (int j=0;j<WIDTH;j++) image[row_idx][j] = row_buf[j];
            }
            free(row_buf);
            if (next_row < HEIGHT) {
                MPI_Send(&next_row, 1, MPI_INT, src, TAG_TASK, MPI_COMM_WORLD);
                next_row++;
            } else {
                int end = -1;
                MPI_Send(&end, 1, MPI_INT, src, TAG_DONE, MPI_COMM_WORLD);
                active--;
            }
        }
    } else {
        // worker loop
        while (1) {
            MPI_Status status;
            int row_idx;
            MPI_Recv(&row_idx, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if (status.MPI_TAG == TAG_DONE || row_idx == -1) break;
            int *row_buf = malloc(WIDTH * sizeof(int));
            for (int j=0;j<WIDTH;j++) {
                struct complex_t c;
                c.real = (j - WIDTH / 2.0) * 4.0 / WIDTH;
                c.imag = (row_idx - HEIGHT / 2.0) * 4.0 / HEIGHT;
                row_buf[j] = cal_pixel(c);
            }
            MPI_Send(&row_idx, 1, MPI_INT, 0, TAG_RESULT, MPI_COMM_WORLD);
            MPI_Send(row_buf, WIDTH, MPI_INT, 0, TAG_RESULT, MPI_COMM_WORLD);
            free(row_buf);
        }
    }

    double t1 = MPI_Wtime();
    double local_time = t1 - t0;
    double max_time;
    MPI_Reduce(&local_time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        save_pgm("mandelbrot_dynamic.pgm", (int (*)[WIDTH])image);
        printf("Dynamic scheduling: max runtime = %f s\n", max_time);
    }

    if (image) free(image);
    MPI_Finalize();
    return 0;
}
