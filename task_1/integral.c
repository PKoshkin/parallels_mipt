#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

// integral from Omega of xy dxdy
// Omega = (0 <= x <= pi) and (0 <= y <= sin(x))
// real value is pi^2 / 8 = 1.2337

const size_t POINTS_TO_THROW = 10000000;

struct Point {
    double x;
    double y;
    double z;
};
typedef struct Point Point;

struct Box {
    struct Point min_point;
    struct Point max_point;
};
typedef struct Box Box;

struct ThreadArguments {
    double* result;
    Box* box;
    sem_t* semaphore;
    bool (*point_belongs_area)(struct Point);
    size_t points_to_throw;
    unsigned int seed;
};
typedef struct ThreadArguments ThreadArguments;

void init_thread_arguments(ThreadArguments* arguments, Box* box, sem_t* semaphore, bool (*point_belongs_area)(struct Point), double* result, size_t points_to_throw, unsigned int seed) {
    arguments->result = result;
    arguments->box = box;
    arguments->semaphore = semaphore;
    arguments->point_belongs_area = point_belongs_area;
    arguments->points_to_throw = points_to_throw;
    arguments->seed = seed;
}

void init_box(Box* box, const double min_x, const double min_y, const double min_z, const double max_x, const double max_y, const double max_z) {
    box->min_point.x = min_x;
    box->min_point.y = min_y;
    box->min_point.z = min_z;
    box->max_point.x = max_x;
    box->max_point.y = max_y;
    box->max_point.z = max_z;
}

double box_area(Box* box) {
    return (box->max_point.x - box->min_point.x) * (box->max_point.y - box->min_point.y) * (box->max_point.z - box->min_point.z);
}

struct Point random_point_in_box(Box* box, unsigned int* seed_p) {
    Point result;
    result.x = box->min_point.x + (double)rand_r(seed_p) / (double)RAND_MAX * (box->max_point.x - box->min_point.x);
    result.y = box->min_point.y + (double)rand_r(seed_p) / (double)RAND_MAX * (box->max_point.y - box->min_point.y);
    result.z = box->min_point.z + (double)rand_r(seed_p) / (double)RAND_MAX * (box->max_point.z - box->min_point.z);
	return result;
}

bool point_belongs_omega(Point point) {
    return (0 <= point.x) && (point.x <= M_PI) && (0 <= point.y) && (point.y <= sin(point.x)) && (0 <= point.z) && (point.z <= point.x * point.y);
}

void* compute_integral_part(void* arguments) {
    ThreadArguments* arguments_structure = (ThreadArguments*)arguments;

	size_t belonging_points = 0;
	for (int i = 0; i < arguments_structure->points_to_throw; ++i) {
		if (arguments_structure->point_belongs_area(random_point_in_box(arguments_structure->box, &(arguments_structure->seed)))) {
			++belonging_points;
		}
	}

    sem_wait(arguments_structure->semaphore);
    *(arguments_structure->result) += (double)belonging_points / POINTS_TO_THROW;
    sem_post(arguments_structure->semaphore);

    pthread_exit(NULL);
}

double compute_integral(Box* box, bool (*point_belongs_area)(Point), const size_t threads_number) {
    unsigned int seed = time(0);
    srand(seed);

    double result = 0;
    size_t points_to_throw_each_step = POINTS_TO_THROW / threads_number;
    pthread_t threads[threads_number];
    ThreadArguments* arguments[threads_number];
    sem_t semaphore;
    sem_init(&semaphore, 0, 1);

    for (int i = 0; i < threads_number; ++i) {
        // decide how many points to throw now
        size_t points_to_throw = points_to_throw_each_step;
        if ((i == threads_number - 1) && (POINTS_TO_THROW % threads_number != 0)) {
            points_to_throw += POINTS_TO_THROW % threads_number;
        }

        arguments[i] = (ThreadArguments*)malloc(sizeof(ThreadArguments));
        init_thread_arguments(arguments[i], box, &semaphore, point_belongs_omega, &result, points_to_throw, seed);
        pthread_create(&threads[i], NULL, compute_integral_part, (void*)(arguments[i]));
    }

    for (int i = 0; i < threads_number; ++i) {
        pthread_join(threads[i], NULL);
        free(arguments[i]);
    }

    return result;
}

int main(int argc, char** argv) {
    // omega belongs box
    Box box;
    init_box(&box, 0, 0, 0, M_PI, 1, M_PI);

	double integral = compute_integral(&box, point_belongs_omega, atoi(argv[1]));
    printf("%f\n", integral);

    return 0;
}
