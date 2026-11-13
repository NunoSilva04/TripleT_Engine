#include "TripleT_Utils.h"
#include <math.h>
#include <stdio.h>

TripleT_Vector_3D t3_calculate_vector_3D(TripleT_Point_3D point1, TripleT_Point_3D point2){
    return (TripleT_Vector_3D){
	point1.x - point2.x,
	point1.y - point2.y,
	point1.z - point2.z,
    };
}

TripleT_Vector_3D t3_subtract_vector_3D(const TripleT_Vector_3D vector1, const TripleT_Vector_3D vector2){
    return (TripleT_Vector_3D){
	vector1.x - vector1.x,
	vector2.y - vector2.y,
	vector1.z - vector2.z,
    };
}

TripleT_Vector_3D t3_normalize_vector_3D(TripleT_Vector_3D vector){
    float magnitude = sqrtf((vector.x * vector.x) + (vector.y * vector.y) + (vector.z * vector.z));
    
    return (TripleT_Vector_3D){
	vector.x / magnitude,
	vector.y / magnitude,
	vector.z / magnitude,
    };
}

TripleT_Vector_3D t3_cross_vector_3D(const TripleT_Vector_3D vector1, const TripleT_Vector_3D vector2){
    return (TripleT_Vector_3D){
	(vector1.y * vector2.z) - (vector1.z * vector2.y),
	(vector1.z * vector2.x) - (vector1.x * vector2.z),
	(vector1.x * vector2.y) - (vector1.y * vector2.x),
    };
}

float t3_dot_product_3D(const TripleT_Vector_3D vector1, const TripleT_Vector_3D vector2){
    return (vector1.x * vector2.x) + (vector1.y * vector2.y) + (vector1.z * vector2.z);
}

void t3_print_mat_collumn_major_4D(const TripleT_Matrix_4D matrix){
    printf("(%f, %f, %f, %f)\n"
          "(%f, %f, %f, %f)\n"
          "(%f, %f, %f, %f)\n"
          "(%f, %f, %f, %f)\n\n", 
          matrix.mat[0].x, matrix.mat[1].x, matrix.mat[2].x, matrix.mat[3].x,
          matrix.mat[0].y, matrix.mat[1].y, matrix.mat[2].y, matrix.mat[3].y,
          matrix.mat[0].z, matrix.mat[1].z, matrix.mat[2].z, matrix.mat[3].z,
          matrix.mat[0].w, matrix.mat[1].w, matrix.mat[2].w, matrix.mat[3].w);
}
