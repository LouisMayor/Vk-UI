#pragma once

#include "Vk-Generator/VkGenerator.hpp"

enum class EKeyState : int
{
	NotPressed = 0,
	Pressed = 1,
	Held = 2
};

enum class EKeyCodes : int
{
	// Mouse

	MouseLB = GLFW_MOUSE_BUTTON_1,
	MouseRB = GLFW_MOUSE_BUTTON_2,
	MouseMB = GLFW_MOUSE_BUTTON_3,

	// Keyboard (line by line of WASD keyboard)

	KeyQ = GLFW_KEY_Q,
	KeyW = GLFW_KEY_W,
	KeyE = GLFW_KEY_E,
	KeyR = GLFW_KEY_R,
	KeyT = GLFW_KEY_T,
	KeyY = GLFW_KEY_Y,
	KeyU = GLFW_KEY_U,
	KeyI = GLFW_KEY_I,
	KeyO = GLFW_KEY_O,
	KeyP = GLFW_KEY_P,
	KeyA = GLFW_KEY_A,
	KeyS = GLFW_KEY_S,
	KeyD = GLFW_KEY_D,
	KeyF = GLFW_KEY_F,
	KeyG = GLFW_KEY_G,
	KeyH = GLFW_KEY_H,
	KeyJ = GLFW_KEY_J,
	KeyK = GLFW_KEY_K,
	KeyL = GLFW_KEY_L,
	KeyZ = GLFW_KEY_Z,
	KeyX = GLFW_KEY_X,
	KeyC = GLFW_KEY_C,
	KeyV = GLFW_KEY_V,
	KeyB = GLFW_KEY_B,
	KeyN = GLFW_KEY_N,
	KeyM = GLFW_KEY_M,

	// Other

	KeySpace = GLFW_KEY_SPACE,
	ArrowUp = GLFW_KEY_UP,
	ArrowDown = GLFW_KEY_DOWN,
	ArrowRight = GLFW_KEY_RIGHT,
	ArrowLeft = GLFW_KEY_LEFT,

	// Numbers

	Key0 = GLFW_KEY_0,
	Key1 = GLFW_KEY_1,
	Key2 = GLFW_KEY_2,
	Key3 = GLFW_KEY_3,
	Key4 = GLFW_KEY_4,
	Key5 = GLFW_KEY_5,
	Key6 = GLFW_KEY_6,
	Key7 = GLFW_KEY_7,
	Key8 = GLFW_KEY_8,
	Key9 = GLFW_KEY_9,

	// Keypad Numbers

	KeyKP0 = GLFW_KEY_KP_0,
	KeyKP1 = GLFW_KEY_KP_1,
	KeyKP2 = GLFW_KEY_KP_2,
	KeyKP3 = GLFW_KEY_KP_3,
	KeyKP4 = GLFW_KEY_KP_4,
	KeyKP5 = GLFW_KEY_KP_5,
	KeyKP6 = GLFW_KEY_KP_6,
	KeyKP7 = GLFW_KEY_KP_7,
	KeyKP8 = GLFW_KEY_KP_8,
	KeyKP9 = GLFW_KEY_KP_9,

	// Operators

	KeyKPDivide = GLFW_KEY_KP_DIVIDE,
	KeyKPMultiply = GLFW_KEY_KP_MULTIPLY,
	KeyKPAdd = GLFW_KEY_KP_ADD,
	KeyKPMinus = GLFW_KEY_KP_SUBTRACT,
	KeyEqual = GLFW_KEY_KP_EQUAL,

	// Modifiers

	ModLCTRL = GLFW_KEY_LEFT_CONTROL,
	ModRCTRL = GLFW_KEY_RIGHT_CONTROL,
	ModLSHIFT = GLFW_KEY_LEFT_SHIFT,
	ModRSHIFT = GLFW_KEY_RIGHT_SHIFT,
	ModLALT = GLFW_KEY_LEFT_ALT,
	ModRALT = GLFW_KEY_RIGHT_ALT,

	// Other keys

	KeyEsc = GLFW_KEY_ESCAPE,

	NumOfKeyCodes = 100
};

class InputManager
{
public:
	void InitialiseInput(GLFWwindow*);

	void Update();

	EKeyState ReportKeyState(EKeyCodes _key_code);

	bool KeyHit(EKeyCodes _keyCode);

	bool KeyHeld(EKeyCodes _key_code);
};
