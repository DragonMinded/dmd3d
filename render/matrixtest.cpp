#include <cstdio>
#include "matrix.h"

#define ASSERT(cond, error) if(!(cond)) { printf("%s:%d - %s (%s)\n", __FILE__, __LINE__, #cond, error); }

void multiply_point_test() {
    Point *point = new Point(10.0, 20.0, 30.0);
    Matrix *matrix = new Matrix();

    matrix->a41 = 2.0;
    matrix->a42 = 3.0;
    matrix->a43 = 4.0;

    Point *newPoint = matrix->multiplyPoint(point);
    ASSERT(newPoint->x == 12.0, "New point has incorrect X value!");
    ASSERT(newPoint->y == 23.0, "New point has incorrect Y value!");
    ASSERT(newPoint->z == 34.0, "New point has incorrect Z value!");

    Matrix *inverted = matrix->clone()->invert();
    Point *origPoint = inverted->multiplyPoint(newPoint);
    ASSERT(origPoint->x == 10.0, "Original point has incorrect X value!");
    ASSERT(origPoint->y == 20.0, "Original point has incorrect Y value!");
    ASSERT(origPoint->z == 30.0, "Original point has incorrect Z value!");

    delete origPoint;
    delete inverted;
    delete newPoint;
    delete matrix;
    delete point;
}

void translate_test() {
    Point *point = new Point(10.0, 20.0, 30.0);
    Matrix *matrix = new Matrix();
    matrix->translateX(2.0)->translateY(3.0)->translateZ(4.0);

    Point *newPoint = matrix->multiplyPoint(point);
    ASSERT(newPoint->x == 12.0, "New point has incorrect X value!");
    ASSERT(newPoint->y == 23.0, "New point has incorrect Y value!");
    ASSERT(newPoint->z == 34.0, "New point has incorrect Z value!");

    Matrix *inverted = matrix->clone()->invert();
    Point *origPoint = inverted->multiplyPoint(newPoint);
    ASSERT(origPoint->x == 10.0, "Original point has incorrect X value!");
    ASSERT(origPoint->y == 20.0, "Original point has incorrect Y value!");
    ASSERT(origPoint->z == 30.0, "Original point has incorrect Z value!");

    delete origPoint;
    delete inverted;
    delete newPoint;
    delete matrix;
    delete point;
}

int main(int argc, char *argv[]) {
    printf("Running matrix tests...\n");

    multiply_point_test();
    translate_test();

    printf("Done!\n");

    return 0;
}
