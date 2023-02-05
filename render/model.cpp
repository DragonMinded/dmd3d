#include <cmath>
#include <cstdlib>
#include "model.h"
#include "matrix.h"
#include "common.h"

#define STL_READER_NO_EXCEPTIONS  1
#include "stl_reader.h"

Polygon::Polygon(Point *x, Point *y, Point *z) {
    polyLength = 3;
    polyPoints = (Point **)malloc(sizeof(polyPoints[0]) * 3);
    transPoints = (Point **)malloc(sizeof(polyPoints[0]) * 3);

    polyPoints[0] = x->clone();
    polyPoints[1] = y->clone();
    polyPoints[2] = z->clone();
    transPoints[0] = x->clone();
    transPoints[1] = y->clone();
    transPoints[2] = z->clone();
}

Polygon::Polygon(Point *points[], int length) {
    polyLength = length;
    polyPoints = (Point **)malloc(sizeof(polyPoints[0]) * length);
    transPoints = (Point **)malloc(sizeof(polyPoints[0]) * length);
    for (int i = 0; i < length; i++) {
        polyPoints[i] = points[i]->clone();
        transPoints[i] = points[i]->clone();
    }
}

Polygon::~Polygon() {
    for (int i = 0; i < polyLength; i++) {
        delete polyPoints[i];
        delete transPoints[i];
    }

    free(polyPoints);
    free(transPoints);
    polyPoints = 0;
    transPoints = 0;
    polyLength = 0;
}

Polygon *Polygon::clone() {
    // Make sure if we clone a polygon that's been transformed, the new is also.
    return new Polygon(transPoints, polyLength);
}

void Polygon::reset() {
    for (int i = 0; i < polyLength; i++) {
        delete transPoints[i];
        transPoints[i] = polyPoints[i]->clone();
    }
}

void Polygon::transform(Matrix *matrix) {
    for (int i = 0; i < polyLength; i++) {
        matrix->multiplyUpdatePoint(transPoints[i]);
    }
}

void Polygon::project(Matrix *matrix) {
    for (int i = 0; i < polyLength; i++) {
        matrix->projectUpdatePoint(transPoints[i]);
    }
}

void Polygon::draw(Screen *screen) {
    screen->drawOccludedPolygon(transPoints, polyLength);
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

        polygons[itri] = new Polygon(triPoints, 3);
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
