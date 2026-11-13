#ifndef __TRIPLET_UTILS_H__
#define __TRIPLET_UTILS_H__

typedef struct TripleT_RGB{
    union{
	float color[4];
	struct{
	    float r, g, b, a;
	};
    };
}TripleT_RGB;

typedef struct TripleT_Point_2D{
    float x, y; 
}TripleT_Point_2D;

typedef struct TripleT_Point_3D{
    float x, y, z; 
}TripleT_Point_3D;

typedef struct TripleT_Point_4D{
    float x, y, z, w;
}TripleT_Point_4D;

typedef struct TripleT_Vector_2D{
    float x, y;
}TripleT_Vector_2D;

typedef struct TripleT_Vector_3D{
    float x, y, z;
}TripleT_Vector_3D;

typedef struct TripleT_Vector_4D{
    float x, y, z, w;
}TripleT_Vector_4D;

typedef struct TripleT_Vertex_3D{
    TripleT_Point_3D position;
    TripleT_RGB color;
}TripleT_Vertex_3D;

// IMPORTANT: THE MATRIXES ARE COLLUMN MAJOR
typedef struct TripleT_Matrix_3D{
    union{
	TripleT_Point_3D mat[3];
	struct{
	    TripleT_Point_3D mat_0;
	    TripleT_Point_3D mat_1;
	    TripleT_Point_3D mat_2;
	};
    };
}TripleT_Matrix_3D;

typedef struct TripleT_Matrix_4D{
    union{
	TripleT_Point_4D mat[4];
	struct{
	    TripleT_Point_4D mat_0;
	    TripleT_Point_4D mat_1;
	    TripleT_Point_4D mat_2;
	    TripleT_Point_4D mat_3;
	};
    };
}TripleT_Matrix_4D;

typedef struct TripleT_Triangle_3D{
    union{
	TripleT_Vertex_3D vertices[3];
	struct{
	    TripleT_Vertex_3D vertice_1;
	    TripleT_Vertex_3D vertice_2;
	    TripleT_Vertex_3D vertice_3;
	};
    };
}TripleT_Triangle_3D;


/*
 *
 * Math Util Functions
 *
 *
 * */
TripleT_Vector_3D t3_calculate_vector_3D(const TripleT_Point_3D point1, const TripleT_Point_3D point2);
TripleT_Vector_3D t3_subtract_vector_3D(const TripleT_Vector_3D vector1, const TripleT_Vector_3D vector2);
TripleT_Vector_3D t3_normalize_vector_3D(const TripleT_Vector_3D vector);
TripleT_Vector_3D t3_cross_vector_3D(const TripleT_Vector_3D vector1, const TripleT_Vector_3D vector2);
float t3_dot_product_3D(const TripleT_Vector_3D vector1, const TripleT_Vector_3D vector2);
void t3_print_mat_collumn_major_4D(const TripleT_Matrix_4D matrix);

#endif // __TRIPLET_UTILS_H__
