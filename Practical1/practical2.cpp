
#include <Windows.h>
#include <gl/GL.h>

#pragma comment (lib, "OpenGL32.lib")

#define WINDOW_TITLE "Practical 2"

int qNo = 0; // Question number

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
		else if (wParam == '0') {
			qNo = 0;
		}
		else if (wParam == '1') {
			qNo = 1;
		}
		else if (wParam == '2') {
			qNo = 2;
		}
		else if (wParam == '3') {
			qNo = 3;
		}
		else if (wParam == '4') {
			qNo = 4;
		}
		else if (wParam == '5') {
			qNo = 5;
		}
		else if (wParam == '6') {
			qNo = 6;
		}
		else if (wParam == '7') {
			qNo = 7;
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
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f); // Set background color to black
	glClear(GL_COLOR_BUFFER_BIT); // Clear the color and depth buffers

	glTranslated(0.0, 0.0, 0.0);
	glRotatef(0.0, 0.0, 0.0, 1.0);
	glScalef(1, 1, 1);
	glBegin(GL_TRIANGLES); // Start drawing triangles
	glVertex2f(-0.5, 0.0);
	glVertex2f(0.0, 0.5);
	glVertex2f(0.5, 0.0);
	glEnd(); // End drawing triangles
}

void translateDemo1() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black
	glClear(GL_COLOR_BUFFER_BIT); // Clear the color and depth buffers

	glTranslated(0.0005, 0.0, 0.0); // Move the origin to the right
	glBegin(GL_TRIANGLES); // Start drawing triangles
	glVertex2f(-0.5, 0.0);
	glVertex2f(0.0, 0.5);
	glVertex2f(0.5, 0.0);
	glEnd(); // End drawing triangles
}

void translateDemo2() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black
	glClear(GL_COLOR_BUFFER_BIT); // Clear the color and depth buffers

	glLoadIdentity(); // Load the identity matrix
	glTranslated(0.4, 0.0, 0.0); // Move the origin to the right
	glBegin(GL_TRIANGLES); // Start drawing triangles
	glVertex2f(-0.5, 0.0);
	glVertex2f(0.0, 0.5);
	glVertex2f(0.5, 0.0);
	glEnd(); // End drawing triangles
}

void rotateDemo1() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black
	glClear(GL_COLOR_BUFFER_BIT); // Clear the color and depth buffers

	glRotatef(0.05, 0.0, 0.0, 1.0);
	glBegin(GL_TRIANGLES); // Start drawing triangles
	glVertex2f(-0.5, 0.0);
	glVertex2f(0.0, 0.5);
	glVertex2f(0.5, 0.0);
	glEnd();
}

void rotateDemo2() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black
	glClear(GL_COLOR_BUFFER_BIT); // Clear the color and depth buffers

	glLoadIdentity(); // Load the identity matrix
	glRotatef(90, 0.0, 0.0, 1.0);
	glBegin(GL_TRIANGLES); // Start drawing triangles
	glVertex2f(-0.5, 0.0);
	glVertex2f(0.0, 0.5);
	glVertex2f(0.5, 0.0);
	glEnd();
}

void scaleDemo1() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black
	glClear(GL_COLOR_BUFFER_BIT); // Clear the color and depth buffers

	glScalef(0.9998, 0.9998, 0.9998);
	glBegin(GL_TRIANGLES); // Start drawing triangles
	glVertex2f(-0.5, 0.0);
	glVertex2f(0.0, 0.5);
	glVertex2f(0.5, 0.0);
	glEnd();
}

void scaleDemo2() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black
	glClear(GL_COLOR_BUFFER_BIT); // Clear the color and depth buffers

	glScalef(1.0002, 1.0002, 1.0002);
	glBegin(GL_TRIANGLES); // Start drawing triangles
	glVertex2f(-0.5, 0.0);
	glVertex2f(0.0, 0.5);
	glVertex2f(0.5, 0.0);
	glEnd();
}

void scaleDemo3() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black
	glClear(GL_COLOR_BUFFER_BIT); // Clear the color and depth buffers

	glLoadIdentity(); // Load the identity matrix
	glScalef(0.5, 0.5, 0.5);
	glBegin(GL_TRIANGLES); // Start drawing triangles
	glVertex2f(-0.5, 0.0);
	glVertex2f(0.0, 0.5);
	glVertex2f(0.5, 0.0);
	glEnd();
}

void display()
{
	switch (qNo)
	{
	case 0:
		demo();
		break;
	case 1:
		translateDemo1();
		break;
	case 2:
		rotateDemo1();
		break;
	case 3:
		scaleDemo1();
		break;
	case 4:
		scaleDemo2();
		break;
	case 5:
		translateDemo2();
		break;
	case 6:
		rotateDemo2();
		break;
	case 7:
		scaleDemo3();
		break;
	default:
		break;
	}
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