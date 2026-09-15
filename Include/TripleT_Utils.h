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

typedef struct TripleT_Vertex_3D{
    TripleT_Point_3D position;
    TripleT_RGB color;
}TripleT_Vertex_3D;

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

#endif // __TRIPLET_UTILS_H__
