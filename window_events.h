#pragma once

namespace window {
	enum class Key {
		// IMPORTANT: The numerical values have been arranged to match the GLFW3 key codes,
		// (https://www.glfw.org/docs/3.3/group__keys.html), and must NOT BE CHANGED.

		Space = 32,
		Apostrophe = 39, /* ' */
		Comma = 44, /* , */
		Minus = 45, /* - */
		Period = 46, /* . */
		Slash = 47, /* / */
		Num0 = 48,
		Num1 = 49,
		Num2 = 50,
		Num3 = 51,
		Num4 = 52,
		Num5 = 53,
		Num6 = 54,
		Num7 = 55,
		Num8 = 56,
		Num9 = 57,
		Semicolon = 59, /* ; */
		Equal = 61, /* = */
		A = 65,
		B = 66,
		C = 67,
		D = 68,
		E = 69,
		F = 70,
		G = 71,
		H = 72,
		I = 73,
		J = 74,
		K = 75,
		L = 76,
		M = 77,
		N = 78,
		O = 79,
		P = 80,
		Q = 81,
		R = 82,
		S = 83,
		T = 84,
		U = 85,
		V = 86,
		W = 87,
		X = 88,
		Y = 89,
		Z = 90,
		LBracket = 91, /* [ */
		Backslash = 92, /* \ */
		RBracket = 93, /* ] */
		GraveAccent = 96, /* ` */
		World1 = 161, /* Non-us #1 */
		World2 = 162, /* Non-us #2 */
		Escape = 256,
		Enter = 257,
		Tab = 258,
		Backspace = 259,
		Insert = 260,
		Delete = 261,
		Right = 262,
		Left = 263,
		Down = 264,
		Up = 265,
		PageUp = 266,
		PageDown = 267,
		Home = 268,
		End = 269,
		CapsLock = 280,
		ScrollLock = 281,
		NumLock = 282,
		PrintScreen = 283,
		Pause = 284,
		F1 = 290,
		F2 = 291,
		F3 = 292,
		F4 = 293,
		F5 = 294,
		F6 = 295,
		F7 = 296,
		F8 = 297,
		F9 = 298,
		F10 = 299,
		F11 = 300,
		F12 = 301,
		F13 = 302,
		F14 = 303,
		F15 = 304,
		F16 = 305,
		F17 = 306,
		F18 = 307,
		F19 = 308,
		F20 = 309,
		F21 = 310,
		F22 = 311,
		F23 = 312,
		F24 = 313,
		F25 = 314,
		Numpad0 = 320,
		Numpad1 = 321,
		Numpad2 = 322,
		Numpad3 = 323,
		Numpad4 = 324,
		Numpad5 = 325,
		Numpad6 = 326,
		Numpad7 = 327,
		Numpad8 = 328,
		Numpad9 = 329,
		NumpadDecimal = 330,
		NumpadDivide = 331,
		NumpadMultiply = 332,
		NumpadSubtract = 333,
		NumpadAdd = 334,
		NumpadEnter = 335,
		NumpadEqual = 336,
		LShift = 340,
		LControl = 341,
		LAlt = 342,
		LSuper = 343,
		RShift = 344,
		RControl = 345,
		RAlt = 346,
		RSuper = 347,
		Menu = 348,
	};

	enum class MouseButton {
		Button1,
		Button2,
		Button3,
		Button4,
		Button5,
		Button6,
		Button7,
		Button8,
		Left = Button1,
		Right = Button2,
		Middle = Button3,
	};

	enum MODIFIER_KEY_FLAGS {
		MODIFIER_KEY_SHIFT = (1 << 1), // If this bit is set one or more Shift keys were held down.
		MODIFIER_KEY_CONTROL = (1 << 2), // If this bit is set one or more Control keys were held down.
		MODIFIER_KEY_ALT = (1 << 3), // If this bit is set one or more Alt keys were held down.
		MODIFIER_KEY_SUPER = (1 << 4), // If this bit is set one or more Super keys were held down.
		MODIFIER_KEY_CAPS_LOCK = (1 << 5), // If this bit is set the Caps Lock key is enabled.
		MODIFIER_KEY_NUM_LOCK = (1 << 6), // If this bit is set the Num Lock key is enabled.
	};

	enum class EventType {
		WindowClose, // User attemped to close the window by clicking the close widget or using a key chord like Alt + F4.
		WindowSize, // Window was resized.
		FramebufferSize, // Window framebuffer was resized.
		KeyPress,
		KeyRepeat,
		KeyRelease,
		MouseButtonPress,
		MouseButtonRelease,
		MouseMove,
	};

	struct SizeEvent {
		int width;
		int height;
	};

	struct KeyEvent {
		Key code;
		int scancode;
		int modifier_key_flags;
	};

	struct MouseButtonEvent {
		MouseButton button;
		int modifier_key_flags;
	};

	struct MouseMoveEvent {
		double x;
		double y;
	};

	struct Event {
		EventType type;
		union {
			SizeEvent size;
			KeyEvent key;
			MouseButtonEvent mouse_button;
			MouseMoveEvent mouse_move;
		};
	};

	void push_event(const Event& ev);
	bool pop_event(Event& ev);
}