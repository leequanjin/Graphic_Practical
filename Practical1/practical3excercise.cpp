
#include <Windows.h>
#include <gl/GL.h>
#include <math.h>

#pragma comment (lib, "OpenGL32.lib")

#define WINDOW_TITLE "OpenGL Window"

float tx1 = 0, ty1 = 0;
float tx2 = 0, ty2 = 0;
float tSpeed = 0.1;

float cloudOffset = 0.0f;
float cloudSpeed = 0.000005f; // Speed of cloud movement
int cloudDirection = 1;

float rAngle = 0;
float rSpeed = 0.05;
int rDirection = 1;

int qNo = 1; 
float x = 0, y = 0; // Circle origin coordinates
float radius = 0; 
float angle = 0;
float x2 = 0, y2 = 0; 
float PI = 3.14159265358979323846f;	
int noOfTri = 30; 

int noOfBlade = 4;

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
			noOfBlade = 3;
		}
		else if (wParam == '4') {
			noOfBlade = 4;
		}
		else if (wParam == '5') {
			noOfBlade = 5;
		}
		else if (wParam == '6') {
			noOfBlade = 6;
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
		else if (wParam == 'R') { // Rotate clockwise
			rDirection = -1;
			cloudDirection = 1;
		}
		else if (wParam == 'L') { // Rotate counter-clockwise
			rDirection = 1;
			cloudDirection = -1;
		}
		else if (wParam == 'Q') { // Cloud LEFT
			cloudDirection = -1;
		}
		else if (wParam == 'E') { // Cloud Right
			cloudDirection = 1;
		}
		else if (wParam == VK_ADD) { // Accelerate rotation
			rSpeed += 0.05 ;
			cloudSpeed += 0.00001f;
		}
		else if (wParam == VK_SUBTRACT) { // Decelerate rotation
			rSpeed -= 0.05;
			cloudSpeed -= 0.00001f;
		}
		else if (wParam == VK_SPACE) { // Stop rotation
			tx1 = 0;
			tx2 = 0;
			ty1 = 0;
			ty2 = 0;
			rSpeed = 0.05;
			rDirection = 0;
			noOfBlade = 4;
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

void drawOvalCloud(float cx, float cy, float radiusX, float radiusY, int segments = 30) {
	cloudOffset += cloudSpeed * cloudDirection;

	if (cloudOffset > 1.5f) cloudOffset = -1.5f;

	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(cx, cy); // Center of the oval
	for (int i = 0; i <= segments; ++i) {
		float angle = 2.0f * PI * float(i) / float(segments);
		float x = radiusX * cosf(angle);
		float y = radiusY * sinf(angle);
		glVertex2f(cx + x, cy + y);
	}
	glEnd();
}

void drawBackground() {
	// Sky background (top part of screen)
	glBegin(GL_QUADS);
	glColor3f(0.53f, 0.81f, 0.92f); // Light blue sky
	glVertex2f(-1.0f, -0.2f);
	glVertex2f(-1.0f, 1.0f);
	glVertex2f(1.0f, 1.0f);
	glVertex2f(1.0f, -0.2f);
	glEnd();

	// Grass (bottom part of screen)
	glBegin(GL_QUADS);
	glColor3f(0.0f, 0.6f, 0.0f); // Green grass
	glVertex2f(-1.0f, -1.0f);
	glVertex2f(-1.0f, -0.2f);
	glVertex2f(1.0f, -0.2f);
	glVertex2f(1.0f, -1.0f);
	glEnd();

	// Cloud 1 (left side)
	glColor3f(1.0f, 1.0f, 1.0f); // White
	drawOvalCloud(-0.6f + cloudOffset, 0.7f, 0.15f, 0.08f);
	drawOvalCloud(-0.45f + cloudOffset, 0.73f, 0.2f, 0.1f);
	drawOvalCloud(-0.3f + cloudOffset, 0.7f, 0.15f, 0.08f);

	// Cloud 2 (right side)
	drawOvalCloud(0.25f + cloudOffset, 0.8f, 0.18f, 0.09f);
	drawOvalCloud(0.4f + cloudOffset, 0.82f, 0.22f, 0.11f);
	drawOvalCloud(0.55f + cloudOffset, 0.8f, 0.18f, 0.09f);
}


void p3q2() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	drawBackground();

	// Draw the windmill stand
	glPushMatrix();
		glBegin(GL_POLYGON);
			glColor3f(0.5f, 0.35f, 0.05f); // Brown
			glVertex2f(-0.3f, -1.0f);
			glVertex2f(-0.2f, 0.2f);
			glVertex2f(-0.15f, 0.25f);
			glVertex2f(-0.1f, 0.4f);
			glVertex2f(0.1f, 0.4f);
			glVertex2f(0.15f, 0.25f);
			glVertex2f(0.2f, 0.2f);
			glVertex2f(0.3f, -1.0f);
		glEnd();

		glBegin(GL_POLYGON);
			glColor3f(0.0f, 0.0f, 0.0f); // Black
			glVertex2f(-0.08f, -1.0f);
			glVertex2f(-0.08f, -0.8f);
			glVertex2f(-0.06f, -0.78f);
			glVertex2f(0.0f, -0.76f);
			glVertex2f(0.06f, -0.78f);
			glVertex2f(0.08f, -0.8f);
			glVertex2f(0.08f, -1.0f);
			
		glEnd();

		glBegin(GL_QUADS);
			glColor3f(0.8f, 0.8f, 0.0f); // Yellow
			glVertex2f(-0.23f, 0.075);
			glVertex2f(-0.23f, 0.1);
			glVertex2f(0.23f, 0.1);
			glVertex2f(0.23f, 0.075);
		glEnd();
	glPopMatrix();

	// Draw the windmill hub (center circle)
	x = 0.0f;
	y = 0.25f;
	radius = 0.05f;
	glBegin(GL_TRIANGLE_FAN);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex2f(x, y); // Center
	for (angle = 0; angle <= 2 * PI; angle += (2 * PI) / noOfTri) {
		x2 = x + radius * cos(angle);
		y2 = y + radius * sin(angle);
		glVertex2f(x2, y2);
	}
	glEnd();

	// Update rotation angle
	rAngle += rSpeed * rDirection; // Clockwise or counterclockwise
	if (rAngle > 360.0f) rAngle -= 360.0f;

	// Draw the blades
	glPushMatrix();
		glTranslatef(x, y, 0.0f); // Move origin to hub center
		glRotatef(rAngle, 0.0f, 0.0f, 1.0f);

		for (int i = 0; i < noOfBlade; i++) {
			glPushMatrix();
				glRotatef(i * (360.0f / noOfBlade), 0.0f, 0.0f, 1.0f);
				glBegin(GL_QUADS);
					glColor3f(0.8f, 0.8f, 0.0f); // Yellow
					glVertex2f(-0.025f, radius);
					glVertex2f(-0.025f, 0.7f);
					glVertex2f(0.025f, 0.7f);
					glVertex2f(0.025f, radius);
				glEnd();

				glBegin(GL_QUADS);
				glColor3f(1.0f, 1.0f, 1.0f); // Yellow
				glVertex2f(0.025f, radius + 0.05f);
				glVertex2f(0.025f, 0.7f);
				glVertex2f(0.2f, 0.7f);
				glVertex2f(0.16f, radius + 0.05f);
				glEnd();
			glPopMatrix();
		}
	glPopMatrix();
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