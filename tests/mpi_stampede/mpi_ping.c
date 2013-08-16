#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MSG_SIZE 1048576

int main (int argc, char *argv[]) {
    double time_end, time_start;
    int count, rank, end, x;
    char *buffer;
    MPI_Status status;

    if (3 > argc) {
      printf("Insuficient arguments (%d), argc\n");
      printf("Usage: ping <times> <delay> [pong]\n");
      exit(1);
    }

    buffer = (char *) malloc(MSG_SIZE * sizeof(char));

    for (x = 0; x < MSG_SIZE; x++) {
        buffer[x] = 'T';
    }

    if (MPI_Init(&argc, &argv) == MPI_SUCCESS) {
        time_start = MPI_Wtime();
        MPI_Comm_size (MPI_COMM_WORLD, &count);
        MPI_Comm_rank (MPI_COMM_WORLD, &rank );
        for (end = 1; end <= atoi(argv[1]); end++) {
            if (rank == 0) {
                printf("(%d) sent token to (%d)\n", rank, rank+1);
                sleep(atoi(argv[2]));
                if (4 == argc) {
                    for (x = 0; x < atoi(argv[3]); x++) {
                        printf("(%d) pong %d\n", rank, x);
                        sleep(atoi(argv[2]));
                    }
                }
                MPI_Send(buffer, MSG_SIZE, MPI_CHAR, 1, 1, MPI_COMM_WORLD);
                MPI_Recv(buffer, MSG_SIZE, MPI_CHAR, count-1, 1, MPI_COMM_WORLD,
                         &status);
            } else {
                MPI_Recv(buffer, MSG_SIZE, MPI_CHAR, rank-1, 1, MPI_COMM_WORLD,
                         &status);
                sleep(atoi(argv[2]));
                if (4 == argc) {
                    for (x = 0; x < atoi(argv[3]); x++) {
                        printf("(%d) pong %d\n", rank, x);
                        sleep(atoi(argv[2]));
                    }
                }
                MPI_Send(buffer, MSG_SIZE, MPI_CHAR,
                         (rank==(count-1) ? 0 : rank+1), 1, MPI_COMM_WORLD);
            }
        }
    }
    time_end = MPI_Wtime();
    MPI_Finalize();

    if (rank == 0) {
        printf("%f\n", time_end - time_start);
    }

    return 0;
}
