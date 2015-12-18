/*This header file defines structures and functions for working with matrices and 
 * vectors*/
#if !defined(LINALG_H)
#define LINALG_H

/*represents a vector.  has an array of m real number elements*/
typedef struct {
    float * elements;
    unsigned int m;
} Vector;

/*Represents a matrix.  has an array of n pointers to vectors as columns*/
typedef struct {
    Vector ** columns;
    unsigned int n;
} Matrix;

Vector * new_vector(unsigned int m);
Vector * scalar_mult(float a, Vector * u);
Vector * vector_add(Vector * u, Vector * v);
void free_vector(Vector * u);
Vector * linear_combo(Matrix * a, Vector * x);
Matrix * new_matrix(unsigned int m, unsigned int n);
Matrix * matrix_multiply(Matrix * a, Matrix * b);
void free_matrix(Matrix * a);
void print_matrix(Matrix * a);
void print_vector(Vector * a);

#endif
