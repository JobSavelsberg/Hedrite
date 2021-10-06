#include "Hedrite.h"

Hedrite* Hedrite::instance = 0;


void Hedrite::openGLCallback() {
    instance->mounted();
}

Hedrite::Hedrite() {
    openGLWindow = std::make_unique<OpenGLWindow>();
}

Hedrite::~Hedrite() {

}

void Hedrite::initialize() {
	DBG("--- Initializing hedrite ---");
}


const float invsqrt2 = 1.0f / sqrt(2.0f);

void Hedrite::mounted() {
	DBG("--- Hedrite mounted ---");  

    Vector3D<float> points[4] = {
       Vector3D<float>(1.0f, 0.0f, invsqrt2),
       Vector3D<float>(-1.0f, 0.0f, invsqrt2),
       Vector3D<float>(0.0f, -1.0f, -invsqrt2),
       Vector3D<float>(0.0f, 1.0f, -invsqrt2),
    };

    int order[12]{
            0, 1, 2,
            0, 3, 1,
            1, 3, 2,
            2, 3, 0
    };
    juce::uint32 indices[12]{ 0,1,2,3,4,5,6,7,8,9,10,11 };
    float vertexPositions[4 * 3 * 3];
    float vertexNormals[4 * 3 * 3];

    int vertexPositionIndex = 0;
    for (int i = 0; i < 12; i++) {
        auto vertexOrderIndex = order[i];
        vertexPositions[vertexPositionIndex * 3] = points[vertexOrderIndex].x;
        vertexPositions[vertexPositionIndex * 3 + 1] = points[vertexOrderIndex].y;
        vertexPositions[vertexPositionIndex * 3 + 2] = points[vertexOrderIndex].z;
        vertexPositionIndex++;
    }


    // Calculate normals for each face at a time
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 3; j++) {
            Vector3D<float> a = points[order[i * 3 + j]];
            Vector3D<float> b = points[order[i * 3 + (j + 1) % 3]];
            Vector3D<float> c = points[order[i * 3 + (j + 2) % 3]];

            auto u = c - a;
            auto v = b - a;
            auto n = -u ^ v;

            vertexNormals[i * 9 + j * 3] = n.x;
            vertexNormals[i * 9 + j * 3 + 1] = n.y;
            vertexNormals[i * 9 + j * 3 + 2] = n.z;

        }
    }

    openGLWindow->shapes.emplace_back(12, vertexPositions, vertexNormals, indices, juce::Colours::crimson, true, juce::Colours::crimson.brighter(1));


    Vector3D<float> points2[4] = {
   Vector3D<float>(3.0f, 2.0f,invsqrt2),
   Vector3D<float>(1.0f, 2.0f, invsqrt2),
   Vector3D<float>(2.0f, 1.0f, -invsqrt2),
   Vector3D<float>(2.0f, 3.0f, -invsqrt2),
    };

    float vertexPositions2[4 * 3 * 3];

    int vertexPositionIndex2 = 0;
    for (int i = 0; i < 12; i++) {
        auto vertexOrderIndex = order[i];
        vertexPositions2[vertexPositionIndex2 * 3] = points2[vertexOrderIndex].x;
        vertexPositions2[vertexPositionIndex2 * 3 + 1] = points2[vertexOrderIndex].y;
        vertexPositions2[vertexPositionIndex2 * 3 + 2] = points2[vertexOrderIndex].z;
        vertexPositionIndex2++;
    }


   

    openGLWindow->shapes.emplace_back(12, vertexPositions2, vertexNormals, indices, juce::Colours::blueviolet, true, juce::Colours::blueviolet.brighter(1));

    openGLWindow->shapes.emplace_back(12, vertexPositions2, vertexNormals, indices, juce::Colours::blueviolet, true, juce::Colours::blueviolet.brighter(1));


}