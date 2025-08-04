
#include <Windows.h>
#include <gl/GL.h>

#pragma comment (lib, "OpenGL32.lib")

#define WINDOW_TITLE "OpenGL Window"

int qNo = 1;
float rx = 0, ry = 0, rz = 0;
bool isRotateLowerArm = false, isRotateWholeArm = false;
float rLowerArm = 0, rWholeArm = 0;

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
		else if (wParam == '3') {
			qNo = 3;
		}
		else if (wParam == '4') {
			qNo = 4;
		}
		else if (wParam == 'X') {
			rx = 0.1;
		}
		else if (wParam == 'Y') {
			ry = 0.1;
		}
		else if (wParam == 'Z') {
			rz = 0.1;
		}
		else if (wParam == VK_SPACE) {
			rx = 0.0;
			ry = 0.0;
			rz = 0.0;
			rLowerArm = 0.0;
			rWholeArm = 0.0;
			isRotateLowerArm = false;
			isRotateWholeArm = false;
			glLoadIdentity();
		}
		else if (wParam == VK_UP) {
			isRotateWholeArm = true;
			break;
		}
		else if (wParam == VK_DOWN) {
			isRotateWholeArm = false;
			break;
		}
		else if (wParam == VK_RIGHT) {
			isRotateLowerArm = true;
			break;
		}
		else if (wParam == VK_LEFT) {
			isRotateLowerArm = false;
			break;
		}
		else if (wParam == '0') {
			rx = 0.0;
			ry = 0.0;
			rz = 0.0;
			rLowerArm = 0.0;
			rWholeArm = 0.0;
			isRotateLowerArm = false;
			isRotateWholeArm = false;
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

void drawCube(float size) {
	glBegin(GL_QUADS);
		// Face 1 : Front
		glColor3f(1.0f, 0.0f, 0.0f);
			glVertex3f(0.0f, 0.0f, size);
			glVertex3f(size, 0.0f, size);
			glVertex3f(size, 0.0f, 0.0f);
			glVertex3f(0.0f, 0.0f, 0.0f);
		// Face 2 : Left
		glColor3f(0.0f, 1.0f, 0.0f);
			glVertex3f(0.0f, 0.0f, 0.0f);
			glVertex3f(0.0f, size, 0.0f);
			glVertex3f(0.0f, size, size);
			glVertex3f(0.0f, 0.0f, size);
		// Face 3 : Top
		glColor3f(0.0f, 0.0f, 1.0f);
			glVertex3f(0.0f, 0.0f, size);
			glVertex3f(0.0f, size, size);
			glVertex3f(size, size, size);
			glVertex3f(size, 0.0f, size);
		// Face 4 : Right
		glColor3f(0.5f, 0.0f, 0.5f);
			glVertex3f(size, 0.0f, size);
			glVertex3f(size, size, size);
			glVertex3f(size, size, 0.0f);
			glVertex3f(size, 0.0f, 0.0f);
		// Face 5 : Bottom
		glColor3f(0.5f, 0.5f, 0.0f);
			glVertex3f(size, 0.0f, 0.0f);
			glVertex3f(0.0f, 0.0f, 0.0f);
			glVertex3f(0.0f, size, 0.0f);
			glVertex3f(size, size, 0.0f);
		// Face 6 : Back
		glColor3f(0.0f, 0.5f, 0.5f);
			glVertex3f(size, size, 0.0f);
			glVertex3f(0.0f, size, 0.0f);
			glVertex3f(0.0f, size, size);
			glVertex3f(size, size, size);
	glEnd();
}

void drawPyramid(float size) {
	glBegin(GL_LINE_LOOP);
	// Face 1 : Base
	glColor3f(1.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, size);
		glVertex3f(size, 0.0f, size);
		glVertex3f(size, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 0.0f);
		// Face 1 
		glVertex3f(size / 2, size, size / 2);
		glVertex3f(0.0f, 0.0f, size);
		// Face 2 
		glVertex3f(size / 2, size, size / 2);
		glVertex3f(size, 0.0f, size);
		// Face 3 
		glVertex3f(size / 2, size, size / 2);
		glVertex3f(size, 0.0f, 0.0f);
		// Face 4 
		glVertex3f(size / 2, size, size / 2);
		glVertex3f(0.0f, 0.0f, 0.0f);
	glEnd();
}

void drawRect(float x, float y, float z) {
	glBegin(GL_LINES);
	glColor3f(1.0f, 1.0f, 1.0f);

	// Bottom rectangle
	glVertex3f(0, 0, 0);
	glVertex3f(x, 0, 0);

	glVertex3f(x, 0, 0);
	glVertex3f(x, 0, z);

	glVertex3f(x, 0, z);
	glVertex3f(0, 0, z);

	glVertex3f(0, 0, z);
	glVertex3f(0, 0, 0);

	// Top rectangle
	glVertex3f(0, y, 0);
	glVertex3f(x, y, 0);

	glVertex3f(x, y, 0);
	glVertex3f(x, y, z);

	glVertex3f(x, y, z);
	glVertex3f(0, y, z);

	glVertex3f(0, y, z);
	glVertex3f(0, y, 0);

	// Vertical lines
	glVertex3f(0, 0, 0);
	glVertex3f(0, y, 0);

	glVertex3f(x, 0, 0);
	glVertex3f(x, y, 0);

	glVertex3f(x, 0, z);
	glVertex3f(x, y, z);

	glVertex3f(0, 0, z);
	glVertex3f(0, y, z);

	glEnd();
}


void demo() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glRotatef(rx, 1.0f, 0.0f, 0.0f);
	glRotatef(ry, 0.0f, 1.0f, 0.0f);
	glRotatef(rz, 0.0f, 0.0f, 1.0f);
	drawCube(0.5f);
	drawCube(-0.5f);
}

void prac4A1() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glRotatef(rx, 1.0f, 0.0f, 0.0f);
	glRotatef(ry, 0.0f, 1.0f, 0.0f);
	glRotatef(rz, 0.0f, 0.0f, 1.0f);
	drawPyramid(0.5f);
}

void prac4A2() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	if (isRotateLowerArm) {
		if (rLowerArm > -120.0f) {
			rLowerArm -= 0.04f;
		}
	}
	else {
		if (rLowerArm < 0.0f) {
			rLowerArm += 0.04f;
		}
	}
	if (isRotateWholeArm) {
		if (rWholeArm > -90.0f) {
			rWholeArm -= 0.04f;
		}
	}
	else {
		if (rWholeArm < 0.0f) {
			rWholeArm += 0.04f;
		}
	}

	glRotatef(rx, 1.0f, 0.0f, 0.0f);
	glRotatef(ry, 0.0f, 1.0f, 0.0f);
	glRotatef(rz, 0.0f, 0.0f, 1.0f);

	glPushMatrix();
	glRotatef(20, 1.0f, 0.0f, 0.0f);
	glRotatef(20, 0.0f, 1.0f, 0.0f);

	glPushMatrix();
		glTranslatef(0.4f, 0.0f, 0.0f);
		glRotatef(rWholeArm, 0.0f, 0.0f, 1.0f); 
		glTranslatef(-0.4f, 0.0f, 0.0f);
		drawRect(0.4f, 0.1f, 0.2f);               

		glPushMatrix();
			glRotatef(rLowerArm, 0.0f, 0.0f, 1.0f); 
			drawRect(-0.4f, 0.1f, 0.2f);   
		glPopMatrix();
	glPopMatrix();

	glPopMatrix();
}


void display()
{
	switch (qNo)
	{
	case 1:
		demo();
		break;
	case 2:
		prac4A1();
		break;
	case 3:
		prac4A2();
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