
#include <Windows.h>
#include <gl/GL.h>
#include <math.h>

#pragma comment (lib, "OpenGL32.lib")

#define WINDOW_TITLE "Practical 2"

int qNo = 1;

float tx = 0, ty = 0, tSpeed = 0.1;

float r = 1, g = 1, b = 1, colorSpeed = 0.005;

float rAngle = 0.05;
int rDirection = 0;

float scale = 1;

float PI = 3.14159265358979323846f;
float outerRadius = 0.6;
float innerRadius = 0.2;
int numPoints = 10;

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
		//switch ques
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
		//translate fixed
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
		//translate continuous
		else if (wParam == 'D') {
			tx = tSpeed / 2;
			ty = 0.0;
		}
		else if (wParam == 'A') {
			tx = -tSpeed / 2;
			ty = 0.0;
		}
		else if (wParam == 'W') {
			tx = 0.0;
			ty = tSpeed / 2;
		}
		else if (wParam == 'S') {
			tx = 0.0;
			ty = -tSpeed / 2;
		}
		//rotate
		else if (wParam == 'Q') {
			rDirection = 1;
		}
		else if (wParam == 'E') {
			rDirection = -1;
		}
		//scale
		else if (wParam == 'Z') {
			scale = 0.9998;
		}
		else if (wParam == 'C') {
			scale = 1.0002;
		}
		//stop
		else if (wParam == 'X') {
			tx = 0.0;
			ty = 0.0;
			rDirection = 0;
			scale = 1;
		}
		//color
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
		//reset
		else if (wParam == VK_SPACE) {
			glLoadIdentity();
			tx = 0;
			ty = 0;
			r = 1;
			g = 1;
			b = 1;
			rDirection = 0;
			scale = 1;
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
void updateColor() {
	static float t = 0.0f;
	r = 0.5f + 0.5f * sin(t);
	g = 0.5f + 0.5f * sin(t + 2 * PI / 3);
	b = 0.5f + 0.5f * sin(t + 4 * PI / 3);
	t += colorSpeed;
}

void star() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glLoadIdentity();
	glTranslated(tx, ty, 0.0);

	glColor3f(r, g, b);
	glBegin(GL_TRIANGLE_FAN);

	glVertex2f(0.0f, 0.0f);

	for (int i = 0; i <= numPoints * 2; i++) {
		float angleDeg = 9 + i * 18;
		float angleRad = angleDeg * (PI / 180.0f);
		float radius = (i % 2 == 0) ? outerRadius : innerRadius;
		glVertex2f(radius * cos(angleRad), radius * sin(angleRad));
	}

	glEnd();
}

void colorStar() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	updateColor();
	glRotatef((rAngle * rDirection), 0.0, 0.0, 1.0);
	glTranslated((tx / 100), (ty / 100), 0.0);
	glScalef(scale, scale, scale);

	glColor3f(r, g, b);
	glBegin(GL_TRIANGLE_FAN);

	glVertex2f(0.0f, 0.0f);

	for (int i = 0; i <= numPoints * 2; i++) {
		float angleDeg = 9 + i * 18;
		float angleRad = angleDeg * (PI / 180.0f);
		float radius = (i % 2 == 0) ? outerRadius : innerRadius;
		glVertex2f(radius * cos(angleRad), radius * sin(angleRad));
	}

	glEnd();
}

void point() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	angle += 0.1;

	glLoadIdentity();
	float angleRad = angle * (PI / 180.0f);
	float x = outerRadius * cos(angleRad);
	float y = outerRadius * sin(angleRad);

	glColor3f(1.0, 0.0, 0.0);
	glPointSize(10.0f);
	glBegin(GL_POINTS);
	glVertex2f(x, y);
	glEnd();
}

void box() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glScalef(1.0002, 1.0002, 1.0002);
	glColor3f(1.0, 0.0, 0.0);
	glPointSize(10.0f);
	glBegin(GL_QUADS);
	glVertex2f(0.1, 0.1);
	glVertex2f(-0.1, 0.1);
	glVertex2f(-0.1, -0.1);
	glVertex2f(0.1, -0.1);
	glEnd();
}

void demo() {
	star();
}

void display()
{
	switch (qNo)
	{
	case 1:
		star();
		break;
	case 2:
		colorStar();
		break;
	case 3:
		point();
		break;
	case 4:
		box();
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