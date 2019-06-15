#include "include/InputManager.h"

#include "include/glfw-3.2.1.bin.WIN32/include/GLFW/glfw3.h"
#include "include\imgui-1.70\imgui.h"

EKeyState g_AllKeyStates[int(EKeyCodes::NumOfKeyCodes)];

static bool        g_MouseJustPressed[5] = { false, false, false, false, false };
static GLFWcursor* g_MouseCursors[ImGuiMouseCursor_COUNT] = { 0 };

static GLFWwindow* g_window = nullptr;
static GLFWmousebuttonfun g_PrevUserCallbackMousebutton = nullptr;
static GLFWscrollfun      g_PrevUserCallbackScroll = nullptr;
static GLFWkeyfun         g_PrevUserCallbackKey = nullptr;
static GLFWcharfun        g_PrevUserCallbackChar = nullptr;

void ImGui_ImplGlfw_MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (g_PrevUserCallbackMousebutton != NULL)
		g_PrevUserCallbackMousebutton(window, button, action, mods);

	if (action == GLFW_PRESS && button >= 0 && button < IM_ARRAYSIZE(g_MouseJustPressed))
		g_MouseJustPressed[button] = true;
}

void ImGui_ImplGlfw_ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (g_PrevUserCallbackScroll != NULL)
		g_PrevUserCallbackScroll(window, xoffset, yoffset);

	ImGuiIO& io = ImGui::GetIO();
	io.MouseWheelH += (float)xoffset;
	io.MouseWheel += (float)yoffset;
}

void ImGui_ImplGlfw_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (g_PrevUserCallbackKey != NULL)
		g_PrevUserCallbackKey(window, key, scancode, action, mods);

	ImGuiIO& io = ImGui::GetIO();
	if (action == GLFW_PRESS)
		io.KeysDown[key] = true;
	if (action == GLFW_RELEASE)
		io.KeysDown[key] = false;

	// Modifiers are not reliable across systems
	io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
	io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
	io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
	io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];

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

void ImGui_ImplGlfw_CharCallback(GLFWwindow* window, unsigned int c)
{
	if (g_PrevUserCallbackChar != NULL)
		g_PrevUserCallbackChar(window, c);

	ImGuiIO& io = ImGui::GetIO();
	io.AddInputCharacter(c);
}

static void ImGui_ImplGlfw_UpdateMousePosAndButtons()
{
	// Update buttons
	ImGuiIO& io = ImGui::GetIO();
	for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++)
	{
		// If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
		io.MouseDown[i] = g_MouseJustPressed[i] || glfwGetMouseButton(g_window, i) != 0;
		g_MouseJustPressed[i] = false;
	}

	// Update mouse position
	const ImVec2 mouse_pos_backup = io.MousePos;
	io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
#ifdef __EMSCRIPTEN__
	const bool focused = true; // Emscripten
#else
	const bool focused = glfwGetWindowAttrib(g_window, GLFW_FOCUSED) != 0;
#endif
	if (focused)
	{
		if (io.WantSetMousePos)
		{
			glfwSetCursorPos(g_window, (double)mouse_pos_backup.x, (double)mouse_pos_backup.y);
		}
		else
		{
			double mouse_x, mouse_y;
			glfwGetCursorPos(g_window, &mouse_x, &mouse_y);
			io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);
		}
	}
}

static void ImGui_ImplGlfw_UpdateMouseCursor()
{
	ImGuiIO& io = ImGui::GetIO();
	if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) || glfwGetInputMode(g_window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED
		)
		return;

	ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
	if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
	{
		// Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
		glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	}
	else
	{
		// Show OS mouse cursor
		// FIXME-PLATFORM: Unfocused windows seems to fail changing the mouse cursor with GLFW 3.2, but 3.3 works here.
		glfwSetCursor(g_window, g_MouseCursors[imgui_cursor] ?
			g_MouseCursors[imgui_cursor] :
			g_MouseCursors[ImGuiMouseCursor_Arrow]);
		glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

void InputManager::InitialiseInput(GLFWwindow* _window)
{
	for (auto i = 0 ; i < int(EKeyCodes::NumOfKeyCodes) ; ++i)
	{
		g_AllKeyStates[i] = EKeyState::NotPressed;
	}

	g_window = _window;

	g_MouseCursors[ImGuiMouseCursor_Arrow]      = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
	g_MouseCursors[ImGuiMouseCursor_TextInput]  = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
	g_MouseCursors[ImGuiMouseCursor_ResizeAll]  = glfwCreateStandardCursor(GLFW_ARROW_CURSOR); // FIXME: GLFW doesn't have this.
	g_MouseCursors[ImGuiMouseCursor_ResizeNS]   = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
	g_MouseCursors[ImGuiMouseCursor_ResizeEW]   = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
	g_MouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR); // FIXME: GLFW doesn't have this.
	g_MouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR); // FIXME: GLFW doesn't have this.
	g_MouseCursors[ImGuiMouseCursor_Hand]       = glfwCreateStandardCursor(GLFW_HAND_CURSOR);

	g_PrevUserCallbackMousebutton = glfwSetMouseButtonCallback(_window, ImGui_ImplGlfw_MouseButtonCallback);
	g_PrevUserCallbackScroll      = glfwSetScrollCallback(_window, ImGui_ImplGlfw_ScrollCallback);
	g_PrevUserCallbackKey         = glfwSetKeyCallback(_window, ImGui_ImplGlfw_KeyCallback);
	g_PrevUserCallbackChar        = glfwSetCharCallback(_window, ImGui_ImplGlfw_CharCallback);
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

void InputManager::Update()
{
	ImGui_ImplGlfw_UpdateMousePosAndButtons();
	ImGui_ImplGlfw_UpdateMouseCursor();
}
