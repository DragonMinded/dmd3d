#include <cstdio>
#include "matrix.h"

#define ASSERT(cond, error) if(!(cond)) { printf("%s:%d - %s (%s)\n", __FILE__, __LINE__, #cond, error); }

void point_key_test() {
    Point point1(10.0, 20.0, 30.0);
    Point point2(10.0, 30.0, 20.0);
    Point point3(10.0, 20.0, 40.0);
    Point point4(10.0, 20.0, 30.0);

    // Points that are obviously unequal.
    ASSERT(point1 < point2, "Point 1 and point 2 don't compare properly!")
    ASSERT(point2 > point1, "Point 1 and point 2 don't compare properly!")
    ASSERT(!(point2 < point1), "Point 2 and point 1 don't compare properly!")
    ASSERT(!(point1 > point2), "Point 2 and point 1 don't compare properly!")
    ASSERT(!(point1 == point2), "Point 1 and point 2 don't compare properly!")

    ASSERT(point1 < point3, "Point 1 and point 3 don't compare properly!")
    ASSERT(point3 > point1, "Point 1 and point 3 don't compare properly!")
    ASSERT(!(point3 < point1), "Point 3 and point 1 don't compare properly!")
    ASSERT(!(point1 > point3), "Point 3 and point 1 don't compare properly!")
    ASSERT(!(point1 == point3), "Point 1 and point 3 don't compare properly!")

    // Points that should be equal.
    ASSERT(!(point1 < point4), "Point 1 and point 4 don't compare properly!")
    ASSERT(!(point4 > point1), "Point 1 and point 4 don't compare properly!")
    ASSERT(!(point4 < point1), "Point 4 and point 1 don't compare properly!")
    ASSERT(!(point1 > point4), "Point 4 and point 1 don't compare properly!")
    ASSERT(point1 == point4, "Point 1 and point 4 don't compare properly!")
}

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

void plane_test() {
    // First, an easy plane.
    {
        Point first(0.0, 0.0, 0.0);
        Point second(1.0, 0.0, 0.0);
        Point third(0.0, 1.0, 0.0);

        Plane xyPlane(&first, &second, &third);

        Point above(1.0, 2.0, 3.0);
        Point below(2.0, 3.0, -4.0);
        ASSERT(xyPlane.isPointAbove(&above), "Point is not above plane but should be?");
        ASSERT(!xyPlane.isPointAbove(&below), "Point is above plane but shouldn't be?");

        Point lineStart(1.0, 2.0, 3.0);
        Point lineEnd(1.0, 2.0, -3.0);
        Point *intersection = xyPlane.intersection(&lineStart, &lineEnd);
        ASSERT(intersection->x == 1.0, "Intersection point is wrong!");
        ASSERT(intersection->y == 2.0, "Intersection point is wrong!");
        ASSERT(intersection->z == 0.0, "Intersection point is wrong!");
        delete intersection;

        lineStart.x = 0.0;
        lineStart.y = 3.0;
        intersection = xyPlane.intersection(&lineStart, &lineEnd);
        ASSERT(intersection->x == 0.5, "Intersection point is wrong!");
        ASSERT(intersection->y == 2.5, "Intersection point is wrong!");
        ASSERT(intersection->z == 0.0, "Intersection point is wrong!");
        delete intersection;
    }

    // Now, a vertical plane.
    {
        Point first(0.0, 0.0, 0.0);
        Point second(0.0, 1.0, 0.0);
        Point third(0.0, 0.0, 1.0);

        Plane yzPlane(&first, &second, &third);
        Point above(1.0, 2.0, 3.0);
        Point below(-2.0, 3.0, 4.0);
        ASSERT(yzPlane.isPointAbove(&above), "Point is not above plane but should be?");
        ASSERT(!yzPlane.isPointAbove(&below), "Point is above plane but shouldn't be?");
    }
}

int main(int argc, char *argv[]) {
    printf("Running matrix tests...\n");

    point_key_test();
    multiply_point_test();
    translate_test();
    plane_test();

    printf("Done!\n");

    return 0;
}
