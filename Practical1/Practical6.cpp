#include <math.h>
#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>

#pragma comment (lib, "OpenGL32.lib")

#define WINDOW_TITLE "OpenGL Window"

int qNo = 1;
int shape = 1; // 1: sphere, 2: pyramid
int rDir = 1; // 1: clockwise, -1: counter-clockwise
float x = 0.0, y = 0.0, z = 0.0, moveSpeed = 0.1; // light position

bool isLightOn = true;
float ambL[3] = { 1.0, 1.0, 1.0 }; // white ambient light
float posA[3] = { 0.0, -0.8, 0.0 }; // ambient light position
float difL[3] = { 1.0, 0.0, 0.0 }; // red diffuse light
float posB[3] = { x, y, z }; // diffuse light position
float ambM[3] = { 0.0, 0.0, 1.0 }; // blue ambient material
float difM[3] = { 0.0, 0.0, 1.0 }; // blue diffuse material

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
		else if (wParam == 'W') {
			y += moveSpeed;
		}
		else if (wParam == 'S') {
			y -= moveSpeed;
		}
		else if (wParam == 'A') {
			x -= moveSpeed;
		}
		else if (wParam == 'D') {
			x += moveSpeed;
		}
		else if (wParam == 'E') {
			z += moveSpeed;
		}
		else if (wParam == 'Q') {
			z -= moveSpeed;
		}
		else if (wParam == VK_UP) {
			rDir = -1; // counter-clockwise
		}
		else if (wParam == '5') {
			rDir = 0; // pause
		}
		else if (wParam == VK_DOWN) {
			rDir = 1; // clockwise
		}
		else if (wParam == VK_SPACE) {
			isLightOn = !isLightOn;
		}
		else if (wParam == 'O') {
			shape = 1; // sphere
		}
		else if (wParam == 'P') {
			shape = 2; // pyramid
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

void drawSphere(double r) {
	GLUquadricObj* sphere = NULL;
	sphere = gluNewQuadric();
	/*gluQuadricDrawStyle(sphere, GLU_LINE);*/
	gluQuadricDrawStyle(sphere, GLU_FILL);
	gluSphere(sphere, r, 30, 30);
	gluDeleteQuadric(sphere);
}

void drawPyramid(float size) {
	float half = size / 2.0f;

	// Base (facing downward in Y)
	glBegin(GL_QUADS);
	glNormal3f(0.0f, -1.0f, 0.0f);
	glVertex3f(-half, -half, half);
	glVertex3f(half, -half, half);
	glVertex3f(half, -half, -half);
	glVertex3f(-half, -half, -half);
	glEnd();

	// Apex position
	float apexY = half;

	// Side 1 (front)
	glBegin(GL_TRIANGLES);
	glNormal3f(0.0f, 0.5f, 1.0f);
	glVertex3f(0.0f, apexY, 0.0f); 
	glVertex3f(-half, -half, half);
	glVertex3f(half, -half, half);
	glEnd();

	// Side 2 (right)
	glBegin(GL_TRIANGLES);
	glNormal3f(1.0f, 0.5f, 0.0f);
	glVertex3f(0.0f, apexY, 0.0f);
	glVertex3f(half, -half, half);
	glVertex3f(half, -half, -half);
	glEnd();

	// Side 3 (back)
	glBegin(GL_TRIANGLES);
	glNormal3f(0.0f, 0.5f, -1.0f);
	glVertex3f(0.0f, apexY, 0.0f);
	glVertex3f(half, -half, -half);
	glVertex3f(-half, -half, -half);
	glEnd();

	// Side 4 (left)
	glBegin(GL_TRIANGLES);
	glNormal3f(-1.0f, 0.5f, 0.0f);
	glVertex3f(0.0f, apexY, 0.0f);
	glVertex3f(-half, -half, -half);
	glVertex3f(-half, -half, half);
	glEnd();
}



void lighting() {
	posB[0] = x;
	posB[1] = y;
	posB[2] = z;

	if (isLightOn) {
		glEnable(GL_LIGHTING);
	}
	else
	{
		glDisable(GL_LIGHTING);
	}
	
	// light 0 : red ambient light at posA(0, 0.8, 0)
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambL);
	glLightfv(GL_LIGHT0, GL_POSITION, posA);
	glEnable(GL_LIGHT0);

	// light 1 : green diffuse light at posB(0.8, 0, 0)
	glLightfv(GL_LIGHT1, GL_DIFFUSE, difL);
	glLightfv(GL_LIGHT1, GL_POSITION, posB);
	glEnable(GL_LIGHT1);
}

void demo() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	lighting();

	glMatrixMode(GL_MODELVIEW);
	/*glRotatef(0.1, 1.0, 1.0, 1.0);*/
	if (rDir == 1)
	{
		glRotatef(0.1, 1.0, 1.0, 1.0);
	}
	else if (rDir == -1)
	{
		glRotatef(-0.1, 1.0, 1.0, 1.0);
	}
	else
	{
		glRotatef(0.0, 1.0, 1.0, 1.0);
	}

	glColor3f(0, 0, 1);
	glMaterialfv(GL_FRONT, GL_AMBIENT, ambM);
	/*glMaterialfv(GL_FRONT, GL_DIFFUSE, difM);*/
	if (shape == 1)
	{
		drawSphere(0.5);
	}
	else
	{
		drawPyramid(0.5);
	}
	
}

void display()
{
	switch (qNo)
	{
	case 1:
		demo();
		break;
	case 2:

		break;
	case 3:

		break;
	case 4:

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