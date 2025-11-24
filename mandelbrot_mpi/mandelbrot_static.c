// mandelbrot_static.c
// Static scheduling: each rank computes a contiguous set of rows; results gathered to rank 0.
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define WIDTH 640
#define HEIGHT 480
#define MAX_ITER 255

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
    int world_rank, world_size;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int rows_per_rank = (HEIGHT + world_size - 1) / world_size; // ceil
    int start_row = world_rank * rows_per_rank;
    int end_row = start_row + rows_per_rank;
    if (end_row > HEIGHT) end_row = HEIGHT;
    int local_count = (end_row > start_row) ? (end_row - start_row) : 0;

    int *local_buf = NULL;
    if (local_count > 0) {
        local_buf = malloc(local_count * WIDTH * sizeof(int));
        if (!local_buf) { fprintf(stderr, "Alloc fail\n"); MPI_Abort(MPI_COMM_WORLD,1); }
    }

    double t0 = MPI_Wtime();

    for (int i = start_row; i < end_row; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            struct complex_t c;
            c.real = (j - WIDTH / 2.0) * 4.0 / WIDTH;
            c.imag = (i - HEIGHT / 2.0) * 4.0 / HEIGHT;
            int iter = cal_pixel(c);
            local_buf[(i - start_row)*WIDTH + j] = iter;
        }
    }

    double t1 = MPI_Wtime();
    double local_time = t1 - t0;
    double max_time;
    MPI_Reduce(&local_time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    int (*image)[WIDTH] = NULL;
    int *recvcounts = NULL;
    int *displs = NULL;
    if (world_rank == 0) {
        image = malloc(sizeof(int) * HEIGHT * WIDTH);
        recvcounts = malloc(world_size * sizeof(int));
        displs = malloc(world_size * sizeof(int));
        for (int r=0;r<world_size;++r) {
            int rs = r * rows_per_rank;
            int re = rs + rows_per_rank;
            if (re > HEIGHT) re = HEIGHT;
            int cnt = (re > rs) ? (re - rs) * WIDTH : 0;
            recvcounts[r] = cnt;
        }
        displs[0] = 0;
        for (int r=1;r<world_size;++r) displs[r] = displs[r-1] + recvcounts[r-1];
    }

    MPI_Gatherv(local_buf, local_count*WIDTH, MPI_INT,
                image, recvcounts, displs, MPI_INT,
                0, MPI_COMM_WORLD);

    if (world_rank == 0) {
        save_pgm("mandelbrot_static.pgm", (int (*)[WIDTH])image);
        printf("Static scheduling: max runtime = %f s\n", max_time);
    }

    if (local_buf) free(local_buf);
    if (image) free(image);
    if (recvcounts) free(recvcounts);
    if (displs) free(displs);

    MPI_Finalize();
    return 0;
}
