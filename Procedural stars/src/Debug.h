#pragma once

#include "Shader.h"
#include "glm/glm.hpp"
#include <vector>

class Debug {
public:
	static void Init();
	static void Exit();
	static void LightingEnable(bool val) { m_lightingEnabled = val; };
	static bool LightingEnabled() { return m_lightingEnabled; };
	static void GeometryEnable(bool val) { m_geometryEnabled = val; };
	static bool GeometryEnabled() { return m_geometryEnabled; };
	static Shader* GetGeometryShader() { return m_geometryShader.get(); };
	static Shader* GetLightingShader() { return m_lightingShader.get(); };
	static void Render(float* projection, float* view);
	
	static void DrawLine(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& color);
private:
	Debug();
	struct DebugPoint {
		glm::vec3 point;
		glm::vec3 color;
	};

	static bool m_lightingEnabled;
	static bool m_geometryEnabled;
	static unsigned int m_VBO;
	static unsigned int m_VAO;
	static std::shared_ptr<Shader> m_geometryShader;
	static std::shared_ptr<Shader> m_lightingShader;
	static std::vector<DebugPoint> m_worldGeometry;
};