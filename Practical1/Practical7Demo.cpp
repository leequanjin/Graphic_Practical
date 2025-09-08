#include <math.h>
#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>

#define WINDOW_TITLE "Practical 7"

int qNo = 1;

//Step 1: Variable Declaration
BITMAP BMP;				//bitmap structure
HBITMAP hBMP = NULL;	//bitmap handle

//Step 2: Save image into same folder as source file and add to existing file

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
	//glColor3f(1.0f, 0.0f, 0.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(0.0f, 0.0f, size);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(size, 0.0f, size);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(size, 0.0f, 0.0f);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(0.0f, 0.0f, 0.0f);
	// Face 2 : Left
	//glColor3f(0.0f, 1.0f, 0.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(0.0f, size, 0.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(0.0f, size, size);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(0.0f, 0.0f, size);
	// Face 3 : Top
	//glColor3f(0.0f, 0.0f, 1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(0.0f, 0.0f, size);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(0.0f, size, size);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(size, size, size);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(size, 0.0f, size);
	// Face 4 : Right
	//glColor3f(0.5f, 0.0f, 0.5f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(size, 0.0f, size);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(size, size, size);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(size, size, 0.0f);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(size, 0.0f, 0.0f);
	// Face 5 : Bottom
	//glColor3f(0.5f, 0.5f, 0.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(size, 0.0f, 0.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(0.0f, size, 0.0f);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(size, size, 0.0f);
	// Face 6 : Back
	//glColor3f(0.0f, 0.5f, 0.5f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(size, size, 0.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(0.0f, size, 0.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(0.0f, size, size);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(size, size, size);
	glEnd();
}
void drawPyramid(float size) {
	glBegin(GL_LINE_LOOP);
	// Face 1 : Base
	//glColor3f(1.0f, 0.0f, 0.0f);
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

void drawSphereFill(double r) {
	GLUquadricObj* sphere = NULL;	// Create a new quadric object pointer
	sphere = gluNewQuadric();		// Create a new quadric object in the memory
	gluQuadricDrawStyle(sphere, GLU_FILL); // Set the draw style to fill
	gluQuadricTexture(sphere, true);
	gluSphere(sphere, r, 30, 30);	// Draw a sphere with radius r, 30 slices and stacks
	gluDeleteQuadric(sphere);	// Delete the quadric object from the memory	
}

void drawCylinderFill(double br, double tr, double h) {
	GLUquadricObj* cylinder = NULL;	// Create a new quadric object pointer
	cylinder = gluNewQuadric();		// Create a new quadric object in the memory
	gluQuadricDrawStyle(cylinder, GLU_FILL); // Set the draw style to fill
	gluQuadricTexture(cylinder,true);
	gluCylinder(cylinder, br, tr, h, 30, 30);	// Draw a cylinder with base radius br, top radius tr, height h, 30 slices and stacks
	gluDeleteQuadric(cylinder);	// Delete the quadric object from the memory	
}

GLuint loadTexture(LPCSTR filename) {
	//Take from step 1
	GLuint texture = 0;		//texture name

	//Step 3: Initialise texture info
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	HBITMAP hBMP = (HBITMAP)LoadImage(GetModuleHandle(NULL),
		filename, IMAGE_BITMAP, 0, 0,
		LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	GetObject(hBMP, sizeof(BMP), &BMP);

	//Step 4: Assign texture to polygon
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, BMP.bmWidth,
		BMP.bmHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, BMP.bmBits);

	//Taken from step 5
	DeleteObject(hBMP);
	return texture;
}

void demo() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glRotatef(0.05, 0.0, 1.0, 1.0);

	GLuint textureArr[2];
	textureArr[0] = loadTexture("Box.bmp");
	drawSphereFill(0.3);
	glDeleteTextures(1, &textureArr[0]);

	textureArr[1] = loadTexture("ice.bmp");
	drawCylinderFill(0.2, 0.2, 0.8);
	glDeleteTextures(1, &textureArr[1]);

	//Step 5: Remove texture info
	glDisable(GL_TEXTURE_2D);

	glDeleteTextures(1, &textureArr[1]);
}

void display()
{
	switch (qNo)
	{
	case 1:
		demo();
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
		950, 10, 800, 800,
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