#include "include/InputManager.h"

EKeyState g_AllKeyStates[int(EKeyCodes::NumOfKeyCodes)];

void KeyEvent(GLFWwindow* _window, int key, int scan_code, int action, int mods)
{
	if (action != 0)
	{
		if (g_AllKeyStates[key] == EKeyState::NotPressed)
		{
			g_AllKeyStates[key] = EKeyState::Pressed;
		}
		else
		{
			g_AllKeyStates[key] = EKeyState::Held;
		}
	}
	else
	{
		g_AllKeyStates[key] = EKeyState::NotPressed;
	}
}

void InputManager::InitialiseInput(GLFWwindow* _window)
{
	for (auto i = 0 ; i < int(EKeyCodes::NumOfKeyCodes) ; ++i)
	{
		g_AllKeyStates[i] = EKeyState::NotPressed;
	}

	// Imgui
	// glfwSetKeyCallback(_window, KeyEvent);
	// glfwSetCursorPosCallback(_window, nullptr);
	// glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

bool InputManager::KeyHeld(EKeyCodes _key_code)
{
	int location = static_cast<int>(_key_code);
	if (g_AllKeyStates[location] == EKeyState::NotPressed)
	{
		return false;
	}
	g_AllKeyStates[location] = EKeyState::Held;
	return true;
}

bool InputManager::KeyHit(EKeyCodes _key_code)
{
	int location = static_cast<int>(_key_code);
	if (g_AllKeyStates[int(location)] == EKeyState::Pressed)
	{
		g_AllKeyStates[int(location)] = EKeyState::Held;
		return true;
	}
	return false;
}

EKeyState InputManager::ReportKeyState(EKeyCodes _key_code)
{
	return g_AllKeyStates[static_cast<int>(_key_code)];
}
