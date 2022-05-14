#include "Planet.h"
#include <iostream>
#include "imgui/imgui.h"
Planet::Planet(Camera* cam) {
	m_meshes.push_back({});

	std::vector<Vertex>* data = m_meshes.back().GetVertexData();
	for (int i = -50; i < 50; i++) {
		for (int j = -50; j < 50; j++) {
			float x = (float)i / 50;
			float y = (float)j / 50;
			float intarpas = (float)1 / 50;
			data->push_back({ {x		 , y		 , 0} });
			data->push_back({ {x+intarpas, y+intarpas, 0} });
			data->push_back({ {x+intarpas, y		 , 0} });
			data->push_back({ {x		 , y		 , 0} });
			data->push_back({ {x		 , y+intarpas, 0} });
			data->push_back({ {x+intarpas, y+intarpas, 0} });
		}
	}

	static float z = 1.0f;
	static float r = 10.0f;
	static int n = 4;

	// z offset
	float R = r + z;
	float d = glm::length(cam->GetPosition());
	float h = sqrtf(d * d - r * r);
	float s = sqrtf(R * R - r * r);
	float zs = (R * R + d * d - (h + s) * (h + s)) / (2 * r * (h + s));

	// face camera
	glm::vec3 b = glm::normalize(cam->GetPosition() + glm::vec3{1, 0, 0});
	b = { 0, 0, 1 };
	glm::vec3 w = glm::normalize(cam->GetPosition());
	glm::vec3 v = glm::normalize(glm::cross(w, b));
	glm::vec3 u = glm::cross(w, v);
	glm::mat3 Rm = glm::mat3(u, v, w);

	//std::cout << d << " " << " " << h << " " << s << " " << zs << '\n';

	for (int i = 0; i < data->size(); i++) {
		glm::vec3 g = data->at(i).position;
		g.z = (1 - pow(data->at(i).position.x, n)) * (1 - pow(data->at(i).position.y, n)) + zs;
		//g = Rm * g;


		glm::vec3 o;
		o.x = g.x * u.x + g.y * v.x + g.z * w.x;
		o.y = g.x * u.y + g.y * v.y + g.z * w.y;
		o.z = g.x * u.z + g.y * v.z + g.z * w.z;

		g = o;


		data->at(i).position = glm::normalize(g) * r;
	}


	if (ImGui::Begin("Settings")) {
		if (ImGui::BeginTabBar("")) {
			if (ImGui::BeginTabItem("debug")) {
				ImGui::DragFloat("z", &z, 0.01f);
				ImGui::DragFloat("r", &r, 0.1f);
				ImGui::DragInt("n", &n);
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
	ImGui::End();

	m_meshes.back().Buffer();
}

Planet::~Planet() {
}