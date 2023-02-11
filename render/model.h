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

        // Perform an affine or perspective transformation on this polygon.
        void transform(Matrix *matrix);

        // Perform a perspective transformation on this polygon given a projection matrix.
        void project(Matrix *matrix);

        // Perform a frustum cull on this polygon.
        virtual void cull(Frustum *frustum);

        // Draw this model to the given surface.
        virtual void draw(Screen *screen);

    protected:
        Point **polyPoints;
        int polyLength;

        Point **transPoints;
        bool *highlights;
        int transPolyLength;

        bool culled;
};

class OccludedWireframePolygon : public Polygon {
    public:
        OccludedWireframePolygon(Point *x, Point *y, Point *z);
        OccludedWireframePolygon(Point *points[], int length);

        // Clone this exact class of polygon.
        virtual Polygon *clone();
        virtual void draw(Screen *screen);
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

        // Perform a frustum cull on this model given a set of planes making up a frustum.
        void cull(Frustum *frustum);

        // Draw this model to the given surface.
        void draw(Screen *screen);

    private:
        Polygon **polygons;
        int modelLength;
};

#endif
