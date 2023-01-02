#include "utility.cpp"
#include <windows.h>

global_variable bool running = true;

struct  Render_State
{
	int height, width;
	void* memory;

	BITMAPINFO bitmap_info;

};

global_variable Render_State render_state;

#include "platform_command.cpp"
#include "renderer.cpp"
#include "game.cpp"

LRESULT CALLBACK window_callback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	LRESULT result = 0;
	switch (uMsg)
	{
	case WM_CLOSE:
	case WM_DESTROY: {
		running = false;
	} break;

	case WM_SIZE: {
		RECT rect;
		GetClientRect(hwnd, &rect);
		render_state.width = rect.right - rect.left;
		render_state.height = rect.bottom - rect.top;

		int size = render_state.width * render_state.height * sizeof(unsigned int);

		if (render_state.memory) VirtualFree(render_state.memory, 0, MEM_RELEASE);
		render_state.memory = VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

		render_state.bitmap_info.bmiHeader.biSize = sizeof(render_state.bitmap_info.bmiHeader);
		render_state.bitmap_info.bmiHeader.biWidth = render_state.width;
		render_state.bitmap_info.bmiHeader.biHeight = render_state.height;
		render_state.bitmap_info.bmiHeader.biPlanes = 1;
		render_state.bitmap_info.bmiHeader.biBitCount = 32;
		render_state.bitmap_info.bmiHeader.biCompression = BI_RGB;

	} break;

	default:
		result = DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return result;
}


int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {

	ShowCursor(FALSE);

	// Create Window Class
	WNDCLASS window_class = {};
	window_class.style = CS_HREDRAW | CS_VREDRAW;
	window_class.lpszClassName = L"Game Window Class";

	/*
	https://stackoverflow.com/a/2230778 L"String" which assign it to a string literal,
	w = Word (2 byte short integer)
	dw = DWord (4 byte integer)
	p = untyped pointer
	lp = generic long pointer
	lpsz = long pointer to a null-terminated string
	https://www.vbforums.com/showthread.php?353039-What-do-w-dw-lpsz-etc-stand-for
	*/

	window_class.lpfnWndProc = window_callback;

	// Register Class
	RegisterClass(&window_class);

	// Create Window
	HWND window = CreateWindow(window_class.lpszClassName, L"Pong", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, 0, 0, hInstance, 0);
	{
		//Fullscreen
		SetWindowLong(window, GWL_STYLE, GetWindowLong(window, GWL_STYLE) & ~WS_OVERLAPPEDWINDOW);
		MONITORINFO mi = { sizeof(mi) };
		GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY), &mi);
		SetWindowPos(window, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}
	HDC hdc = GetDC(window);

	/*
	The A in "CreateWindowA" stands for ANSI chars which means that the characters are
	gonna use for the window name and class name doesnt have a special characters,
	if we delete it the windows will decide the best call for us (ANSI or WIDE)
	https://stackoverflow.com/questions/55823179/uint-getdrivetypealpcstr-cannot-convert-argument-1-from-lpcwstr-to-lpcstr
	In this case since I wanted to just start working I used the wide-character function option
	### NEEDS TO BE FIXED LATER ###
	*/

	Input input = {};

	float delta_time = 0.016666f; // for fixing the speed of the game when changing the size of the window
	LARGE_INTEGER frame_begin_time;
	QueryPerformanceCounter(&frame_begin_time);

	float performance_frequency;
	{
		LARGE_INTEGER pref;
		QueryPerformanceFrequency(&pref);
		performance_frequency = (float)pref.QuadPart;
	}

	while (running)
	{
		// Input
		MSG message;

		for (int i = 0; i < BUTTON_COUNT; i++)
		{
			input.buttons[i].changed = false;
		}

		while (PeekMessage(&message, window, 0, 0, PM_REMOVE)) {


			switch (message.message) {
			case WM_KEYUP:
			case WM_KEYDOWN: {
				u32 vk_code = (u32)message.wParam;
				bool is_down = ((message.lParam & (1 << 31)) == 0); // 31 The transition state. The value is always 0 for a WM_KEYDOWN message.

#define process_button(b,vk)\
case vk: {\
input.buttons[b].changed = is_down !=input.buttons[b].is_down;\
input.buttons[b].is_down = is_down;\
} break;

				switch (vk_code) {
					process_button(BUTTON_UP, VK_UP);
					process_button(BUTTON_DOWN, VK_DOWN);
					process_button(BUTTON_W, 'W');
					process_button(BUTTON_S, 'S');
					process_button(BUTTON_LEFT, VK_LEFT);
					process_button(BUTTON_RIGHT, VK_RIGHT);
					process_button(BUTTON_ENTER, VK_RETURN);
				}
				/*
				case VK_UP: {
					input.buttons[BUTTON_UP].is_down = is_down; "Either 4 of these cases
					input.buttons[BUTTON_UP].changed = true;	or from the #define to the end of the switch
				} break;										in the video he said something about having a Marco for better readability?"
				https://www.geeksforgeeks.org/macros-and-its-types-in-c-cpp/
				https://www.geeksforgeeks.org/cc-preprocessors/
				*/
			} break;

			default: {
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
			}

		}
		// Simulate
		simulate_game(&input, delta_time);


		// Render
		StretchDIBits(hdc, 0, 0, render_state.width, render_state.height, 0, 0, render_state.width, render_state.height, render_state.memory, &render_state.bitmap_info, DIB_RGB_COLORS, SRCCOPY);

		LARGE_INTEGER frame_end_time;
		QueryPerformanceCounter(&frame_end_time);
		delta_time = (float)(frame_end_time.QuadPart - frame_begin_time.QuadPart) / performance_frequency;
		frame_begin_time = frame_end_time;
	}
};