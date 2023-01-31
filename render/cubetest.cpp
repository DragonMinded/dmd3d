#include <cstdio>
#include <cmath>
#include <cstring>
#include "matrix.h"
#include "raster.h"

int main (int argc, char *argv[]) {
    printf("Running cube tests...\n");

    Screen *screen = new Screen();
    int count = 0;

    while ( 1 ) {
        // Set up our pixel buffer.
        screen->clear();

        // Set up our throbbing cube.
        double val = (0.5 + (sin((count / 30.0) * M_PI) / 16.0));
        Point *coords[8] = {
            new Point(-val, -val, -val),
            new Point( val, -val, -val),
            new Point( val,  val, -val),
            new Point(-val,  val, -val),
            new Point(-val, -val,  val),
            new Point( val, -val,  val),
            new Point( val,  val,  val),
            new Point(-val,  val,  val),
        };

        // Manipulate location of object in world.
        Matrix *effectsMatrix = new Matrix();
        effectsMatrix->translateZ(2.5);
        effectsMatrix->rotateX(60 + count);
        effectsMatrix->rotateY(30 + count);
        effectsMatrix->multiplyPoints(coords, sizeof(coords) / sizeof(coords[0]));

        // Set up the view matrix.
        Matrix *viewMatrix = new Matrix(SIGN_WIDTH, SIGN_HEIGHT, 90.0, 1.0, 1000.0);

        // Move the cube to where it should go.
        viewMatrix->multiplyPoints(coords, sizeof(coords) / sizeof(coords[0]));

        // Draw the cube.
        screen->drawQuad(coords[0], coords[1], coords[2], coords[3], true);
        screen->drawQuad(coords[4], coords[5], coords[6], coords[7], true);
        screen->drawLine(coords[0], coords[4], true);
        screen->drawLine(coords[1], coords[5], true);
        screen->drawLine(coords[2], coords[6], true);
        screen->drawLine(coords[3], coords[7], true);

        // Render it to the screen.
        screen->waitForVBlank();
        screen->renderFrame();

        // Clean up.
        for (int i = 0; i < sizeof(coords) / sizeof(coords[0]); i++) {
            delete coords[i];
        }

        // Keep track of location.
        count++;
    }

    delete screen;
    printf("Done!\n");

    return 0;
}
