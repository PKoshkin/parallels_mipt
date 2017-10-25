#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <omp.h>

typedef struct Field {
	short** matrix;
	size_t height;
	size_t width;
} Field;

void init_field(Field* field, size_t height, size_t width) {
	field->width = width;
	field->height = height;
	field->matrix = (short**)malloc(sizeof(short*) * field->height);
	for (size_t i = 0; i < height; ++i) {
		field->matrix[i] = (short*)malloc(sizeof(short) * field->width);
	}
}

void free_field(Field* field) {
	for (size_t i = 0; i < field->height; ++i) {
		free(field->matrix[i]);
	}
	free(field->matrix);
}

Field* copy(Field* field) {
	Field* new_field = (Field*)malloc(sizeof(Field));
	init_field(new_field, field->height, field->width);
    #pragma omp parallel for collapse(2)
	for (size_t i = 0; i < field->height; ++i) {
		for (size_t j = 0; j < field->width; ++j) {
			new_field->matrix[i][j] = field->matrix[i][j];
		}
	}
	return new_field;
}

short get_element(Field* field, size_t i, size_t j) {
	return field->matrix[(i + field->height) % field->height][(j + field->width) % field->width];
}

size_t neighbors(Field* field, size_t line, size_t column) {
	size_t result = 0;
	for (int i = -1; i <= 1; ++i) {
		for (int j = -1; j <= 1; ++j) {
			if (get_element(field, line + i, column + j) == 1) {
				++result;
			}
		}
	}
	return result - get_element(field, line, column);
}

void step(Field* field) {
	Field* old_field = copy(field);
    #pragma omp parallel for collapse(2)
    for (size_t i = 0; i < field->height; ++i) {
        for (size_t j = 0; j < field->width; ++j) {
            if (old_field->matrix[i][j] == 1) {
                if ((neighbors(old_field, i, j) != 2) && (neighbors(old_field, i, j) != 3)) {
                    field->matrix[i][j] = 0;
                }
            } else {
                if (neighbors(old_field, i, j) == 3) {
                    field->matrix[i][j] = 1;
                }
            }
        }
    }
	free_field(old_field);
	free(old_field);
}

void print_field(Field* field) {
	for (size_t i = 0; i < field->height; ++i) {
		for (size_t j = 0; j < field->width; ++j) {
			printf("%d ", field->matrix[i][j]);
		}
		printf("\n");
	}
}

void fill(Field* field, short value) {
    for (size_t i = 0; i < field->height; ++i) {
        for (size_t j = 0; j < field->width; ++j) {
            field->matrix[i][j] = value;
        }
    }
}

Field* read_field(const char* file_name) {
    FILE* file = fopen(file_name, "r");
    Field* field = (Field*)malloc(sizeof(Field));

    size_t lines, columns;
    fscanf(file, "%d %d", &lines, &columns);
	init_field(field, (int)lines, (int)columns);
    fill(field, 0);
    size_t x, y;
    while (fscanf(file, "%d %d", &x, &y) != EOF) {
        field->matrix[(int)x][(int)y] = 1;
    }
    return field;
}

const size_t THREADS_NUM = 8;

int main(size_t argc, char** argv) {
    if (argc < 4) {
        printf("Not enough arguments!\n");
        return 0;
    }

    size_t steps = atoi(argv[1]);
    if (strcmp(argv[3], "time") == 0) {
        for (size_t threads = 1; threads < THREADS_NUM; ++threads) {
            omp_set_num_threads(threads);
            Field* field = read_field(argv[2]);
            double begin = omp_get_wtime();
            for (size_t i = 0; i < steps; ++i) {
                step(field);
            }
            printf("%lf\n", omp_get_wtime() - begin);
            free(field);
        }
        return 0;
    } else {
        Field* field = read_field(argv[2]);
        printf("%d %d %d\n", field->height, field->width, atoi(argv[1]));
        print_field(field);
        for (size_t i = 0; i < steps; ++i) {
            step(field);
            print_field(field);
        }
        free(field);
    }

	return 0;
}
