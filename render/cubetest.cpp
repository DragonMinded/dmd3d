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

        // Set up the view matrix.
        Matrix *viewMatrix = new Matrix(SIGN_WIDTH, SIGN_HEIGHT, 90.0, 1.0, 1000.0);

        // Set up our throbbing cube.
        double val = (0.5 + (sin((count / 30.0) * M_PI) / 16.0));
        Point *leftCoords[8] = {
            new Point(-val,  val, -val),
            new Point( val,  val, -val),
            new Point( val, -val, -val),
            new Point(-val, -val, -val),
            new Point(-val,  val,  val),
            new Point( val,  val,  val),
            new Point( val, -val,  val),
            new Point(-val, -val,  val),
        };

        // Manipulate location of object in world.
        Matrix *effectsMatrix = new Matrix();
        effectsMatrix->translateZ(2.75);
        effectsMatrix->translateX(-1.0);
        effectsMatrix->rotateX(60 + (count * 1.0));
        effectsMatrix->rotateY(30 + (count * 1.1));
        effectsMatrix->multiplyPoints(leftCoords, sizeof(leftCoords) / sizeof(leftCoords[0]));
        delete effectsMatrix;

        // Move the cube to where it should go.
        viewMatrix->projectPoints(leftCoords, sizeof(leftCoords) / sizeof(leftCoords[0]));

        // Draw the cube.
        screen->drawQuad(leftCoords[0], leftCoords[1], leftCoords[2], leftCoords[3], true);
        screen->drawQuad(leftCoords[4], leftCoords[5], leftCoords[6], leftCoords[7], true);
        screen->drawLine(leftCoords[0], leftCoords[4], true);
        screen->drawLine(leftCoords[1], leftCoords[5], true);
        screen->drawLine(leftCoords[2], leftCoords[6], true);
        screen->drawLine(leftCoords[3], leftCoords[7], true);

        // Set up our second throbbing cube, this time with culling of wireframe stuff.
        Point *rightCoords[8] = {
            new Point(-val,  val, -val),
            new Point( val,  val, -val),
            new Point( val, -val, -val),
            new Point(-val, -val, -val),
            new Point(-val,  val,  val),
            new Point( val,  val,  val),
            new Point( val, -val,  val),
            new Point(-val, -val,  val),
        };

        // Manipulate location of object in world.
        effectsMatrix = new Matrix();
        effectsMatrix->translateZ(2.75);
        effectsMatrix->translateX(1.0);
        effectsMatrix->rotateX(60 + (count * 1.2));
        effectsMatrix->rotateY(30 + (count * 1.3));
        effectsMatrix->multiplyPoints(rightCoords, sizeof(rightCoords) / sizeof(rightCoords[0]));
        delete effectsMatrix;

        // Move the cube to where it should go.
        viewMatrix->projectPoints(rightCoords, sizeof(rightCoords) / sizeof(rightCoords[0]));

        // Draw the cube.
        screen->drawOccludedQuad(rightCoords[0], rightCoords[1], rightCoords[2], rightCoords[3]);
        screen->drawOccludedQuad(rightCoords[5], rightCoords[4], rightCoords[7], rightCoords[6]);
        screen->drawOccludedQuad(rightCoords[0], rightCoords[4], rightCoords[5], rightCoords[1]);
        screen->drawOccludedQuad(rightCoords[1], rightCoords[5], rightCoords[6], rightCoords[2]);
        screen->drawOccludedQuad(rightCoords[2], rightCoords[6], rightCoords[7], rightCoords[3]);
        screen->drawOccludedQuad(rightCoords[0], rightCoords[3], rightCoords[7], rightCoords[4]);

        // Render it to the screen.
        screen->waitForVBlank();
        screen->renderFrame();

        // Clean up.
        delete viewMatrix;
        for (int i = 0; i < sizeof(leftCoords) / sizeof(leftCoords[0]); i++) {
            delete leftCoords[i];
        }
        for (int i = 0; i < sizeof(rightCoords) / sizeof(rightCoords[0]); i++) {
            delete rightCoords[i];
        }

        // Keep track of location.
        count++;
    }

    delete screen;
    printf("Done!\n");

    return 0;
}
