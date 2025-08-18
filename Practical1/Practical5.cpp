#include <math.h>
#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>

#pragma comment (lib, "OpenGL32.lib")

#define WINDOW_TITLE "OpenGL Window"

int qNo = 1;
float rx = 0, ry = 0, rz = 0;
float tx = 0, ty = 0, tz = -30;
float tSpeed = 10;
int r1 = 20; // size of the shape 
float ONear = -5.0, OFar = 60.0;
float PNear = 5.0, PFar = 60.0;
bool isOrtho = false, isFrustum = false, isPerspective = true;
float ptx = 0, pty = 0; //translate x and y for projection
float ptSpeed = 1; //translation speed for projection

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
		else if (wParam == '5') {
			qNo = 5;
		}
		else if (wParam == '6') {
			qNo = 6;
		}
		else if (wParam == 'X') {
			rx += 0.02;
		}
		else if (wParam == 'Y') {
			ry += 0.02;
		}
		else if (wParam == 'Z') {
			rz += 0.02;
		}

		else if (wParam == 'A') {
			ptx -= ptSpeed;
		}
		else if (wParam == 'D') {
			ptx += ptSpeed;
		}

		else if (wParam == 'O') {
			isOrtho = true;
			isFrustum = false;
			isPerspective = false;
			tz = -30;
		}
		else if (wParam == 'F') {
			isOrtho = false;
			isFrustum = true;
			isPerspective = false;
			tz = -30;
		}
		else if (wParam == 'P') {
			isOrtho = false;
			isFrustum = false;
			isPerspective = true;
			tz = -30;
		}
		else if (wParam == VK_UP) { // press "UP" aarrow key
			if (isOrtho)
			{
				if (tz > -OFar + r1)
				{
					tz -= tSpeed;
				}
			}
			else
			{
				if (tz > PNear - PFar + r1)
				{
					tz -= tSpeed;
				}
			}
		}
		else if (wParam == VK_DOWN) { // press "DOWN" arrow key
			if (isOrtho)
			{
				if (tz < -ONear - r1)
				{
					tz += tSpeed;
				}
			}
			else
			{
				if (tz < PNear - r1)
				{
					tz += tSpeed;
				}
			}
		}
		else if (wParam == VK_LEFT) {
			tx -= tSpeed;
		}
		else if (wParam == VK_RIGHT) {
			tx += tSpeed;
		}
		else if (wParam == '0') {
			rx = 0.0;
			ry = 0.0;
			rz = 0.0;
			tz = 0.0;
		}
		else if (wParam == VK_SPACE) {
			rx = 0.0;
			ry = 0.0;
			rz = 0.0;
			tz = 0.0;
			glLoadIdentity();
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
	GLUquadricObj* sphere = NULL;	// Create a new quadric object pointer
	sphere = gluNewQuadric();		// Create a new quadric object in the memory
	gluQuadricDrawStyle(sphere, GLU_LINE); // Set the draw style to line
	gluSphere(sphere, r, 30, 30);	// Draw a sphere with radius r, 30 slices and stacks
	gluDeleteQuadric(sphere);	// Delete the quadric object from the memory	
}

void drawSphereFill(double r) {
	GLUquadricObj* sphere = NULL;	// Create a new quadric object pointer
	sphere = gluNewQuadric();		// Create a new quadric object in the memory
	gluQuadricDrawStyle(sphere, GLU_FILL); // Set the draw style to fill
	gluSphere(sphere, r, 30, 30);	// Draw a sphere with radius r, 30 slices and stacks
	gluDeleteQuadric(sphere);	// Delete the quadric object from the memory	
}

void drawCylinder(double br, double tr, double h) {
	GLUquadricObj* cylinder = NULL;	// Create a new quadric object pointer
	cylinder = gluNewQuadric();		// Create a new quadric object in the memory
	gluQuadricDrawStyle(cylinder, GLU_LINE); // Set the draw style to line
	gluCylinder(cylinder, br, tr, h, 30, 30);	// Draw a cylinder with base radius br, top radius tr, height h, 30 slices and stacks
	gluDeleteQuadric(cylinder);	// Delete the quadric object from the memory	
}

void drawCylinderFill(double br, double tr, double h) {
	GLUquadricObj* cylinder = NULL;	// Create a new quadric object pointer
	cylinder = gluNewQuadric();		// Create a new quadric object in the memory
	gluQuadricDrawStyle(cylinder, GLU_FILL); // Set the draw style to fill
	gluCylinder(cylinder, br, tr, h, 30, 30);	// Draw a cylinder with base radius br, top radius tr, height h, 30 slices and stacks
	gluDeleteQuadric(cylinder);	// Delete the quadric object from the memory	
}

void drawCone(double tr, double h) {
	GLUquadricObj* cone = NULL;	// Create a new quadric object pointer
	cone = gluNewQuadric();		// Create a new quadric object in the memory
	gluQuadricDrawStyle(cone, GLU_LINE); // Set the draw style to line
	gluCylinder(cone, 0, tr, h, 15, 15);	// Draw a cone with top radius tr, height h, 30 slices and stacks
	gluDeleteQuadric(cone);	// Delete the quadric object from the memory	
}

void drawConeFill(double tr, double h) {
	GLUquadricObj* cone = NULL;	// Create a new quadric object pointer
	cone = gluNewQuadric();		// Create a new quadric object in the memory
	gluQuadricDrawStyle(cone, GLU_FILL); // Set the draw style to fill
	gluCylinder(cone, 0, tr, h, 30, 30);	// Draw a cone with top radius tr, height h, 30 slices and stacks
	gluDeleteQuadric(cone);	// Delete the quadric object from the memory	
}

void drawDisk(double inr, double outr) {
	GLUquadricObj* disk = NULL;	// Create a new quadric object pointer
	disk = gluNewQuadric();		// Create a new quadric object in the memory
	gluQuadricDrawStyle(disk, GLU_LINE); // Set the draw style to line
	gluDisk(disk, inr, outr, 30, 30);	// Draw a disk with inner radius inr, outer radius outr, 30 slices and loops
	gluDeleteQuadric(disk);	// Delete the quadric object from the memory	
}

void drawPartialDisk(double inr, double outr, double startAngle, double sweepAngle) {
	GLUquadricObj* disk = NULL;	// Create a new quadric object pointer
	disk = gluNewQuadric();		// Create a new quadric object in the memory
	gluQuadricDrawStyle(disk, GLU_LINE); // Set the draw style to line
	gluPartialDisk(disk, inr, outr, 30, 30, startAngle, sweepAngle); // Draw a partial disk with inner radius inr, outer radius outr, 30 slices and loops, starting angle startAngle and sweep angle sweepAngle
	gluDeleteQuadric(disk);	// Delete the quadric object from the memory	
}

void drawSphereWithoutGLU()
{
	const float PI = 3.141592f;
	GLfloat x, y, z, sliceA, stackA;
	GLfloat radius = 0.5;
	int sliceNo = 30, stackNo = 30;

	for (sliceA = 0.0; sliceA < 2 * PI; sliceA += PI / sliceNo)
	{
		glBegin(GL_LINE_STRIP);
		for (stackA = 0.0; stackA < 2 * PI; stackA += PI / stackNo)
		{
			x = radius * cos(stackA) * sin(sliceA);
			y = radius * sin(stackA) * sin(sliceA);
			z = radius * cos(sliceA);
			glVertex3f(x, y, z);
			x = radius * cos(stackA) * sin(sliceA + PI / stackNo);
			y = radius * sin(stackA) * sin(sliceA + PI / sliceNo);
			z = radius * cos(sliceA + PI / sliceNo);
			glVertex3f(x, y, z);
		}
		glEnd();
	}
}


void demoSphere() {
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glRotatef(rx, 1.0f, 0.0f, 0.0f);
	glRotatef(ry, 0.0f, 1.0f, 0.0f);
	glRotatef(rz, 0.0f, 0.0f, 1.0f);
	glColor3f(1.0f, 0.0f, 0.0f); // Set color to red

	drawSphere(5); // Draw a sphere with radius 0.5
}

void demoCylinder() {
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glRotatef(rx, 1.0f, 0.0f, 0.0f);
	glRotatef(ry, 0.0f, 1.0f, 0.0f);
	glRotatef(rz, 0.0f, 0.0f, 1.0f);
	glColor3f(1.0f, 0.0f, 0.0f); // Set color to red

	drawCylinder(0.2, 0.4, 0.6); // Draw a cylinder with base radius 0.2, top radius 0.4, height 0.6
}

void demoCone() {
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glRotatef(rx, 1.0f, 0.0f, 0.0f);
	glRotatef(ry, 0.0f, 1.0f, 0.0f);
	glRotatef(rz, 0.0f, 0.0f, 1.0f);
	glColor3f(1.0f, 0.0f, 0.0f); // Set color to red

	drawCone(0.4, 0.6); // Draw a cone with top radius 0.4, height 0.6
}

void demoDisk() {
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glRotatef(rx, 1.0f, 0.0f, 0.0f);
	glRotatef(ry, 0.0f, 1.0f, 0.0f);
	glRotatef(rz, 0.0f, 0.0f, 1.0f);
	glColor3f(1.0f, 0.0f, 0.0f); // Set color to red

	drawDisk(0.2, 0.4); // Draw a disk with inner radius 0.2, outer radius 0.4
}

void demoPartialDisk() {
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glRotatef(rx, 1.0f, 0.0f, 0.0f);
	glRotatef(ry, 0.0f, 1.0f, 0.0f);
	glRotatef(rz, 0.0f, 0.0f, 1.0f);
	glColor3f(1.0f, 0.0f, 0.0f); // Set color to red

	drawPartialDisk(0.2, 0.4, 45.0, 270.0); // Draw a partial disk with inner radius 0.2, outer radius 0.4, starting angle 45 degrees and sweep angle 270 degrees
}

void drawIceCream() {
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glRotatef(rx, 1.0f, 0.0f, 0.0f);
	glRotatef(ry, 0.0f, 1.0f, 0.0f);
	glRotatef(rz, 0.0f, 0.0f, 1.0f);

	glTranslatef(tx, ty, 0); // Translate the ice cream to the desired position

	glPushMatrix(); // topping 1 cylinder
		glTranslated(0.5, 6, -0.5);
		glRotatef(270, 1.0f, 0.0f, 0.0f);
		glRotatef(20, 0.0f, 1.0f, 0.0f);
		glColor3f(0.286, 0.012, 0.027);
		drawCylinderFill(0.2, 0.2, 4);
	glPopMatrix();

	glPushMatrix(); // topping 2 gold ring
		glTranslated(-1, 6.5, 0.0);
		glRotatef(0, 1.0f, 0.0f, 0.0f);
		glColor3f(1.0, 0.843, 0.0);
		drawDisk(0.9, 1);
	glPopMatrix();

	glPushMatrix(); // topping 3 cherry on top
		glTranslated(1, 6.5, 0.5);
		glColor3f(0.824, 0.039, 0.180);
		drawSphereFill(0.6);
	glPopMatrix();

	glPushMatrix(); // top ice scream
		glTranslated(0.0, 4, 0.0);
		glColor3f(0.988, 0.353, 0.553);
		drawSphereFill(2.5);
	glPopMatrix();

	glPushMatrix(); // bottom ice scream
		glTranslated(0.0, 0.5, 0.0);
		glColor3f(0.82, 0.41, 0.12);
		drawSphereFill(3);
	glPopMatrix();

	glPushMatrix(); // ice scream cone
		glTranslated(0.0, -8.0, 0.0);
		glRotatef(270, 1.0f, 0.0f, 0.0f);
		glColor3f(0.89, 0.82, 0.75);
		drawConeFill(3.2, 8.0);
	glPopMatrix();

	glPushMatrix(); // ice scream cone
		glTranslated(0.0, -8.0, 0.0);
		glRotatef(270, 1.0f, 0.0f, 0.0f);
		glColor3f(1.0, 1.0, 1.0);
		drawCone(3.25, 8.0);
	glPopMatrix();
}

void demo() {
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glMatrixMode(GL_MODELVIEW);		//refer to the modelview matrix
	
	glLoadIdentity();
	glTranslatef(0.0, 0.0, tz);

	glTranslatef(ptx, pty, 0.0f);
	// demoSphere();
	drawIceCream();
}

void projection() {
	glMatrixMode(GL_PROJECTION); // <-- Add this for Orthographic Projection
	glLoadIdentity(); // Remember to reset the projection matrix

	if (isOrtho) {
		glOrtho(-10.0, 10.0, -10.0, 10.0, ONear, OFar); // default viewing model
	} else if (isFrustum) {
		glFrustum(-10.0, 10.0, -10.0, 10.0, -10.0, 10.0); // perspective viewing model
	} else if (isPerspective) {
		gluPerspective(60.0, 1.0, PNear, PFar); // perspective viewing model
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
	case 5:

		break;
	case 6:
		
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

	projection(); // Set the projection matrix

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