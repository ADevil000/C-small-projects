#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

// ideal and vector_b is input data that is matrix and vector b
// all_matrix, matrix, res is using for finding solution
// ans has a solution, while we are searching better solution in start_iteration,

void clear(double **matrix, double *ideal, double *all_matrix, double *vector_b, double *res, double *ans) {
    free(matrix);
    free(ideal);
    free(all_matrix);
    free(vector_b);
    free(res);
    free(ans);
}

void swap(double **matrix, double *vector_b, size_t a, size_t b) {
    double *tmp = matrix[a];
    matrix[a] = matrix[b];
    matrix[b] = tmp;
    double tmp2 = vector_b[a];
    vector_b[a] = vector_b[b];
    vector_b[b] = tmp2;
}

size_t find_max(double **matrix, size_t  h, size_t w, size_t size) {
    size_t res = h;
    for (size_t i = h + 1; i < size; i++) {
        if (fabs(matrix[i][w]) > fabs(matrix[res][w])) {
            res = i;
        }
    }
    return res;
}

void delete_under(double **matrix, double *vector_b, size_t h, size_t w, size_t size) {
    for (size_t i = h + 1; i < size; i++) {
        double k = matrix[i][w] / matrix[h][w];
        matrix[i][w] = 0.0;
        for (size_t j = w + 1; j < size; j++) {
            matrix[i][j] = matrix[i][j] - matrix[h][j] * k;
        }
        vector_b[i] = vector_b[i] - vector_b[h] * k;
    }
}

void to_Gauss(double **matrix, double *vector_b, size_t size) {
    double epsilons = (double) size;
    for (size_t h = 0, w = 0; h < size && w < size; w++) {
        size_t line = find_max(matrix, h, w, size);
        swap(matrix, vector_b, line, h);
        if (fabs(matrix[h][w]) > epsilons * DBL_EPSILON) {
            delete_under(matrix, vector_b, h, w, size);
            h++;
        }
    }
}

int check_for_solvability(double **matrix, double *vector_b, size_t size) {
    double epsilons = (double) size;
    for (size_t h = 0; h < size; h++) {
        for (size_t w = 0; w < size; w++) {
            if (fabs(matrix[size - h - 1][w]) > epsilons * DBL_EPSILON) {
                if (h == 0) {
                    return 0;
                } else {
                    return 1;
                }
            }
        }
        if (fabs(vector_b[size - h - 1]) > epsilons * DBL_EPSILON) {
            return -1;
        }
    }
    return 1;
}

void delete_upper(double **matrix, double *vector_b, size_t h) {
    for (size_t i = 0; i < h; i++) {
        vector_b[i] = vector_b[i] - vector_b[h] * matrix[i][h];
        matrix[i][h] = 0.0;
    }
}

void back_Gauss(double **matrix, double *vector_b, size_t size) {
    for (size_t h = 0; h < size; h++) {
        vector_b[size - h - 1] = vector_b[size - h - 1] / matrix[size - h - 1][size - h - 1];
        matrix[size - h - 1][size - h - 1] = 1.0;
        delete_upper(matrix, vector_b, size - h - 1);
    }
}

void fill_matrix(double *all_matrix, double *ideal, double **matrix, size_t size) {
    memcpy(all_matrix, ideal, size * size * sizeof(double));
    for (size_t i = 0; i < size; i++) {
        matrix[i] = all_matrix + i * size;
    }
}

void fill_vect(double *res, double *vector_b, size_t size) {
    memcpy(res, vector_b, size * sizeof(double));
}

void mul(const double *A, const double *x, double *b, size_t size) {
    for (size_t i = 0; i < size; i++) {
        double tmp = 0.0;
        for (size_t j = 0; j < size; j++) {
            tmp += A[i * size + j] * x[j];
        }
        b[i] = tmp;
    }
}

void division(const double *a, const double *b, double *c, size_t size) {
    for (size_t i = 0; i < size; i++) {
        c[i] = a[i] - b[i];
    }
}

// Pre: res = x(i), ans = x(i-1), x(0) is nothing
void start_iteration(double *all_matrix, double *ideal, double *vector_b, double **matrix, double *res, double *ans, size_t size) {
    fill_matrix(all_matrix, ideal, matrix, size);
    fill_vect(ans, res, size); // save x(i)
    mul(ideal, ans, res, size); // res =  A*x(i)
    division(res, vector_b, res, size); // res = A*x(i) - b
    to_Gauss(matrix, res, size);
    back_Gauss(matrix, res, size); // res is solution. A * res = A * x(i) - b
}
// Post: res = y, ans = x(i), x(i) is better result than x(i-1)

_Bool check_res_on_zero(double *res, size_t size) {
    double  epsilons = (double) size;
    for (size_t i = 0; i < size; i++) {
        if (fabs(res[i]) > epsilons * DBL_EPSILON) {
            return false;
        }
    }
    return true;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Wait 2 arguments: names of input and output files, but have %i argument(s)", argc - 1);
        return -1;
    }
    FILE *f;
    if ((f = fopen(argv[1], "r")) == NULL) {
        printf("File %s didn't find", argv[1]);
        return -1;
    }
    size_t size = 0;
    int check = fscanf(f, "%zu", &size);
    if (check != 1) {
        printf("Read %i arguments than wanted 1", check);
        fclose(f);
        return -1;
    }
    const double ZERO = 0.0;
    double **matrix, *vector_b, *ideal, *res, *ans, *all_matrix;
    if (!(matrix = malloc(size * sizeof(double*)))) {
        printf("Fail in matrix memory allocation (1)");
        fclose(f);
        return -1;
    }
    if (!(ideal = malloc(size *  size * sizeof(double)))) {
        printf("Fail in matrix memory allocation (2)");
        clear(matrix, NULL, NULL, NULL, NULL, NULL);
        fclose(f);
        return -1;
    }
    if (!(all_matrix = malloc(size *  size * sizeof(double)))) {
        printf("Fail in matrix memory allocation (3)");
        clear(matrix, ideal, NULL, NULL, NULL, NULL);
        fclose(f);
        return -1;
    }
    if (!(vector_b = malloc(size * sizeof(double)))) {
        printf("Fail in vector memory allocation (1)");
        clear(matrix, ideal, all_matrix, NULL, NULL, NULL);
        fclose(f);
        return -1;
    }
    if (!(res = malloc(size * sizeof(double)))) {
        printf("Fail in vector memory allocation (2)");
        clear(matrix, ideal, all_matrix, vector_b, NULL, NULL);
        fclose(f);
        return -1;
    }
    if (!(ans = malloc(size * sizeof(double)))) {
        printf("Fail in vector memory allocation (3)");
        clear(matrix, ideal, all_matrix, vector_b, res, NULL);
        fclose(f);
        return -1;
    }
    for (size_t h = 0; h < size; h++) {
        for (size_t w = 0; w < size; w++) {
            if ((check = fscanf(f, "%lf", &ideal[h * size + w])) != 1) {
                printf("Read %i arguments when wanted (1)", check);
                clear(matrix, ideal, all_matrix, vector_b, res, ans);
                fclose(f);
                return -1;
            }
        }
        if ((check = fscanf(f, "%lf", &vector_b[h])) != 1) {
            printf("Read %i arguments when wanted (1)", check);
            clear(matrix, ideal, all_matrix, vector_b, res, ans);
            fclose(f);
            return -1;
        }
    }
    fclose(f);
    f = NULL;
    fill_matrix(all_matrix, ideal, matrix, size);
    fill_vect(res, vector_b, size);
    to_Gauss(matrix, res, size);
    if ((f = fopen(argv[2], "w")) == NULL) {
        printf("File %s didn't open or create", argv[2]);
        clear(matrix, ideal, all_matrix, vector_b, res, ans);
        return -1;
    }
    switch (check_for_solvability(matrix, res, size)) {
        case 0:
            back_Gauss(matrix, res, size);
            for (size_t i = 0; i < 100; i++) {
                start_iteration(all_matrix, ideal, vector_b, matrix, res, ans, size);
                if (check_res_on_zero(res, size)) {
                    break;
                } else {
                    division(ans, res, res, size);
                }
            }
            for (size_t i = 0; i < size; i++) {
                if (fabs(ans[i]) < DBL_EPSILON * size) {
                    check = fprintf(f, "%lf\n", ZERO);
                } else {
                    check = fprintf(f, "%lf\n", ans[i]);
                }
                if (check < 1) {
                    printf("Write %i arguments when wanted more", check);
                    clear(matrix, ideal, all_matrix, vector_b, res, ans);
                    fclose(f);
                    return -1;
                }
            }
            break;
        case 1:
            if (size == 0) {
                break;
            }
            check = fprintf(f, "many solutions");
            if (check < 1) {
                printf("Write %i arguments when wanted more", check);
                clear(matrix, ideal, all_matrix, vector_b, res, ans);
                fclose(f);
                return -1;
            }
            break;
        case -1:
            check = fprintf(f, "no solution");
            if (check < 1) {
                printf("Write %i arguments when wanted more", check);
                clear(matrix, ideal, all_matrix, vector_b, res, ans);
                fclose(f);
                return -1;
            }
            break;
    }
    clear(matrix, ideal, all_matrix, vector_b, res, ans);
    fclose(f);
    return 0;
}