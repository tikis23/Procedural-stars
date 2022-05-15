#include "Planet.h"
#include <iostream>
#include "imgui/imgui.h"
#include "glm/ext/matrix_transform.hpp"
#include <GLFW/glfw3.h>
Planet::Planet() {
	if (m_meshes.empty()) {
		int lods = 16;
		for (int l = 1; l <= lods; l++) {
			m_meshes.push_back(new Mesh);
			std::vector<Vertex>* data = m_meshes.back()->GetVertexData();
			int detail = 500 / std::max(l*l*0.25f, 1.0f);
			for (int i = -detail; i < detail; i++) {
				for (int j = -detail; j < detail; j++) {
					float x = (float)i / detail;
					float y = (float)j / detail;
					float intarpas = (float)1 / detail;
					data->push_back({ {x		 , y		 , 0} });
					data->push_back({ {x + intarpas, y + intarpas, 0} });
					data->push_back({ {x + intarpas, y		 , 0} });
					data->push_back({ {x		 , y		 , 0} });
					data->push_back({ {x		 , y + intarpas, 0} });
					data->push_back({ {x + intarpas, y + intarpas, 0} });
				}
			}
			m_meshes.back()->Buffer();
		}
	}
}

Planet::~Planet() {

}

void Planet::Update() {
	static float z = 40.0f;
	static float r = 200.0f;
	static float k = 0;
	if (ImGui::Begin("Settings")) {
		if (ImGui::BeginTabBar("")) {
			if (ImGui::BeginTabItem("debug")) {
				ImGui::DragFloat("z", &z, 0.01f);
				ImGui::DragFloat("r", &r, 0.1f);
				ImGui::DragFloat("k", &k, 1.f);
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
	ImGui::End();
	//m_position = { 0 * sin(glfwGetTime()) * 200, 0, 0 };
	//m_rotation = { 0 * sin(glfwGetTime() * 0.07) * 360, 0, 0 };
	m_radius = r;
	m_radiusMax = r + z;

	m_modelPos = glm::mat4(1);
	m_modelPos = glm::translate(m_modelPos, m_position);

	m_modelRot = glm::mat4(1);
	m_modelRot = glm::rotate(m_modelRot, glm::radians(m_rotation.x), glm::vec3(1, 0, 0));
	m_modelRot = glm::rotate(m_modelRot, glm::radians(m_rotation.y), glm::vec3(0, 1, 0));
	m_modelRot = glm::rotate(m_modelRot, glm::radians(m_rotation.z), glm::vec3(0, 0, 1));
}

std::vector<Mesh*> Planet::m_meshes;