#ifndef MODEL_H
#define MODEL_H

#include "matrix.h"
#include "raster.h"

class Polygon {
    friend class Model;

    public:
        Polygon(Point *x, Point *y, Point *z);
        Polygon(Point *points[], int length);
        ~Polygon();

        // Clone this polygon, including any intermediate transformations applied.
        virtual Polygon *clone();

        // Undo any transformations applied to this polygon.
        virtual void reset();

        // Perform an affine or perspective transformation on this model.
        void transform(Matrix *matrix);

        // Perform a perspective transformation on this model given a projection matrix.
        void project(Matrix *matrix);

        // Draw this model to the given surface.
        virtual void draw(Screen *screen);

    protected:
        Point **polyPoints;
        int polyLength;

        Point **transPoints;
        int transPolyLength;

        bool culled;
};

class OccludedWireframePolygon : public Polygon {
    public:
        OccludedWireframePolygon(Point *x, Point *y, Point *z);
        OccludedWireframePolygon(Point *points[], int length);
        ~OccludedWireframePolygon();

        virtual Polygon *clone();
        virtual void reset();
        virtual void draw(Screen *screen);

    protected:
        bool *highlights;
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

        // Return a point representing the origin of this mode.
        Point *getOrigin();

        // Return a point representing the maximum x, y and z distance between two furthest points on the model.
        Point *getDimensions();

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
