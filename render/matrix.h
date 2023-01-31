#ifndef MATRIX_H
#define MATRIX_H

class Point {
    public:
        // Constructor
        Point(double x, double y, double z);

        double x;
        double y;
        double z;
};

class Matrix {
    public:
        // Constructor (makes the identity matrix).
        Matrix();

        // Constructor (makes a perspective matrix given a fov in degrees).
        Matrix(int width, int height, double fov, double zNear, double zFar);

        // Return a clone of this matrix.
        Matrix *clone();

        // Invert this matrix, if it can be inverted.
        Matrix *invert();

        // Apply another matrix to this matrix, by multiply them.
        Matrix *multiply(Matrix *other);

        // Multiply a point to translate/rotate/scale that point in 3D space.
        Point *multiplyPoint(Point *point);

        // Multiply an array of poitns to translate/rotate/scale those points in 3D space, updating the points in-place.
        void multiplyPoints(Point *points[], int length);

        // Translate this matrix by an X/Y/Z value represented by a point.
        Matrix *translate(Point *point);
        Matrix *translate(double x, double y, double z);

        // Translate this matrix by an arbitrary axis.
        Matrix *translateX(double x);
        Matrix *translateY(double y);
        Matrix *translateZ(double z);

        // Scale this matrix by X/Y/Z scaling constants represented by a point.
        Matrix *scale(Point *point);
        Matrix *scale(double x, double y, double z);

        // Scale this matrix by an arbitrary axis.
        Matrix *scaleX(double x);
        Matrix *scaleY(double y);
        Matrix *scaleZ(double z);

        // Rotate this matrix about an arbitrary axis by an angle in degrees.
        Matrix *rotateX(double degs);
        Matrix *rotateY(double degs);
        Matrix *rotateZ(double degs);

        // Rotate this matrix about an arbitrary axis against an origin represented by a point.
        Matrix *rotateOriginX(Point *origin, double degs);
        Matrix *rotateOriginY(Point *origin, double degs);
        Matrix *rotateOriginZ(Point *origin, double degs);

        // The actual bits of the matrix.
        double a11;
        double a12;
        double a13;
        double a14;
        double a21;
        double a22;
        double a23;
        double a24;
        double a31;
        double a32;
        double a33;
        double a34;
        double a41;
        double a42;
        double a43;
        double a44;
};

#endif
