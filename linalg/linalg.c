/*this source file defines various functions for working with vectors and 
 * matrices*/
#include "linalg.h"
#include <stdio.h>
#include <stdlib.h>

/*creates a new dynamically allocated vector with m elements, initialized to 0. 
 * terminates if allocation fails*/
Vector * new_vector(unsigned int m) {
        Vector * result = (Vector *) malloc(sizeof(Vector));
        if (result == NULL) exit(-1);
        result->m = m;
        result->elements = (double *) calloc(m, sizeof(double));
        if (result->elements == NULL) exit (-1);
        return result;
}
/*Scalar multiplies a vector. returns a new dynamically allocated vector. 
 * returns null if input vector is null.*/
Vector * scalar_mult(double a, Vector * u) {
    Vector * result = NULL;
    int idx;
    if (u != NULL) {
        result = new_vector(u->m);
        for (idx = 0; idx < u->m; idx++) 
            result->elements[idx] = a * u->elements[idx];
    }

    return result;
}

/*adds two vectors and returns a new dynamically allocated vector.  Returns null 
 * if either input is null or if they are not of equal size*/
Vector * vector_add(Vector * u, Vector * v) {
    Vector * result = NULL;
    int idx;
    if (u != NULL && v != NULL && u->m == v->m) {
        result = new_vector(u->m);
        for (idx = 0; idx < u->m; idx++)
            result->elements[idx] = u->elements[idx] + v->elements[idx];
    }
    return result;
}

/*frees a dynamically allocated vector*/
void free_vector(Vector * u) {
    if (u != NULL) {
        free((void *)u->elements);
        free((void *)u);
    }
}

/*returns the vector that is the result of multiplying the matrix a by the 
 * vector x, otherwise known as a linear combination of the column vectors of a 
 * with weights in x.  returns a dynamically allocated vecotr if successful, 
 * returns null if inpuits are null or if the dimensions are not correct for a 
 * multiplication*/
Vector * linear_combo(Matrix * a, Vector * x) {
    Vector * result = NULL;
    Vector * tmp1 = NULL, * tmp2 = NULL; /*here to hold dynamic vectors so can free them without leaks*/
    int idx;
    
    if (a != NULL && x != NULL && x->m == a->n) {
        /*inits result vector to have same dimension as columns of a*/
        result = new_vector(a->columns[0]->m);

        for (idx = 0; idx < x->m; idx++) {
            tmp1 = scalar_mult(x->elements[idx], a->columns[idx]);
            tmp2 = vector_add(result, tmp1);
            free_vector(result);
            result = tmp2;
            free_vector(tmp1);

        }

    }

    return result;
}

/*returns a dynamically allocated matrix that has all elements init to 0. 
 * terminates if failure to allocate*/
Matrix * new_matrix(unsigned int m, unsigned int n) {
    int idx;
    Matrix * result = (Matrix *) malloc(sizeof(Matrix));
    if (result == NULL) exit(-1);

    result->columns = (Vector **) malloc(n * sizeof(Vector *));
    if (result->columns == NULL) exit(-1);
    
    for(idx = 0; idx < n; idx++)
        result->columns[idx] = new_vector(m);
    
    result->n = n;

    return result;
}

/*multiplies the two given matrices. returns null if inputs are null or if 
 * dimensions are not right*/
Matrix * matrix_multiply(Matrix * a, Matrix * b) {
    Matrix * result = NULL;
    int idx;
    if (a != NULL && b != NULL && a->n == b->columns[0]->m) {
        /*create new matrix*/
        result = new_matrix(a->columns[0]->m, b->n);
        for (idx = 0; idx < b->n; idx++) {
            free_vector(result->columns[idx]);
            result->columns[idx] = linear_combo(a, b->columns[idx]);
        }
    }
    return result;
}

/*frees all dynamic memory associated with a matrix*/
void free_matrix(Matrix * a) {
    int idx;
    if (a != NULL) {
        for (idx = 0; idx < a->n; idx++)
            free_vector(a->columns[idx]);
        free((void *) a->columns);
        free((void *) a);
    }
}

/*prints a matrix*/
void print_matrix(Matrix * a) {
    int x, y;
    printf(" _%*c_\n", 12*a->n -1, ' ');
        
    for (y = 0; y < a->columns[0]->m; y++) {
        printf("| ");
        for (x = 0; x < a->n; x++) {
            printf("%+011.4lf ", a->columns[x]->elements[y]);
        }
        printf("|\n");
    }

    printf(" -%*c-\n", 12*a->n -1, ' ');
}
void print_vector(Vector * a) {
    int idx;
    printf(" _%*c_\n", 11, ' ');

    for (idx = 0; idx < a->m; idx++) 
        printf("| %+011.4lf |\n", a->elements[idx]);

    printf(" -%*c-\n", 11, ' ');
       
}
       
