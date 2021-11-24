#ifndef MATRIX_H_INCLUDED
#define MATRIX_H_INCLUDED

#define PI 3.1415926535897932384626433832795f
#define DEG2RAD PI / 360.0f

#define FLOAT_SIGN_MASK 0x80000000
#define FLOAT_GET_SIGN(f) (*((int *)&f)) & FLOAT_SIGN_MASK

typedef struct
{
  float x;
  float y;
  float z;
  float w;
} Vector4_t;

extern Vector4_t Vector4_Zero;
extern Vector4_t Vector4_Up;
extern Vector4_t Vector4_Right;
extern Vector4_t Vector4_Forward;

typedef struct
{
  Vector4_t front;
  Vector4_t back;
  Vector4_t top;
  Vector4_t bottom;
  Vector4_t left;
  Vector4_t right;
} Frustum;

inline void vector4_set(Vector4_t *pDest, float x, float y, float z, float w)
{
  pDest->x = x;
  pDest->y = y;
  pDest->z = z;
  pDest->w = w;
}

typedef Vector4_t mat44[4];
typedef struct
{
  mat44 m;
} Matrix_t;

extern Matrix_t Matrix_t_IDENTITY;
//
// \brief multiply matrix specified by result with a perspective matrix and return new matrix in result
/// \param result Specifies the input matrix.  new matrix is returned in result.
/// \param left, right Coordinates for the left and right vertical clipping planes
/// \param bottom, top Coordinates for the bottom and top horizontal clipping planes
/// \param nearZ, farZ Distances to the near and far depth clipping planes.  Both distances must be positive.
//
void matrix_create_frustum(Matrix_t *result, float left, float right, float bottom, float top, float nearZ, float farZ);

//
/// \brief multiply matrix specified by result with a perspective matrix and return new matrix in result
/// \param result Specifies the input matrix.  new matrix is returned in result.
/// \param fovy Field of view y angle in degrees
/// \param aspect Aspect ratio of screen
/// \param nearZ Near plane distance
/// \param farZ Far plane distance
//
void matrix_create_perspective(Matrix_t *result, float fovy, float aspect, float nearZ, float farZ);

//
/// \brief multiply matrix specified by result with a perspective matrix and return new matrix in result
/// \param result Specifies the input matrix.  new matrix is returned in result.
/// \param left, right Coordinates for the left and right vertical clipping planes
/// \param bottom, top Coordinates for the bottom and top horizontal clipping planes
/// \param nearZ, farZ Distances to the near and far depth clipping planes.  These values are negative if plane is behind the viewer
//
void matrix_create_ortho(Matrix_t *result, float left, float right, float bottom, float top, float nearZ, float farZ);

//
/// \brief perform the following operation - result matrix = srcA matrix * srcB matrix
/// \param result Returns multiplied matrix
/// \param srcA, srcB Input matrices to be multiplied
//
void multiply_res(Matrix_t *result, Matrix_t *srcA, Matrix_t *srcB);
void multiply(Matrix_t *srcA, Matrix_t *srcB);
void multiply_vec4(Vector4_t *r, const Matrix_t *m, Vector4_t v);
void multiply33(Vector4_t *r, const Matrix_t *m, Vector4_t v);

void scale(Vector4_t *pResult, const Vector4_t *pVector, float scalar);
void div_scalar(Vector4_t *pResult, const Vector4_t *pVector, float scalar);

void add_scalar(Vector4_t *pResult, const Vector4_t *pVector, float scalar);
void sub_scalar(Vector4_t *pResult, const Vector4_t *pVector, float scalar);

void add_vec4(Vector4_t *pResult, const Vector4_t *pVector, const Vector4_t *pRhs);
void sub_vec4(Vector4_t *pResult, const Vector4_t *pVector, const Vector4_t *pRhs);

float dot_vec(const Vector4_t *pVector, const Vector4_t *pRhs);
float dot_scalar(const Vector4_t *pVector, const Vector4_t *pRhs, float scalar);
void math_cross(Vector4_t *result, const Vector4_t *v1, const Vector4_t *v2);

float vector4_length(const Vector4_t *pVector);
void vector4_normalize(Vector4_t *v);

void plane_normalize_res(Vector4_t *vResult, Vector4_t *vPlane);

void plane_normalize(Vector4_t *vPlane);

int matrix_equal(const Matrix_t *lhs, const Matrix_t *rhs);

void matrix_rotation_x(Matrix_t *result, float fRadianAngle);
void matrix_rotation_y(Matrix_t *result, float fRadianAngle);
void matrix_rotation_z(Matrix_t *result, float fRadianAngle);

void matrix_inverse_nonui(Matrix_t *rmat, const Matrix_t *mat);

void transpose_res(Matrix_t *rmat, const Matrix_t *mat);
void transpose(Matrix_t *mat);

void rotate(Matrix_t *result, float angle, const Vector4_t *axis);
void rotate_comp(Matrix_t *result, float angle, float x, float y, float z);

void scale_comp(Matrix_t *result, float sx, float sy, float sz);
void translate(Matrix_t *result, float tx, float ty, float tz);
void translate_vec4(Matrix_t *result, Vector4_t *vPos);

float math_min(float a, float b);
float math_max(float a, float b);

#endif
