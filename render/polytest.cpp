#include <cstdio>
#include <cmath>
#include <cstring>
#include "matrix.h"
#include "raster.h"

int main (int argc, char *argv[]) {
    printf("Running poly tests...\n");

    Screen *screen = new Screen();
    int count = 0;

    while ( 1 ) {
        // Set up our pixel buffer.
        screen->clear();

        // Set up the view matrix.
        Matrix *viewMatrix = new Matrix(SIGN_WIDTH, SIGN_HEIGHT, 60.0, 1.0, 1000.0);

        // Set up our gemstone.
        Point *leftCoords[] = {
            // Top part of gemstone.
            new Point(-3.3, 0,    0),
            new Point(-1.5, 3.0,  0),
            new Point(1.5,  3.0,  0),
            new Point(3.3,  0,    0),
            new Point(1.5,  -3.0, 0),
            new Point(-1.5, -3.0, 0),

            // Bottom part of gemstone.
            new Point(-1.5, -3.0, 3),
            new Point(1.5,  -3.0, 3),
            new Point(3.3,  0,    3),
            new Point(1.5,  3.0,  3),
            new Point(-1.5, 3.0,  3),
            new Point(-3.3, 0,    3),
        };

        // Manipulate location of object in world.
        Matrix *effectsMatrix = new Matrix();
        effectsMatrix->translateZ(10.0);
        effectsMatrix->translateX(-5.0);
        effectsMatrix->rotateX(60 + (count * 1.0));
        effectsMatrix->rotateY(30 + (count * 1.1));
        effectsMatrix->rotateZ(count * 3.0);
        effectsMatrix->multiplyPoints(leftCoords, sizeof(leftCoords) / sizeof(leftCoords[0]));
        delete effectsMatrix;

        // Move the cube to where it should go.
        viewMatrix->projectPoints(leftCoords, sizeof(leftCoords) / sizeof(leftCoords[0]));

        // Draw the polygons.
        screen->drawPolygon(&leftCoords[0], 6, true);
        screen->drawPolygon(&leftCoords[6], 6, true);

        // Draw the squares between each bit, which is "fun" to work out.
        screen->drawQuad(leftCoords[0], leftCoords[11], leftCoords[10], leftCoords[1], true);
        screen->drawQuad(leftCoords[1], leftCoords[10], leftCoords[9], leftCoords[2], true);
        screen->drawQuad(leftCoords[2], leftCoords[9], leftCoords[8], leftCoords[3], true);
        screen->drawQuad(leftCoords[3], leftCoords[8], leftCoords[7], leftCoords[4], true);
        screen->drawQuad(leftCoords[4], leftCoords[7], leftCoords[6], leftCoords[5], true);
        screen->drawQuad(leftCoords[5], leftCoords[6], leftCoords[11], leftCoords[0], true);

        // Set up our second gemstone, this time with culling of wireframe stuff.
        Point *rightCoords[] = {
            // Top part of gemstone.
            new Point(-3.3, 0,    0),
            new Point(-1.5, 3.0,  0),
            new Point(1.5,  3.0,  0),
            new Point(3.3,  0,    0),
            new Point(1.5,  -3.0, 0),
            new Point(-1.5, -3.0, 0),

            // Bottom part of gemstone.
            new Point(-1.5, -3.0, 3),
            new Point(1.5,  -3.0, 3),
            new Point(3.3,  0,    3),
            new Point(1.5,  3.0,  3),
            new Point(-1.5, 3.0,  3),
            new Point(-3.3, 0,    3),
        };

        // Manipulate location of object in world.
        effectsMatrix = new Matrix();
        effectsMatrix->translateZ(10.0);
        effectsMatrix->translateX(5.0);
        effectsMatrix->rotateX(60 + (count * 1.2));
        effectsMatrix->rotateY(30 + (count * 1.3));
        effectsMatrix->rotateZ(count * -3.0);
        effectsMatrix->multiplyPoints(rightCoords, sizeof(rightCoords) / sizeof(rightCoords[0]));
        delete effectsMatrix;

        // Move the cube to where it should go.
        viewMatrix->projectPoints(rightCoords, sizeof(rightCoords) / sizeof(rightCoords[0]));

        // Draw the polygons.
        screen->drawOccludedPolygon(&rightCoords[0], 6);
        screen->drawOccludedPolygon(&rightCoords[6], 6);

        // Draw the squares between each bit, which is "fun" to work out.
        screen->drawOccludedQuad(rightCoords[0], rightCoords[11], rightCoords[10], rightCoords[1]);
        screen->drawOccludedQuad(rightCoords[1], rightCoords[10], rightCoords[9], rightCoords[2]);
        screen->drawOccludedQuad(rightCoords[2], rightCoords[9], rightCoords[8], rightCoords[3]);
        screen->drawOccludedQuad(rightCoords[3], rightCoords[8], rightCoords[7], rightCoords[4]);
        screen->drawOccludedQuad(rightCoords[4], rightCoords[7], rightCoords[6], rightCoords[5]);
        screen->drawOccludedQuad(rightCoords[5], rightCoords[6], rightCoords[11], rightCoords[0]);

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
