#include <cmath>
#include <cstdlib>
#include <cstring>
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
    // Number of planes we are inside. Should equal the number of planes in the frustum
    // if we are entirely inside the frustum.
    int insidePlaneCount = 0;

    for (int j = 0; j < frustum->length; j++) {
        // Count how many edges are inside this particular plane. Should equal the count
        // of points if we're entirely inside this plane.
        int insidePointCount = 0;

        for (int i = 0; i < transPolyLength; i++) {
            bool inside = frustum->planes[j]->isPointAbove(transPoints[i]);
            insidePointCount += (inside ? 1 : 0);
        }

        if (insidePointCount == 0) {
            // This polygon is entirely outside this particular plane.
            culled = true;
            return;
        }

        if (insidePointCount == transPolyLength) {
            // This polygon is entirely inside this particular plane.
            insidePlaneCount ++;
        }
    }

    if (insidePlaneCount == frustum->length) {
        // We're entirely inside the frustum, no need to do any dividing up below.
        culled = false;
        return;
    }

    // The polygon is at least partially visible.
    culled = false;

    // We need to spin around the polygon and introduce extra polygons at intersection points.
    // We also need to do it for each plane as a unit operation so that we can clip polygons
    // for each plane.
    for (int j = 0; j < frustum->length; j++) {
        bool inside = frustum->planes[j]->isPointAbove(transPoints[0]);
        int start = 0;

        while (start < transPolyLength) {
            // The end node we're looking at can wrap around.
            int end = (start + 1) % transPolyLength;

            bool newInside = frustum->planes[j]->isPointAbove(transPoints[end]);

            if (newInside == inside) {
                // Simply mark this line for inclusion or exclusion, continuing the trend.
                highlights[start] = highlights[start] ? newInside : false;

                // We didn't intersect, simply move on.
                start++;
                continue;
            }

            // We intersected this plane with this line. Introduce a new point at the intersection.
            Point *intersection = frustum->planes[j]->intersection(transPoints[start], transPoints[end]);

            // Insert that point.
            transPoints = (Point **)realloc(transPoints, sizeof(transPoints[0]) * (transPolyLength + 1));
            highlights = (bool *)realloc(highlights, sizeof(highlights[0]) * (transPolyLength + 1));
            if (end != 0) {
                // Move the rest of the points to make room.
                memmove(&transPoints[end + 1], &transPoints[end], sizeof(transPoints[0]) * (transPolyLength - end));
                memmove(&highlights[end + 1], &highlights[end], sizeof(highlights[0]) * (transPolyLength - end));
            }
            transPoints[start + 1] = intersection;
            transPolyLength++;

            // Mark the points themselves.
            highlights[start] = highlights[start] ? inside : false;
            highlights[start + 1] = highlights[start + 1] ? newInside : false;

            // Continue on.
            inside = newInside;
            start += 2;
        }

        // Get rid of runs of invisible edges by collapsing down.
        int edge = 0;
        while (edge < transPolyLength) {
            int next = (edge + 1) % transPolyLength;

            if (!highlights[edge] && !highlights[next]) {
                // We can get rid of the next node entirely, since we aren't going to draw it.
                delete transPoints[next];

                if ((transPolyLength - (next + 1)) > 0) {
                    memmove(&transPoints[next], &transPoints[next + 1], sizeof(transPoints[0]) * (transPolyLength - (next + 1)));
                    memmove(&highlights[next], &highlights[next + 1], sizeof(highlights[0]) * (transPolyLength - (next + 1)));
                }

                transPolyLength --;
            } else {
                edge ++;
            }
        }
    }
}

void Polygon::draw(Screen *screen) {
    if (!culled) {
        for (int i = 0; i < transPolyLength; i++) {
            int j = (i + 1) % transPolyLength;

            if (highlights[i]) {
                screen->drawLine(transPoints[i], transPoints[j], true);
            }
        }
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
