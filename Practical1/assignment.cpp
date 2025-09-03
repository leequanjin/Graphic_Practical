#define _USE_MATH_DEFINES
#include <math.h>
#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>

#pragma comment (lib, "OpenGL32.lib")

#define WINDOW_TITLE "OpenGL Window"

int qNo = 1;

int noOfSides = 30; // for circle and cylinder

// light parameters
float lightX = 0.0, lightY = 0.0, lightZ = 0.0, moveSpeed = 0.1; // light position
bool isLightOn = true;
float ambL[3] = { 1.0, 1.0, 1.0 }; // white ambient light
float posA[3] = { 0.0, -0.8, 0.0 }; // ambient light position
float difL[3] = { 1.0, 0.0, 0.0 }; // red diffuse light
float posB[3] = { lightX, lightY, lightZ }; // diffuse light position
float ambM[3] = { 0.0, 0.0, 1.0 }; // blue ambient material
float difM[3] = { 0.0, 0.0, 1.0 }; // blue diffuse material

// projection parameters
float PNear = 1.0, PFar = 100.0;
bool isOrtho = false, isPerspective = true;

float camX = 0.f, camY = 0.f, camZ = 25.f; // camera position
float camRotX = 10.f, camRotY = -20.f, camRotZ = 0.f; // camera rotation
float camMoveSpeed = 0.5f;
float camRotSpeed = 2.0f;

// human position
float hx = 0.f, hy = 0.f, hz = 0.f;
float walkAnimSpeed = 0.5f; // speed of swing
float humanSpeed = 0.02f; // speed of movement

// walk cycle
float walkPhase = 0.f;       // angle in degrees
bool isWalking = false;      // walking state
float walkDirX = 0.f, walkDirZ = 0.f; // direction
float humanAngle = 0.f; // facing direction in degrees


struct HumanDims {
	float unit = 1.f; // base unit

	float armLimbR = unit * 0.24f;
	float armJointR = unit * 0.24f;
	float legLimbR = unit * 0.3f;
	float legJointR = unit * 0.3f;

	// overall proportions (units)
	// head + neck = 1 (1/8 of total)
	float headR = unit * 0.7f;
	float neckH = unit * 0.3f;
	float neckR = headR * 0.3f;

	// chest + pelvis = 3 (3/8 of total)
	float chestH = unit * 1.5f;
	float shoulderW = chestH * 1.f;
	float chestD = chestH * 0.6f;

	float pelvisH = unit * 1.5f;
	float pelvisW = pelvisH * 1.f;
	float pelvisD = pelvisH * 0.6f;

	// arms + hand = 4 (4/8 of total)
	float upperArmL = unit * 1.5f;
	float forearmL = unit * 1.5f;
	
	float handH = unit * 1.f;
	float handD = handH * 0.8;
	float handW = handH * 0.2f;

	// legs + feet = 4 (4/8 of total)
	float upperLegL = unit * 2.0f;
	float lowerLegL = unit * 1.6f;

	float footH = unit * 0.2f;
	float footD = footH * 6.f;
	float footW = footH * 2.f;
};

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
		else if (wParam == '1') qNo = 1;
		else if (wParam == 'O') {
			isOrtho = true;
			isPerspective = false;
		}
		else if (wParam == 'P') {
			isOrtho = false;
			isPerspective = true;
		}
		// Camera movement
		else if (wParam == VK_UP)    camY += camMoveSpeed;
		else if (wParam == VK_DOWN)  camY -= camMoveSpeed;
		else if (wParam == VK_LEFT)  camX -= camMoveSpeed;
		else if (wParam == VK_RIGHT) camX += camMoveSpeed;
		else if (wParam == VK_PRIOR) camZ -= camMoveSpeed; // PageUp
		else if (wParam == VK_NEXT)  camZ += camMoveSpeed; // PageDown

		// Camera rotation
		else if (wParam == 'W') camRotX += camRotSpeed;
		else if (wParam == 'S') camRotX -= camRotSpeed;
		else if (wParam == 'A') camRotY += camRotSpeed;
		else if (wParam == 'D') camRotY -= camRotSpeed;
		else if (wParam == 'Q') camRotZ += camRotSpeed;
		else if (wParam == 'E') camRotZ -= camRotSpeed;

		// Light position
		else if (wParam == 'T') lightY += moveSpeed; // move light up
		else if (wParam == 'G') lightY -= moveSpeed; // move light down
		else if (wParam == 'F') lightX -= moveSpeed; // move light left
		else if (wParam == 'H') lightX += moveSpeed; // move light right
		else if (wParam == 'R') lightZ -= moveSpeed; // move light forward
		else if (wParam == 'Y') lightZ += moveSpeed; // move light backward

		else if (wParam == 'I') { // forward
			walkDirX = 0.f; walkDirZ = -1.f;
			isWalking = true;
		}
		else if (wParam == 'K') { // backward
			walkDirX = 0.f; walkDirZ = 1.f;
			isWalking = true;
		}
		else if (wParam == 'J') { // left
			walkDirX = -1.f; walkDirZ = 0.f;
			isWalking = true;
		}
		else if (wParam == 'L') { // right
			walkDirX = 1.f; walkDirZ = 0.f;
			isWalking = true;
		}
		else if (wParam == VK_SPACE) { // stop walking
			isWalking = false;
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

void setWire(bool on) {
	glPolygonMode(GL_FRONT_AND_BACK, on ? GL_LINE : GL_FILL);
}

void drawSphere(double r) {
	GLUquadricObj* sphere = NULL;
	sphere = gluNewQuadric();
	gluQuadricDrawStyle(sphere, GLU_FILL);
	gluSphere(sphere, r, noOfSides, noOfSides);
	gluDeleteQuadric(sphere);
}

void drawCylinder(double br, double tr, double h) {
	GLUquadricObj* cylinder = NULL;
	cylinder = gluNewQuadric();
	gluQuadricDrawStyle(cylinder, GLU_FILL);
	gluCylinder(cylinder, br, tr, h, noOfSides, noOfSides);
	gluDeleteQuadric(cylinder);
}

void drawYCylinder(double br, double tr, float h) {
	glPushMatrix();
		glRotatef(90.0f, 1.f, 0.f, 0.f);
		drawCylinder(br, tr, h);
	glPopMatrix();
}

void drawBlock(double w, double h, double d) {
	double x = w / 2.0;
	double y = h / 2.0;
	double z = d / 2.0;

	glBegin(GL_QUADS);

	// Front face
	glNormal3f(0.0, 0.0, 1.0);
	glVertex3f(-x, -y, z);
	glVertex3f(x, -y, z);
	glVertex3f(x, y, z);
	glVertex3f(-x, y, z);

	// Back face
	glNormal3f(0.0, 0.0, -1.0);
	glVertex3f(-x, -y, -z);
	glVertex3f(x, -y, -z);
	glVertex3f(x, y, -z);
	glVertex3f(-x, y, -z);

	// Left face
	glNormal3f(-1.0, 0.0, 0.0);
	glVertex3f(-x, -y, -z);
	glVertex3f(-x, -y, z);
	glVertex3f(-x, y, z);
	glVertex3f(-x, y, -z);

	// Right face
	glNormal3f(1.0, 0.0, 0.0);
	glVertex3f(x, -y, -z);
	glVertex3f(x, -y, z);
	glVertex3f(x, y, z);
	glVertex3f(x, y, -z);

	// Top face
	glNormal3f(0.0, 1.0, 0.0);
	glVertex3f(-x, y, -z);
	glVertex3f(x, y, -z);
	glVertex3f(x, y, z);
	glVertex3f(-x, y, z);

	// Bottom face
	glNormal3f(0.0, -1.0, 0.0);
	glVertex3f(-x, -y, -z);
	glVertex3f(x, -y, -z);
	glVertex3f(x, -y, z);
	glVertex3f(-x, -y, z);

	glEnd();
}

void drawTrapezoidBlock(double bottomW, double topW, double h, double bottomD, double topD)
{
	double halfBottomW = bottomW / 2.0;
	double halfTopW = topW / 2.0;
	double halfH = h / 2.0;
	double halfBottomD = bottomD / 2.0;
	double halfTopD = topD / 2.0;

	glBegin(GL_QUADS);

	// --- Front face (Z positive) ---
	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-halfBottomW, -halfH, halfBottomD);
	glVertex3f(halfBottomW, -halfH, halfBottomD);
	glVertex3f(halfTopW, halfH, halfTopD);
	glVertex3f(-halfTopW, halfH, halfTopD);

	// --- Back face (Z negative) ---
	glNormal3f(0.0f, 0.0f, -1.0f);
	glVertex3f(-halfBottomW, -halfH, -halfBottomD);
	glVertex3f(halfBottomW, -halfH, -halfBottomD);
	glVertex3f(halfTopW, halfH, -halfTopD);
	glVertex3f(-halfTopW, halfH, -halfTopD);

	// --- Left face (slanted in X & Z) ---
	{
		// Two vectors defining the face
		float v1[3] = { 0, h, halfTopD - halfBottomD };     // vertical slope (y,z)
		float v2[3] = { halfBottomW - halfTopW, h, 0 };    // slope (x,y)
		// Cross product for normal
		float nx = v1[1] * v2[2] - v1[2] * v2[1];
		float ny = v1[2] * v2[0] - v1[0] * v2[2];
		float nz = v1[0] * v2[1] - v1[1] * v2[0];
		float len = sqrt(nx * nx + ny * ny + nz * nz);
		nx /= len; ny /= len; nz /= len;

		glNormal3f(nx, ny, nz);
		glVertex3f(-halfBottomW, -halfH, -halfBottomD);
		glVertex3f(-halfBottomW, -halfH, halfBottomD);
		glVertex3f(-halfTopW, halfH, halfTopD);
		glVertex3f(-halfTopW, halfH, -halfTopD);
	}

	// --- Right face (slanted in X & Z) ---
	{
		float v1[3] = { 0, h, halfTopD - halfBottomD };
		float v2[3] = { halfTopW - halfBottomW, h, 0 };
		float nx = v1[1] * v2[2] - v1[2] * v2[1];
		float ny = v1[2] * v2[0] - v1[0] * v2[2];
		float nz = v1[0] * v2[1] - v1[1] * v2[0];
		float len = sqrt(nx * nx + ny * ny + nz * nz);
		nx /= len; ny /= len; nz /= len;

		glNormal3f(nx, ny, nz);
		glVertex3f(halfBottomW, -halfH, -halfBottomD);
		glVertex3f(halfBottomW, -halfH, halfBottomD);
		glVertex3f(halfTopW, halfH, halfTopD);
		glVertex3f(halfTopW, halfH, -halfTopD);
	}

	// --- Top face ---
	glNormal3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-halfTopW, halfH, -halfTopD);
	glVertex3f(halfTopW, halfH, -halfTopD);
	glVertex3f(halfTopW, halfH, halfTopD);
	glVertex3f(-halfTopW, halfH, halfTopD);

	// --- Bottom face ---
	glNormal3f(0.0f, -1.0f, 0.0f);
	glVertex3f(-halfBottomW, -halfH, -halfBottomD);
	glVertex3f(halfBottomW, -halfH, -halfBottomD);
	glVertex3f(halfBottomW, -halfH, halfBottomD);
	glVertex3f(-halfBottomW, -halfH, halfBottomD);

	glEnd();
}

void drawJoint(float r) {
	drawSphere(r);
}

void drawLimbSegment(float br, float tr, float len) {
	// bottom cap
	drawJoint(br);
	// shaft
	drawYCylinder(br, tr, len);
}

void drawTorso(const HumanDims& d) {
	// pelvis is centered at current origin
	// bottomW > topW for pelvis taper
	drawTrapezoidBlock(
		d.pelvisW,                 // bottom width
		d.pelvisW * 0.7f,          // top width (slimmer waist)
		d.pelvisH,                 // height
		d.pelvisD,                 // bottom depth
		d.pelvisD * 0.8f           // top depth
	);

	// chest center = pelvis_full/2 + chest_half
	float chestCenterY = (d.pelvisH + d.chestH) * 0.5f;
	glPushMatrix();
	glTranslatef(0.f, chestCenterY, 0.f);
	// chest tapers opposite: top wider than bottom (shoulders broader)
	drawTrapezoidBlock(
		d.shoulderW * 0.7f,    // bottom width (narrower near waist)
		d.shoulderW,           // top width (broad shoulders)
		d.chestH,              // height
		d.chestD * 0.7f,       // bottom depth
		d.chestD               // top depth
	);
	glPopMatrix();

	// neck center = pelvis_full + chest_full + neck_half
	float neckCenterY = d.chestH + (d.pelvisH + d.neckH) * 0.5f;
	glPushMatrix();
	glTranslatef(0.f, neckCenterY, 0.f);
	drawLimbSegment(d.neckR, d.neckR, d.neckH);
	glPopMatrix();
}

void drawHead(const HumanDims& d) {
	// head center = pelvis_full + chest_full + neck_full + head_radius
	float headCenterY = d.chestH + d.neckH + d.headR + (d.pelvisH * 0.5f);
	glPushMatrix();
	glTranslatef(0.f, headCenterY, 0.f);
	drawSphere(d.headR);
	glPopMatrix();
}

void drawSword(float bladeL = 4.0f, float bladeW = 0.2f, float bladeD = 0.1f,
	float handleL = 1.0f, float handleW = 0.3f, float handleD = 0.2f,
	float guardW = 1.0f, float guardH = 0.2f, float guardD = 0.2f)
{
	// Blade
	glPushMatrix();
	glTranslatef(0.f, -(bladeL * 0.5f + handleL + guardH), 0.f); // position below guard
	drawBlock(bladeW, bladeL, bladeD);
	glPopMatrix();

	// Handle
	glPushMatrix();
	glTranslatef(0.f, -(handleL * 0.5f), 0.f);
	drawBlock(handleW, handleL, handleD);
	glPopMatrix();

	// Guard
	glPushMatrix();
	glTranslatef(0.f, -(handleL + guardH * 0.5f), 0.f);
	drawBlock(guardW, guardH, guardD);
	glPopMatrix();
}

void drawArm(const HumanDims& d, bool left, float shoulder_rx_deg = 0.f, float elbow_rx_deg = 0.f) {
	float side = left ? -1.f : 1.f;
	float shoulderX = side * (d.shoulderW * 0.5f + d.armJointR * 0.5);
	// shoulderY = chest top = pelvis_half + chest_full
	float shoulderY = (d.pelvisH * 0.5f) + d.chestH - d.armJointR * 0.5;

	glPushMatrix();
	glTranslatef(shoulderX, shoulderY, 0.f);

	//glRotatef(side * 90.f, 0.f, 0.f, 1.f); // rotate arm out to the side
	// upper arm
	glRotatef(shoulder_rx_deg, 1.f, 0.f, 0.f);
	drawLimbSegment(d.armLimbR, d.armLimbR, d.upperArmL);

	// forearm + hand
	glTranslatef(0.f, -d.upperArmL, 0.f);
	glRotatef(elbow_rx_deg, 1.f, 0.f, 0.f);
	
	drawLimbSegment(d.armLimbR, d.armLimbR, d.forearmL);
	glTranslatef(0.f, -(d.forearmL + d.handH * 0.5f), 0.f);
	drawBlock(d.handW, d.handH, d.handD);

	if (!left) {
		glPushMatrix();
		glTranslatef(0.f, -d.handH * 0.5f, 0.f);   // attach at bottom of hand
		glRotatef(-90.f, 1.f, 0.f, 0.f);          // rotate blade to point forward
		drawSword();
		glPopMatrix();
	}
	glPopMatrix();
}

void drawLeg(const HumanDims& d, bool left, float hip_rx_deg = 0.f, float knee_rx_deg = 0.f) {
	float side = left ? -1.f : 1.f;
	float hipX = side * (d.pelvisW * 0.35f);
	float hipY = -d.pelvisH * 0.5f;

	glPushMatrix();
		glTranslatef(hipX, hipY, 0.f);

		glRotatef(-hip_rx_deg, 1.f, 0.f, 0.f);
		drawLimbSegment(d.legLimbR, d.legLimbR, d.upperLegL);

		// knee
		glTranslatef(0.f, -d.upperLegL, 0.f);
		glRotatef(-knee_rx_deg, 1.f, 0.f, 0.f);
		drawLimbSegment(d.armLimbR, d.armLimbR, d.lowerLegL);

		// foot
		glTranslatef(0.f, -(d.lowerLegL + d.footH * 0.5f), d.footD * 0.25f);
		drawBlock(d.footW, d.footH, d.footD);
	glPopMatrix();
}

void drawGround(float size = 20.f, int steps = 10) {
	glBegin(GL_LINES);
	for (int i = -steps; i <= steps; ++i) {
		float t = (size / steps) * i;
		glVertex3f(-size, 0.f, t); glVertex3f(size, 0.f, t);
		glVertex3f(t, 0.f, -size); glVertex3f(t, 0.f, size);
	}
	glEnd();
}

void drawHuman(const HumanDims& d, float walkPhaseDeg = 0.f, bool walking = false) {
	//setWire(true);
	
	glPushMatrix();
	float bodyLift = d.upperLegL + d.lowerLegL + d.footH + d.pelvisH * 0.5f;

	// add bobbing while walking
	float bob = walking ? (sin(walkPhaseDeg * M_PI / 180.0f) * 0.1f) : 0.f;
	glTranslatef(0.f, bodyLift + bob, 0.f);

	// swing angles
	float swing = walking ? sin(walkPhaseDeg * M_PI / 180.0f) * 30.f : 0.f;

	// Legs
	drawLeg(d, true, swing, -swing * 0.5f);   // left leg
	drawLeg(d, false, -swing, swing * 0.5f);  // right leg

	// Torso
	drawTorso(d);

	// Arms (opposite swing of legs)
	drawArm(d, true, -swing, 0.f);   // left arm
	drawArm(d, false, swing, 0.f);   // right arm

	// Head (slight counter bob if wanted)
	drawHead(d);

	glPopMatrix();
}

void lighting() {
	posB[0] = lightX;
	posB[1] = lightY;
	posB[2] = lightZ;

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
	//setWire(true);
	if (isWalking) {
		walkPhase += walkAnimSpeed;
		if (walkPhase > 360.f) walkPhase -= 360.f;

		// Move position
		hx += walkDirX * humanSpeed;
		hz += walkDirZ * humanSpeed;

		// Update facing angle (convert radians -> degrees)
		humanAngle = atan2f(walkDirX, walkDirZ) * 180.0f / M_PI;
	}


	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	lighting();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// camera transform
	glTranslatef(-camX, -camY, -camZ);
	glRotatef(camRotX, 1.f, 0.f, 0.f);
	glRotatef(camRotY, 0.f, 1.f, 0.f);
	glRotatef(camRotZ, 0.f, 0.f, 1.f);

	drawGround();
	HumanDims d;
	glPushMatrix();
		glTranslatef(hx, hy, hz);
		glRotatef(humanAngle, 0.f, 1.f, 0.f);
		drawHuman(d, walkPhase, isWalking);
	glPopMatrix();

}

void projection() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	if (isOrtho) {
		glOrtho(-10, 10, -10, 10, 1, 100);
	}
	else if (isPerspective) {
		gluPerspective(60.0, 1.0, PNear, PFar);
	}
}

void display()
{
	projection();
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
		950, 10, 1000, 1000,
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