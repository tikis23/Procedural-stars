#include "Planet.h"
#include <iostream>
#include "imgui/imgui.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtx/vector_angle.hpp"
#include <GLFW/glfw3.h>
#include <thread>
#include <iostream>
#include "Timer.h"

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
	m_lodAmount = 1;
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
	if (node->mesh->NeedUnmap()) {
		node->mesh->UnmapBuffer();
		node->mesh->NeedUnmap(false);
		node->mesh->IsBuffered(true);
	}
	if (node->mesh->IsBuffered()) {
		node->mesh->Draw(GL_PATCHES);
	}
}

void Planet::Render(glm::vec3 cameraPos) {
	for (int i = 0; i < 6; i++) {
		auto branch = m_tree->GetBranch(i);
		RenderLod(branch, cameraPos);
	}
}

float simplex3d(glm::vec3 p);
glm::vec3 Planet::ApplyNoise(glm::vec3 pos) {
	return pos + pos * simplex3d(pos * 0.05f) * 0.2f;
}

inline glm::vec3 GetNormal(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3) {
	return glm::normalize(glm::cross(p2 - p1, p3 - p1));
}

inline glm::vec3 GetSmoothNormal(const glm::vec3& p0,
	const glm::vec3& p1, const glm::vec3& p2,
	const glm::vec3& p3, const glm::vec3& p4,
	const glm::vec3& p5, const glm::vec3& p6) {
	glm::vec3 n0 = GetNormal(p0, p1, p2);
	glm::vec3 n1 = GetNormal(p0, p2, p3);
	glm::vec3 n2 = GetNormal(p0, p3, p4);
	glm::vec3 n3 = GetNormal(p0, p4, p5);
	glm::vec3 n4 = GetNormal(p0, p5, p6);
	glm::vec3 n5 = GetNormal(p0, p6, p1);
	return glm::normalize(n0 + n1 + n2 + n3 + n4 + n5);
}

void Planet::MeshCreateData(Mesh* mesh, void* ptr, int face, glm::vec3 localPosition, glm::vec3 dimensions) {
	Timer timer;
	auto data = mesh->GetVertexData();
	double step = std::max(dimensions.x, std::max(dimensions.y, dimensions.z)) * 2 / m_nodeVertexAmount;
	switch (face)
	{
	case PLANET_FACE::TOP:
		for (double i = localPosition.x - dimensions.x; i < localPosition.x + dimensions.x; i += step) {
			for (double j = localPosition.z - dimensions.z; j < localPosition.z + dimensions.z; j += step) {
				glm::vec3 point0 = ApplyNoise(m_radius * MapToSphere(glm::vec3{ i + step, localPosition.y, j + step }));
				glm::vec3 point1 = ApplyNoise(m_radius * MapToSphere(glm::vec3{ i + step, localPosition.y, j }));
				glm::vec3 point2 = ApplyNoise(m_radius * MapToSphere(glm::vec3{ i, localPosition.y, j + step }));
				glm::vec3 point3 = ApplyNoise(m_radius * MapToSphere(glm::vec3{ i, localPosition.y, j }));
				// neighbour points
				glm::vec3 np0 = ApplyNoise(m_radius * MapToSphere(glm::vec3{ i - step, localPosition.y, j - step }));
				glm::vec3 np1 = ApplyNoise(m_radius * MapToSphere(glm::vec3{ i - step, localPosition.y, j }));
				glm::vec3 np2 = ApplyNoise(m_radius * MapToSphere(glm::vec3{ i - step, localPosition.y, j + step }));
				glm::vec3 np3 = ApplyNoise(m_radius * MapToSphere(glm::vec3{ i, localPosition.y, j + step * 2 }));
				glm::vec3 np4 = ApplyNoise(m_radius * MapToSphere(glm::vec3{ i + step, localPosition.y, j + step * 2 }));
				glm::vec3 np5 = ApplyNoise(m_radius * MapToSphere(glm::vec3{ i + step * 2, localPosition.y, j + step * 2 }));
				glm::vec3 np6 = ApplyNoise(m_radius * MapToSphere(glm::vec3{ i + step * 2, localPosition.y, j + step }));
				glm::vec3 np7 = ApplyNoise(m_radius * MapToSphere(glm::vec3{ i + step * 2, localPosition.y, j }));
				glm::vec3 np8 = ApplyNoise(m_radius * MapToSphere(glm::vec3{ i + step, localPosition.y, j - step }));
				glm::vec3 np9 = ApplyNoise(m_radius * MapToSphere(glm::vec3{ i, localPosition.y, j - step }));
				
				glm::vec3 npoint0 = GetSmoothNormal(point0,
					np4, np5,
					np6, point1,
					point3, point2
					);
				glm::vec3 npoint1 = GetSmoothNormal(point1,
					point0, np6,
					np7, np8,
					np9, point3
					);
				glm::vec3 npoint2 = GetSmoothNormal(point2,
					np3, np4,
					point0, point3,
					np1, np2
				);
				glm::vec3 npoint3 = GetSmoothNormal(point3,
					point2, point0,
					point1, np9,
					np0, np1
				);

				data->push_back({ point0, npoint0, {1, 0, 1} });
				data->push_back({ point1, npoint1, {1, 0, 1} });
				data->push_back({ point3, npoint3, {1, 0, 1} });
				data->push_back({ point3, npoint3, {1, 0, 1} });
				data->push_back({ point2, npoint2, {1, 0, 1} });
				data->push_back({ point0, npoint0, {1, 0, 1} });
			}
		}
		break;
	case PLANET_FACE::BOTTOM:
		for (double i = localPosition.x - dimensions.x; i < localPosition.x + dimensions.x; i += step) {
			for (double j = localPosition.z - dimensions.z; j < localPosition.z + dimensions.z; j += step) {
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{i, localPosition.y, j})), {}, {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{i + step, localPosition.y, j})), {}, {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{i + step, localPosition.y, j + step})), {}, {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{i + step, localPosition.y, j + step})), {}, {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{i, localPosition.y, j + step})), {}, {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{i, localPosition.y, j})), {}, {1, 0, 1} });
			}
		}
		break;
	case PLANET_FACE::LEFT:
		for (double i = localPosition.y - dimensions.y; i < localPosition.y + dimensions.y; i += step) {
			for (double j = localPosition.z - dimensions.z; j < localPosition.z + dimensions.z; j += step) {
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{localPosition.x, i,  j})), {}, {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{localPosition.x, i + step,  j})), {}, {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{localPosition.x, i + step,  j + step})), {}, {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{localPosition.x, i + step, j + step})), {}, {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{localPosition.x, i,  j + step})), {}, {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{localPosition.x, i,  j})), {}, {1, 0, 1} });
			}
		}
		break;
	case PLANET_FACE::RIGHT:
		for (double i = localPosition.y - dimensions.y; i < localPosition.y + dimensions.y; i += step) {
			for (double j = localPosition.z - dimensions.z; j < localPosition.z + dimensions.z; j += step) {
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{localPosition.x, i + step,  j + step})), {}, {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{localPosition.x, i + step,  j})), {}, {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{localPosition.x, i,  j})), {}, {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{localPosition.x, i,  j})), {}, {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{localPosition.x, i,  j + step})), {}, {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{localPosition.x, i + step, j + step})), {}, {1, 0, 1} });
			}
		}
		break;
	case PLANET_FACE::FRONT:
		for (double i = localPosition.y - dimensions.y; i < localPosition.y + dimensions.y; i += step) {
			for (double j = localPosition.x - dimensions.x; j < localPosition.x + dimensions.x; j += step) {
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{j + step, i + step,  localPosition.z})), {}, {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{j, i + step,  localPosition.z})), {}, {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{j, i,  localPosition.z})), {}, {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{j, i,  localPosition.z})), {}, {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{j + step, i,  localPosition.z})), {}, {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{j + step, i + step, localPosition.z})), {}, {1, 0, 1} });
			}
		}
		break;
	case PLANET_FACE::BACK:
		for (double i = localPosition.y - dimensions.y; i < localPosition.y + dimensions.y; i += step) {
			for (double j = localPosition.x - dimensions.x; j < localPosition.x + dimensions.x; j += step) {
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{j, i,  localPosition.z})), {}, {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{j, i + step,  localPosition.z})), {}, {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{j + step, i + step,  localPosition.z})), {}, {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{j + step, i + step, localPosition.z})), {}, {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{j + step, i,  localPosition.z})), {}, {1, 0, 1} });
				data->push_back({ ApplyNoise(m_radius * MapToSphere(glm::vec3{j, i,  localPosition.z})), {}, {1, 0, 1} });
			}
		}
		break;
	default:
		break;
	}
	timer.PrintElapsed("Mesh Created Data");

	memcpy(ptr, data->data(), sizeof(Vertex) * data->size());
	mesh->NeedUnmap(true);
}

void Planet::GenerateMesh(QUADTREE_NODE* node) {
	if (node->mesh != nullptr)
		delete node->mesh;
	node->mesh = new Mesh;
	node->mesh->Allocate(m_nodeVertexAmount * m_nodeVertexAmount * 6);
	void* ptr = node->mesh->MapBuffer();
	auto thread = std::thread(&Planet::MeshCreateData, this, node->mesh, ptr, node->face, node->localPosition, node->dimensions);
	thread.detach();
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