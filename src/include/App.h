#pragma once

#include "Logger.h"
#include "Vk-Generator/VkGenerator.hpp"
#include "InputManager.h"

class VkApp
{
public:
	void Start();

	void Update(float);

	void Close();

	bool ShouldStop();

	void SetWindowTitle(std::string);

private:

	void UpdateWindowTitle();

	static void WindowCloseCallback(GLFWwindow*);

	bool Input();

	static bool m_force_close;

	InputManager m_input_manager;

	float m_total_time = 0.0f;
	float m_last_delta;

	std::string m_window_title = "";
};
