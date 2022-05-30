#include "Planet.h"
#include <iostream>
#include "imgui/imgui.h"
#include "glm/ext/matrix_transform.hpp"
#include <GLFW/glfw3.h>
#include <thread>

glm::vec3 MapToSphere(glm::vec3 pos) {
	//return pos;
	double sqrx = pos.x * pos.x;
	double sqry = pos.y * pos.y;
	double sqrz = pos.z * pos.z;
	return {
		pos.x * sqrt(1 - sqry / 2 - sqrz / 2 + sqry * sqrz / 3),
		pos.y * sqrt(1 - sqrz / 2 - sqrx / 2 + sqrz * sqrx / 3),
		pos.z * sqrt(1 - sqrx / 2 - sqry / 2 + sqrx * sqry / 3),
	};
}

Planet::Planet() {
	m_lodAmount = 8;
	m_radius = 100;
	m_nodeVertexAmount = 36;
	m_tree = new QuadTree(6);
	m_position = { 0, 0, 0 };
	for (int i = 0; i < 6; i++) {
		auto node = m_tree->GetBranch(i);
		node->face = i;
		switch (i)
		{
		case PLANET_FACE::TOP:
			node->localPosition = { 0, 1, 0 };
			node->dimensions = { 1, 0, 1 };
			break;
		case PLANET_FACE::BOTTOM:
			node->localPosition = { 0, -1, 0 };
			node->dimensions = { 1, 0, 1 };
			break;
		case PLANET_FACE::LEFT:
			node->localPosition = { 1, 0, 0 };
			node->dimensions = { 0, 1, 1 };
			break;
		case PLANET_FACE::RIGHT:
			node->localPosition = { -1, 0, 0 };
			node->dimensions = { 0, 1, 1 };
			break;
		case PLANET_FACE::FRONT:
			node->localPosition = { 0, 0, 1 };
			node->dimensions = { 1, 1, 0 };
			break;
		case PLANET_FACE::BACK:
			node->localPosition = { 0, 0, -1 };
			node->dimensions = { 1, 1, 0 };
			break;
		default:
			break;
		}
		//GenerateMesh(node);
	}
}

Planet::~Planet() {

}

void Planet::Update() {
	m_modelPos = glm::mat4(1);
	m_modelPos = glm::translate(m_modelPos, m_position);

	m_modelRot = glm::mat4(1);
	m_modelRot = glm::rotate(m_modelRot, glm::radians(m_rotation.x), glm::vec3(1, 0, 0));
	m_modelRot = glm::rotate(m_modelRot, glm::radians(m_rotation.y), glm::vec3(0, 1, 0));
	m_modelRot = glm::rotate(m_modelRot, glm::radians(m_rotation.z), glm::vec3(0, 0, 1));
}

void Planet::RenderLod(QUADTREE_NODE* node, glm::vec3 cameraPos) {
	glm::vec3 pointPos = m_radius * MapToSphere(node->localPosition) + m_position;
	glm::vec3 diff = pointPos - cameraPos;
	double dist = sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z)
		- sqrt(((node->dimensions.x + node->dimensions.y + node->dimensions.z) * 0.5)+
			((node->dimensions.x + node->dimensions.y + node->dimensions.z) * 0.5));

	double chunkSize = pow(2, m_lodAmount - node->level);
	double perspectiveScale = 1000.0f / (2.f * tan(60.f * 0.5f));
	double error = (chunkSize / dist) * perspectiveScale;
	if (error < -100 && node->level + 1 <= m_lodAmount) {
		node->Split();
		RenderLod(node->child[0], cameraPos);
		RenderLod(node->child[1], cameraPos);
		RenderLod(node->child[2], cameraPos);
		RenderLod(node->child[3], cameraPos);
		return;
	}
	if (node->mesh == nullptr) {
		GenerateMesh(node);
	}
	if (!node->mesh->IsBuffered() && node->mesh->ReadyToBuffer()) {
		node->mesh->Buffer();
		node->mesh->ReadyToBuffer(false);
	}
	node->mesh->Draw();
}

void Planet::Render(glm::vec3 cameraPos) {
	for (int i = 0; i < 6; i++) {
		auto branch = m_tree->GetBranch(i);
		RenderLod(branch, cameraPos);
	}
}

float simplex3d(glm::vec3 p);
glm::vec3 Planet::ApplyNoise(glm::vec3 pos) {
	return pos + pos * simplex3d(pos * 0.05f) * 0.1f;
}

void Planet::MeshCreateData(Mesh* mesh, int face, glm::vec3 localPosition, glm::vec3 dimensions) {
	auto data = mesh->GetVertexData();
	float step = std::max(dimensions.x, std::max(dimensions.y, dimensions.z)) * 2 / m_nodeVertexAmount;
	switch (face)
	{
	case PLANET_FACE::TOP:
		for (float i = localPosition.x - dimensions.x; i < localPosition.x + dimensions.x; i += step) {
			for (float j = localPosition.z - dimensions.z; j < localPosition.z + dimensions.z; j += step) {
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{i + step, localPosition.y, j + step})), {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{i + step, localPosition.y, j})), {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{i, localPosition.y, j})), {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{i, localPosition.y, j})), {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{i, localPosition.y, j + step})), {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{i + step, localPosition.y, j + step})), {1, 0, 1} });
			}
		}
		break;
	case PLANET_FACE::BOTTOM:
		for (float i = localPosition.x - dimensions.x; i < localPosition.x + dimensions.x; i += step) {
			for (float j = localPosition.z - dimensions.z; j < localPosition.z + dimensions.z; j += step) {
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{i, localPosition.y, j})), {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{i + step, localPosition.y, j})), {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{i + step, localPosition.y, j + step})), {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{i + step, localPosition.y, j + step})), {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{i, localPosition.y, j + step})), {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{i, localPosition.y, j})), {1, 0, 1} });
			}
		}
		break;
	case PLANET_FACE::LEFT:
		for (float i = localPosition.y - dimensions.y; i < localPosition.y + dimensions.y; i += step) {
			for (float j = localPosition.z - dimensions.z; j < localPosition.z + dimensions.z; j += step) {
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{localPosition.x, i,  j})), {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{localPosition.x, i + step,  j})), {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{localPosition.x, i + step,  j + step})), {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{localPosition.x, i + step, j + step})), {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{localPosition.x, i,  j + step})), {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{localPosition.x, i,  j})), {1, 0, 1} });
			}
		}
		break;
	case PLANET_FACE::RIGHT:
		for (float i = localPosition.y - dimensions.y; i < localPosition.y + dimensions.y; i += step) {
			for (float j = localPosition.z - dimensions.z; j < localPosition.z + dimensions.z; j += step) {
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{localPosition.x, i + step,  j + step})), {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{localPosition.x, i + step,  j})), {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{localPosition.x, i,  j})), {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{localPosition.x, i,  j})), {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{localPosition.x, i,  j + step})), {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{localPosition.x, i + step, j + step})), {1, 0, 1} });
			}
		}
		break;
	case PLANET_FACE::FRONT:
		for (float i = localPosition.y - dimensions.y; i < localPosition.y + dimensions.y; i += step) {
			for (float j = localPosition.x - dimensions.x; j < localPosition.x + dimensions.x; j += step) {
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{j + step, i + step,  localPosition.z})), {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{j, i + step,  localPosition.z})), {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{j, i,  localPosition.z})), {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{j, i,  localPosition.z})), {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{j + step, i,  localPosition.z})), {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{j + step, i + step, localPosition.z})), {1, 0, 1} });
			}
		}
		break;
	case PLANET_FACE::BACK:
		for (float i = localPosition.y - dimensions.y; i < localPosition.y + dimensions.y; i += step) {
			for (float j = localPosition.x - dimensions.x; j < localPosition.x + dimensions.x; j += step) {
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{j, i,  localPosition.z})), {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{j, i + step,  localPosition.z})), {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{j + step, i + step,  localPosition.z})), {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{j + step, i + step, localPosition.z})), {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{j + step, i,  localPosition.z})), {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{j, i,  localPosition.z})), {1, 0, 1} });
			}
		}
		break;
	default:
		break;
	}
	mesh->ReadyToBuffer(true);
}

void Planet::GenerateMesh(QUADTREE_NODE* node) {
	if (node->mesh != nullptr)
		delete node->mesh;
	node->mesh = new Mesh;
	auto thread = std::thread(&Planet::MeshCreateData, this, node->mesh, node->face, node->localPosition, node->dimensions);
	thread.detach();
	//MeshCreateData(node->mesh, node->face, node->localPosition, node->dimensions);
}



/* discontinuous pseudorandom uniformly distributed in [-0.5, +0.5]^3 */
glm::vec3 random3(glm::vec3 c) {
	float j = 4096.0 * sin(glm::dot(c, glm::vec3(17.0, 59.4, 15.0)));
	glm::vec3 r;
	r.z = glm::fract(512.0 * j);
	j *= .125;
	r.x = glm::fract(512.0 * j);
	j *= .125;
	r.y = glm::fract(512.0 * j);
	return r - glm::vec3(0.5);
}

/* skew constants for 3d simplex functions */
const float F3 = 0.3333333;
const float G3 = 0.1666667;

/* 3d simplex noise */
float simplex3d(glm::vec3 p) {
	/* 1. find current tetrahedron T and it's four vertices */
	/* s, s+i1, s+i2, s+1.0 - absolute skewed (integer) coordinates of T vertices */
	/* x, x1, x2, x3 - unskewed coordinates of p relative to each of T vertices*/

	/* calculate s and x */
	glm::vec3 s = floor(p + glm::dot(p, glm::vec3(F3)));
	glm::vec3 x = p - s + glm::dot(s, glm::vec3(G3));

	/* calculate i1 and i2 */
	glm::vec3 e = glm::step(glm::vec3(0.0), x - glm::vec3(x.y, x.z, x.x));
	glm::vec3 i1 = e * (glm::vec3(1) - glm::vec3(e.z, e.x, e.y));
	glm::vec3 i2 = glm::vec3(1) - glm::vec3(e.z, e.x, e.y) * (glm::vec3(1) - e);

	/* x1, x2, x3 */
	glm::vec3 x1 = x - i1 + G3;
	glm::vec3 x2 = x - i2 + glm::vec3(2.0) * G3;
	glm::vec3 x3 = x - glm::vec3(1.0) + glm::vec3(3.0) * G3;

	/* 2. find four surflets and store them in d */
	glm::vec4 w, d;

	/* calculate surflet weights */
	w.x = glm::dot(x, x);
	w.y = glm::dot(x1, x1);
	w.z = glm::dot(x2, x2);
	w.w = glm::dot(x3, x3);

	/* w fades from 0.6 at the center of the surflet to 0.0 at the margin */
	w = glm::max(glm::vec4(0.6) - w, glm::vec4(0.0));

	/* calculate surflet components */
	d.x = dot(random3(s), x);
	d.y = dot(random3(s + i1), x1);
	d.z = dot(random3(s + i2), x2);
	d.w = dot(random3(s + glm::vec3(1.0)), x3);

	/* multiply d by w^4 */
	w *= w;
	w *= w;
	d *= w;

	/* 3. return the sum of the four surflets */
	return glm::dot(d, glm::vec4(52.0));
}