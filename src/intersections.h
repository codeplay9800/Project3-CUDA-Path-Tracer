#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/normal.hpp>

#include "sceneStructs.h"
#include "utilities.h"

/**
 * Handy-dandy hash function that provides seeds for random number generation.
 */
__host__ __device__ inline unsigned int utilhash(unsigned int a) {
    a = (a + 0x7ed55d16) + (a << 12);
    a = (a ^ 0xc761c23c) ^ (a >> 19);
    a = (a + 0x165667b1) + (a << 5);
    a = (a + 0xd3a2646c) ^ (a << 9);
    a = (a + 0xfd7046c5) + (a << 3);
    a = (a ^ 0xb55a4f09) ^ (a >> 16);
    return a;
}

// CHECKITOUT
/**
 * Compute a point at parameter value `t` on ray `r`.
 * Falls slightly short so that it doesn't intersect the object it's hitting.
 */
__host__ __device__ glm::vec3 getPointOnRay(Ray r, float t) {
    return r.origin + (t - .0001f) * glm::normalize(r.direction);
}

/**
 * Multiplies a mat4 and a vec4 and returns a vec3 clipped from the vec4.
 */
__host__ __device__ glm::vec3 multiplyMV(glm::mat4 m, glm::vec4 v) {
    return glm::vec3(m * v);
}

// CHECKITOUT
/**
 * Test intersection between a ray and a transformed cube. Untransformed,
 * the cube ranges from -0.5 to 0.5 in each axis and is centered at the origin.
 *
 * @param intersectionPoint  Output parameter for point of intersection.
 * @param normal             Output parameter for surface normal.
 * @param outside            Output param for whether the ray came from outside.
 * @return                   Ray parameter `t` value. -1 if no intersection.
 */
__host__ __device__ float boxIntersectionTest(Geom box, Ray r,
        glm::vec3 &intersectionPoint, glm::vec3 &normal, bool &outside) {
    Ray q;
    q.origin    =                multiplyMV(box.inverseTransform, glm::vec4(r.origin   , 1.0f));
    q.direction = glm::normalize(multiplyMV(box.inverseTransform, glm::vec4(r.direction, 0.0f)));

    float tmin = -1e38f;
    float tmax = 1e38f;
    glm::vec3 tmin_n;
    glm::vec3 tmax_n;
    for (int xyz = 0; xyz < 3; ++xyz) {
        float qdxyz = q.direction[xyz];
        /*if (glm::abs(qdxyz) > 0.00001f)*/ {
            float t1 = (-0.5f - q.origin[xyz]) / qdxyz;
            float t2 = (+0.5f - q.origin[xyz]) / qdxyz;
            float ta = glm::min(t1, t2);
            float tb = glm::max(t1, t2);
            glm::vec3 n;
            n[xyz] = t2 < t1 ? +1 : -1;
            if (ta > 0 && ta > tmin) {
                tmin = ta;
                tmin_n = n;
            }
            if (tb < tmax) {
                tmax = tb;
                tmax_n = n;
            }
        }
    }

    if (tmax >= tmin && tmax > 0) {
        outside = true;
        if (tmin <= 0) {
            tmin = tmax;
            tmin_n = tmax_n;
            outside = false;
        }
        intersectionPoint = multiplyMV(box.transform, glm::vec4(getPointOnRay(q, tmin), 1.0f));
        normal = glm::normalize(multiplyMV(box.invTranspose, glm::vec4(tmin_n, 0.0f)));
        return glm::length(r.origin - intersectionPoint);
    }
    return -1;
}

// CHECKITOUT
/**
 * Test intersection between a ray and a transformed sphere. Untransformed,
 * the sphere always has radius 0.5 and is centered at the origin.
 *
 * @param intersectionPoint  Output parameter for point of intersection.
 * @param normal             Output parameter for surface normal.
 * @param outside            Output param for whether the ray came from outside.
 * @return                   Ray parameter `t` value. -1 if no intersection.
 */
__host__ __device__ float sphereIntersectionTest(Geom sphere, Ray r,
        glm::vec3 &intersectionPoint, glm::vec3 &normal, bool &outside) {
    float radius = .5;

    glm::vec3 ro = multiplyMV(sphere.inverseTransform, glm::vec4(r.origin, 1.0f));
    glm::vec3 rd = glm::normalize(multiplyMV(sphere.inverseTransform, glm::vec4(r.direction, 0.0f)));

    Ray rt;
    rt.origin = ro;
    rt.direction = rd;

    float vDotDirection = glm::dot(rt.origin, rt.direction);
    float radicand = vDotDirection * vDotDirection - (glm::dot(rt.origin, rt.origin) - powf(radius, 2));
    if (radicand < 0) {
        return -1;
    }

    float squareRoot = sqrt(radicand);
    float firstTerm = -vDotDirection;
    float t1 = firstTerm + squareRoot;
    float t2 = firstTerm - squareRoot;

    float t = 0;
    if (t1 < 0 && t2 < 0) {
        return -1;
    } else if (t1 > 0 && t2 > 0) {
        t = min(t1, t2);
        outside = true;
    } else {
        t = max(t1, t2);
        outside = false;
    }

    glm::vec3 objspaceIntersection = getPointOnRay(rt, t);

    intersectionPoint = multiplyMV(sphere.transform, glm::vec4(objspaceIntersection, 1.f));
    normal = glm::normalize(multiplyMV(sphere.invTranspose, glm::vec4(objspaceIntersection, 0.f)));
    if (!outside) {
        normal = -normal;
    }

    return glm::length(r.origin - intersectionPoint);
}


__host__ __device__ float TriangleArea(glm::vec4 a_p1, glm::vec4 a_p2, glm::vec4 a_p3)
{
    glm::vec4 p1 = a_p1;
    glm::vec4 p2 = a_p2;
    glm::vec4 p3 = a_p3;
    float A = 0.5f * glm::length(glm::cross(glm::vec3(a_p2[0] - a_p1[0], a_p2[1] - a_p1[1], a_p2[2] - a_p1[2]),
        glm::vec3(a_p3[0] - a_p1[0], a_p3[1] - a_p1[1], a_p3[2] - a_p1[2])));
    return A;
}


__host__ __device__ glm::vec4 GetBarycentricWeightedNormal(glm::vec4 a_p1, glm::vec4 a_n1, glm::vec4 a_p2, glm::vec4 a_n2, glm::vec4 a_p3, glm::vec4 a_n3, glm::vec4 a_p)
{

    float A = TriangleArea(a_p1, a_p2, a_p3);
    float A1 = TriangleArea(a_p2, a_p3, a_p);
    float A2 = TriangleArea(a_p1, a_p3, a_p);
    float A3 = TriangleArea(a_p1, a_p2, a_p);
    glm::vec4 a_surfaceNormal = a_p[2] * ((a_n1 * A1) / (A * a_p1[2]) + (a_n2 * A2) / (A * a_p2[2]) + (a_n3 * A3) / (A * a_p3[2]));
    return a_surfaceNormal;
}

__host__ __device__
inline bool intersect( Geom &geom,
    const Ray ray,
    float& tNear, float& tFar, uint8_t& planeIndex)
{
    const int kNumPlaneSetNormals = 7;
    const glm::vec3 planeSetNormals[kNumPlaneSetNormals] = {
    glm::vec3(1, 0, 0),
    glm::vec3(0, 1, 0),
    glm::vec3(0, 0, 1),
    glm::vec3(sqrtf(3) / 3.f,  sqrtf(3) / 3.f, sqrtf(3) / 3.f),
    glm::vec3(-sqrtf(3) / 3.f,  sqrtf(3) / 3.f, sqrtf(3) / 3.f),
    glm::vec3(-sqrtf(3) / 3.f, -sqrtf(3) / 3.f, sqrtf(3) / 3.f),
    glm::vec3(sqrtf(3) / 3.f, -sqrtf(3) / 3.f, sqrtf(3) / 3.f) };

    
    
    float precomputedNumerator[kNumPlaneSetNormals], precomputeDenominator[kNumPlaneSetNormals];
    for (uint8_t i = 0; i < kNumPlaneSetNormals; ++i) {
        precomputedNumerator[i] = glm::dot(planeSetNormals[i], ray.origin);
        precomputeDenominator[i] = glm::dot(planeSetNormals[i], ray.direction);
    }
    for (int i = 0; i < kNumPlaneSetNormals; ++i) {
        float tn = (geom.Device_BVH[2 * i] - precomputedNumerator[i]) / precomputeDenominator[i];
        float tf = (geom.Device_BVH[2 * i + 1] - precomputedNumerator[i]) / precomputeDenominator[i];
        if (precomputeDenominator[i] < 0)
        {
            float temp = tn;
            tn = tf;
            tf = temp;
            //std::swap(tn, tf);
        }
        if (tn > tNear) tNear = tn, planeIndex = i;
        if (tf < tFar) tFar = tf;
        if (tNear > tFar) return false;
    }

    return true;
}

__host__ __device__
const bool intersect(const Ray ray, Geom &currGeom) 
{
    float tClosest = FLT_MAX_10_EXP;
    bool hit = false;
        float tNear = -FLT_MAX_10_EXP, tFar = FLT_MAX_10_EXP;
        uint8_t planeIndex;
        hit = intersect(currGeom, ray, tNear, tFar, planeIndex);
        if (hit) {
            if (tNear < tClosest)
            {
                tClosest = tNear;
            }
        }
        return hit;
    }


__host__ __device__ float MeshIntersectionTest(Geom &objGeom, Ray r,
    glm::vec3& intersectionPoint, glm::vec3& normal, bool& outside) {
    
    bool intersection = false;
    glm::vec3 interPoint;
    glm::vec3 internormal;
    int count = objGeom.triangleCount;

    glm::vec3 minBary = glm::vec3(INT_MAX, INT_MAX, INT_MAX);
    glm::vec3 BaryPosition;
    glm::vec4 n1, n2, n3;

    for (int i = 0; i < count; i++)
    {
        int triangleOffset = 6 * i;
        //glm::mat4 modelMat = objGeom.transform;
        glm::vec4 p1 = objGeom.transform * objGeom.Device_Triangle_points_normals[triangleOffset + 0];
        glm::vec4 p2 = objGeom.transform * objGeom.Device_Triangle_points_normals[triangleOffset + 2];
        glm::vec4 p3  = objGeom.transform * objGeom.Device_Triangle_points_normals[triangleOffset + 4];

        intersection = glm::intersectRayTriangle(r.origin, r.direction, glm::vec3(p1), glm::vec3(p2), glm::vec3(p3), BaryPosition);

        if (intersection)
        {
            if (BaryPosition[2] >= 0 && BaryPosition[2] < minBary[2])
            {
                minBary = BaryPosition;
            }
            n1 = objGeom.Device_Triangle_points_normals[triangleOffset + 1];
            n2 = objGeom.Device_Triangle_points_normals[triangleOffset + 3];
            n3 = objGeom.Device_Triangle_points_normals[triangleOffset + 5];
            break;
        }
    }
    if (intersection)
    {
        float u = minBary[0];
        float v = minBary[1];
        float t = minBary[2];
        interPoint = getPointOnRay(r, t);
        internormal = glm::vec3(u * n1 + v * n2 + (1 - u - v) * n3);
        intersectionPoint = interPoint;
        normal = internormal;
        return glm::length(r.origin - intersectionPoint);
    }
    return -1;
}

