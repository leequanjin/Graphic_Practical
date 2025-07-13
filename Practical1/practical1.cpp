
#include <Windows.h>
#include <gl/GL.h>
#include <math.h>

#pragma comment (lib, "OpenGL32.lib")

#define WINDOW_TITLE "Practical 1"

int qNo = 0; // Question number
float x = 0, y = 0; // Circle origin coordinates
float radius = 0.3; // Circle radius
float angle = 0;
float x2 = 0, y2 = 0; // Circle point coordinates
float PI = 3.14159265358979323846f;	// Pi constant
int noOfTri = 30; // Number of triangles to draw the circle

float startAngle = 0;
float endAngle = 0;


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
		} else if (wParam == '0') {
			qNo = 0;
		} else if (wParam == '1') {
			qNo = 1;
		} else if (wParam == '2') {
			qNo = 2;
		} else if (wParam == '3') {
			qNo = 3;
		} else if (wParam == '4') {
			qNo = 4;
		} else if (wParam == '5') {
			qNo = 5;
		} else if (wParam == '6') {
			qNo = 6;
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
	glClearColor(0.0, 0.0, 0.0, 0.0); 
	glClear(GL_COLOR_BUFFER_BIT); // Clear the color buffer

	glLineWidth(5.0f); // Set the line width
	glPointSize(10.0f); // Set the point size

	glBegin(GL_POLYGON); // Start drawing a line loop
		glVertex2f(-0.5, 0.0); // P1 vertex
		glColor3f(1.0, 0.0, 0.0); // Red vertex
		glVertex2f(0.0, 0.5); // P2 vertex
		glColor3f(0.0, 1.0, 0.0); // Green vertex
		glVertex2f(0.5, 0.0); // P3 vertex
		glColor3f(0.0, 0.0, 1.0); // Blue vertex
	glEnd();
}

void pahangFlag() {
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glLineWidth(2.0);
	glBegin(GL_LINE_LOOP);
	glColor3f(0, 0, 1.0);
	glVertex2f(-0.8, -0.5);
	glVertex2f(-0.8, 0.5);
	glVertex2f(0.8, 0.5);
	glVertex2f(0.8, -0.5);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(1.0, 1.0, 1.0);
	glVertex2f(-0.8, 0.0);
	glVertex2f(-0.8, 0.5);
	glVertex2f(0.8, 0.5);
	glVertex2f(0.8, 0.0);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(0.0, 0.0, 0.0);
	glVertex2f(-0.8, -0.5);
	glVertex2f(-0.8, 0.0);
	glVertex2f(0.8, 0.0);
	glVertex2f(0.8, -0.5);
	glEnd();
}

void negeriSembilanFlag() {
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glBegin(GL_QUADS);
	glColor3f(1.0, 1.0, 0.0); // Yellow
	glVertex2f(-0.8, -0.5);
	glVertex2f(-0.8, 0.5);
	glVertex2f(0.8, 0.5);
	glVertex2f(0.8, -0.5);
	glEnd();

	glBegin(GL_TRIANGLES);
	glColor3f(0.0, 0.0, 0.0); // Black
	glVertex2f(-0.8, 0.5);
	glVertex2f(-0.8, 0.0);
	glVertex2f(0.0, 0.0);
	glEnd();

	glEnd();
	glBegin(GL_TRIANGLES);
	glColor3f(1.0, 0.0, 0.0); // Red
	glVertex2f(-0.8, 0.5);
	glVertex2f(0.0, 0.5);
	glVertex2f(0.0, 0.0);
	glEnd();
}

void englandFlag() {
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glBegin(GL_QUADS);
	glColor3f(1.0, 1.0, 1.0);
	glVertex2f(-0.8, -0.5);
	glVertex2f(-0.8, 0.5);
	glVertex2f(0.8, 0.5);
	glVertex2f(0.8, -0.5);
	glEnd();

	glLineWidth(20.0f);
	glBegin(GL_LINES);
	glColor3f(1.0, 0.0, 0.0);
	glVertex2f(-0.8, 0.0);
	glVertex2f(0.8, 0.0);
	glEnd();

	glLineWidth(20.0f);
	glBegin(GL_LINES);
	glColor3f(1.0, 0.0, 0.0);
	glVertex2f(0.0, 0.5);
	glVertex2f(0.0, -0.5);
	glEnd();
}

void scotlandFlag() {
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glLineWidth(2.0);
	glBegin(GL_LINE_LOOP);
	glColor3f(0, 0, 1.0);
	glVertex2f(-0.8, -0.5);
	glVertex2f(-0.8, 0.5);
	glVertex2f(0.8, 0.5);
	glVertex2f(0.8, -0.5);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(1.0, 1.0, 1.0);
	glVertex2f(-0.8, -0.5);
	glVertex2f(-0.8, 0.5);
	glVertex2f(0.8, 0.5);
	glVertex2f(0.8, -0.5);
	glEnd();

	glBegin(GL_TRIANGLES);
	glColor3f(0, 0, 1.0);
	glVertex2f(-0.7, 0.5);
	glVertex2f(0.7, 0.5);
	glVertex2f(0.0, 0.1);
	glEnd();

	glBegin(GL_TRIANGLES);
	glColor3f(0, 0, 1.0);
	glVertex2f(0.8, 0.4);
	glVertex2f(0.8, -0.4);
	glVertex2f(0.1, 0.0);
	glEnd();

	glBegin(GL_TRIANGLES);
	glColor3f(0, 0, 1.0);
	glVertex2f(-0.7, -0.5);
	glVertex2f(0.7, -0.5);
	glVertex2f(0.0, -0.1);
	glEnd();

	glBegin(GL_TRIANGLES);
	glColor3f(0, 0, 1.0);
	glVertex2f(-0.8, 0.4);
	glVertex2f(-0.8, -0.4);
	glVertex2f(-0.1, 0.0);
	glEnd();
}

void japanFlag() {
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glLineWidth(2.0);
	glBegin(GL_LINE_LOOP);
	glColor3f(0, 0, 0);
	glVertex2f(-0.8, -0.5);
	glVertex2f(-0.8, 0.5);
	glVertex2f(0.8, 0.5);
	glVertex2f(0.8, -0.5);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(1.0, 1.0, 1.0);
	glVertex2f(-0.8, -0.5);
	glVertex2f(-0.8, 0.5);
	glVertex2f(0.8, 0.5);
	glVertex2f(0.8, -0.5);
	glEnd();

	glBegin(GL_TRIANGLE_FAN);
	glColor3f(1, 0, 0);
	glVertex2f(x, y); // Center of the circle
	for (angle = 0;  angle < 2 * PI; angle += (2 * PI) / noOfTri) {
		x2 = x + radius * cos(angle);
		y2 = y + radius * sin(angle);
		glVertex2f(x2, y2); // Calculate the circle point coordinates
	}
	glEnd();
}

void challenge() {
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glBegin(GL_QUADS); // Right Hair
	glColor3f(0.0, 0.0, 0.0);
	glVertex2f(0.68, -0.1);
	glVertex2f(0.68, 0.7);
	glVertex2f(0.55, 0.7);
	glVertex2f(0.55, -0.1);
	glEnd();
	
	glBegin(GL_TRIANGLE_FAN); // Head
	glColor3f(1, 1, 0);
	glVertex2f(x, y);
	for (angle = 0; angle < 2 * PI; angle += (2 * PI) / noOfTri) {
		x2 = x + 0.7 * cos(angle);
		y2 = y + 0.7 * sin(angle);
		glVertex2f(x2, y2);
	}
	glEnd();

	glBegin(GL_LINE_STRIP);
	glColor3f(0, 0, 0);

	startAngle = PI / 6.0;
	endAngle = PI - (PI / 6.0);

	for (angle = startAngle; angle <= endAngle; angle += (endAngle - startAngle) / noOfTri) {
		x2 = 0.4 + 0.2 * cos(angle);
		y2 = 0.1 + 0.2 * sin(angle);
		glVertex2f(x2, y2);
	}

	glEnd();
	
	glBegin(GL_TRIANGLE_FAN); //Right Lens
	glColor3f(0, 0, 0);
	glVertex2f(0.7, 0.1);
	for (angle = PI; angle <= 2 * PI; angle += PI / noOfTri) {
		x2 = 0.4 + 0.3 * cos(angle);
		y2 = 0.1 + 0.3 * sin(angle);
		glVertex2f(x2, y2);
	}
	glEnd();

	glBegin(GL_LINE_STRIP);
	glColor3f(0, 0, 0);
	glVertex2f(-0.4, 0.2);
	glVertex2f(0.0, 0.2);
	glEnd();

	glBegin(GL_QUADS);// Right Lens Shine
	glColor3f(1.0, 1.0, 1.0);

	glVertex2f(-0.14 + 0.6, 0.1);
	glVertex2f(-0.12 + 0.6, 0.1);
	glVertex2f(-0.18 + 0.6, -0.1);
	glVertex2f(-0.20 + 0.6, -0.1);

	glEnd();

	glBegin(GL_QUADS);// Right Lens Shine
	glColor3f(1.0, 1.0, 1.0);

	glVertex2f(-0.20 + 0.6, 0.1);
	glVertex2f(-0.16 + 0.6, 0.1);
	glVertex2f(-0.22 + 0.6, -0.1);
	glVertex2f(-0.26 + 0.6, -0.1);

	glEnd();

	glBegin(GL_TRIANGLE_FAN); //Left Lens
	glColor3f(0, 0, 0);
	glVertex2f(-0.2, 0.1);
	for (angle = PI; angle <= 2 * PI; angle += PI / noOfTri) {
		x2 = -0.2 + 0.3 * cos(angle);
		y2 = 0.1 + 0.3 * sin(angle);
		glVertex2f(x2, y2);
	}
	glEnd();

	glBegin(GL_QUADS);// Left Lens Shine
	glColor3f(1.0, 1.0, 1.0);

	glVertex2f(-0.14, 0.1);
	glVertex2f(-0.12, 0.1);
	glVertex2f(-0.18, -0.1);
	glVertex2f(-0.20, -0.1);

	glEnd();

	glBegin(GL_QUADS);// Left Lens Shine
	glColor3f(1.0, 1.0, 1.0);

	glVertex2f(-0.20, 0.1);
	glVertex2f(-0.16, 0.1);
	glVertex2f(-0.22, -0.1);
	glVertex2f(-0.26, -0.1);

	glEnd();

	glBegin(GL_QUADS); // Glasses Bridge
	glColor3f(0.0, 0.0, 0.0);
	glVertex2f(-0.7, 0.1);
	glVertex2f(-0.7, 0.05);
	glVertex2f(0.7, 0.05);
	glVertex2f(0.7, 0.1);
	glEnd();

	glBegin(GL_QUADS); // Left Hair
	glColor3f(0.0, 0.0, 0.0);
	glVertex2f(-0.8, -0.1);
	glVertex2f(-0.8, 0.7);
	glVertex2f(-0.55, 0.7);
	glVertex2f(-0.55, -0.1);
	glEnd();

	glBegin(GL_QUADS); // Middle Hair
	glColor3f(0.0, 0.0, 0.0);
	glVertex2f(-0.6, 0.4);
	glVertex2f(-0.6, 0.8);
	glVertex2f(0.8, 0.8);
	glVertex2f(0.8, 0.4);
	glEnd();

	glBegin(GL_LINE_STRIP); // Mouth
	glColor3f(0, 0, 0);

	startAngle = PI + (PI / 6.0);
	endAngle = PI + (PI * 5.0 / 6.0);

	for (angle = startAngle; angle <= endAngle; angle += (endAngle - startAngle) / noOfTri) {
		x2 = 0.1 + 0.3 * cos(angle);  // center at (0.1, -0.2)
		y2 = -0.2 + 0.3 * sin(angle);
		glVertex2f(x2, y2);
	}

	glEnd();

	glBegin(GL_TRIANGLES);
	glColor3f(0.6, 0.3, 0.0);
	glVertex2f(0.1, 0.0);
	glVertex2f(0.1, -0.3);
	glVertex2f(0.25, -0.3);
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
			pahangFlag();
			break;
		case 2:
			negeriSembilanFlag();
			break;
		case 3:
			englandFlag();
			break;
		case 4:
			scotlandFlag();
			break;
		case 5:
			japanFlag();
			break;
		case 6:
			challenge();
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