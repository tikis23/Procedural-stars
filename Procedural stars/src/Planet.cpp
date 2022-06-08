#include "Planet.h"
#include <iostream>
#include "imgui/imgui.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtx/vector_angle.hpp"
#include <GLFW/glfw3.h>
#include <thread>
#include <limits>
#include <FastNoise/FastNoise.h>
#include "Timer.h"
#include "Debug.h"

#define MAX_MESHES_GENERATED_PER_FRAME 20
#define NODE_CLEANUP_INTERVAL 10
#define NODE_LIFETIME 20
#define NUM_THREADS 10

glm::vec3 MapToSphere(glm::vec3 pos) {
	double sqrx = pos.x * pos.x;
	double sqry = pos.y * pos.y;
	double sqrz = pos.z * pos.z;
	return {
		pos.x * sqrt(1 - sqry / 2 - sqrz / 2 + sqry * sqrz / 3),
		pos.y * sqrt(1 - sqrz / 2 - sqrx / 2 + sqrz * sqrx / 3),
		pos.z * sqrt(1 - sqrx / 2 - sqry / 2 + sqrx * sqry / 3),
	};
}
double GetAABBDistance(glm::vec3 position, glm::vec3 pmin, glm::vec3 pmax) {
	glm::vec3 cp = position - glm::vec3{
		std::clamp(position.x, pmin.x, pmax.x),
		std::clamp(position.y, pmin.y, pmax.y),
		std::clamp(position.z, pmin.z, pmax.z)
	};
	double dist = sqrt(cp.x * cp.x + cp.y * cp.y + cp.z * cp.z);
	return dist;
}
void DrawBoundingBox(glm::vec3 pmin, glm::vec3 pmax) {
	if (Debug::GeometryEnabled()) {
		glm::vec3 c1 = { pmin.x, pmin.y, pmin.z };
		glm::vec3 c2 = { pmax.x, pmin.y, pmin.z };
		glm::vec3 c3 = { pmax.x, pmax.y, pmin.z };
		glm::vec3 c4 = { pmin.x, pmax.y, pmin.z };
		glm::vec3 c5 = { pmin.x, pmin.y, pmax.z };
		glm::vec3 c6 = { pmax.x, pmin.y, pmax.z };
		glm::vec3 c7 = { pmax.x, pmax.y, pmax.z };
		glm::vec3 c8 = { pmin.x, pmax.y, pmax.z };

		Debug::DrawLine(c1, c2, { 1, 1, 0 });
		Debug::DrawLine(c2, c3, { 1, 1, 0 });
		Debug::DrawLine(c3, c4, { 1, 1, 0 });
		Debug::DrawLine(c4, c1, { 1, 1, 0 });
		Debug::DrawLine(c5, c6, { 1, 1, 0 });
		Debug::DrawLine(c6, c7, { 1, 1, 0 });
		Debug::DrawLine(c7, c8, { 1, 1, 0 });
		Debug::DrawLine(c8, c5, { 1, 1, 0 });
		Debug::DrawLine(c1, c5, { 1, 1, 0 });
		Debug::DrawLine(c2, c6, { 1, 1, 0 });
		Debug::DrawLine(c3, c7, { 1, 1, 0 });
		Debug::DrawLine(c4, c8, { 1, 1, 0 });
	}
}
glm::vec3 GetMinPoint(const glm::vec3& p1, const glm::vec3& p2) {
	return {
		std::min(p1.x, p2.x),
		std::min(p1.y, p2.y),
		std::min(p1.z, p2.z)
	};
}
glm::vec3 GetMaxPoint(const glm::vec3& p1, const glm::vec3& p2) {
	return {
		std::max(p1.x, p2.x),
		std::max(p1.y, p2.y),
		std::max(p1.z, p2.z)
	};
}

Planet::Planet() {
	m_radius = 10000;
	m_maxHeight = 400;
	m_lodAmount = 12;
	m_nodeVertexAmount = 20;

	m_tree = new QuadTree(6);
	m_position = { 0, 0, 0 };
	for (int i = 0; i < 6; i++) {
		auto node = m_tree->GetBranch(i);
		node->face = i;
		node->localPosition = { 0, 0 };
		node->size = pow(2, m_lodAmount);
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

// dir: 0 - top, 1 - bottom, 2 - left, 3 - right
QUADTREE_NODE* Planet::GetNeighbor(QUADTREE_NODE * node, int dir) {
	auto parent = node->parent;
	if (parent == nullptr)
		return nullptr;
	if (dir == 0) {
		if (parent->child[2] == node)
			return parent->child[3];
		if (parent->child[0] == node)
			return parent->child[1];
		auto newNode = GetNeighbor(parent, dir);
		if (newNode == nullptr || newNode->type == QUADTREENODETYPE_LEAF)
			return newNode;
		if (parent->child[3] == node)
			return newNode->child[2];
		return newNode->child[0];
	}
	if (dir == 1) {
		if (parent->child[3] == node)
			return parent->child[2];
		if (parent->child[1] == node)
			return parent->child[0];
		auto newNode = GetNeighbor(parent, dir);
		if (newNode == nullptr || newNode->type == QUADTREENODETYPE_LEAF)
			return newNode;
		if (parent->child[2] == node)
			return newNode->child[3];
		return newNode->child[1];
	}
	if (dir == 2) {
		if (parent->child[2] == node)
			return parent->child[0];
		if (parent->child[3] == node)
			return parent->child[1];
		auto newNode = GetNeighbor(parent, dir);
		if (newNode == nullptr || newNode->type == QUADTREENODETYPE_LEAF)
			return newNode;
		if (parent->child[0] == node)
			return newNode->child[2];
		return newNode->child[3];
	}
	if (dir == 3) {
		if (parent->child[0] == node)
			return parent->child[2];
		if (parent->child[1] == node)
			return parent->child[3];
		auto newNode = GetNeighbor(parent, dir);
		if (newNode == nullptr || newNode->type == QUADTREENODETYPE_LEAF)
			return newNode;
		if (parent->child[2] == node)
			return newNode->child[0];
		return newNode->child[1];
	}
}

void Planet::GetLod(QUADTREE_NODE* node, std::vector<QUADTREE_NODE*>& queue, glm::vec3 cameraPos) {
	node->ignoreRender = false;
	if (node->mesh != nullptr && node->mesh->IsBuffered()) {
		double dist = GetAABBDistance(cameraPos, node->minPoint, node->maxPoint);
		double chunkSize = glm::length(node->minPoint - node->maxPoint);
		double error = chunkSize / dist;
		double maxError = 1;
		if (error > maxError && node->level + 1 < m_lodAmount) {
			node->Split();
			GetLod(node->child[0], queue, cameraPos);
			GetLod(node->child[1], queue, cameraPos);
			GetLod(node->child[2], queue, cameraPos);
			GetLod(node->child[3], queue, cameraPos);
			return;
		}
	}
	queue.push_back(node);

	// get edge nodes for crack filling
	if (node->parent != nullptr) {

		// determine edge node is on
		std::vector<int> edgePath;
		int edgey = -1;
		int edgex = -1;
		auto otherParent = node;
		if (node->index == 0) {
			edgey = 1;
			while (otherParent->parent != nullptr) {
				if (otherParent->parent->index == 2 || otherParent->parent->index == 3) {
					edgey = -1;
					break;
				}
				if (otherParent->parent->index == -1) {
					break;
				}
				if (otherParent->parent->index == 1) {
					edgePath.push_back(1);
				}
				else {
					edgePath.push_back(0);
				}
				otherParent = otherParent->parent;
			}
		}
		if (node->index == 1) {
			edgex = 1;
			while (otherParent->parent != nullptr) {
				if (otherParent->parent->index == 0 || otherParent->parent->index == 2) {
					edgex = -1;
					break;
				}
				if (otherParent->parent->index == -1) {
					break;
				}
				if (otherParent->parent->index == 3) {
					edgePath.push_back(3);
				}
				else {
					edgePath.push_back(1);
				}
				otherParent = otherParent->parent;
			}
		}
		if (node->index == 2) {
			edgex = 0;
			while (otherParent->parent != nullptr) {
				if (otherParent->parent->index == 1 || otherParent->parent->index == 3) {
					edgex = -1;
					break;
				}
				if (otherParent->parent->index == -1) {
					break;
				}
				if (otherParent->parent->index == 0) {
					edgePath.push_back(0);
				}
				else {
					edgePath.push_back(2);
				}
				otherParent = otherParent->parent;
			}
		}
		if (node->index == 3) {
			edgey = 0;
			while (otherParent->parent != nullptr) {
				if (otherParent->parent->index == 0 || otherParent->parent->index == 1) {
					edgey = -1;
					break;
				}
				if (otherParent->parent->index == -1) {
					break;
				}
				if (otherParent->parent->index == 2) {
					edgePath.push_back(2);
				}
				else {
					edgePath.push_back(3);
				}
				otherParent = otherParent->parent;
			}
		}
		// find edge neighbours
		if (edgey == 0) {
			switch (node->face)
			{
			case PLANET_FACE::TOP:
			{
				auto branch = m_tree->GetBranch(PLANET_FACE::LEFT);
				for (int i = edgePath.size() - 1; i >= 0; i--) {
					if (branch->child[-2 + edgePath[i]] != nullptr) {
						branch = branch->child[-2 + edgePath[i]];
					}
				}
				branch->edge[2] = 1;
			}
			break;
			case PLANET_FACE::BOTTOM:
			{
				auto branch = m_tree->GetBranch(PLANET_FACE::LEFT);
				for (int i = edgePath.size() - 1; i >= 0; i--) {
					if (branch->child[5 - edgePath[i]] != nullptr) {
						branch = branch->child[5 - edgePath[i]];
					}
				}
				branch->edge[3] = 1;
			}
			break;
			case PLANET_FACE::LEFT:
			{
				auto branch = m_tree->GetBranch(PLANET_FACE::TOP);
				for (int i = edgePath.size() - 1; i >= 0; i--) {
					if (branch->child[edgePath[i]] != nullptr) {
						branch = branch->child[edgePath[i]];
					}
				}
				branch->edge[3] = 1;
			}
			break;
			case PLANET_FACE::RIGHT:
			{
				auto branch = m_tree->GetBranch(PLANET_FACE::TOP);
				for (int i = edgePath.size() - 1; i >= 0; i--) {
					if (branch->child[-2 + edgePath[i]] != nullptr) {
						branch = branch->child[-2 + edgePath[i]];
					}
				}
				branch->edge[2] = 1;
			}
			break;
			case PLANET_FACE::FRONT:
			{
				auto branch = m_tree->GetBranch(PLANET_FACE::LEFT);
				for (int i = edgePath.size() - 1; i >= 0; i--) {
					if (branch->child[edgePath[i] % 3] != nullptr) {
						branch = branch->child[edgePath[i] % 3];
					}
				}
				branch->edge[1] = 1;
			}
			break;
			case PLANET_FACE::BACK:
			{
				auto branch = m_tree->GetBranch(PLANET_FACE::LEFT);
				for (int i = edgePath.size() - 1; i >= 0; i--) {
					if (branch->child[(edgePath[i] - 2) * 2 + 1] != nullptr) {
						branch = branch->child[(edgePath[i] - 2) * 2 + 1];
					}
				}
				branch->edge[0] = 1;
			}
			break;
			default:
				break;
			}
		}
		else if (edgey == 1) {
			switch (node->face)
			{
			case PLANET_FACE::TOP:
			{
				auto branch = m_tree->GetBranch(PLANET_FACE::RIGHT);
				for (int i = edgePath.size() - 1; i >= 0; i--) {
					if (branch->child[2 + edgePath[i]] != nullptr) {
						branch = branch->child[2 + edgePath[i]];
					}
				}
				branch->edge[3] = 1;
			}
			break;
			case PLANET_FACE::BOTTOM:
			{
				auto branch = m_tree->GetBranch(PLANET_FACE::RIGHT);
				for (int i = edgePath.size() - 1; i >= 0; i--) {
					if (branch->child[1 - edgePath[i]] != nullptr) {
						branch = branch->child[1 - edgePath[i]];
					}
				}
				branch->edge[2] = 1;
			}
			break;
			case PLANET_FACE::LEFT:
			{
				auto branch = m_tree->GetBranch(PLANET_FACE::BOTTOM);
				for (int i = edgePath.size() - 1; i >= 0; i--) {
					if (branch->child[3 - edgePath[i]] != nullptr) {
						branch = branch->child[3 - edgePath[i]];
					}
				}
				branch->edge[3] = 1;
			}
			break;
			case PLANET_FACE::RIGHT:
			{
				auto branch = m_tree->GetBranch(PLANET_FACE::BOTTOM);
				for (int i = edgePath.size() - 1; i >= 0; i--) {
					if (branch->child[1 - edgePath[i]] != nullptr) {
						branch = branch->child[1 - edgePath[i]];
					}
				}
				branch->edge[2] = 1;
			}
			break;
			case PLANET_FACE::FRONT:
			{
				auto branch = m_tree->GetBranch(PLANET_FACE::RIGHT);
				for (int i = edgePath.size() - 1; i >= 0; i--) {
					if (branch->child[edgePath[i] * 2] != nullptr) {
						branch = branch->child[edgePath[i]  * 2];
					}
				}
				branch->edge[1] = 1;
			}
			break;
			case PLANET_FACE::BACK:
			{
				auto branch = m_tree->GetBranch(PLANET_FACE::RIGHT);
				for (int i = edgePath.size() - 1; i >= 0; i--) {
					if (branch->child[4 - (edgePath[i] * 2 + 1)] != nullptr) {
						branch = branch->child[4 -(edgePath[i] * 2 + 1)];
					}
				}
				branch->edge[0] = 1;
			}
			break;
			default:
				break;
			}
		}
		if (edgex == 0) {
			switch (node->face)
			{
			case PLANET_FACE::TOP:
			{
				auto branch = m_tree->GetBranch(PLANET_FACE::FRONT);
				for (int i = edgePath.size() - 1; i >= 0; i--) {
					if (branch->child[1 + edgePath[i]] != nullptr) {
						branch = branch->child[1 + edgePath[i]];
					}
				}
				branch->edge[0] = 1;
			}
			break;
			case PLANET_FACE::BOTTOM:
			{
				auto branch = m_tree->GetBranch(PLANET_FACE::BACK);
				for (int i = edgePath.size() - 1; i >= 0; i--) {
					if (branch->child[1 + edgePath[i]] != nullptr) {
						branch = branch->child[1 + edgePath[i]];
					}
				}
				branch->edge[0] = 1;
			}
			break;
			case PLANET_FACE::LEFT:
			{
				auto branch = m_tree->GetBranch(PLANET_FACE::BACK);
				for (int i = edgePath.size() - 1; i >= 0; i--) {
					if (branch->child[std::clamp(1 + edgePath[i], 2, 3)] != nullptr) {
						branch = branch->child[std::clamp(1 + edgePath[i], 2, 3)];
					}
				}
				branch->edge[3] = 1;
			}
			break;
			case PLANET_FACE::RIGHT:
			{
				auto branch = m_tree->GetBranch(PLANET_FACE::FRONT);
				for (int i = edgePath.size() - 1; i >= 0; i--) {
					if (branch->child[std::clamp(edgePath[i], 0, 1)] != nullptr) {
						branch = branch->child[std::clamp(edgePath[i], 0, 1)];
					}
				}
				branch->edge[2] = 1;
			}
			break;
			case PLANET_FACE::FRONT:
			{
				auto branch = m_tree->GetBranch(PLANET_FACE::BOTTOM);
				for (int i = edgePath.size() - 1; i >= 0; i--) {
					if (branch->child[edgePath[i] + 1] != nullptr) {
						branch = branch->child[edgePath[i] + 1];
					}
				}
				branch->edge[0] = 1;
			}
			break;
			case PLANET_FACE::BACK:
			{
				auto branch = m_tree->GetBranch(PLANET_FACE::TOP);
				for (int i = edgePath.size() - 1; i >= 0; i--) {
					if (branch->child[1 + edgePath[i]] != nullptr) {
						branch = branch->child[1 + edgePath[i]];
					}
				}
				branch->edge[0] = 1;
			}
			break;
			default:
				break;
			}
		}
		else if (edgex == 1) {
			switch (node->face)
			{
			case PLANET_FACE::TOP:
			{
				auto branch = m_tree->GetBranch(PLANET_FACE::BACK);
				for (int i = edgePath.size() - 1; i >= 0; i--) {
					if (branch->child[-1 + edgePath[i]] != nullptr) {
						branch = branch->child[-1 + edgePath[i]];
					}
				}
				branch->edge[1] = 1;
			}
			break;
			case PLANET_FACE::BOTTOM:
			{
				auto branch = m_tree->GetBranch(PLANET_FACE::FRONT);
				for (int i = edgePath.size() - 1; i >= 0; i--) {
					if (branch->child[-1 + edgePath[i]] != nullptr) {
						branch = branch->child[-1 + edgePath[i]];
					}
				}
				branch->edge[1] = 1;
			}
			break;
			case PLANET_FACE::LEFT:
			{
				auto branch = m_tree->GetBranch(PLANET_FACE::FRONT);
				for (int i = edgePath.size() - 1; i >= 0; i--) {
					if (branch->child[5 - std::clamp(1 + edgePath[i], 2, 3)] != nullptr) {
						branch = branch->child[5 - std::clamp(1 + edgePath[i], 2, 3)];
					}
				}
				branch->edge[3] = 1;
			}
			break;
			case PLANET_FACE::RIGHT:
			{
				auto branch = m_tree->GetBranch(PLANET_FACE::BACK);
				for (int i = edgePath.size() - 1; i >= 0; i--) {
					if (branch->child[1 - std::clamp(edgePath[i] - 1, 0, 1)] != nullptr) {
						branch = branch->child[1 - std::clamp(edgePath[i] - 1, 0, 1)];
					}
				}
				branch->edge[2] = 1;
			}
			break;
			case PLANET_FACE::FRONT:
			{
				auto branch = m_tree->GetBranch(PLANET_FACE::TOP);
				for (int i = edgePath.size() - 1; i >= 0; i--) {
					if (branch->child[-1 + edgePath[i]] != nullptr) {
						branch = branch->child[-1 + edgePath[i]];
					}
				}
				branch->edge[1] = 1;
			}
			break;
			case PLANET_FACE::BACK:
			{
				auto branch = m_tree->GetBranch(PLANET_FACE::BOTTOM);
				for (int i = edgePath.size() - 1; i >= 0; i--) {
					if (branch->child[-1 + edgePath[i]] != nullptr) {
						branch = branch->child[-1 + edgePath[i]];
					}
				}
				branch->edge[1] = 1;
			}
			break;
			default:
				break;
			}
		}

		// same face neighbours
		auto parent = node->parent;
		if (parent != nullptr) {
			if (node->index == 0) {
				auto neighbor = GetNeighbor(parent, 1);
				if (neighbor != nullptr) {
					neighbor->edge[0] = 1;
				}
			}
			else if (node->index == 1) {
				auto neighbor = GetNeighbor(parent, 2);
				if (neighbor != nullptr) {
					neighbor->edge[3] = 1;
				}
			}
			else if (node->index == 2) {
				auto neighbor = GetNeighbor(parent, 3);
				if (neighbor != nullptr) {
					neighbor->edge[2] = 1;
				}
			}
			else if (node->index == 3) {
				auto neighbor = GetNeighbor(parent, 0);
				if (neighbor != nullptr) {
					neighbor->edge[1] = 1;
				}
			}
		}
	}
}

bool Planet::Cleanup(QUADTREE_NODE* node, double currentTime, int& meshCount) {
	if (node->type == QUADTREENODETYPE_NODE) {
		int clean = 0;
		for (int i = 0; i < 4; i++) {
			clean += Cleanup(node->child[i], currentTime, meshCount);
		}
		if (clean == 4) {
			node->Merge();
			meshCount += 4;
		}
	}
	if (node->mesh != nullptr && node->mesh->IsBuffered()) {
		if (node->lastRenderTime + NODE_LIFETIME < currentTime) {
			return true;
		}
	}
	return false;
}

void Planet::Render(glm::vec3 cameraPos, Shader* shader) {
	Timer timer;
	double currentTime = glfwGetTime();
	// select nodes for rendering
	std::vector<QUADTREE_NODE*> queue;
	for (int i = 0; i < 6; i++) {
		auto branch = m_tree->GetBranch(i);
		GetLod(branch, queue, cameraPos);
	}
	timer.PrintElapsed("Node selection");
	timer.Reset();
	// render queued nodes
	int limitCounter = 0;
	for (int i = 0; i < queue.size(); i++) {
		auto node = queue[i];
		// generate mesh if node doesn't have one
		if (node->mesh == nullptr && limitCounter < MAX_MESHES_GENERATED_PER_FRAME) {
			GenerateMesh(node);
			limitCounter++;
		}

		// if node can't be rendered render parent
		auto parent = node->parent;
		if (parent != nullptr) {
			for (int j = 0; j < 4; j++) {
				if (parent->child[j]->mesh == nullptr or !parent->child[j]->mesh->IsBuffered()) {
					for (int k = 0; k < 4; k++) {
						parent->child[j]->ignoreRender = true;
					}
					queue.push_back(parent);
					break;
				}
			}
		}
		if (node->mesh != nullptr) {
			// unmap buffer
			if (node->mesh->NeedUnmap()) {
				node->mesh->UnmapBuffer();
				node->mesh->NeedUnmap(false);
				node->mesh->IsBuffered(true);
			}
			// render
			if (node->mesh->IsBuffered() && !node->ignoreRender) {
				shader->uniformArr1i("u_edges", 4, node->edge);
				node->edge[0] = 0;
				node->edge[1] = 0;
				node->edge[2] = 0;
				node->edge[3] = 0;
				node->mesh->Draw(GL_PATCHES);
				DrawBoundingBox(node->minPoint, node->maxPoint);

				// update parent render time so it isn't deleted in cleanup
				node->lastRenderTime = currentTime;
				auto nextParent = node->parent;
				while (nextParent != nullptr) {
					nextParent->lastRenderTime = currentTime;
					nextParent = nextParent->parent;
				}
			}
		}
	}
	timer.PrintElapsed("Node render");


	// node cleanup
	if (currentTime > m_nodeCleanupTimer) {
		Timer cleanupTimer;
		m_nodeCleanupTimer = currentTime + NODE_CLEANUP_INTERVAL;
		int meshCount = 0;
		for (int i = 0; i < 6; i++) {
			auto branch = m_tree->GetBranch(i);
			Cleanup(branch, currentTime, meshCount);
		}
		if (TIMER_ENABLE_PRINT)
			std::cout << std::format("[NODE CLEANUP] removed {} meshes in {}ms\n", meshCount, cleanupTimer.Elapsed());
	}
}

void Planet::MeshCreateData(QUADTREE_NODE* node,void* ptr) {
	ScopedTimer timer("MESH GEN");
	auto data = node->mesh->GetVertexData();
	data->clear();

	{
		glm::vec3 color = { 
			floor((node->level % 3) * 0.55),
			floor(((node->level + 1) % 3) * 0.55),
			floor(((node->level + 2) % 3) * 0.55)
		};

		glm::vec3 rotation(0);
		float angle = 0;
		switch (node->face)
		{
		case PLANET_FACE::TOP:
			rotation = glm::vec3(1, 0, 0);
			angle = glm::radians(0.f);
			break;
		case PLANET_FACE::BOTTOM:
			rotation = glm::vec3(0, 0, 1);
			angle = glm::radians(180.f);
			break;
		case PLANET_FACE::LEFT:
			rotation = glm::vec3(1, 0, 0);
			angle = glm::radians(90.f);
			break;
		case PLANET_FACE::RIGHT:
			rotation = glm::vec3(1, 0, 0);
			angle = glm::radians(-90.f);
			break;
		case PLANET_FACE::FRONT:
			rotation = glm::vec3(0, 0, 1);
			angle = glm::radians(90.f);
			break;
		case PLANET_FACE::BACK:
			rotation = glm::vec3(0, 0, -1);
			angle = glm::radians(90.f);
			break;
		default:
			break;
		}

		int dataSize = (2 * m_nodeVertexAmount + 3) * (2 * m_nodeVertexAmount + 3);
		std::vector<float> xData;
		std::vector<float> yData;
		std::vector<float> zData;
		std::vector<glm::vec3> nData;
		xData.reserve(dataSize);
		yData.reserve(dataSize);
		zData.reserve(dataSize);
		nData.reserve(dataSize);

		// generate sphere vertices
		glm::vec3 offset = glm::vec3(node->localPosition.x, 0, node->localPosition.y) / powf(2, m_lodAmount);
		float scalar = pow(2, node->level);
		{
			// unit sphere
			for (int i = -m_nodeVertexAmount - 1; i <= m_nodeVertexAmount + 1; i++) {
				for (int j = -m_nodeVertexAmount - 1; j <= m_nodeVertexAmount + 1; j++) {
					glm::vec3 point = offset + glm::vec3{ float(i) / m_nodeVertexAmount / scalar, 1, float(j) / m_nodeVertexAmount / scalar };
					point = MapToSphere(glm::rotate(point, angle, rotation));
					xData.push_back(point.x);
					yData.push_back(point.y);
					zData.push_back(point.z);
					nData.push_back({ 0, 0, 0 });
					//// get edge for crack filling
					//int edgeBottom = 0;
					//int edgeTop = 0;
					//int edgeLeft = 0;
					//int edgeRight = 0;
					//if (i == -m_nodeVertexAmount)
					//	edgeBottom = 1;
					//else if (i == m_nodeVertexAmount - 1)
					//	edgeTop = 2;
					//if (j == -m_nodeVertexAmount)
					//	edgeLeft = 1;
					//else if (j == m_nodeVertexAmount - 1)
					//	edgeRight = 2;
					//data->push_back({ point0, color, edgeLeft });
					//data->push_back({ point1, color, 0 });
					//data->push_back({ point3, color, edgeTop });
					//data->push_back({ point3, color, edgeRight });
					//data->push_back({ point2, color, 0 });
					//data->push_back({ point0, color, edgeBottom });
				}
			}
			// noise
			m_maxHeight = 500;
			float power_bigHills = 1.0;
			float power_medHills = 0.0;
			float power_lowHills = 0.0;

			std::vector<float> bigHillData(dataSize);
			std::vector<float> medHillData(dataSize);
			std::vector<float> lowHillData(dataSize);

			//// big hills
			//{
			//	int seed = 1337;
			//	float scale = 2.0f;
			//	float gain = 1.0;
			//	float weight = 0.5;
			//	int octaves = 4;
			//	float lacunarity = 1.0;
			//	FastNoise::New<FastNoise::
			//	auto fPerlin = FastNoise::New<FastNoise::Perlin>();
			//	auto fScale = FastNoise::New<FastNoise::DomainScale>();
			//	auto fNoise = FastNoise::New<FastNoise::FractalFBm>();
			//	fScale->SetSource(fPerlin);
			//	fScale->SetScale(scale);
			//	fNoise->SetSource(fScale);
			//	fNoise->SetGain(gain);
			//	fNoise->SetWeightedStrength(weight);
			//	fNoise->SetOctaveCount(octaves);
			//	fNoise->SetLacunarity(lacunarity);
			//	fNoise->GenPositionArray3D(bigHillData.data(), dataSize,
			//	xData.data(), yData.data(), zData.data(), 0, 0, 0, seed);
			//}
			//// med hills
			//{
			//	int seed = 1337;
			//	float scale = 10.0f;
			//	float gain = 0.5;
			//	float weight = 0.5;
			//	int octaves = 6;
			//	float lacunarity = 2.0;
			//	auto fPerlin = FastNoise::New<FastNoise::Perlin>();
			//	auto fScale = FastNoise::New<FastNoise::DomainScale>();
			//	auto fNoise = FastNoise::New<FastNoise::FractalFBm>();
			//	fScale->SetSource(fPerlin);
			//	fScale->SetScale(scale);
			//	fNoise->SetSource(fScale);
			//	fNoise->SetGain(gain);
			//	fNoise->SetWeightedStrength(weight);
			//	fNoise->SetOctaveCount(octaves);
			//	fNoise->SetLacunarity(lacunarity);
			//	fNoise->GenPositionArray3D(medHillData.data(), dataSize,
			//		xData.data(), yData.data(), zData.data(), 0, 0, 0, seed);
			//}
			//// low hills
			//{
			//	int seed = 1337;
			//	float scale = 10.0f;
			//	float gain = 0.5;
			//	float weight = 0.5;
			//	int octaves = 6;
			//	float lacunarity = 2.0;
			//	auto fPerlin = FastNoise::New<FastNoise::Perlin>();
			//	auto fScale = FastNoise::New<FastNoise::DomainScale>();
			//	auto fNoise = FastNoise::New<FastNoise::FractalFBm>();
			//	fScale->SetSource(fPerlin);
			//	fScale->SetScale(scale);
			//	fNoise->SetSource(fScale);
			//	fNoise->SetGain(gain);
			//	fNoise->SetWeightedStrength(weight);
			//	fNoise->SetOctaveCount(octaves);
			//	fNoise->SetLacunarity(lacunarity);
			//	fNoise->GenPositionArray3D(lowHillData.data(), dataSize,
			//		xData.data(), yData.data(), zData.data(), 0, 0, 0, seed);
			//}

			std::vector<float> noiseData(dataSize);
			{
				auto generator = FastNoise::New<FastNoise::Perlin>();
				// big hills
				auto bdomainScale = FastNoise::New<FastNoise::DomainScale>();
				bdomainScale->SetSource(generator);
				bdomainScale->SetScale(10);
				auto bfractalFbm = FastNoise::New<FastNoise::FractalFBm>();
				bfractalFbm->SetSource(bdomainScale);
				bfractalFbm->SetGain(0.4);
				bfractalFbm->SetWeightedStrength(0);
				bfractalFbm->SetOctaveCount(4);
				bfractalFbm->SetLacunarity(2.5);
				auto bmodifierRemap = FastNoise::New<FastNoise::Remap>();
				bmodifierRemap->SetSource(bfractalFbm);
				bmodifierRemap->SetRemap(-1, 1, -2, 1);
				auto bblendMaxSmooth = FastNoise::New<FastNoise::MaxSmooth>();
				bblendMaxSmooth->SetLHS(bmodifierRemap);
				bblendMaxSmooth->SetRHS(-0.2);
				bblendMaxSmooth->SetSmoothness(0.1);

				// med hills
				auto mdomainScale = FastNoise::New<FastNoise::DomainScale>();
				mdomainScale->SetSource(generator);
				mdomainScale->SetScale(20);
				auto mfractalFbm = FastNoise::New<FastNoise::FractalFBm>();
				mfractalFbm->SetSource(mdomainScale);
				mfractalFbm->SetGain(0.5);
				mfractalFbm->SetWeightedStrength(0);
				mfractalFbm->SetOctaveCount(4);
				mfractalFbm->SetLacunarity(2);
				auto mmodifierRemap = FastNoise::New<FastNoise::Remap>();
				mmodifierRemap->SetSource(mfractalFbm);
				mmodifierRemap->SetRemap(-1, 1, -0.2, 0.1);

				// low hills
				auto ldomainScale = FastNoise::New<FastNoise::DomainScale>();
				ldomainScale->SetSource(generator);
				ldomainScale->SetScale(40);
				auto lfractalFbm = FastNoise::New<FastNoise::FractalFBm>();
				lfractalFbm->SetSource(ldomainScale);
				lfractalFbm->SetGain(0.5);
				lfractalFbm->SetWeightedStrength(0);
				lfractalFbm->SetOctaveCount(5);
				lfractalFbm->SetLacunarity(3);
				auto lmodifierRemap = FastNoise::New<FastNoise::Remap>();
				lmodifierRemap->SetSource(lfractalFbm);
				lmodifierRemap->SetRemap(-1, 1, -0.03, 0.03);

				// extra noise
				auto edomainScale = FastNoise::New<FastNoise::DomainScale>();
				edomainScale->SetSource(generator);
				edomainScale->SetScale(50);
				auto efractalFbm = FastNoise::New<FastNoise::FractalFBm>();
				efractalFbm->SetSource(edomainScale);
				efractalFbm->SetGain(2);
				efractalFbm->SetWeightedStrength(0);
				efractalFbm->SetOctaveCount(5);
				efractalFbm->SetLacunarity(3);
				auto emodifierRemap = FastNoise::New<FastNoise::Remap>();
				emodifierRemap->SetSource(efractalFbm);
				emodifierRemap->SetRemap(-1, 1, -0.002, 0.002);

				// combine
				auto cAdd1 = FastNoise::New<FastNoise::Add>();
				cAdd1->SetLHS(bblendMaxSmooth);
				cAdd1->SetRHS(mmodifierRemap);
				auto cAdd2 = FastNoise::New<FastNoise::Add>();
				cAdd2->SetLHS(cAdd1);
				cAdd2->SetRHS(lmodifierRemap);
				auto cAdd3 = FastNoise::New<FastNoise::Add>();
				cAdd3->SetLHS(cAdd2);
				cAdd3->SetRHS(emodifierRemap);

				cAdd3->GenPositionArray3D(noiseData.data(), dataSize,
					xData.data(), yData.data(), zData.data(), 0, 0, 0, 1337);
			}
			// apply noise
			for (int i = 0; i < dataSize; i++) {
				//float currentNoise =
				//	bigHillData[i] * power_bigHills +
				//	medHillData[i] * power_medHills +
				//	lowHillData[i] * power_lowHills;
				//currentNoise /= power_bigHills + power_medHills + power_lowHills;
				float currentNoise = noiseData[i];
				glm::vec3 point = glm::vec3{ xData[i], yData[i], zData[i] };
				point = point * m_radius + point * currentNoise * m_maxHeight;
				xData[i] = point.x;
				yData[i] = point.y;
				zData[i] = point.z;
			}
			bigHillData.clear();
			medHillData.clear();
			lowHillData.clear();
		}
		// get triangle indices and normals
		std::vector<glm::ivec3> triangles(m_nodeVertexAmount * m_nodeVertexAmount * 4);
		int dataOffset = (2 * m_nodeVertexAmount + 3);
		{
			for (int i = 0; i < m_nodeVertexAmount * 2; i++) {
				for (int j = 0; j < m_nodeVertexAmount * 2; j++) {
					// get indexes
					int inx0 = i + 2;
					int iny0 = j + 1;
					int inx1 = i + 2;
					int iny1 = j + 2;
					int inx2 = i + 1;
					int iny2 = j + 1;
					int inx3 = i + 1;
					int iny3 = j + 2;
					int index0 = inx0 * dataOffset + iny0;
					int index1 = inx1 * dataOffset + iny1;
					int index2 = inx2 * dataOffset + iny2;
					int index3 = inx3 * dataOffset + iny3;
					triangles.push_back({ index3, index1, index0 });
					triangles.push_back({ index0, index2, index3 });

					// get normals
					glm::vec3 point0 = glm::vec3{ xData[index3], yData[index3], zData[index3] };
					glm::vec3 point1 = glm::vec3{ xData[index1], yData[index1], zData[index1] };
					glm::vec3 point2 = glm::vec3{ xData[index0], yData[index0], zData[index0] };
					glm::vec3 normal = glm::normalize(glm::cross(point2 - point0, point1 - point0));
					nData[index3] += normal;
					nData[index1] += normal;
					nData[index0] += normal;

					point0 = glm::vec3{ xData[index0], yData[index0], zData[index0] };
					point1 = glm::vec3{ xData[index2], yData[index2], zData[index2] };
					point2 = glm::vec3{ xData[index3], yData[index3], zData[index3] };
					normal = glm::normalize(glm::cross(point2 - point0, point1 - point0));
					nData[index0] += normal;
					nData[index2] += normal;
					nData[index3] += normal;
				}
			}
			// get normals on edges
			for (int i = 0; i < 2 * m_nodeVertexAmount + 2; i++) {
				for (int j = 0; j < 2 * m_nodeVertexAmount + 2; j += 2 * m_nodeVertexAmount + 1) {
					// get indexes
					int inx0 = i + 1;
					int iny0 = j + 0;
					int inx1 = i + 1;
					int iny1 = j + 1;
					int inx2 = i + 0;
					int iny2 = j + 0;
					int inx3 = i + 0;
					int iny3 = j + 1;
					int index0 = inx0 * dataOffset + iny0;
					int index1 = inx1 * dataOffset + iny1;
					int index2 = inx2 * dataOffset + iny2;
					int index3 = inx3 * dataOffset + iny3;

					// get normals
					glm::vec3 point0 = glm::vec3{ xData[index3], yData[index3], zData[index3] };
					glm::vec3 point1 = glm::vec3{ xData[index1], yData[index1], zData[index1] };
					glm::vec3 point2 = glm::vec3{ xData[index0], yData[index0], zData[index0] };
					glm::vec3 normal = glm::normalize(glm::cross(point2 - point0, point1 - point0));
					nData[index3] += normal;
					nData[index1] += normal;
					nData[index0] += normal;

					point0 = glm::vec3{ xData[index0], yData[index0], zData[index0] };
					point1 = glm::vec3{ xData[index2], yData[index2], zData[index2] };
					point2 = glm::vec3{ xData[index3], yData[index3], zData[index3] };
					normal = glm::normalize(glm::cross(point2 - point0, point1 - point0));
					nData[index0] += normal;
					nData[index2] += normal;
					nData[index3] += normal;
				}
			}
			for (int j = 1; j < 2 * m_nodeVertexAmount + 1; j++) {
				for (int i = 0; i < 2 * m_nodeVertexAmount + 2; i += 2 * m_nodeVertexAmount + 1) {
					// get indexes
					int inx0 = i + 1;
					int iny0 = j + 0;
					int inx1 = i + 1;
					int iny1 = j + 1;
					int inx2 = i + 0;
					int iny2 = j + 0;
					int inx3 = i + 0;
					int iny3 = j + 1;
					int index0 = inx0 * dataOffset + iny0;
					int index1 = inx1 * dataOffset + iny1;
					int index2 = inx2 * dataOffset + iny2;
					int index3 = inx3 * dataOffset + iny3;

					// get normals
					glm::vec3 point0 = glm::vec3{ xData[index3], yData[index3], zData[index3] };
					glm::vec3 point1 = glm::vec3{ xData[index1], yData[index1], zData[index1] };
					glm::vec3 point2 = glm::vec3{ xData[index0], yData[index0], zData[index0] };
					glm::vec3 normal = glm::normalize(glm::cross(point2 - point0, point1 - point0));
					nData[index3] += normal;
					nData[index1] += normal;
					nData[index0] += normal;

					point0 = glm::vec3{ xData[index0], yData[index0], zData[index0] };
					point1 = glm::vec3{ xData[index2], yData[index2], zData[index2] };
					point2 = glm::vec3{ xData[index3], yData[index3], zData[index3] };
					normal = glm::normalize(glm::cross(point2 - point0, point1 - point0));
					nData[index0] += normal;
					nData[index2] += normal;
					nData[index3] += normal;
				}
			}
		}

		// combine position and normal data
		{
			node->minPoint = glm::rotate(m_radius * MapToSphere(offset + glm::vec3{ -1.f / scalar, 1, -1.f / scalar }), angle, rotation);
			node->maxPoint = glm::rotate(m_radius * MapToSphere(offset + glm::vec3{ 1.f / scalar, 1,  1.f / scalar }), angle, rotation);
			for (int i = 0; i < triangles.size(); i++) {
				glm::vec3 point0 = glm::vec3{ xData[triangles[i].x], yData[triangles[i].x], zData[triangles[i].x] };
				glm::vec3 point1 = glm::vec3{ xData[triangles[i].y], yData[triangles[i].y], zData[triangles[i].y] };
				glm::vec3 point2 = glm::vec3{ xData[triangles[i].z], yData[triangles[i].z], zData[triangles[i].z] };

				data->push_back({ point0, color, glm::normalize(nData[triangles[i].x]), 0 });
				data->push_back({ point1, color, glm::normalize(nData[triangles[i].y]), 0 });
				data->push_back({ point2, color, glm::normalize(nData[triangles[i].z]), 0 });
				node->minPoint = GetMinPoint(node->minPoint, GetMinPoint(point0, GetMinPoint(point1, point2)));
				node->maxPoint = GetMaxPoint(node->maxPoint, GetMaxPoint(point0, GetMaxPoint(point1, point2)));
			}
		}
	}

	if (!data->empty())
		memcpy(ptr, data->data(), sizeof(Vertex) * data->size());
	node->mesh->NeedUnmap(true);
}

void Planet::GenerateMesh(QUADTREE_NODE* node) {
	if (node->mesh != nullptr)
		delete node->mesh;
	node->mesh = new Mesh;
	node->mesh->Allocate(14440);
	void* ptr = node->mesh->MapBuffer();
	m_threadPool.AddJob(std::bind(&Planet::MeshCreateData, this, node, ptr));
}

ThreadPool Planet::m_threadPool(NUM_THREADS);