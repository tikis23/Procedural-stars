#pragma once
#include "glm/glm.hpp"
class FrustumCulling {
public:
	static void UpdateFrustum(const glm::dmat4& projection, const glm::dmat4& view);
	static bool CheckSphere(const glm::dvec3& pos, const double& radius);
private:
	static void NormalizePlanes();
	static double GetDistanceToPlane(const int plane, const glm::dvec3& point);
	static glm::dvec4 m_plane[6];
	FrustumCulling();
};