#include <cstdio>
#include <cmath>
#include <cstring>
#include "matrix.h"
#include "model.h"
#include "raster.h"
#include "common.h"

int main (int argc, char *argv[]) {
    printf("Running STL model tests...\n");

    Screen *screen = new Screen(SIGN_WIDTH, SIGN_HEIGHT);
    int count = 0;

    // Load the model.
    Model *model = new Model("testmodel.stl", FLAGS_OCCLUDED);
    model->coalesce();
    Point *origin = model->getOrigin();

    Point *dimensions = model->getDimensions();
    double maxDimension = MAX(MAX(dimensions->x, dimensions->y), dimensions->z) / 2.25;
    delete dimensions;

    // Set up a simple frustum for culling.
    Frustum *frustum = new Frustum(SIGN_WIDTH, SIGN_HEIGHT, 60.0, 1.0, 1000.0);

    while ( 1 ) {
        // Set up our pixel buffer.
        screen->clear();
        model->reset();

        // Set up the view matrix.
        Matrix *viewMatrix = new Matrix(SIGN_WIDTH, SIGN_HEIGHT, 60.0, 1.0, 1000.0);

        // Manipulate location of object in world.
        Matrix *effectsMatrix = new Matrix();

        // Move it back into the screen so it's visible.
        effectsMatrix->translateZ(2.5);

        // Get it in the center, normalize its size.
        effectsMatrix->scale(1.0 / maxDimension, 1.0 / maxDimension, 1.0 / maxDimension);

        // Rotate it about its origin randomly.
        effectsMatrix->translateZ(origin->z);
        effectsMatrix->rotateX((count * 0.2));
        effectsMatrix->rotateY((count * 2.5));
        effectsMatrix->translate(-origin->x, -origin->y, -origin->z);

        // Transform the full cube based on our effects above (in reverse order).
        model->transform(effectsMatrix);
        delete effectsMatrix;

        // Cull any polygons outside of our frustum.
        model->cull(frustum);

        // Move the cube to where it should go.
        model->project(viewMatrix);

        // Draw the model to the screen.
        model->draw(screen);

        // Render it to the screen.
        screen->waitForVBlank();
        screen->renderFrame();

        // Clean up.
        delete viewMatrix;

        // Keep track of location.
        count++;
    }

    delete frustum;
    delete model;
    delete screen;
    printf("Done!\n");

    return 0;
}
