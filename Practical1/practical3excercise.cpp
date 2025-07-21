
#include <Windows.h>
#include <gl/GL.h>
#include <math.h>

#pragma comment (lib, "OpenGL32.lib")

#define WINDOW_TITLE "OpenGL Window"

float tx1 = 0, ty1 = 0;
float tx2 = 0, ty2 = 0;
float tSpeed = 0.1;

float rAngle = 0;
float rSpeed = 0.05;
int rDirection = 0;

int qNo = 1; 
float x = 0, y = 0; // Circle origin coordinates
float radius = 0; 
float angle = 0;
float x2 = 0, y2 = 0; 
float PI = 3.14159265358979323846f;	
int noOfTri = 30; 

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
		else if (wParam == '1') {
			qNo = 1;
		}
		else if (wParam == '2') {
			qNo = 2;
		}
		else if (wParam == VK_LEFT) {
			tx1 += -tSpeed;
			tx2 += tSpeed;
		}
		else if (wParam == VK_RIGHT) {
			tx1 += tSpeed;
			tx2 += -tSpeed;
		}
		else if (wParam == VK_DOWN) {
			ty1 += -tSpeed;
			ty2 += tSpeed;
		}
		else if (wParam == VK_UP) {
			ty1 += tSpeed;
			ty2 += -tSpeed;
		}
		else if (wParam == VK_SPACE) {
			tx1 = 0;
			tx2 = 0;
			ty1 = 0;
			ty2 = 0;
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

void p3q1() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	//BLOCK 1 : obj 1 : Blue Quad
	glPushMatrix();
		glTranslated(tx1, ty1, 0.0); // tx(-0.5) LEFT
		glBegin(GL_QUADS);
			glColor3f(0.0, 0.0, 1.0);
			glVertex2f(-0.25, -0.25);
			glVertex2f(-0.25, 0.25);
			glVertex2f(0.25, 0.25);
			glVertex2f(0.25, -0.25);
		glEnd();
	glPopMatrix();

	//BLOCK 2 : obj 2 : Green Quad
	glPushMatrix();
		glTranslated(tx2, ty2, 0.0); // tx(0.5) RIGHT
		glBegin(GL_QUADS);
			glColor3f(0.0, 1.0, 0.0);
			glVertex2f(-0.25, -0.25);
			glVertex2f(-0.25, 0.25);
			glVertex2f(0.25, 0.25);
			glVertex2f(0.25, -0.25);
		glEnd();
	glPopMatrix();
}

void fan() {
	glPushMatrix();
	glRotatef((rAngle * rDirection), 0.0, 0.0, 1.0);
		glBegin(GL_QUADS);
			glColor3f(0.0, 1.0, 0.0);
			glVertex2f(-0.25, -0.25);
			glVertex2f(-0.25, 0.25);
			glVertex2f(0.25, 0.25);
			glVertex2f(0.25, -0.25);
		glEnd();
	glPopMatrix();
}

void p3q2() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	x = 0;
	y = 0;
	radius = 0.2;
	glBegin(GL_TRIANGLE_FAN);
	glColor3f(1, 1, 1);
	glVertex2f(x, y); // Center of the circle
	for (angle = 0; angle < 2 * PI; angle += (2 * PI) / noOfTri) {
		x2 = x + radius * cos(angle);
		y2 = y + radius * sin(angle);
		glVertex2f(x2, y2); // Calculate the circle point coordinates
	}
	glEnd();
}

void display()
{
	switch (qNo)
	{
	case 1:
		p3q1();
		break;
	case 2:
		p3q2();
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