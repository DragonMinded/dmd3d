#include <cmath>
#include "matrix.h"


Point::Point(double x, double y, double z) {
    this->x = x;
    this->y = y;
    this->z = z;
}


Matrix::Matrix() {
    a11 = 1.0;
    a12 = 0.0;
    a13 = 0.0;
    a14 = 0.0;
    a21 = 0.0;
    a22 = 1.0;
    a23 = 0.0;
    a24 = 0.0;
    a31 = 0.0;
    a32 = 0.0;
    a33 = 1.0;
    a34 = 0.0;
    a41 = 0.0;
    a42 = 0.0;
    a43 = 0.0;
    a44 = 1.0;
}

Matrix::Matrix(int width, int height, double fov, double zNear, double zFar) {
    a11 = 1.0;
    a12 = 0.0;
    a13 = 0.0;
    a14 = 0.0;
    a21 = 0.0;
    a22 = 1.0;
    a23 = 0.0;
    a24 = 0.0;
    a31 = 0.0;
    a32 = 0.0;
    a33 = 1.0;
    a34 = 0.0;
    a41 = 0.0;
    a42 = 0.0;
    a43 = 0.0;
    a44 = 1.0;

    // Create the part of the matrix that will give us the correct destination coordinates
    double halfwidth = width / 2.0;
    double halfheight = height / 2.0;

    Matrix *screenViewMatrix = new Matrix();
    screenViewMatrix->a11 = halfwidth;
    screenViewMatrix->a22 = halfheight;
    screenViewMatrix->a41 = halfwidth;
    screenViewMatrix->a42 = halfheight;

    // Create a projection matrix which allows for perspective projection.
    double fovrads = (fov / 180.0) * M_PI;
    double aspect = halfwidth / halfheight;
    double cot_fovy_2 = cos(fovrads / 2.0) / sin(fovrads / 2.0);

    Matrix *projectionMatrix = new Matrix();
    projectionMatrix->a11 = -cot_fovy_2 / aspect;
    projectionMatrix->a22 = cot_fovy_2;
    projectionMatrix->a33 = (zFar+zNear)/(zNear-zFar);
    projectionMatrix->a43 = 2*zFar*zNear/(zNear-zFar);
    
    multiply(screenViewMatrix);
    multiply(projectionMatrix);

    delete screenViewMatrix;
    delete projectionMatrix;
}

Point *Matrix::multiplyPoint(Point *point) {
    return new Point(
        (a11 * point->x) + (a21 * point->y) + (a31 * point->z) + a41,
        (a12 * point->x) + (a22 * point->y) + (a32 * point->z) + a42,
        (a13 * point->x) + (a23 * point->y) + (a33 * point->z) + a43
    );
}

Matrix *Matrix::translate(double x, double y, double z) {
    Point *point = new Point(x, y, z);
    translate(point->x, point->y, point->z);

    delete point;

    return this;
}

Matrix *Matrix::translate(Point *point) {
    Point *newPoint = multiplyPoint(point);

    a41 = newPoint->x;
    a42 = newPoint->y;
    a43 = newPoint->z;

    delete newPoint;

    return this;
}

Matrix *Matrix::translateX(double x) {
    Point *point = new Point(x, 0.0, 0.0);
    translate(point);
    delete point;

    return this;
}

Matrix *Matrix::translateY(double y) {
    Point *point = new Point(0.0, y, 0.0);
    translate(point);
    delete point;

    return this;
}

Matrix *Matrix::translateZ(double z) {
    Point *point = new Point(0.0, 0.0, z);
    translate(point);
    delete point;

    return this;
}

Matrix *Matrix::scale(double x, double y, double z) {
    Matrix *tmp = new Matrix();
    tmp->a11 = x;
    tmp->a22 = y;
    tmp->a33 = z;
    
    multiply(tmp);
    delete tmp;

    return this;
}

Matrix *Matrix::scale(Point *point) {
    return scale(point->x, point->y, point->z);
}

Matrix *Matrix::scaleX(double x) {
    return scale(x, 1.0, 1.0);
}

Matrix *Matrix::scaleY(double y) {
    return scale(1.0, y, 1.0);
}

Matrix *Matrix::scaleZ(double z) {
    return scale(1.0, 1.0, z);
}

Matrix *Matrix::rotateX(double degrees) {
    Matrix *tmp = new Matrix();
    tmp->a22 = tmp->a33 = cos((degrees / 180.0) * M_PI);
    tmp->a23 = -(tmp->a32 = sin((degrees / 180.0) * M_PI));

    multiply(tmp);
    delete tmp;

    return this;
}

Matrix *Matrix::rotateY(double degrees) {
    Matrix *tmp = new Matrix();
    tmp->a11 = tmp->a33 = cos((degrees / 180.0) * M_PI);
    tmp->a31 = -(tmp->a13 = sin((degrees / 180.0) * M_PI));

    multiply(tmp);
    delete tmp;

    return this;
}

Matrix *Matrix::rotateZ(double degrees) {
    Matrix *tmp = new Matrix();
    tmp->a11 = tmp->a22 = cos((degrees / 180.0) * M_PI);
    tmp->a12 = -(tmp->a21 = sin((degrees / 180.0) * M_PI));

    multiply(tmp);
    delete tmp;

    return this;
}

Matrix *Matrix::rotateOriginX(Point *origin, double degrees) {
    Matrix *move = new Matrix();
    move->a41 = origin->x;
    move->a42 = origin->y;
    move->a43 = origin->z;
    multiply(move);

    rotateX(degrees);

    move->a41 = -origin->x;
    move->a42 = -origin->y;
    move->a43 = -origin->z;
    multiply(move);

    delete move;

    return this;
}

Matrix *Matrix::rotateOriginY(Point *origin, double degrees) {
    Matrix *move = new Matrix();
    move->a41 = origin->x;
    move->a42 = origin->y;
    move->a43 = origin->z;
    multiply(move);

    rotateY(degrees);

    move->a41 = -origin->x;
    move->a42 = -origin->y;
    move->a43 = -origin->z;
    multiply(move);

    delete move;

    return this;
}

Matrix *Matrix::rotateOriginZ(Point *origin, double degrees) {
    Matrix *move = new Matrix();
    move->a41 = origin->x;
    move->a42 = origin->y;
    move->a43 = origin->z;
    multiply(move);

    rotateZ(degrees);

    move->a41 = -origin->x;
    move->a42 = -origin->y;
    move->a43 = -origin->z;
    multiply(move);

    delete move;

    return this;
}

Matrix *Matrix::clone() {
    Matrix *newMatrix = new Matrix();

    newMatrix->a11 = a11;
    newMatrix->a12 = a12;
    newMatrix->a13 = a13;
    newMatrix->a14 = a14;
    newMatrix->a21 = a21;
    newMatrix->a22 = a22;
    newMatrix->a23 = a23;
    newMatrix->a24 = a24;
    newMatrix->a31 = a31;
    newMatrix->a32 = a32;
    newMatrix->a33 = a33;
    newMatrix->a34 = a34;
    newMatrix->a41 = a41;
    newMatrix->a42 = a42;
    newMatrix->a43 = a43;
    newMatrix->a44 = a44;

    return newMatrix;
}

double _minor(double m[16], int r0, int r1, int r2, int c0, int c1, int c2)
{
    return (
        m[4*r0+c0] * (m[4*r1+c1] * m[4*r2+c2] - m[4*r2+c1] * m[4*r1+c2]) -
        m[4*r0+c1] * (m[4*r1+c0] * m[4*r2+c2] - m[4*r2+c0] * m[4*r1+c2]) +
        m[4*r0+c2] * (m[4*r1+c0] * m[4*r2+c1] - m[4*r2+c0] * m[4*r1+c1])
    );
}

void _adjoint(double m[16], double adjOut[16])
{
    adjOut[0] = _minor(m,1,2,3,1,2,3);
    adjOut[1] = -_minor(m,0,2,3,1,2,3);
    adjOut[2] = _minor(m,0,1,3,1,2,3);
    adjOut[3] = -_minor(m,0,1,2,1,2,3);
    adjOut[4] = -_minor(m,1,2,3,0,2,3);
    adjOut[5] = _minor(m,0,2,3,0,2,3);
    adjOut[6] = -_minor(m,0,1,3,0,2,3);
    adjOut[7] = _minor(m,0,1,2,0,2,3);
    adjOut[8] = _minor(m,1,2,3,0,1,3);
    adjOut[9] = -_minor(m,0,2,3,0,1,3);
    adjOut[10] = _minor(m,0,1,3,0,1,3);
    adjOut[11] = -_minor(m,0,1,2,0,1,3);
    adjOut[12] = -_minor(m,1,2,3,0,1,2);
    adjOut[13] = _minor(m,0,2,3,0,1,2);
    adjOut[14] = -_minor(m,0,1,3,0,1,2);
    adjOut[15] = _minor(m,0,1,2,0,1,2);
}

double _det(double m[16])
{
    return (
        m[0] * _minor(m, 1, 2, 3, 1, 2, 3) -
        m[1] * _minor(m, 1, 2, 3, 0, 2, 3) +
        m[2] * _minor(m, 1, 2, 3, 0, 1, 3) -
        m[3] * _minor(m, 1, 2, 3, 0, 1, 2)
    );
}

Matrix *Matrix::invert()
{
    double orig[16] = {
        a11, a12, a13, a14,
        a21, a22, a23, a24,
        a31, a32, a33, a34,
        a41, a42, a43, a44
    };
    double invOut[16];

    _adjoint(orig, invOut);

    double inv_det = 1.0f / _det(orig);
    for(int i = 0; i < 16; i++)
    {
        invOut[i] = invOut[i] * inv_det;
    }

    a11 = invOut[0];
    a12 = invOut[1];
    a13 = invOut[2];
    a14 = invOut[3];
    a21 = invOut[4];
    a22 = invOut[5];
    a23 = invOut[6];
    a24 = invOut[7];
    a31 = invOut[8];
    a32 = invOut[9];
    a33 = invOut[10];
    a34 = invOut[11];
    a41 = invOut[12];
    a42 = invOut[13];
    a43 = invOut[14];
    a44 = invOut[15];

    return this;
}

Matrix *Matrix::multiply(Matrix *other) {
    // First row.
    a11 = (a11 * other->a11) + (a12 * other->a21) + (a13 * other->a31) + (a14 * other->a41);
    a12 = (a11 * other->a12) + (a12 * other->a22) + (a13 * other->a32) + (a14 * other->a42);
    a13 = (a11 * other->a13) + (a12 * other->a23) + (a13 * other->a33) + (a14 * other->a43);
    a14 = (a11 * other->a14) + (a12 * other->a24) + (a13 * other->a34) + (a14 * other->a44);

    // Second row.
    a21 = (a21 * other->a11) + (a22 * other->a21) + (a23 * other->a31) + (a24 * other->a41);
    a22 = (a21 * other->a12) + (a22 * other->a22) + (a23 * other->a32) + (a24 * other->a42);
    a23 = (a21 * other->a13) + (a22 * other->a23) + (a23 * other->a33) + (a24 * other->a43);
    a24 = (a21 * other->a14) + (a22 * other->a24) + (a23 * other->a34) + (a24 * other->a44);

    // Third row.
    a31 = (a31 * other->a11) + (a32 * other->a21) + (a33 * other->a31) + (a34 * other->a41);
    a32 = (a31 * other->a12) + (a32 * other->a22) + (a33 * other->a32) + (a34 * other->a42);
    a33 = (a31 * other->a13) + (a32 * other->a23) + (a33 * other->a33) + (a34 * other->a43);
    a34 = (a31 * other->a14) + (a32 * other->a24) + (a33 * other->a34) + (a34 * other->a44);

    // Forth row.
    a41 = (a41 * other->a11) + (a42 * other->a21) + (a43 * other->a31) + (a44 * other->a41);
    a42 = (a41 * other->a12) + (a42 * other->a22) + (a43 * other->a32) + (a44 * other->a42);
    a43 = (a41 * other->a13) + (a42 * other->a23) + (a43 * other->a33) + (a44 * other->a43);
    a44 = (a41 * other->a14) + (a42 * other->a24) + (a43 * other->a34) + (a44 * other->a44);

    return this;
}
