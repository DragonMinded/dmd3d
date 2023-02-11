#include <cmath>
#include <cstdlib>
#include "model.h"
#include "matrix.h"
#include "common.h"

#define STL_READER_NO_EXCEPTIONS  1
#include "stl_reader.h"

Polygon::Polygon(Point *x, Point *y, Point *z) {
    // Set up initial polygon.
    polyLength = 3;
    polyPoints = (Point **)malloc(sizeof(polyPoints[0]) * 3);

    polyPoints[0] = x->clone();
    polyPoints[1] = y->clone();
    polyPoints[2] = z->clone();

    // Set up transformed polygon copy.
    transPolyLength = 3;
    transPoints = (Point **)malloc(sizeof(transPoints[0]) * 3);

    transPoints[0] = x->clone();
    transPoints[1] = y->clone();
    transPoints[2] = z->clone();

    // Set up whether we are highlighting this polygon's edge or not.
    highlights = (bool *)malloc(sizeof(bool) * 3);
    highlights[0] = true;
    highlights[1] = true;
    highlights[2] = true;

    // We aren't completely culled.
    culled = false;
}

Polygon::Polygon(Point *points[], int length) {
    polyLength = length;
    transPolyLength = length;

    polyPoints = (Point **)malloc(sizeof(polyPoints[0]) * length);
    transPoints = (Point **)malloc(sizeof(transPoints[0]) * length);

    // Safe to do this in one loop because polyLength == transPolyLength here.
    for (int i = 0; i < length; i++) {
        polyPoints[i] = points[i]->clone();
        transPoints[i] = points[i]->clone();
    }

    // Also set up highlights.
    highlights = (bool *)malloc(sizeof(bool) * length);

    for (int i = 0; i < length; i++) {
        highlights[i] = true;
    }

    // We aren't completely culled.
    culled = false;
}

Polygon::~Polygon() {
    for (int i = 0; i < polyLength; i++) {
        delete polyPoints[i];
    }
    for (int i = 0; i < transPolyLength; i++) {
        delete transPoints[i];
    }

    free(polyPoints);
    free(transPoints);
    free(highlights);
    polyPoints = 0;
    transPoints = 0;
    highlights = 0;

    polyLength = 0;
    transPolyLength = 0;
}

void Polygon::transform(Matrix *matrix) {
    for (int i = 0; i < transPolyLength; i++) {
        matrix->multiplyUpdatePoint(transPoints[i]);
    }
}

void Polygon::project(Matrix *matrix) {
    for (int i = 0; i < transPolyLength; i++) {
        matrix->projectUpdatePoint(transPoints[i]);
    }
}

Polygon *Polygon::clone() {
    // Make sure if we clone a polygon that's been transformed, the new is also.
    Polygon *newPoly = new Polygon(transPoints, transPolyLength);
    for (int i = 0; i < transPolyLength; i++) {
        newPoly->highlights[i] = highlights[i];
    }

    return newPoly;
}

void Polygon::reset() {
    // Kill old transformed polygon entirely, it could be frustum culled and have more points than
    // our original.
    for (int i = 0; i < transPolyLength; i++) {
        delete transPoints[i];
    }
    free(transPoints);
    free(highlights);

    transPolyLength = polyLength;
    transPoints = (Point **)malloc(sizeof(transPoints[0]) * polyLength);

    for (int i = 0; i < polyLength; i++) {
        transPoints[i] = polyPoints[i]->clone();
    }

    // Also reset highlights.
    highlights = (bool *)malloc(sizeof(bool) * polyLength);

    for (int i = 0; i < polyLength; i++) {
        highlights[i] = true;
    }

    // We aren't culled.
    culled = false;
}

void Polygon::cull(Frustum *frustum) {
    bool allIn = true;
    bool allOut = true;

    for (int i = 0; i < transPolyLength; i++) {
        for (int j = 0; j < frustum->length; j++) {
            bool inside = frustum->planes[j]->isPointAbove(transPoints[i]);
            if (!inside) {
                allIn = false;

                // No need to check the rest of the planes, we know this is out of the frustum
                // already, so skip the rest.
                break;
            }
            if (inside) {
                allOut = false;
            }
        }
    }

    if (allIn) {
        // Nothing to do, we're already fully inside the bounds.
        return;
    }

    if (allOut) {
        // We're entirely outside the frustum, so simply do not display this polygon.
        culled = true;
        return;
    }

    // TODO: We've got some work to do dividing up this polygon.
    culled = false;
}

void Polygon::draw(Screen *screen) {
    if (!culled) {
        for (int i = 0; i < transPolyLength - 1; i++) {
            if (highlights[i]) {
                screen->drawLine(transPoints[i], transPoints[i + 1], true);
            }
        }

        if (highlights[transPolyLength - 1]) { screen->drawLine(transPoints[transPolyLength - 1], transPoints[0], true); }
    }
}

OccludedWireframePolygon::OccludedWireframePolygon(Point *x, Point *y, Point *z) : Polygon(x, y, z) {}

OccludedWireframePolygon::OccludedWireframePolygon(Point *points[], int length) : Polygon(points, length) {}

Polygon *OccludedWireframePolygon::clone() {
    // Make sure if we clone a polygon that's been transformed, the new is also. Also make sure
    // to copy culled edge information.
    OccludedWireframePolygon *newPoly = new OccludedWireframePolygon(transPoints, transPolyLength);
    for (int i = 0; i < transPolyLength; i++) {
        newPoly->highlights[i] = highlights[i];
    }

    return newPoly;
}

void OccludedWireframePolygon::draw(Screen *screen) {
    if (!culled) {
        screen->drawOccludedPolygon(transPoints, highlights, transPolyLength);
    }
}

Model::Model(Polygon *polygons[], int length) {
    modelLength = length;
    this->polygons = (Polygon **)malloc(sizeof(this->polygons[0]) * length);
    for (int i = 0; i < length; i++) {
        this->polygons[i] = polygons[i]->clone();
    }
}

Model::Model(const char * const modelFile) {
    // TODO: Here would be a good place to figure out if it is a STL file or otherwise.
    stl_reader::StlMesh <float, unsigned int> mesh(modelFile);

    modelLength = mesh.num_tris();
    polygons = (Polygon **)malloc(sizeof(this->polygons[0]) * modelLength);

    for(size_t itri = 0; itri < modelLength; ++itri) {
        // Grab each corner, create a point out of it.
        Point *triPoints[3];

        for(size_t icorner = 0; icorner < 3; ++icorner) {
            const float* c = mesh.tri_corner_coords(itri, icorner);
            triPoints[icorner] = new Point(c[0], c[1], c[2]);
        }

        polygons[itri] = new OccludedWireframePolygon(triPoints, 3);
        delete triPoints[0];
        delete triPoints[1];
        delete triPoints[2];
    }
}

Model::~Model() {
    for (int i = 0; i < modelLength; i++) {
        delete polygons[i];
    }

    free(polygons);
    polygons = 0;
    modelLength = 0;
}

Model *Model::clone() {
    return new Model(polygons, modelLength);
}

void Model::reset() {
    for (int i = 0; i < modelLength; i++) {
        polygons[i]->reset();
    }
}

void Model::transform(Matrix *matrix) {
    for (int i = 0; i < modelLength; i++) {
        polygons[i]->transform(matrix);
    }
}

void Model::project(Matrix *matrix) {
    for (int i = 0; i < modelLength; i++) {
        polygons[i]->project(matrix);
    }
}

void Model::cull(Frustum *frustum) {
    for (int i = 0; i < modelLength; i++) {
        polygons[i]->cull(frustum);
    }
}

void Model::draw(Screen *screen) {
    for (int i = 0; i < modelLength; i++) {
        polygons[i]->draw(screen);
    }
}

Point *Model::getOrigin() {
    double minX, maxX;
    double minY, maxY;
    double minZ, maxZ;

    minX = maxX = polygons[0]->transPoints[0]->x;
    minY = maxY = polygons[0]->transPoints[0]->y;
    minZ = maxZ = polygons[0]->transPoints[0]->z;

    for (int i = 0; i < modelLength; i++) {
        for (int j = 0; j < polygons[i]->polyLength; j++) {
            minX = MIN(minX, polygons[i]->transPoints[j]->x);
            maxX = MAX(maxX, polygons[i]->transPoints[j]->x);
            minY = MIN(minY, polygons[i]->transPoints[j]->y);
            maxY = MAX(maxY, polygons[i]->transPoints[j]->y);
            minZ = MIN(minZ, polygons[i]->transPoints[j]->z);
            maxZ = MAX(maxZ, polygons[i]->transPoints[j]->z);
        }
    }

    return new Point((minX + maxX) / 2.0, (minY + maxY) / 2.0, (minZ + maxZ) / 2.0);
}

Point *Model::getDimensions() {
    double minX, maxX;
    double minY, maxY;
    double minZ, maxZ;

    minX = maxX = polygons[0]->transPoints[0]->x;
    minY = maxY = polygons[0]->transPoints[0]->y;
    minZ = maxZ = polygons[0]->transPoints[0]->z;

    for (int i = 0; i < modelLength; i++) {
        for (int j = 0; j < polygons[i]->polyLength; j++) {
            minX = MIN(minX, polygons[i]->transPoints[j]->x);
            maxX = MAX(maxX, polygons[i]->transPoints[j]->x);
            minY = MIN(minY, polygons[i]->transPoints[j]->y);
            maxY = MAX(maxY, polygons[i]->transPoints[j]->y);
            minZ = MIN(minZ, polygons[i]->transPoints[j]->z);
            maxZ = MAX(maxZ, polygons[i]->transPoints[j]->z);
        }
    }

    return new Point(fabs(maxX - minX), fabs(maxY - minY), fabs(maxZ - minZ));
}
