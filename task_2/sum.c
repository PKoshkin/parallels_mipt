#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char** argv) {
    int process_rank, process_number, N;
    MPI_Status Status;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &process_number);
    MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);

    // Check that there are enough arguments
    if (argc < 2) {
        printf("Not enough arguments!\n");
        MPI_Finalize();
        return 0;
    }

    // Getting number of elements from command line
    N = atoi(argv[1]);

    // elements to sum in each process
    size_t part_length = N / (process_number - 1);

    // Base process
    if (process_rank == 0) {
        int array[N];
        for (int i = 0; i < N; ++i) {
            array[i] = i;
        }

        // Sending elements to other processes
        for (int i = 1; i < process_number; ++i) {
            MPI_Send(&array[(i - 1) * part_length], part_length, MPI_INT, i, i, MPI_COMM_WORLD);
        }

        // May be we have to sum some elements in base process
        int base_process_sum = 0;
        if (N % (process_number - 1) != 0) {
            for (int i = (process_number - 1) * part_length; i < N; ++i) {
                base_process_sum += array[i];
            }
        }

        for (int i = 1; i < process_number; ++i) {
            int tmp_sum;
            MPI_Recv(&tmp_sum, 1, MPI_INT, i, i * 2, MPI_COMM_WORLD, &Status);
            base_process_sum += tmp_sum;
            printf("Sum from process number %d: %d\n", i, tmp_sum);
        }

        int simple_sum = 0;
        for (int i = 0; i < N; ++i) {
            simple_sum += array[i];
        }
        printf("Sum by different processes: %d\nSimple sum: %d\n", base_process_sum, simple_sum);
        if (simple_sum == base_process_sum) {
            printf("They are equal.\n");
        } else {
            printf("They not are equal!\n");
        }
    }

    // Working processes
    if (process_rank != 0) {
        int array_part[part_length];
        MPI_Recv(&array_part[0], part_length, MPI_INT, 0, process_rank, MPI_COMM_WORLD, &Status);
        int sum = 0;
        for (int i = 0; i < part_length; ++i) {
            sum += array_part[i];
        }

        MPI_Send(&sum, 1, MPI_INT, 0, process_rank * 2, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
