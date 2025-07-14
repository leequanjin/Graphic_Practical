
#include <Windows.h>
#include <gl/GL.h>
#include <math.h>

#pragma comment (lib, "OpenGL32.lib")

#define WINDOW_TITLE "Practical 2"

int qNo = 0;
float tx = 0, ty = 0, tSpeed = 0.1;
float r = 1, g = 1, b = 1;

float PI = 3.14159265358979323846f;
float radius = 0.3;
float angle = 0;

LRESULT WINAPI WindowProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) {
			PostQuitMessage(0);
		}
		else if (wParam == VK_RIGHT) {
			tx += tSpeed;
		}
		else if (wParam == VK_LEFT) {
			tx -= tSpeed;
		}
		else if (wParam == VK_UP) {
			ty += tSpeed;
		}
		else if (wParam == VK_DOWN) {
			ty -= tSpeed;
		}
		else if (wParam == 'R') {
			r = 1;
			g = 0;
			b = 0;
		}
		else if (wParam == 'G') {
			r = 0;
			g = 1;
			b = 0;
		}
		else if (wParam == 'B') {
			r = 0;
			g = 0;
			b = 1;
		}
		else if (wParam == VK_SPACE) {
			tx = 0;
			ty = 0;
			r = 1;
			g = 1;
			b = 1;
		}
		break;

	default:
		break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}
//--------------------------------------------------------------------

bool initPixelFormat(HDC hdc)
{
	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

	pfd.cAlphaBits = 8;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 0;

	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;

	pfd.iLayerType = PFD_MAIN_PLANE;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;

	// choose pixel format returns the number most similar pixel format available
	int n = ChoosePixelFormat(hdc, &pfd);

	// set pixel format returns whether it sucessfully set the pixel format
	if (SetPixelFormat(hdc, n, &pfd))
	{
		return true;
	}
	else
	{
		return false;
	}
}
//--------------------------------------------------------------------
void demo() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 
	glClear(GL_COLOR_BUFFER_BIT); 

	float outerRadius = 0.6f;
	float innerRadius = 0.2f;
	int numPoints = 5;
	
	r -= 0.0;
	g -= 0.0;
	b -= 0.0;

	glLoadIdentity();
	glTranslated(tx, ty, 0.0);
	
	glBegin(GL_LINE_LOOP);
	glColor3f(1, 0, 0);
	for (int i = 0; i <= numPoints * 2; i++) {
		float angleDeg = 18 + i * 36;
		float angleRad = angleDeg * (PI / 180.0f);
		float radius = (i % 2 == 0) ? outerRadius : innerRadius;
		glVertex2f(radius * cos(angleRad), radius * sin(angleRad));
	}
	glEnd();

	glColor3f(r, g, b);
	glBegin(GL_TRIANGLE_FAN);

	glVertex2f(0.0f, 0.0f);

	for (int i = 0; i <= numPoints * 2; i++) {
		float angleDeg = 18 + i * 36;
		float angleRad = angleDeg * (PI / 180.0f);
		float radius = (i % 2 == 0) ? outerRadius : innerRadius;
		glVertex2f(radius * cos(angleRad), radius * sin(angleRad));
	}

	glEnd();
}

void display()
{
	demo();
}
//--------------------------------------------------------------------

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)
{
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpfnWndProc = WindowProcedure;
	wc.lpszClassName = WINDOW_TITLE;
	wc.style = CS_HREDRAW | CS_VREDRAW;

	if (!RegisterClassEx(&wc)) return false;

	HWND hWnd = CreateWindow(WINDOW_TITLE, WINDOW_TITLE, WS_OVERLAPPEDWINDOW,
		950, 10, 300, 300,
		NULL, NULL, wc.hInstance, NULL);

	//--------------------------------
	//	Initialize window for OpenGL
	//--------------------------------

	HDC hdc = GetDC(hWnd);

	//	initialize pixel format for the window
	initPixelFormat(hdc);

	//	get an openGL context
	HGLRC hglrc = wglCreateContext(hdc);

	//	make context current
	if (!wglMakeCurrent(hdc, hglrc)) return false;

	//--------------------------------
	//	End initialization
	//--------------------------------

	ShowWindow(hWnd, nCmdShow);

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		display();

		SwapBuffers(hdc);
	}

	UnregisterClass(WINDOW_TITLE, wc.hInstance);

	return true;
}
//--------------------------------------------------------------------