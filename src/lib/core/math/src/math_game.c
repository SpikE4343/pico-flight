
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "math_game.h"
#include "math_util.h"

Vector4_t Vector4_Zero = {0.0f, 0.0f, 0.0f, 1.0f};
Vector4_t Vector4_Up = {0.0f, 1.0f, 0.0f, 1.0f};
Vector4_t Vector4_Right = {1.0f, 0.0f, 0.0f, 1.0f};
Vector4_t Vector4_Forward = {0.0f, 0.0f, 1.0f, 1.0f};

Matrix_t Matrix_t_IDENTITY = {
    {// mat44
     {1.0f, 0.0f, 0.0f, 0.0f},
     {0.0f, 1.0f, 0.0f, 0.0f},
     {0.0f, 0.0f, 1.0f, 0.0f},
     {0.0f, 0.0f, 0.0f, 1.0f}}};

void scale_comp(Matrix_t *result, float sx, float sy, float sz)
{
  result->m[0].x *= sx;
  result->m[0].y *= sx;
  result->m[0].z *= sx;
  result->m[0].w *= sx;

  result->m[1].x *= sy;
  result->m[1].y *= sy;
  result->m[1].z *= sy;
  result->m[1].w *= sy;

  result->m[2].x *= sz;
  result->m[2].y *= sz;
  result->m[2].z *= sz;
  result->m[2].w *= sz;
}

void translate(Matrix_t *result, float tx, float ty, float tz)
{
  //Vector4_t v = { tx, ty, tz, 0.0f };
  //multiply33( &v, result, v );
  //add( &result->m[3], &result->m[3], &v);

  result->m[3].x += (result->m[0].x * tx + result->m[1].x * ty + result->m[2].x * tz);
  result->m[3].y += (result->m[0].y * tx + result->m[1].y * ty + result->m[2].y * tz);
  result->m[3].z += (result->m[0].z * tx + result->m[1].z * ty + result->m[2].z * tz);
  //result->m[3].w += (result->m[0].w * tx + result->m[1].w * ty + result->m[2].w * tz);
}

void rotate(Matrix_t *result, float angle, const Vector4_t *axis)
{
  rotate_comp(result, angle, axis->x, axis->y, axis->z);
}

void rotate_comp(Matrix_t *result, float angle, float x, float y, float z)
{
  float sinAngle, cosAngle;
  float mag = sqrtf(x * x + y * y + z * z);
  angle *= (PI / 180.0f);

  sinAngle = sinf(angle);
  cosAngle = cosf(angle);

  float xx, yy, zz, xy, yz, zx, xs, ys, zs;
  float oneMinusCos;
  Matrix_t rotMat;

  x /= mag;
  y /= mag;
  z /= mag;

  xx = x * x;
  yy = y * y;
  zz = z * z;
  xy = x * y;
  yz = y * z;
  zx = z * x;
  xs = x * sinAngle;
  ys = y * sinAngle;
  zs = z * sinAngle;
  oneMinusCos = 1.0f - cosAngle;

  rotMat.m[0].x = (oneMinusCos * xx) + cosAngle;
  rotMat.m[0].y = (oneMinusCos * xy) - zs;
  rotMat.m[0].z = (oneMinusCos * zx) + ys;
  rotMat.m[0].w = 0.0F;

  rotMat.m[1].x = (oneMinusCos * xy) + zs;
  rotMat.m[1].y = (oneMinusCos * yy) + cosAngle;
  rotMat.m[1].z = (oneMinusCos * yz) - xs;
  rotMat.m[1].w = 0.0F;

  rotMat.m[2].x = (oneMinusCos * zx) - ys;
  rotMat.m[2].y = (oneMinusCos * yz) + xs;
  rotMat.m[2].z = (oneMinusCos * zz) + cosAngle;
  rotMat.m[2].w = 0.0F;

  rotMat.m[3].x = 0.0F;
  rotMat.m[3].y = 0.0F;
  rotMat.m[3].z = 0.0F;
  rotMat.m[3].w = 1.0F;

  multiply_res(result, &rotMat, result);
}

void matrix_create_perspective(Matrix_t *result, float fovy, float aspect, float nearZ, float farZ)
{
  float frustumW, frustumH;

  frustumH = tanf((fovy / 360.0f * PI) * 0.5f) * nearZ;
  frustumW = frustumH * aspect;

  float left = -frustumW;
  float right = frustumW;
  float bottom = -frustumH;
  float top = frustumH;

  float deltaX = right - left;
  float deltaY = top - bottom;
  float deltaZ = farZ - nearZ;

  if ((nearZ <= 0.0f) || (farZ <= 0.0f) || (deltaX <= 0.0f) || (deltaY <= 0.0f) || (deltaZ <= 0.0f))
    return;

  vector4_set(&result->m[0], 2.0f * nearZ / deltaX, 0.0f, (right + left) / deltaX, 0.0f);
  vector4_set(&result->m[1], 0.0f, 2.0f * nearZ / deltaY, (top + bottom) / deltaY, 0.0f);
  vector4_set(&result->m[2], 0.0f, 0.0f, -(nearZ + farZ) / deltaZ, -2.0f * nearZ * farZ / deltaZ);
  vector4_set(&result->m[3], 0.0f, 0.0f, -1.0f, 0.0f);

  //transpose( result );
}

void matrix_create_ortho(Matrix_t *result, float left, float right, float bottom, float top, float nearZ, float farZ)
{
  float deltaX = right - left;
  float deltaY = top - bottom;
  float deltaZ = farZ - nearZ;
  Matrix_t ortho;

  if ((deltaX == 0.0f) || (deltaY == 0.0f) || (deltaZ == 0.0f))
    return;

  ortho = Matrix_t_IDENTITY;
  ortho.m[0].x = 2.0f / deltaX;
  ortho.m[3].x = -(right + left) / deltaX;
  ortho.m[1].y = 2.0f / deltaY;
  ortho.m[3].y = -(top + bottom) / deltaY;
  ortho.m[2].z = -2.0f / deltaZ;
  ortho.m[3].z = -(nearZ + farZ) / deltaZ;

  multiply_res(result, &ortho, result);
}

void matrix_rotation_x(Matrix_t *result, float fRadianAngle)
{
  float fCos = cosf(fRadianAngle);
  float fSin = sinf(fRadianAngle);

  vector4_set(&result->m[0], 1.0f, 0.0f, 0.0f, 0.0f);
  vector4_set(&result->m[1], 0.0f, fCos, -fSin, 0.0f);
  vector4_set(&result->m[2], 0.0f, fSin, fCos, 0.0f);
  vector4_set(&result->m[3], 0.0f, 0.0f, 0.0f, 1.0f);
}

void Matrix_t_rotation_y(Matrix_t *result, float fRadianAngle)
{
  float fCos = cosf(fRadianAngle);
  float fSin = sinf(fRadianAngle);

  vector4_set(&result->m[0], fCos, 0.0f, fSin, 0.0f);
  vector4_set(&result->m[1], 0.0f, 1.0f, 0.0f, 0.0f);
  vector4_set(&result->m[2], -fSin, 0.0f, fCos, 0.0f);
  vector4_set(&result->m[3], 0.0f, 0.0f, 0.0f, 0.0f);
}

void matrix_rotation_z(Matrix_t *result, float fRadianAngle)
{
  float fCos = cosf(fRadianAngle);
  float fSin = sinf(fRadianAngle);

  vector4_set(&result->m[0], fCos, -fSin, 0.0f, 0.0f);
  vector4_set(&result->m[1], fSin, fCos, 0.0f, 0.0f);
  vector4_set(&result->m[2], 0.0f, 0.0f, 1.0f, 0.0f);
  vector4_set(&result->m[3], 0.0f, 0.0f, 0.0f, 1.0f);
}

void multiply_res(Matrix_t *result, Matrix_t *srcA, Matrix_t *srcB)
{
  Matrix_t tmp;
  int i;

  for (i = 0; i < 4; i++)
  {
    tmp.m[i].x = (srcA->m[i].x * srcB->m[0].x) +
                 (srcA->m[i].y * srcB->m[1].x) +
                 (srcA->m[i].z * srcB->m[2].x) +
                 (srcA->m[i].w * srcB->m[3].x);

    tmp.m[i].y = (srcA->m[i].x * srcB->m[0].y) +
                 (srcA->m[i].y * srcB->m[1].y) +
                 (srcA->m[i].z * srcB->m[2].y) +
                 (srcA->m[i].w * srcB->m[3].y);

    tmp.m[i].z = (srcA->m[i].x * srcB->m[0].z) +
                 (srcA->m[i].y * srcB->m[1].z) +
                 (srcA->m[i].z * srcB->m[2].z) +
                 (srcA->m[i].w * srcB->m[3].z);

    tmp.m[i].w = (srcA->m[i].x * srcB->m[0].w) +
                 (srcA->m[i].y * srcB->m[1].w) +
                 (srcA->m[i].z * srcB->m[2].w) +
                 (srcA->m[i].w * srcB->m[3].w);
  }

  memcpy(result, &tmp, sizeof(Matrix_t));
}

void multiply(Matrix_t *srcA, Matrix_t *srcB)
{
  Matrix_t tmp;
  int i;

  for (i = 0; i < 4; i++)
  {
    tmp.m[i].x = (srcA->m[i].x * srcB->m[0].x) +
                 (srcA->m[i].y * srcB->m[1].x) +
                 (srcA->m[i].z * srcB->m[2].x) +
                 (srcA->m[i].w * srcB->m[3].x);

    tmp.m[i].y = (srcA->m[i].x * srcB->m[0].y) +
                 (srcA->m[i].y * srcB->m[1].y) +
                 (srcA->m[i].z * srcB->m[2].y) +
                 (srcA->m[i].w * srcB->m[3].y);

    tmp.m[i].z = (srcA->m[i].x * srcB->m[0].z) +
                 (srcA->m[i].y * srcB->m[1].z) +
                 (srcA->m[i].z * srcB->m[2].z) +
                 (srcA->m[i].w * srcB->m[3].z);

    tmp.m[i].w = (srcA->m[i].x * srcB->m[0].w) +
                 (srcA->m[i].y * srcB->m[1].w) +
                 (srcA->m[i].z * srcB->m[2].w) +
                 (srcA->m[i].w * srcB->m[3].w);
  }

  memcpy(srcA, &tmp, sizeof(Matrix_t));
}

void multiply_vec4(Vector4_t *r, const Matrix_t *m, Vector4_t v)
{
  float fInvW = 1.0f / (m->m[3].x * v.x + m->m[3].y * v.y + m->m[3].z * v.z + m->m[3].w);

  r->x = (m->m[0].x * v.x + m->m[0].y * v.y + m->m[0].z * v.z + m->m[0].w) * fInvW;
  r->y = (m->m[1].x * v.x + m->m[1].y * v.y + m->m[1].z * v.z + m->m[1].w) * fInvW;
  r->z = (m->m[2].x * v.x + m->m[2].y * v.y + m->m[2].z * v.z + m->m[2].w) * fInvW;
  //r->w = fInvW;
}

void multiply33(Vector4_t *r, const Matrix_t *m, Vector4_t v)
{
  r->x = (m->m[0].x * v.x + m->m[0].y * v.y + m->m[0].z * v.z);
  r->y = (m->m[1].x * v.x + m->m[1].y * v.y + m->m[1].z * v.z);
  r->z = (m->m[2].x * v.x + m->m[2].y * v.y + m->m[2].z * v.z);
}

void scale(Vector4_t *pResult, const Vector4_t *pVector, float scalar)
{
  pResult->x = pVector->x * scalar;
  pResult->y = pVector->y * scalar;
  pResult->z = pVector->z * scalar;
  pResult->z = pVector->w * scalar;
}

void div_scalar(Vector4_t *pResult, const Vector4_t *pVector, float scalar)
{
  float inv = 1.0f / scalar;
  pResult->x = pVector->x * inv;
  pResult->y = pVector->y * inv;
  pResult->z = pVector->z * inv;
  pResult->z = pVector->w * inv;
}

void add_scalar(Vector4_t *pResult, const Vector4_t *pVector, float scalar)
{
  pResult->x = pVector->x + scalar;
  pResult->y = pVector->y + scalar;
  pResult->z = pVector->z + scalar;
  pResult->w = pVector->w + scalar;
}

void sub_scalar(Vector4_t *pResult, const Vector4_t *pVector, float scalar)
{
  pResult->x = pVector->x - scalar;
  pResult->y = pVector->y - scalar;
  pResult->z = pVector->z - scalar;
  pResult->w = pVector->w - scalar;
}

float dot_vec(const Vector4_t *pVector, const Vector4_t *pRhs)
{
  return pVector->x * pRhs->x +
         pVector->y * pRhs->y +
         pVector->z * pRhs->z;
}

float dot_scalar(const Vector4_t *pVector, const Vector4_t *pRhs, float scalar)
{
  return pVector->x * scalar * pRhs->x +
         pVector->y * scalar * pRhs->y +
         pVector->z * scalar * pRhs->z +
         pVector->w * scalar * pRhs->w;
}

void math_cross(Vector4_t *result, const Vector4_t *v1, const Vector4_t *v2)
{
  result->x = v1->y * v2->z - v1->z * v2->y;
  result->y = v1->z * v2->x - v1->x * v2->z;
  result->z = v1->x * v2->y - v1->y * v2->x;
}

float length(const Vector4_t *pVector)
{
  return sqrtf(dot_vec(pVector, pVector));
}

void normalize(Vector4_t *v)
{
  scale(v, v, 1.0f / length(v));
}

void plane_normalize_res(Vector4_t *vResult, Vector4_t *vPlane)
{
  *vResult = *vPlane;
  normalize(vResult);
  vResult->w *= vResult->x / vPlane->x;
}

void plane_normalize(Vector4_t *vPlane)
{
  float fOldX = vPlane->x;
  normalize(vPlane);
  vPlane->w *= vPlane->x / fOldX;
}

void add_vec4(Vector4_t *pResult, const Vector4_t *pVector, const Vector4_t *pRhs)
{
  pResult->x = pVector->x + pRhs->x;
  pResult->y = pVector->y + pRhs->y;
  pResult->z = pVector->z + pRhs->z;
  pResult->w = pVector->w + pRhs->w;
}

void sub_vec4(Vector4_t *pResult, const Vector4_t *pVector, const Vector4_t *pRhs)
{
  pResult->x = pVector->x - pRhs->x;
  pResult->y = pVector->y - pRhs->y;
  pResult->z = pVector->z - pRhs->z;
  pResult->w = pVector->w - pRhs->w;
}

int equal(const Matrix_t *lhs, const Matrix_t *rhs)
{
  return lhs->m[0].x == rhs->m[0].x &&
         lhs->m[0].y == rhs->m[0].y &&
         lhs->m[0].z == rhs->m[0].z &&
         lhs->m[0].w == rhs->m[0].w &&

         lhs->m[1].x == rhs->m[1].x &&
         lhs->m[1].y == rhs->m[1].y &&
         lhs->m[1].z == rhs->m[1].z &&
         lhs->m[1].w == rhs->m[1].w &&

         lhs->m[2].x == rhs->m[2].x &&
         lhs->m[2].y == rhs->m[2].y &&
         lhs->m[2].z == rhs->m[2].z &&
         lhs->m[2].w == rhs->m[2].w &&

         lhs->m[3].x == rhs->m[3].x &&
         lhs->m[3].y == rhs->m[3].y &&
         lhs->m[3].z == rhs->m[3].z &&
         lhs->m[3].w == rhs->m[3].w;
}

void transpose_res(Matrix_t *rmat, const Matrix_t *mat)
{
  const mat44 *m = &mat->m;
  mat44 *r = &rmat->m;

  (*r)[0].x = (*m)[0].x;
  (*r)[0].y = (*m)[1].x;
  (*r)[0].z = (*m)[2].x;
  (*r)[0].w = (*m)[3].x;

  (*r)[1].x = (*m)[0].y;
  (*r)[1].y = (*m)[1].y;
  (*r)[1].z = (*m)[2].y;
  (*r)[1].w = (*m)[3].y;

  (*r)[2].x = (*m)[0].z;
  (*r)[2].y = (*m)[1].z;
  (*r)[2].z = (*m)[2].z;
  (*r)[2].w = (*m)[3].z;

  (*r)[3].x = (*m)[0].w;
  (*r)[3].y = (*m)[1].w;
  (*r)[3].z = (*m)[2].w;
  (*r)[3].w = (*m)[3].w;
}

void transpose(Matrix_t *mat)
{
  Matrix_t old = *mat;
  mat44 *r = &mat->m;

  (*r)[0].x = old.m[0].x;
  (*r)[0].y = old.m[1].x;
  (*r)[0].z = old.m[2].x;
  (*r)[0].w = old.m[3].x;

  (*r)[1].x = old.m[0].y;
  (*r)[1].y = old.m[1].y;
  (*r)[1].z = old.m[2].y;
  (*r)[1].w = old.m[3].y;

  (*r)[2].x = old.m[0].z;
  (*r)[2].y = old.m[1].z;
  (*r)[2].z = old.m[2].z;
  (*r)[2].w = old.m[3].z;

  (*r)[3].x = old.m[0].w;
  (*r)[3].y = old.m[1].w;
  (*r)[3].z = old.m[2].w;
  (*r)[3].w = old.m[3].w;
}

void matrix_inverse_nonui(Matrix_t *rmat, const Matrix_t *mat)
{
  const mat44 *m = &mat->m;
  mat44 *r = &rmat->m;

  float m00 = (*m)[0].x, m01 = (*m)[0].y, m02 = (*m)[0].z, m03 = (*m)[0].w;
  float m10 = (*m)[1].x, m11 = (*m)[1].y, m12 = (*m)[1].z, m13 = (*m)[1].w;
  float m20 = (*m)[2].x, m21 = (*m)[2].y, m22 = (*m)[2].z, m23 = (*m)[2].w;
  float m30 = (*m)[3].x, m31 = (*m)[3].y, m32 = (*m)[3].z, m33 = (*m)[3].w;

  float v0 = m20 * m31 - m21 * m30;
  float v1 = m20 * m32 - m22 * m30;
  float v2 = m20 * m33 - m23 * m30;
  float v3 = m21 * m32 - m22 * m31;
  float v4 = m21 * m33 - m23 * m31;
  float v5 = m22 * m33 - m23 * m32;

  float t00 = +(v5 * m11 - v4 * m12 + v3 * m13);
  float t10 = -(v5 * m10 - v2 * m12 + v1 * m13);
  float t20 = +(v4 * m10 - v2 * m11 + v0 * m13);
  float t30 = -(v3 * m10 - v1 * m11 + v0 * m12);

  float invDet = 1 / (t00 * m00 + t10 * m01 + t20 * m02 + t30 * m03);

  (*r)[0].x = t00 * invDet;
  (*r)[1].x = t10 * invDet;
  (*r)[2].x = t20 * invDet;
  (*r)[3].x = t30 * invDet;

  (*r)[0].y = -(v5 * m01 - v4 * m02 + v3 * m03) * invDet;
  (*r)[1].y = +(v5 * m00 - v2 * m02 + v1 * m03) * invDet;
  (*r)[2].y = -(v4 * m00 - v2 * m01 + v0 * m03) * invDet;
  (*r)[3].y = +(v3 * m00 - v1 * m01 + v0 * m02) * invDet;

  v0 = m10 * m31 - m11 * m30;
  v1 = m10 * m32 - m12 * m30;
  v2 = m10 * m33 - m13 * m30;
  v3 = m11 * m32 - m12 * m31;
  v4 = m11 * m33 - m13 * m31;
  v5 = m12 * m33 - m13 * m32;

  (*r)[0].z = +(v5 * m01 - v4 * m02 + v3 * m03) * invDet;
  (*r)[1].z = -(v5 * m00 - v2 * m02 + v1 * m03) * invDet;
  (*r)[2].z = +(v4 * m00 - v2 * m01 + v0 * m03) * invDet;
  (*r)[3].z = -(v3 * m00 - v1 * m01 + v0 * m02) * invDet;

  v0 = m21 * m10 - m20 * m11;
  v1 = m22 * m10 - m20 * m12;
  v2 = m23 * m10 - m20 * m13;
  v3 = m22 * m11 - m21 * m12;
  v4 = m23 * m11 - m21 * m13;
  v5 = m23 * m12 - m22 * m13;

  (*r)[0].w = -(v5 * m01 - v4 * m02 + v3 * m03) * invDet;
  (*r)[1].w = +(v5 * m00 - v2 * m02 + v1 * m03) * invDet;
  (*r)[2].w = -(v4 * m00 - v2 * m01 + v0 * m03) * invDet;
  (*r)[3].w = +(v3 * m00 - v1 * m01 + v0 * m02) * invDet;
}

void plane_create(Vector4_t *plane, const Vector4_t *kP1, const Vector4_t *kP2, const Vector4_t *kP3)
{
  Vector4_t kEdge1;
  Vector4_t kEdge2;

  sub_vec4(&kEdge1, kP2, kP1);
  sub_vec4(&kEdge2, kP1, kP2);

  math_cross(plane, &kEdge1, &kEdge2);
  normalize(plane);

  plane->w = -dot_vec(plane, kP1);
}

float math_min(float a, float b)
{
  return a <= b ? a : b;
}

float math_max(float a, float b)
{
  return a >= b ? a : b;
}

#if 0
float esPlanePointIntersection( const ESPlane* plane, const Vector4_t* vOrigin )
{
  return Vector4_t_dot( plane->vNormal, vOrigin ) + plane.d;
}

float esPlaneSphereIntersection( const ESPlane& plane, const Vector4_t& vOrigin, float fRadius )
{
  return esPlanePointIntersection( plane, vOrigin );
}

bool esSphereIntersectsPlane( const ESPlane& plane, const Vector4_t& vOrigin, float fRadius )
{
  return ( Vector4_tDot( plane.vNormal, vOrigin ) + plane.d ) <= fRadius;
}
*/

#endif

float standardDeviation(Vector3f_t *samples, int axis, int count, float average)
{
  double diffSum = 0.0f;
  for (int i = 0; i < count; ++i)
  {
    double value = samples[i].axis[axis];
    double diff = value - average;
    diffSum += diff * diff;
  }

  double std = sqrt(diffSum / (float)(count ));

  return (float)std;
}
