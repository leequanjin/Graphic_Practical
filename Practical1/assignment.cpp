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

// projection parameters
float PNear = 1.0, PFar = 100.0;
bool isOrtho = false, isPerspective = true;

float camX = 0.f, camY = 0.f, camZ = 25.f; // camera position
float camRotX = 10.f, camRotY = -20.f, camRotZ = 0.f; // camera rotation
float camMoveSpeed = 0.5f;
float camRotSpeed = 2.0f;

// human position
float hx = 0.f, hy = 0.f, hz = 0.f;
float walkAnimSpeed = 0.2f; // speed of swing
float humanSpeed = 0.01f; // speed of movement

// walk cycle
float walkPhase = 0.f;       // angle in degrees
bool isWalking = false;      // walking state
float walkDirX = 0.f, walkDirZ = 0.f; // direction
float humanAngle = 0.f; // facing direction in degrees

// ----- armor toggle (press U to show/hide) -----
bool g_armorOn = true;

int weaponType = 1;

// small offset so plates don't z-fight with the body
const float Z_EPS = 0.01f;

// simple materials
inline void useMetal(float r = 0.75f, float g = 0.75f, float b = 0.80f) {
	GLfloat diff[4] = { r,g,b,1.f };
	GLfloat spec[4] = { 0.9f,0.9f,0.95f,1.f };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, diff);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 64.f);
}

inline void useCloth(float r = 0.70f, float g = 0.20f, float b = 0.20f) {
	GLfloat diff[4] = { r,g,b,1.f };
	GLfloat spec[4] = { 0.f,0.f,0.f,1.f };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, diff);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 8.f);
}

inline void useNeutral() {
	GLfloat diff[4] = { 0.95f, 0.95f, 0.95f, 1.0f };
	GLfloat spec[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, diff);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);
}

inline void useWood(float r = 0.45f, float g = 0.28f, float b = 0.12f) {
	GLfloat diff[4] = { r,g,b,1.f };
	GLfloat spec[4] = { 0.05f,0.05f,0.05f,1.f };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, diff);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 8.f);
}

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
	float neckR = headR * 0.5f;

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

		// Armor and Weapon
		else if (wParam == 'U') { g_armorOn = !g_armorOn; }
		else if (wParam == VK_TAB) { weaponType = 1 - weaponType; }

		// Walking controls
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

// ---------- Helmet + plume ----------
void drawHelmet(const HumanDims& d) {
	// We are already at world origin in drawHuman, so compute absolute Y
	float pelvisHalf = d.pelvisH * 0.5f;
	float headCenterY = pelvisHalf + d.chestH + d.neckH + d.headR;

	useMetal(); // silver
	glPushMatrix();
	glTranslatef(0.f, headCenterY, 0.f);

	// bowl
	float bowlH = d.headR * 0.7f;
	glPushMatrix();
	glTranslatef(0.f, d.headR * 0.15f, 0.f);
	drawYCylinder(d.headR * 1.05f, d.headR * 1.05f, bowlH);
	glPopMatrix();

	// brim
	glPushMatrix();
	glTranslatef(0.f, d.headR * 0.5f, 0.f);
	drawBlock(d.headR * 2.1f, d.headR * 0.15f, d.headR * 2.1f);
	glPopMatrix();

	// plume (two cones)
	glPushMatrix();
		glTranslatef(0.f, d.headR * 0.95f, 0.f);
		glRotatef(-18.f, 0, 0, 1);
		useCloth(0.85f, 0.15f, 0.15f);
		drawSphere(0.1);
		glRotatef(36.f, 0, 0, 1);
	glPopMatrix();

	glPopMatrix();
}

// ---------- Shoulder pauldron (layered) ----------
void drawShoulderPadPiece(float w, float h, float d) {
	drawTrapezoidBlock(w, w * 0.92f, h, d, d * 0.95f);
}

void drawShoulderPadAt(const HumanDims& d, bool left) {
	useMetal();
	float side = left ? -1.f : 1.f;
	float y = d.pelvisH * 0.5f + d.chestH;       // shoulder line (top of chest)
	float x = side * (d.shoulderW * 0.5f + 0.05f);

	glPushMatrix();
	glTranslatef(x, y, 0.f);
	glRotatef(-side * 8.f, 0, 0, 1);
	drawShoulderPadPiece(d.shoulderW, d.chestH * 0.18f, d.chestD);
	glTranslatef(0, d.chestH * 0.12f, 0);
	drawShoulderPadPiece(d.shoulderW * 0.9, d.chestH * 0.14f, d.chestD * 0.9);
	glPopMatrix();
}


// ---------- Chest/back plate + belt ----------
void drawCuirass(const HumanDims& d) {
	useMetal();
	const float chestCenterY = d.pelvisH * 0.5f + d.chestH * 0.5f;

	// front plate (thin, in front of chest)
	glPushMatrix();
	glTranslatef(0.f, chestCenterY, d.chestD * 0.52f + Z_EPS);
	drawTrapezoidBlock(d.shoulderW * 0.80f, d.shoulderW * 0.95f,
		d.chestH * 0.98f, d.chestD * 0.02f, d.chestD * 0.02f);
	glPopMatrix();

	// back plate (behind chest)
	glPushMatrix();
	glTranslatef(0.f, chestCenterY, -(d.chestD * 0.52f + Z_EPS));
	drawTrapezoidBlock(d.shoulderW * 0.80f, d.shoulderW * 0.95f,
		d.chestH * 0.98f, d.chestD * 0.02f, d.chestD * 0.02f);
	glPopMatrix();

	// belt around upper pelvis
	useMetal(0.85f, 0.55f, 0.20f);
	glPushMatrix();
	const float beltY = d.pelvisH * 0.25f;      // a bit below waist
	glTranslatef(0.f, beltY, 0.f);
	drawBlock(d.pelvisW * 1.05f, d.pelvisH * 0.18f, d.pelvisD * 1.02f);
	glPopMatrix();
}


// ---------- Layered skirt / tassets (front/back + sides) ----------
void drawSkirtArmor(const HumanDims& d) {
	const float waistY = d.pelvisH * 0.5f;

	// cloth front / back
	useCloth(0.75f, 0.15f, 0.15f);
	glPushMatrix();
	glTranslatef(0.f, waistY - d.pelvisH * 0.25f, d.pelvisD * 0.55f + Z_EPS);
	drawTrapezoidBlock(d.pelvisW * 0.90f, d.pelvisW * 0.70f,
		d.upperLegL * 0.70f, d.pelvisD * 0.02f, d.pelvisD * 0.02f);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.f, waistY - d.pelvisH * 0.25f, -(d.pelvisD * 0.55f + Z_EPS));
	drawTrapezoidBlock(d.pelvisW * 0.90f, d.pelvisW * 0.70f,
		d.upperLegL * 0.70f, d.pelvisD * 0.02f, d.pelvisD * 0.02f);
	glPopMatrix();

	// metal side tassets
	useMetal();
	for (int i = 0; i < 2; ++i) {
		float side = (i == 0) ? -1.f : 1.f;
		glPushMatrix();
		glTranslatef(side * (d.pelvisW * 0.55f + Z_EPS), waistY - d.pelvisH * 0.20f, 0.f);
		glRotatef(side * 20.f, 0, 1, 0);
		drawTrapezoidBlock(d.pelvisW * 0.10f, d.pelvisW * 0.1f,
			d.upperLegL * 0.55f, d.pelvisD * 0.85f, d.pelvisD * 0.70f);
			glPushMatrix();
				glRotatef(side * 20.f, 0, 1, 0);
				drawTrapezoidBlock(d.pelvisW * 0.10f, d.pelvisW * 0.1f,
					d.upperLegL * 0.55f, d.pelvisD * 0.85f, d.pelvisD * 0.70f);
			glPopMatrix();
		glPopMatrix();
	}
}


// ---------- Forearm bracer (call inside right/left arm local space after elbow) ----------
void drawForearmBracer(const HumanDims& d) {
	useMetal();
	glPushMatrix();
	glTranslatef(0.f, -d.forearmL * 0.35f, 0.f);
	drawYCylinder(d.armLimbR * 1.15f, d.armLimbR * 1.10f, d.forearmL * 0.7f);
	glPopMatrix();
}

// ---------- Greave + knee plate (call inside leg local space after knee) ----------
void drawShinGreave(const HumanDims& d) {
	useMetal();
	// shin tube
	glPushMatrix();
	glTranslatef(0.f, -d.lowerLegL * 0.4f, 0.f);
	drawYCylinder(d.legLimbR * 1.10f, d.legLimbR * 1.05f, d.lowerLegL * 0.8f);
	glPopMatrix();
	// knee cap
	glPushMatrix();
	drawSphere(d.legLimbR * 0.65f);
	glPopMatrix();
}

void drawCenterTasset(const HumanDims& d) {
	useMetal();
	const float waistY = d.pelvisH * 0.5f;
	glPushMatrix();
	glTranslatef(0.f, waistY - d.pelvisH * 0.25f, d.pelvisD * 0.58f + Z_EPS);
	drawTrapezoidBlock(d.pelvisW * 0.45f, d.pelvisW * 0.28f,
		d.upperLegL * 0.55f, d.pelvisD * 0.06f, d.pelvisD * 0.04f);
	glPopMatrix();
}

void drawUpperArmPlate(const HumanDims& d) {
	useMetal();
	glPushMatrix();               // upper arm local spacea
	glTranslatef(0.f, -d.upperArmL * 0.1f, 0.f);
	drawYCylinder(d.armLimbR * 1.10f, d.armLimbR * 1.5f, d.upperArmL * 1.3f);
	//drawBlock(d.armLimbR * 3.f, d.upperArmL * 0.7f, d.armLimbR * 3.f);
	glPopMatrix();
}

void drawThighPlate(const HumanDims& d) {
	useMetal();
	glPushMatrix(); // upper leg local
	glTranslatef(0.f, -d.upperLegL * 0.35f, d.legLimbR * 0.3f);
	drawTrapezoidBlock(d.legLimbR * 2.2f, d.legLimbR * 1.9f,
		d.upperLegL * 0.7f, d.legLimbR * 1.6f, d.legLimbR * 1.8f);
		glPushMatrix();
			glRotatef(90.f, 1, 0, 0);
			drawCylinder(d.legLimbR * 1.2f, d.legLimbR * 1.0f, d.upperLegL * 0.4f);
			drawCylinder(d.legLimbR * 1.0f, d.legLimbR * 0.8f, d.upperLegL * 0.6f);
			drawCylinder(d.legLimbR * 0.8f, d.legLimbR * 0.6f, d.upperLegL * 0.8f);
		glPopMatrix();
	glPopMatrix();
}
void drawSabaton(const HumanDims& d) {
	useMetal();
	glPushMatrix();
	glTranslatef(0.f, 0.f, d.footD * 0.35f);
	drawTrapezoidBlock(d.footW * 1.1f, d.footW * 0.7f,
		d.footH * 0.9f, d.footD * 1.0f, d.footD * 0.7f);
	glPopMatrix();
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
	float headCenterY = d.chestH + d.neckH + d.headR + (d.pelvisH * 0.5f) - 0.1f;
	glPushMatrix();
	glTranslatef(0.f, headCenterY, 0.f);
	drawSphere(d.headR);
	glPopMatrix();
}

void drawSword(
	// blade
	float bladeL = 5.2f,         // total blade length (ricasso+mid+tip)
	float baseW = 0.35f,        // blade width at guard
	float tipW = 0.06f,        // width at tip
	float thick0 = 0.10f,        // thickness near guard
	float thick1 = 0.04f,        // thickness at tip
	// hilt
	float handleL = 1.1f,        // grip length (downwards)
	float gripR = 0.18f,       // grip radius
	float guardW = 1.20f,       // crossguard span (X)
	float guardH = 0.12f,       // crossguard thickness (Y)
	float guardD = 0.25f,       // crossguard depth (Z)
	float pommelR = 0.22f,
	float pommelH = 0.26f
) {
	const float EPS = 0.002f;

	glPushMatrix();
	glRotatef(180.f, 1.f, 0.f, 0.f);
	glTranslatef(0.f, 0.5f, 0.0f);

	// ------- CROSSGUARD (sits above the grip, top at y=guardH) -------
	useMetal(0.82f, 0.82f, 0.87f);
	glPushMatrix();
	glTranslatef(0.f, guardH * 0.5f, 0.f);
	drawBlock(guardW, guardH, guardD);           // central block

	// curved quillons (simple rotated blocks)
	glPushMatrix();
	glTranslatef(guardW * 0.50f - guardH * 0.25f, 0.f, 0.f);
	glRotatef(12.f, 0, 0, 1);
	drawBlock(guardW * 0.60f, guardH * 0.35f, guardD * 0.60f);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(-guardW * 0.50f + guardH * 0.25f, 0.f, 0.f);
	glRotatef(-12.f, 0, 0, 1);
	drawBlock(guardW * 0.60f, guardH * 0.35f, guardD * 0.60f);
	glPopMatrix();
	glPopMatrix();

	// ------- GRIP -------
	useCloth(0.25f, 0.12f, 0.08f);
	glPushMatrix();
	glTranslatef(0.f, 0.0f, 0.f);         // base at -handleL
	drawYCylinder(gripR, gripR, handleL + EPS);      // reaches (slightly past) y=0
	glPopMatrix();


	// small metal rings on grip
	useMetal(0.78f, 0.78f, 0.83f);
	glPushMatrix(); glTranslatef(0.f, -0.15f, 0.f);
	drawYCylinder(gripR * 1.07f, gripR * 1.07f, 0.03f);
	glPopMatrix();
	glPushMatrix(); glTranslatef(0.f, -handleL + 0.15f, 0.f);
	drawYCylinder(gripR * 1.07f, gripR * 1.07f, 0.03f);
	glPopMatrix();

	// ------- POMMEL -------
	glPushMatrix();
	glTranslatef(0.f, -handleL - pommelH * 0.5f - EPS, 0.f);
	drawYCylinder(pommelR * 0.85f, pommelR, pommelH + EPS);
	glPopMatrix();

	// ------- BLADE -------
	useMetal(0.86f, 0.86f, 0.91f);

	const float ricassoL = 0.25f;
	const float tipL = bladeL * 0.22f;
	const float midL = bladeL - tipL - ricassoL;

	// ricasso (thicker/taller section just above guard)
	glPushMatrix();
	glTranslatef(0.f, guardH + ricassoL * 0.5f - EPS, 0.f);
	drawTrapezoidBlock(baseW * 0.95f, baseW * 0.90f, ricassoL,
		thick0, thick0 * 0.95f);
	glPopMatrix();

	// main taper
	glPushMatrix();
	glTranslatef(0.f, guardH + ricassoL + midL * 0.5f - EPS, 0.f);
	drawTrapezoidBlock(baseW, (baseW + tipW) * 0.5f, midL,
		thick0, (thick0 + thick1) * 0.5f);
	glPopMatrix();

	// tip
	glPushMatrix();
	glTranslatef(0.f, guardH + ricassoL + midL + tipL * 0.5f - EPS, 0.f);
	drawTrapezoidBlock((baseW + tipW) * 0.5f, tipW, tipL,
		(thick0 + thick1) * 0.5f, thick1);
	glPopMatrix();

	// fuller (shallow groove) – darker metal, inset to avoid z-fight
	useMetal(0.65f, 0.67f, 0.72f);
	float fullerL = midL * 0.75f;
	glPushMatrix();
	glTranslatef(0.f, guardH + ricassoL + fullerL * 0.5f, 0.f);
	drawBlock(baseW * 0.34f, fullerL, thick0 * 0.32f);
	glPopMatrix();

	glPopMatrix();
}

void drawSpear(float shaftL = 9.0f, float shaftR = 0.09f, float holdFracFromButt = 0.5f)
{
	const float buttY = holdFracFromButt * shaftL;          // behind the hand
	const float tipY = (1.0f - holdFracFromButt) * shaftL; // in front of the hand

	glPushMatrix();
	glTranslatef(0.f, 1.0f, 0.f); // raise spear to hand level
	// --- WOODEN SHAFT ---
	useWood();
	glPushMatrix();
		glTranslatef(0.f, buttY, 0.f);           // start at the butt
		drawYCylinder(shaftR, shaftR, shaftL);   // runs to the tip
	glPopMatrix();

	// Butt cap (little metal piece at the very end)
	useMetal(0.7f, 0.7f, 0.75f);
	glPushMatrix();
		glTranslatef(0.f, buttY - 0.02f, 0.f);
		drawYCylinder(shaftR * 1.05f, shaftR * 1.05f, 0.12f);
	glPopMatrix();

	// Hand wrap centered on the grip (origin)
	useCloth(0.35f, 0.18f, 0.12f); // leather-ish
	glPushMatrix();
		glTranslatef(0.f, -0.15f, 0.f);
		drawYCylinder(shaftR * 1.1f, shaftR * 1.1f, 2.0f);
	glPopMatrix();

	glPushMatrix();
	glRotatef(180.f, 1.f, 0.f, 0.f);

	// Ferrule under the head
	useMetal();
	glPushMatrix();
	glTranslatef(0.f, tipY - 0.45f, 0.f);
	drawYCylinder(shaftR * 1.12f, shaftR * 1.12f, 0.22f);
	glPopMatrix();

	// Small square collar
	glPushMatrix();
	glTranslatef(0.f, tipY - 0.05f, 0.f);
	drawBlock(shaftR * 3.0f, 0.15f, shaftR * 3.0f);
	glPopMatrix();

	// --- SPEAR HEAD (leaf/diamond) ---
	useMetal(0.85f, 0.85f, 0.90f);

	// main leaf to the tip
	const float bladeH = 1.9f;
	glPushMatrix();
	glTranslatef(0.f, tipY + bladeH * 0.5f, 0.f); // bottom sits at tipY
	drawTrapezoidBlock(
		/*bottomW*/ shaftR * 2.6f,  /*topW*/ 0.0f,
		/*h*/       bladeH,
		/*bottomD*/ shaftR * 1.2f,  /*topD*/ 0.0f
	);
	glPopMatrix();

	// small back taper to give a diamond profile (optional)
	glPushMatrix();
	glTranslatef(0.f, tipY - 0.20f, 0.f);
	drawTrapezoidBlock(shaftR * 2.0f, 0.0f, 0.40f, shaftR * 0.9f, 0.0f);
	glPopMatrix();

	glPopMatrix();
	glPopMatrix();
}

void drawArm(const HumanDims& d, bool left, float shoulder_rx_deg = 0.f, float elbow_rx_deg = 0.f) {
	float side = left ? -1.f : 1.f;
	float shoulderX = side * (d.shoulderW * 0.5f + d.armJointR * 0.5);
	float shoulderY = (d.pelvisH * 0.5f) + d.chestH - d.armJointR * 0.5;

	glPushMatrix();
	glTranslatef(shoulderX, shoulderY, 0.f);

	//glRotatef(side * 90.f, 0.f, 0.f, 1.f); // rotate arm out to the side

	// upper arm
	glRotatef(shoulder_rx_deg, 1.f, 0.f, 0.f);
	if (g_armorOn) drawUpperArmPlate(d);
	drawLimbSegment(d.armLimbR, d.armLimbR, d.upperArmL);

	// forearm + hand
	glTranslatef(0.f, -d.upperArmL, 0.f);
	glRotatef(elbow_rx_deg, 1.f, 0.f, 0.f);

	// NEW: forearm bracer that follows elbow rotation
	if (g_armorOn) drawForearmBracer(d);

	drawLimbSegment(d.armLimbR, d.armLimbR, d.forearmL);
	glTranslatef(0.f, -(d.forearmL + d.handH * 0.5f), 0.f);
	drawBlock(d.handW, d.handH, d.handD);

	if (!left) {
		glPushMatrix();
		glTranslatef(0.f, -d.handH * 0.5f, 0.f);   // attach at bottom of hand
		glRotatef(-90.f, 1.f, 0.f, 0.f);          // rotate blade to point forward
		if (weaponType == 0) drawSword();
		else                 drawSpear();
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
	if (g_armorOn) drawThighPlate(d);

	drawLimbSegment(d.legLimbR, d.legLimbR, d.upperLegL);

	// knee
	glTranslatef(0.f, -d.upperLegL, 0.f);
	glRotatef(-knee_rx_deg, 1.f, 0.f, 0.f);

	// NEW: greave + kneecap
	if (g_armorOn) drawShinGreave(d);

	drawLimbSegment(d.armLimbR, d.armLimbR, d.lowerLegL);


	// foot
	glTranslatef(0.f, -(d.lowerLegL + d.footH * 0.5f), d.footD * 0.25f);
	drawBlock(d.footW, d.footH, d.footD);
	if (g_armorOn) drawSabaton(d);
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

	// armor on torso/waist
	if (g_armorOn) {
		drawCuirass(d);
		drawCenterTasset(d);
		drawSkirtArmor(d);
		drawShoulderPadAt(d, true);
		drawShoulderPadAt(d, false);
	}

	// Arms (opposite swing of legs)
	drawArm(d, true, -swing, 0.f);   // left arm
	drawArm(d, false, swing, 0.f);   // right arm

	// Head (slight counter bob if wanted)
	drawHead(d);

	// NEW: helmet+plume
	if (g_armorOn) drawHelmet(d);

	glPopMatrix();
}

void lighting() {
	if (isLightOn) glEnable(GL_LIGHTING); else glDisable(GL_LIGHTING);

	// global ambient
	GLfloat globalAmb[4] = { 0.15f, 0.15f, 0.15f, 1.0f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmb);

	// KEY light — white directional (w = 0)
	GLfloat keyPos[4] = { -0.4f, 1.0f, 0.6f, 0.0f }; // direction vector
	GLfloat white[4] = { 1,1,1,1 };
	GLfloat grey[4] = { 0.35f,0.35f,0.35f,1 };
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_POSITION, keyPos);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
	glLightfv(GL_LIGHT0, GL_SPECULAR, white);

	// FILL light — neutral movable point (w = 1)
	GLfloat fillPos[4] = { lightX, lightY, lightZ, 1.0f };
	GLfloat fillDif[4] = { 0.35f, 0.35f, 0.35f, 1.0f };
	glEnable(GL_LIGHT1);
	glLightfv(GL_LIGHT1, GL_POSITION, fillPos);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, grey);
	glLightfv(GL_LIGHT1, GL_SPECULAR, grey);
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
	glEnable(GL_NORMALIZE); 
	useNeutral();

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