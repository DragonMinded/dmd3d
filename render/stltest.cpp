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
    Model *model = new Model("testmodel.stl");

    while ( 1 ) {
        // Set up our pixel buffer.
        screen->clear();
        model->reset();

        // Set up the view matrix.
        Matrix *viewMatrix = new Matrix(SIGN_WIDTH, SIGN_HEIGHT, 90.0, 1.0, 1000.0);

        // Manipulate location of object in world.
        Matrix *effectsMatrix = new Matrix();

        // Move it back into the screen so it's visible.
        effectsMatrix->translateZ(2.5);

        // Rotate it about its origin randomly.
        effectsMatrix->rotateX((count * 0.2));
        effectsMatrix->rotateY((count * 2.5));

        // Get it in the center.
        effectsMatrix->translate(-0.5, -0.5, -0.5);

        // Transform the full cube based on our effects above (in reverse order).
        model->transform(effectsMatrix);
        delete effectsMatrix;

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

    delete model;
    delete screen;
    printf("Done!\n");

    return 0;
}
