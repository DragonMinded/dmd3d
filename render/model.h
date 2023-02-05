#ifndef MODEL_H
#define MODEL_H

#include "matrix.h"
#include "raster.h"

class Polygon {
    public:
        Polygon(Point *x, Point *y, Point *z);
        Polygon(Point *points[], int length);
        ~Polygon();

        // Clone this polygon, including any intermediate transformations applied.
        Polygon *clone();

        // Undo any transformations applied to this polygon.
        void reset();

        // Perform an affine or perspective transformation on this model.
        void transform(Matrix *matrix);

        // Perform a perspective transformation on this model given a projection matrix.
        void project(Matrix *matrix);

        // Draw this model to the given surface.
        void draw(Screen *screen);

    private:
        Point **polyPoints;
        Point **transPoints;
        int polyLength;
};

class Model {
    public:
        Model(Polygon *polygons[], int length);
        Model(const char *const modelFile);
        ~Model();

        // Clone this model, including any intermediate transformations applied.
        Model *clone();

        // Undo any transformations applied to this model.
        void reset();

        // Perform an affine or perspective transformation on this model.
        void transform(Matrix *matrix);

        // Perform a perspective transformation on this model given a projection matrix.
        void project(Matrix *matrix);

        // Draw this model to the given surface.
        void draw(Screen *screen);

    private:
        Polygon **polygons;
        int modelLength;
};

#endif
