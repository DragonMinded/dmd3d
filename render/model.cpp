#include <cmath>
#include <cstdlib>
#include <cstring>
#include <map>
#include <vector>
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

    // Set up the transformed (and culled) highlights.
    transHighlights = (bool *)malloc(sizeof(bool) * 3);
    transHighlights[0] = true;
    transHighlights[1] = true;
    transHighlights[2] = true;

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
    transHighlights = (bool *)malloc(sizeof(bool) * length);

    for (int i = 0; i < length; i++) {
        highlights[i] = true;
        transHighlights[i] = true;
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
    free(transHighlights);
    polyPoints = 0;
    transPoints = 0;
    highlights = 0;
    transHighlights = 0;

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
        newPoly->transHighlights[i] = transHighlights[i];
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
    free(transHighlights);

    transPolyLength = polyLength;
    transPoints = (Point **)malloc(sizeof(transPoints[0]) * polyLength);
    transHighlights = (bool *)malloc(sizeof(bool) * polyLength);

    for (int i = 0; i < polyLength; i++) {
        transPoints[i] = polyPoints[i]->clone();
        transHighlights[i] = highlights[i];
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
                transHighlights[start] = transHighlights[start] ? newInside : false;

                // We didn't intersect, simply move on.
                start++;
                continue;
            }

            // We intersected this plane with this line. Introduce a new point at the intersection.
            Point *intersection = frustum->planes[j]->intersection(transPoints[start], transPoints[end]);

            // Insert that point.
            transPoints = (Point **)realloc(transPoints, sizeof(transPoints[0]) * (transPolyLength + 1));
            transHighlights = (bool *)realloc(transHighlights, sizeof(transHighlights[0]) * (transPolyLength + 1));
            if (end != 0) {
                // Move the rest of the points to make room.
                memmove(&transPoints[end + 1], &transPoints[end], sizeof(transPoints[0]) * (transPolyLength - end));
                memmove(&transHighlights[end + 1], &transHighlights[end], sizeof(transHighlights[0]) * (transPolyLength - end));
            }
            transPoints[start + 1] = intersection;
            transPolyLength++;

            // Mark the points themselves.
            transHighlights[start] = transHighlights[start] ? inside : false;
            transHighlights[start + 1] = transHighlights[start + 1] ? newInside : false;

            // Continue on.
            inside = newInside;
            start += 2;
        }

        // Get rid of runs of invisible edges by collapsing down.
        int edge = 0;
        while (edge < transPolyLength) {
            int next = (edge + 1) % transPolyLength;

            if (!transHighlights[edge] && !transHighlights[next]) {
                // We can get rid of the next node entirely, since we aren't going to draw it.
                delete transPoints[next];

                if ((transPolyLength - (next + 1)) > 0) {
                    memmove(&transPoints[next], &transPoints[next + 1], sizeof(transPoints[0]) * (transPolyLength - (next + 1)));
                    memmove(&transHighlights[next], &transHighlights[next + 1], sizeof(transHighlights[0]) * (transPolyLength - (next + 1)));
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

            if (transHighlights[i]) {
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
        newPoly->highlights[i] = transHighlights[i];
    }

    return newPoly;
}

void OccludedWireframePolygon::draw(Screen *screen) {
    if (!culled) {
        screen->drawOccludedPolygon(transPoints, transHighlights, transPolyLength);
    }
}

Model::Model(Polygon *polygons[], int length) {
    modelLength = length;
    this->polygons = (Polygon **)malloc(sizeof(this->polygons[0]) * length);
    for (int i = 0; i < length; i++) {
        // We can have deleted polygons here. It's a pain in the ass to properly keep the
        // normal map in sync so we just allow for that.
        this->polygons[i] = polygons[i]->clone();
    }
}

Model::Model(const char * const modelFile, int flags) {
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

        // Grab the normal so we can keep a mapping of it.
        const float* n = mesh.tri_normal(itri);
        Point normalPoint(n[0], n[1], n[2]);
        if (normalMap.count(normalPoint) == 0) {
            normalMap[normalPoint] = PolyOffset();
        }
        normalMap[normalPoint].push_back(itri);

        if (flags & FLAGS_OCCLUDED) {
            polygons[itri] = new OccludedWireframePolygon(triPoints, 3);
        } else {
            polygons[itri] = new Polygon(triPoints, 3);
        }

        delete triPoints[0];
        delete triPoints[1];
        delete triPoints[2];
    }
}

void Model::coalesce() {
    // Now go through each group of normals and figure out if any line segments on
    // any two polygons are equal. If so, turn the highlight off on both polygons.
    NormalMap::iterator it;
    for (it = normalMap.begin(); it != normalMap.end(); it++)
    {
        // Now, make a candidate list to go through.
        PolyOffset candidates;
        for (auto pIt = it->second.begin(); pIt != it->second.end(); pIt++)
        {
            candidates.push_back(*pIt);
        }

        // We want to compare each element to the rest of the list of elements.
        // However, we can optimize by not comparing backwards, so if we visualize
        // the comparison it is a triangle instead of a quad. This is still O(N^2)
        // but it halves the comparisons.
        while (!candidates.empty()) {
            int currentCandidate = candidates[candidates.size() - 1];
            candidates.pop_back();

            if (!candidates.empty()) {
                for (auto pCan = candidates.begin(); pCan != candidates.end(); pCan++) {
                    Polygon *srcPoly = polygons[currentCandidate];
                    Polygon *dstPoly = polygons[*pCan];

                    for (int srcStart = 0; srcStart < srcPoly->polyLength; srcStart++) {
                        int srcEnd = (srcStart + 1) % srcPoly->polyLength;

                        for (int dstStart = 0; dstStart < dstPoly->polyLength; dstStart++) {
                            int dstEnd = (dstStart + 1) % dstPoly->polyLength;

                            if (!srcPoly->highlights[srcStart] && !dstPoly->highlights[dstStart]) { continue; }

                            if ((
                                (*srcPoly->polyPoints[srcStart] == *dstPoly->polyPoints[dstStart]) &&
                                (*srcPoly->polyPoints[srcEnd] == *dstPoly->polyPoints[dstEnd])
                            ) || (
                                (*srcPoly->polyPoints[srcStart] == *dstPoly->polyPoints[dstEnd]) &&
                                (*srcPoly->polyPoints[srcEnd] == *dstPoly->polyPoints[dstStart])
                            )) {
                                srcPoly->highlights[srcStart] = false;
                                dstPoly->highlights[dstStart] = false;
                            }
                        }
                    }
                }
            }
        }

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
    Model *newModel = new Model(polygons, modelLength);

    NormalMap::iterator it;
    for (it = normalMap.begin(); it != normalMap.end(); it++)
    {
        newModel->normalMap[it->first] = PolyOffset();
        for (auto pIt = it->second.begin(); pIt != it->second.end(); pIt++)
        {
            newModel->normalMap[it->first].push_back(*pIt);
        }
    }

    return newModel;
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
