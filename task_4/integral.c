#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <omp.h>

double f(double x) {
    return 1 / (1 + x * x);
}

double compute_integral_reduction(double (*function)(double), double segment_begin, double segment_end, size_t subsegments_number) {
    double h = (segment_end - segment_begin) / subsegments_number;
    double result = h * (function(segment_begin) + function(segment_end)) / 2;
    #pragma omp parallel reduction(+: result)
    {
        #pragma omp for
            for (size_t i = 1; i <= subsegments_number - 1; ++i) {
                result += h * function(segment_begin + h * i);
            }
    }
    return result;
}

double compute_integral_critical(double (*function)(double), double segment_begin, double segment_end, size_t subsegments_number) {
    double h = (segment_end - segment_begin) / subsegments_number;
    double result = h * (function(segment_begin) + function(segment_end)) / 2;
    #pragma omp parallel shared(result)
    {
        #pragma omp for
        for (size_t i = 1; i <= subsegments_number - 1; ++i) {
            #pragma omp critical
            {
                result += h * function(segment_begin + h * i);
            }
        }
    }
    return result;
}

const double EPSILON = 0.00001;

int main(int argc, char** argv) {
    double old_integral = compute_integral_critical(f, 0, 1, 1);
    for (size_t n = 2;; n *= 2) {
        double new_integral = compute_integral_critical(f, 0, 1, n);
        if (fabs(old_integral - new_integral) / 3 < EPSILON) {
            printf("result: %lf, n = %d\n", new_integral, n);
            break;
        } else {
            old_integral = new_integral;
        }
    }
    return 0;
}
