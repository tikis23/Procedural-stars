#include "FrustumCulling.h"
#include <glm/gtc/matrix_access.hpp>

void FrustumCulling::UpdateFrustum(const glm::dmat4& projection, const glm::dmat4& view) {
    glm::dmat4 mvp = projection * view;

    m_plane[0] = glm::row(mvp, 3) - glm::row(mvp, 0);
    m_plane[1] = glm::row(mvp, 3) + glm::row(mvp, 0);
    m_plane[2] = glm::row(mvp, 3) - glm::row(mvp, 2);
    m_plane[3] = glm::row(mvp, 3) + glm::row(mvp, 2);
    m_plane[4] = glm::row(mvp, 3) - glm::row(mvp, 1);
    m_plane[5] = glm::row(mvp, 3) + glm::row(mvp, 1);

    NormalizePlanes();
}

void FrustumCulling::NormalizePlanes() {
    for (unsigned int i = 0; i < 6; i++) {
        double normalLength = glm::length(glm::dvec3{ m_plane[i].x, m_plane[i].y, m_plane[i].z });
        if (normalLength != 0.0)
            m_plane[i] /= normalLength;
    }
}

double FrustumCulling::GetDistanceToPlane(const int plane, const glm::dvec3& point) {
    return m_plane[plane].x * point.x +
           m_plane[plane].y * point.y +
           m_plane[plane].z * point.z +
           m_plane[plane].w;
}

bool FrustumCulling::CheckSphere(const glm::dvec3& position, const double& radius) {
    return true; ////////////// disables culling
    for (unsigned int i = 0; i < 6; i++) {
        const double distance = GetDistanceToPlane(i, position);
        if (distance <= -radius)
            return false;
    }
    return true;
}

glm::dvec4 FrustumCulling::m_plane[6];