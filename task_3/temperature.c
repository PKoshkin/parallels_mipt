#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>

MPI_Status status;
size_t n, m;
size_t TIME;
int process_rank, process_number;

const double K = 401.0 / (8960.0 * 380.0);
double H;
double TAU;

struct Cell {
    size_t width;
    size_t height;
	int number;

    double** temperature;
};
typedef struct Cell Cell;

size_t get_cell_height(size_t cell_number) {
    size_t cells_height = n / (process_number - 1);
    return ((cell_number + 1) * cells_height <= n) ? cells_height : (n - cell_number * cells_height);
}

size_t get_index(size_t start, size_t time, size_t process, size_t step) {
    return start + (time * process_number + process) * 4 + step;
}

void init_cell(Cell* cell, size_t width, size_t height, int cell_number) {
    cell->temperature = (double**)malloc(sizeof(double*) * height);
    for (size_t i = 0; i < height; ++i) {
        cell->temperature[i] = (double*)malloc(sizeof(double) * (m + 2));
    }

    cell->width = width;
    cell->height = height;
	cell->number = cell_number;
}

void free_cell(Cell* cell) {
    for (size_t i = 0; i < cell->height; ++i) {
        free(cell->temperature[i]);
    }
    free(cell->temperature);
}

void fill(Cell* cell, double value) {
    for (size_t i = 0; i < cell->height; ++i) {
        for (size_t j = 0; j < cell->width; ++j) {
            cell->temperature[i][j] = value;
        }
    }
}

void process_cell(Cell* cell) {
    // Calculating temporary matrix
    double** tmp = (double**)malloc(sizeof(double*) * cell->height);
    for (int i = 0; i < cell->height; ++i) {
        tmp[i] = (double*)malloc(sizeof(double) * cell->width);
        tmp[i][0] = cell->temperature[i][0];
        tmp[i][cell->width - 1] = cell->temperature[i][cell->width - 1];
    }
    for (int j = 0; j < cell->width; ++j) {
        tmp[0][j] = cell->temperature[cell->height - 1][j];
        tmp[cell->height - 1][j] = cell->temperature[cell->height - 1][j];
    }
    for (int i = 1; i < cell->height - 1; ++i) {
        for (int j = 1; j < cell->width - 1; ++j) {
            tmp[i][j] = cell->temperature[i][j] + K * TAU / (H * H) * (cell->temperature[i + 1][j] - 2 * cell->temperature[i][j] + cell->temperature[i - 1][j]);
        }
    }

    // Calculating temperature matrix
    for(size_t i = 1; i < cell->height - 1; ++i) {
        for (size_t j = 1; j < cell->width - 1; ++j) {
            cell->temperature[i][j] = tmp[i][j] + K * TAU / (H * H) * (tmp[i][j + 1] - 2 * tmp[i][j] + tmp[i][j - 1]);
        }
    }

    // free memory
    for (int i = 0; i < cell->height; ++i) {
        free(tmp[i]);
    }
    free(tmp);
}

void send_string(Cell* cell, size_t string_number, size_t receiveing_process, int index) {
    MPI_Send(&cell->temperature[string_number][0], m + 2, MPI_DOUBLE, receiveing_process, index, MPI_COMM_WORLD);
}

void receive_string(Cell* cell, size_t string_number, size_t sending_process, int index) {
	if (process_rank == cell->number) {
		MPI_Recv(&cell->temperature[string_number][0], m + 2, MPI_DOUBLE, sending_process, index, MPI_COMM_WORLD, &status);
	}
}

void send_up(Cell* cell, int index) {
    if (process_rank != 0) {
        send_string(cell, 1, process_rank - 1, index);
    }
}

void send_down(Cell* cell, int index) {
    size_t last_process;
    if (n % (process_number - 1) == 0) {
        last_process = process_number - 2;
    } else {
        last_process = process_number - 1;
    }
    if (process_rank != last_process) {
        send_string(cell, cell->height - 2, process_rank + 1, index);
    }
}

void receive_up(Cell* cell, int index) {
    if (process_rank != 0) {
        receive_string(cell, 0, process_rank - 1, index);
    }

/*
    else {
        for (int i = 0; i < cell->width; ++i) {
            cell->temperature[0][i] = cell->temperature[1][i];
        }
    }
*/
}

void receive_down(Cell* cell, int index) {
    size_t last_process;
    if (n % (process_number - 1) == 0) {
        last_process = process_number - 2;
    } else {
        last_process = process_number - 1;
    }
    if (process_rank != last_process) {
        receive_string(cell, cell->height - 1, process_rank + 1, index);
    }

/*
    else {
        for (int i = 0; i < cell->width; ++i) {
            cell->temperature[cell->height - 1][i] = cell->temperature[cell->height - 2][i];
        }
    }
*/
}

void send_cell(Cell* cell, size_t receiveing_proces, int index) {
    for (int i = 0; i < cell->height; ++i) {
        send_string(cell, i, receiveing_proces, index + i);
    }
}

void receive_cell(Cell* cell, size_t sending_proces, int index) {
    init_cell(cell, m + 2, get_cell_height(process_rank) + 2, process_rank);
    for (int i = 0; i < cell->height; ++i) {
        receive_string(cell, i, sending_proces, index + i);
    }
}

void print_cell(Cell* cell) {
    for (size_t i = 1; i < cell->height - 1; ++i) {
        for (size_t j = 1; j < cell->width - 1; ++j) {
            printf("%lf ", cell->temperature[i][j]);
        }
        printf("\n");
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &process_number);
    MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);

    // Check that there are enough arguments
    if (argc < 7) {
        printf("Not enough arguments!\n");
        MPI_Finalize();
        return 0;
    }
    double begin_time, end_time;

    H = atof(argv[5]);
    int seconds = atoi(argv[6]);
    TAU = H * H / (2 * K);
    TIME = seconds / TAU + ((seconds / TAU - (int)(seconds / TAU) < 0.5) ? 0 : 1);

    // Getting matrix size from command line
    n = (size_t)(atof(argv[3]) / H); // strings
    m = (size_t)(atof(argv[4]) / H); // columns

    if (process_number == 1) {
        printf("Need at least 2 processes!\n");
        MPI_Finalize();
        return 0;
    }

    size_t cells_height = n / (process_number - 1);
    size_t cells_number = process_number - 1;
    if (n % (process_number - 1) != 0) {
        ++cells_number;
    }

	// Base process
	if (process_rank == process_number - 1) {
		for (size_t i = 0; i < cells_number; ++i) {
			Cell* new_cell = (Cell*)malloc(sizeof(Cell));
			init_cell(new_cell, m + 2, get_cell_height(i) + 2, i);

			// initial conditions
            fill(new_cell, 5);
			for (size_t j = 0; j < get_cell_height(i) + 2; ++j) {
				new_cell->temperature[j][0] = 80.0;
				new_cell->temperature[j][m + 1] = 30.0;
			}
            if (i == 0) {
                for (size_t j = 0; j < m + 2; ++j) {
                    new_cell->temperature[0][j] = 100;
                }
            }
            if (i == cells_number - 1) {
                for (size_t j = 0; j < m + 2; ++j) {
                    new_cell->temperature[get_cell_height(i) + 1][j] = 0;
                }
            }

            // The last cell may be for base process
            if ((n % (process_number - 1) == 0) || (i != cells_number)) {
                send_cell(new_cell, i, i * n);
            }
            free_cell(new_cell);
            free(new_cell);
		}
	}

    Cell cell;
    // Working processes
    if ((process_rank != process_number - 1) || (n % (process_number - 1) != 0)) {
        receive_cell(&cell, process_number - 1, process_rank * n);
    }

    const char* algorithm = argv[2];
    MPI_Barrier(MPI_COMM_WORLD);
    if (process_rank == process_number - 1) {
        begin_time = MPI_Wtime();
    }

    if (strcmp(algorithm, "slow") == 0) {
        if ((process_rank != process_number - 1) || (n % (process_number - 1) != 0)) {
			size_t last_process = (n % (process_number - 1) == 0) ? process_number - 2 : process_number - 1;
			if (cells_number > 2) {
				for (size_t time = 0; time < TIME; ++time) {
					for (size_t process = 0; process <= last_process - 1; ++process) {
						if (process_rank == process) {
							send_down(&cell, 0);
						}
						if (process_rank == process + 1) {
							receive_up(&cell, 0);
						}
					}
					for (size_t process = last_process; process >= 1; --process) {
						if (process_rank == process) {
							send_up(&cell, 0);
						}
						if (process_rank == process - 1) {
							receive_down(&cell, 0);
						}
					}

					process_cell(&cell);
				}
			}
			else if (cells_number == 2) {
				for (size_t time = 0; time < TIME; ++time) {
					if (process_rank == 0) {
						send_down(&cell, get_index(n * cells_number, time, process_rank, 0));
					} else {
						receive_up(&cell, get_index(n * cells_number, time, process_rank - 1, 0));
					}

					if (process_rank == 0) {
						receive_down(&cell, get_index(n * cells_number, time, process_rank, 1));
					} else {
						send_up(&cell, get_index(n * cells_number, time, process_rank - 1, 1));
					}
					process_cell(&cell);
				}
			} else {
				for (size_t time = 0; time < TIME; ++time) {
					process_cell(&cell);
				}
			}
		}
    } else {
        if ((process_rank != process_number - 1) || (n % (process_number - 1) != 0)) {
            for (size_t time = 0; time < TIME; ++time) {
            
                if (process_rank % 2 == 0) {
                    send_down(&cell, get_index(n * cells_number, time, process_rank, 0));
                } else {
                    receive_up(&cell, get_index(n * cells_number, time, process_rank - 1, 0));
                }

                if (process_rank % 2 == 0) {
                    receive_down(&cell, get_index(n * cells_number, time, process_rank + 1, 1));
                } else {
                    send_up(&cell, get_index(n * cells_number, time, process_rank, 1));
                }

                if (process_rank % 2 == 0) {
                    receive_up(&cell, get_index(n * cells_number, time, process_rank - 1, 2));
                } else {
                    send_down(&cell, get_index(n * cells_number, time, process_rank, 2));
                }

                if (process_rank % 2 == 0) {
                    send_up(&cell, get_index(n * cells_number, time, process_rank, 3));
                } else {
                    receive_down(&cell, get_index(n * cells_number, time, process_rank + 1, 3));
                }

                process_cell(&cell);
            }
        }
    }

    const char* mode = argv[1];

    MPI_Barrier(MPI_COMM_WORLD);
    if (process_rank == process_number - 1) {
        end_time = MPI_Wtime();
        if (strcmp(mode, "time") == 0) {
            printf("%lf ", end_time - begin_time);
        }
    }

    if (strcmp(mode, "temperature") == 0) {
        if (process_rank == process_number - 1) {
            printf("%lf %d %d\n", H, n, m);
        }
        MPI_Barrier(MPI_COMM_WORLD);
        for (size_t i = 0; i < cells_number; ++i) {
            MPI_Barrier(MPI_COMM_WORLD);
            if (process_rank == i) {
                print_cell(&cell);
            }
            MPI_Barrier(MPI_COMM_WORLD);
        }
    }

    //free_cell(&cell);

    MPI_Finalize();
    return 0;
}
