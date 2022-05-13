#include "Input.h"

Input::Input() {
}

Input::~Input() {
	sm_inputMap.erase(m_handle);
	RemoveCallbacks();
}

void Input::Init(GLFWwindow* handle) {
	m_handle = handle;
	sm_inputMap[m_handle] = this;
	SetCallbacks();
}

void Input::Poll() {
	for (auto& key : m_keys) {
		key.second.press = false;
		key.second.release = false;
	}
	for (auto& key : m_buttons) {
		key.second.press = false;
		key.second.release = false;
	}
	m_scrollOffsetX = 0;
	m_scrollOffsetY = 0;
}

bool Input::IsPressed(KEYBOARD key) {
	return m_keys[key].press;
}

bool Input::IsPressed(MOUSE key) {
	return m_buttons[key].press;
}

bool Input::IsHeld(KEYBOARD key) {
	return m_keys[key].hold;
}

bool Input::IsHeld(MOUSE key) {
	return m_buttons[key].hold;
}

bool Input::IsReleased(KEYBOARD key) {
	return m_keys[key].release;
}

bool Input::IsReleased(MOUSE key) {
	return m_buttons[key].release;
}

glm::vec2 Input::GetMousePosition() {
	double tempx, tempy;
	int w, h;
	glfwGetFramebufferSize(m_handle, &w, &h);
	glfwGetCursorPos(m_handle, &tempx, &tempy);
	return { tempx, h - tempy };
}

float Input::GetMouseScrollX() {
	return m_scrollOffsetX;
}

float Input::GetMouseScrollY() {
	return m_scrollOffsetY;
}

void Input::SetCallbacks() {
	glfwSetKeyCallback(m_handle, KeyCallback);
	glfwSetMouseButtonCallback(m_handle, ButtonCallback);
	glfwSetScrollCallback(m_handle, ScrollCallback);
}
void Input::RemoveCallbacks() {
	glfwSetKeyCallback(m_handle, NULL);
	glfwSetMouseButtonCallback(m_handle, NULL);
	glfwSetScrollCallback(m_handle, NULL);
}

void Input::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	Input* loc = sm_inputMap.at(window);
	if (loc != nullptr) {
		if (action == GLFW_PRESS) {
			loc->m_keys[static_cast<KEYBOARD>(key)].press = true;
			loc->m_keys[static_cast<KEYBOARD>(key)].hold = true;
			loc->m_keys[static_cast<KEYBOARD>(key)].release = false;
		}
		if (action == GLFW_RELEASE) {
			loc->m_keys[static_cast<KEYBOARD>(key)].press = false;
			loc->m_keys[static_cast<KEYBOARD>(key)].hold = false;
			loc->m_keys[static_cast<KEYBOARD>(key)].release = true;
		}
	}
}
void Input::ButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	Input* loc = sm_inputMap.at(window);
	if (loc != nullptr) {
		if (action == GLFW_PRESS) {
			loc->m_buttons[static_cast<MOUSE>(button)].press = true;
			loc->m_buttons[static_cast<MOUSE>(button)].hold = true;
			loc->m_buttons[static_cast<MOUSE>(button)].release = false;
		}
		if (action == GLFW_RELEASE) {
			loc->m_buttons[static_cast<MOUSE>(button)].press = false;
			loc->m_buttons[static_cast<MOUSE>(button)].hold = false;
			loc->m_buttons[static_cast<MOUSE>(button)].release = true;
		}
	}
}
void Input::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	Input* loc = sm_inputMap.at(window);
	if (loc != nullptr) {
		loc->m_scrollOffsetX = xoffset;
		loc->m_scrollOffsetY = yoffset;
	}
}

std::map<GLFWwindow*, Input*> Input::sm_inputMap;