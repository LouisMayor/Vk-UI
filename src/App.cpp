#include "include/App.h"

extern VkGen::VkGenerator g_VkGenerator;
extern Logger             g_Logger;

bool VkApp::m_force_close = false;

void VkApp::Start()
{
	m_input_manager.InitialiseInput(g_VkGenerator.WindowHdle());
	g_VkGenerator.DisplayWindow(true);
	glfwSetWindowCloseCallback(g_VkGenerator.WindowHdle(), &WindowCloseCallback);

#ifdef _DEBUG
	const int x = glfwGetVideoMode(glfwGetPrimaryMonitor())->width;
	SetWindowPos(GetConsoleWindow(), 0, x - 1024, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	g_Logger.Info("VkApp::Start() Executed");
#endif

	glfwSetInputMode(g_VkGenerator.WindowHdle(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void VkApp::Update(float _delta)
{
	m_last_delta = _delta;
	m_total_time += _delta;

	m_input_manager.Update();

	UpdateWindowTitle();
}

void VkApp::UpdateWindowTitle()
{
	std::string title = m_window_title + " ||";
	title += " Delta " + std::to_string(m_last_delta);
	title += " FPS " + std::to_string(1.0f / m_last_delta);
	title += " Timer " + std::to_string(m_total_time);

	glfwSetWindowTitle(g_VkGenerator.WindowHdle(), title.c_str());
}

void VkApp::WindowCloseCallback(GLFWwindow* _window)
{
	m_force_close = true;
}

bool VkApp::ShouldStop()
{
	if (m_force_close)
	{
		return true;
	}

	return Input() || glfwWindowShouldClose(g_VkGenerator.WindowHdle());
}

void VkApp::SetWindowTitle(std::string _title)
{
	m_window_title = _title;
}

void VkApp::Close()
{
#ifndef NDEBUG
	g_Logger.Info("VkApp::Close() Executed");
#endif

	glfwDestroyWindow(g_VkGenerator.WindowHdle());
	glfwTerminate();
}

bool VkApp::Input()
{
	glfwPollEvents();
	return m_input_manager.KeyHit(EKeyCodes::KeyEsc);
}
