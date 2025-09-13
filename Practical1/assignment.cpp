#define _USE_MATH_DEFINES
#include <math.h>
#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <stdio.h>

#pragma comment (lib, "OpenGL32.lib")

#define WINDOW_TITLE "OpenGL Window  [Mode: CAMERA]"

// ----- Input Modes -----
enum InputMode { MODE_CAM = 1, MODE_CHAR = 2, MODE_RIG = 3 };
int g_mode = MODE_CAM;

// ---- Rig (per-joint control) ----
enum RigJoint {
	J_SPINE = 0,          // torso base
	J_NECK,               // neck/head
	J_SHOULDER_L, J_ELBOW_L, J_WRIST_L,
	J_SHOULDER_R, J_ELBOW_R, J_WRIST_R,
	J_HIP_L, J_KNEE_L, J_ANKLE_L,
	J_HIP_R, J_KNEE_R, J_ANKLE_R,
	J_MAX
};

// gJ[joint][axis], axis: 0=X (pitch), 1=Y (yaw), 2=Z (roll)
float gJ[J_MAX][3] = { 0 };

int   g_selJoint = J_SHOULDER_L;
int   g_selAxis = 0;         // 0=X,1=Y,2=Z
float g_rigStep = 2.0f;

static inline const char* jointName(int j) {
	switch (j) {
	case J_SPINE:      return "Spine";
	case J_NECK:       return "Neck/Head";
	case J_SHOULDER_L: return "Shoulder L";
	case J_ELBOW_L:    return "Elbow L";
	case J_WRIST_L:    return "Wrist L";
	case J_SHOULDER_R: return "Shoulder R";
	case J_ELBOW_R:    return "Elbow R";
	case J_WRIST_R:    return "Wrist R";
	case J_HIP_L:      return "Hip L";
	case J_KNEE_L:     return "Knee L";
	case J_ANKLE_L:    return "Ankle L";
	case J_HIP_R:      return "Hip R";
	case J_KNEE_R:     return "Knee R";
	case J_ANKLE_R:    return "Ankle R";
	default:           return "?";
	}
}
static inline char axisName(int a) { return a == 0 ? 'X' : (a == 1 ? 'Y' : 'Z'); }

// Apply the selected joint’s full XYZ rotation to the current matrix
static inline void applyJointRot(int j) {
	glRotatef(gJ[j][0], 1, 0, 0); // X
	glRotatef(gJ[j][1], 0, 1, 0); // Y
	glRotatef(gJ[j][2], 0, 0, 1); // Z
}

// --- Window handle so we can change the title ---
HWND g_hWnd = nullptr;

// --- Bitmap font (wglUseFontBitmaps) ---
GLuint g_fontBase = 0;
HFONT  g_font = nullptr;

// Arrow key states (for smooth, held-down movement)
bool g_keyUp = false, g_keyDown = false, g_keyLeft = false, g_keyRight = false;

int qNo = 1;

int noOfSides = 30; // for circle and cylinder

// light parameters
float lightX = 8.0f, lightY = 10.0f, lightZ = 8.0f; // default: top-right-front
float lightMoveSpeed = 1.0f;
bool isLightOn = true;

bool  fillOn = true;      // toggle GL_LIGHT1 (your “sun”)
float ambientLevel = 0.6f; // global ambient intensity (0..1)

// Light tuning
float fillIntensity = 0.35f;             // scales GL_LIGHT1
GLfloat fillColor[4] = { 1.f, 1.f, 1.f, 1.0f }; // start grey

// ---- Light gizmos (visible "suns") ----
bool  showSuns = true;
float sunSizeFill = 0.6f;
float sunSizeKey = 0.8f;

// projection parameters
float PNear = 1.0, PFar = 100.0;
bool isOrtho = false, isPerspective = true;

float camX = 0.f, camY = 0.f, camZ = 50.f; // camera position
float camRotX = 10.f, camRotY = -20.f, camRotZ = 0.f; // camera rotation
float camMoveSpeed = 0.2f;
float camRotSpeed = 2.0f;

// human position
float hx = 0.f, hy = 0.f, hz = 0.f;
float walkAnimSpeed = 0.2f; // speed of swing
float humanSpeed = 0.02f; // speed of movement

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

inline void useSkin(float r = 0.90f, float g = 0.76f, float b = 0.66f) {
	GLfloat diff[4] = { r, g, b, 1.f };
	GLfloat spec[4] = { 0.15f, 0.12f, 0.10f, 1.f };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, diff);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 16.f);
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

	case WM_KEYDOWN: {
		if (wParam == VK_ESCAPE) { PostQuitMessage(0); break; }

		// ---- Mode switching ----
		else if (wParam == '1') {
			g_mode = MODE_CAM;
			if (g_hWnd) SetWindowTextA(g_hWnd, "OpenGL Window  [Mode: CAMERA]");
		}
		else if (wParam == '2') {
			g_mode = MODE_CHAR;
			if (g_hWnd) SetWindowTextA(g_hWnd, "OpenGL Window  [Mode: CHARACTER]");
		}
		else if (wParam == '3') {
			g_mode = MODE_RIG;
			if (g_hWnd) SetWindowTextA(g_hWnd, "OpenGL Window  [Mode: RIG]");
			break;
		}


		// ---- Projection toggles (unchanged) ----
		if (wParam == 'O') { isOrtho = true;  isPerspective = false; break; }
		if (wParam == 'P') { isOrtho = false; isPerspective = true;  break; }

		// ---- Camera rotation (always available) ----
		if (wParam == 'W') { camRotX += camRotSpeed; break; }
		if (wParam == 'S') { camRotX -= camRotSpeed; break; }
		if (wParam == 'A') { camRotY += camRotSpeed; break; }
		if (wParam == 'D') { camRotY -= camRotSpeed; break; }
		if (wParam == 'Q') { camRotZ += camRotSpeed; break; }
		if (wParam == 'E') { camRotZ -= camRotSpeed; break; }

		// ---- Armor / weapon (unchanged) ----
		if (wParam == 'U') { g_armorOn = !g_armorOn; break; }
		if (wParam == VK_TAB) { weaponType = 1 - weaponType; break; }

		// ---- Character quick stop ----
		if (wParam == VK_SPACE) { isWalking = false; break; }

		// ---- PageUp/PageDown for camera Z (works in any mode) ----
		if (wParam == VK_PRIOR) { camZ -= camMoveSpeed * 5; break; } // PageUp
		if (wParam == VK_NEXT) { camZ += camMoveSpeed * 5; break; } // PageDown

		// --- RIG MODE: per-joint controls ---
		if (g_mode == MODE_RIG) {
			// Joint selection
			if (wParam == VK_OEM_4) { // '[' prev joint
				g_selJoint = (g_selJoint - 1 + J_MAX) % J_MAX; break;
			}
			if (wParam == VK_OEM_6) { // ']' next joint
				g_selJoint = (g_selJoint + 1) % J_MAX; break;
			}

			// Axis selection
			if (wParam == 'X') { g_selAxis = 0; break; }
			if (wParam == 'Y') { g_selAxis = 1; break; }
			if (wParam == 'Z') { g_selAxis = 2; break; }
			if (wParam == VK_OEM_COMMA) { // ',' prev axis
				g_selAxis = (g_selAxis + 2) % 3; break;
			}
			if (wParam == VK_OEM_PERIOD) { // '.' next axis
				g_selAxis = (g_selAxis + 1) % 3; break;
			}

			// Bend on the selected axis — all 4 arrow directions
			if (wParam == VK_LEFT) { gJ[g_selJoint][g_selAxis] -= g_rigStep; break; }
			if (wParam == VK_RIGHT) { gJ[g_selJoint][g_selAxis] += g_rigStep; break; }
			if (wParam == VK_UP) { gJ[g_selJoint][g_selAxis] += g_rigStep; break; }
			if (wParam == VK_DOWN) { gJ[g_selJoint][g_selAxis] -= g_rigStep; break; }

			// Step size
			if (wParam == VK_OEM_MINUS) { g_rigStep = max(0.1f, g_rigStep - 0.5f); break; }
			if (wParam == VK_OEM_PLUS) { g_rigStep += 0.5f; break; }

			// Reset: current axis, current joint, or all
			if (wParam == '8') { gJ[g_selJoint][g_selAxis] = 0.f; break; }      // reset axis
			if (wParam == '9') { gJ[g_selJoint][0] = gJ[g_selJoint][1] = gJ[g_selJoint][2] = 0.f; break; }
			if (wParam == '0') {
				for (int j = 0; j < J_MAX; ++j) gJ[j][0] = gJ[j][1] = gJ[j][2] = 0.f;
				break;
			}
		}

		// ---- Light position (CAMERA mode only) ----
		if (g_mode == MODE_CAM) {
			if (wParam == 'T') { lightY += lightMoveSpeed; break; }
			if (wParam == 'G') { lightY -= lightMoveSpeed; break; }
			if (wParam == 'F') { lightX -= lightMoveSpeed; break; }
			if (wParam == 'H') { lightX += lightMoveSpeed; break; }
			if (wParam == 'R') { lightZ -= lightMoveSpeed; break; }
			if (wParam == 'Y') { lightZ += lightMoveSpeed; break; }
		}

		// Toggle fill light (GL_LIGHT1)
		if (wParam == 'L') { fillOn = !fillOn; break; }

		// Ambient down/up
		if (wParam == 'J') { ambientLevel = max(0.0f, ambientLevel - 0.05f); break; }
		if (wParam == 'K') { ambientLevel = min(1.0f, ambientLevel + 0.05f); break; }


		if (wParam == VK_OEM_PLUS) { fillIntensity = min(2.0f, fillIntensity + 0.05f); break; }
		if (wParam == VK_OEM_MINUS) { fillIntensity = max(0.0f, fillIntensity - 0.05f); break; }
		if (wParam == 'C') { // cycle fill color: white -> warm -> cool
			static int idx = 0;
			const float presets[][3] = { {1,1,1}, {1.0f,0.85f,0.65f}, {0.75f,0.85f,1.0f} };
			idx = (idx + 1) % 3;
			fillColor[0] = presets[idx][0]; fillColor[1] = presets[idx][1]; fillColor[2] = presets[idx][2];
			break;
		}

		// Toggle and tweak the visible suns
		if (wParam == 'B') { showSuns = !showSuns; break; }          // show/hide
		if (wParam == 'N') { sunSizeFill = max(0.1f, sunSizeFill - 0.1f); break; } // smaller fill sun
		if (wParam == 'M') { sunSizeFill += 0.1f; break; }                          // bigger  fill sun


		// ---- Arrow keys: set key state true (both modes use them) ----
		// ---- Arrow keys for CAMERA/CHAR only (rig mode consumes them above) ----
		if (g_mode != MODE_RIG) {
			if (wParam == VK_UP) { g_keyUp = true;    break; }
			if (wParam == VK_DOWN) { g_keyDown = true;  break; }
			if (wParam == VK_LEFT) { g_keyLeft = true;  break; }
			if (wParam == VK_RIGHT) { g_keyRight = true; break; }
		}


		break;
	}
	case WM_KEYUP: {
		if (g_mode != MODE_RIG) {
			if (wParam == VK_UP)    g_keyUp = false;
			if (wParam == VK_DOWN)  g_keyDown = false;
			if (wParam == VK_LEFT)  g_keyLeft = false;
			if (wParam == VK_RIGHT) g_keyRight = false;
		}
		break;
	}

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

// Build an OpenGL bitmap font from the current HDC
void BuildFont(HDC hdc, int height = 16, const char* face = "Consolas") {
	if (g_fontBase) return; // already built
	g_fontBase = glGenLists(96);

	// Create a Windows font
	g_font = CreateFontA(
		height, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
		FF_DONTCARE | DEFAULT_PITCH, face
	);

	SelectObject(hdc, g_font);
	// Build display lists for ASCII 32..127
	wglUseFontBitmapsA(hdc, 32, 96, g_fontBase);
}

// Clean up font (optional)
void KillFont() {
	if (g_fontBase) {
		glDeleteLists(g_fontBase, 96);
		g_fontBase = 0;
	}
	if (g_font) {
		DeleteObject(g_font);
		g_font = nullptr;
	}
}

// Print text at current raster pos
void glPrint(const char* fmt, ...) {
	if (!g_fontBase) return;
	char text[512];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(text, sizeof(text), fmt, ap);
	va_end(ap);

	glPushAttrib(GL_LIST_BIT);
	glListBase(g_fontBase - 32);
	glCallLists((GLsizei)strlen(text), GL_UNSIGNED_BYTE, text);
	glPopAttrib();
}

// 2D overlay HUD. Call this at the END of your 3D render (in demo()).
void drawHUD() {
	if (!g_hWnd) return;

	RECT rc; GetClientRect(g_hWnd, &rc);
	int w = rc.right - rc.left;
	int h = rc.bottom - rc.top;

	// Save states
	glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT | GL_COLOR_BUFFER_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	// Setup orthographic 2D
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, w, 0, h, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// Text color
	glColor3f(1.f, 1.f, 1.f);

	// Top-left block
	glRasterPos2i(10, h - 20);
	const char* modeName =
		(g_mode == MODE_CAM) ? "CAMERA" :
		(g_mode == MODE_CHAR) ? "CHARACTER" : "RIG";
	glPrint("Mode: %s", modeName);

	glRasterPos2i(10, h - 40);
	glPrint("Cam Pos: (%.2f, %.2f, %.2f)  Rot: (%.1f, %.1f, %.1f)",
		camX, camY, camZ, camRotX, camRotY, camRotZ);

	glRasterPos2i(10, h - 60);
	glPrint("Char Pos: (%.2f, %.2f, %.2f)  Angle: %.1f  Walking: %s",
		hx, hy, hz, humanAngle, isWalking ? "YES" : "NO");

	if (g_mode == MODE_CAM) {
		glRasterPos2i(10, h - 80);
		glPrint("Fill light pos: (%.2f, %.2f, %.2f)  Move: F/H=X  T/G=Y  R/Y=Z",
			lightX, lightY, lightZ);

		glRasterPos2i(10, h - 100);
		glPrint("Fill: %s  Intensity: +/- -> %.2f  Tone: C to cycle  Color=(%.2f,%.2f,%.2f)",
			fillOn ? "ON (L toggles)" : "OFF (L toggles)",
			fillIntensity, fillColor[0], fillColor[1], fillColor[2]);

		glRasterPos2i(10, h - 120);
		glPrint("Ambient: %.2f  Adjust: J-/K+", ambientLevel);
	}

	if (g_mode == MODE_RIG) {
		glRasterPos2i(10, h - 80);
		glPrint("RIG: Joint=[%s]  Axis=%c  Angle=%.1f deg  Step=%.1f",
			jointName(g_selJoint), axisName(g_selAxis), gJ[g_selJoint][g_selAxis], g_rigStep);
		glRasterPos2i(10, h - 100);
		glPrint("[/]=joint  ,/.=axis  X/Y/Z=pick axis  Arrows=rotate  -=/+=step  8=reset axis  9=reset joint  0=reset all");
	}

	


	// Restore matrices
	glPopMatrix(); // model
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glPopAttrib();
}


void setWire(bool on) {
	glPolygonMode(GL_FRONT_AND_BACK, on ? GL_LINE : GL_FILL);
}

void handleInputPerFrame() {
	// Mode-specific handling for arrow keys
	if (g_mode == MODE_CAM) {
		// Camera movement with arrows, Z with PageUp/PageDown (kept in WM_KEYDOWN too if you like)
		if (g_keyUp)    camY += camMoveSpeed;
		if (g_keyDown)  camY -= camMoveSpeed;
		if (g_keyLeft)  camX -= camMoveSpeed;
		if (g_keyRight) camX += camMoveSpeed;
		// Z moves are still available via PageUp/PageDown in WM_KEYDOWN
	}
	else if (g_mode == MODE_CHAR) {
		float dx = 0.f, dz = 0.f;
		if (g_keyUp)    dz -= 1.f;   // forward (negative Z in your world)
		if (g_keyDown)  dz += 1.f;   // backward
		if (g_keyLeft)  dx -= 1.f;   // left
		if (g_keyRight) dx += 1.f;   // right

		if (dx != 0.f || dz != 0.f) {
			// normalise diagonal movement
			float len = sqrtf(dx * dx + dz * dz);
			dx /= len; dz /= len;

			walkDirX = dx;
			walkDirZ = dz;
			isWalking = true;  // walk while any arrow key is held
		}
		else {
			// no arrows held
			isWalking = false;
		}
	}
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

// --- Helper: dome (hemisphere) ---
void drawHemisphere(float r, int slices = 30, int stacks = 15) {
	GLUquadric* quad = gluNewQuadric();
	gluQuadricNormals(quad, GLU_SMOOTH);
	for (int i = 0; i < stacks; i++) {
		float phi1 = (M_PI / 2.0f) * i / stacks;
		float phi2 = (M_PI / 2.0f) * (i + 1) / stacks;
		glBegin(GL_TRIANGLE_STRIP);
		for (int j = 0; j <= slices; j++) {
			float theta = 2.0f * M_PI * j / slices;
			float x1 = cos(theta) * cos(phi1);
			float y1 = sin(phi1);
			float z1 = sin(theta) * cos(phi1);
			float x2 = cos(theta) * cos(phi2);
			float y2 = sin(phi2);
			float z2 = sin(theta) * cos(phi2);
			glNormal3f(x1, y1, z1);
			glVertex3f(r * x1, r * y1, r * z1);
			glNormal3f(x2, y2, z2);
			glVertex3f(r * x2, r * y2, r * z2);
		}
		glEnd();
	}
	gluDeleteQuadric(quad);
}

void drawUnlitSphere(float x, float y, float z, float r, float cr, float cg, float cb) {
	glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);
	glColor3f(cr, cg, cb);
	glPushMatrix();
	glTranslatef(x, y, z);
	drawSphere(r);
	glPopMatrix();
	glPopAttrib();
}

void drawLine(const float a[3], const float b[3]) {
	glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_LINES);
	glVertex3f(a[0], a[1], a[2]);
	glVertex3f(b[0], b[1], b[2]);
	glEnd();
	glPopAttrib();
}


// --- Helper: feather plume ---
void drawFeather(float h = 1.5f, float w = 0.1f) {
	useCloth(0.9f, 0.1f, 0.1f); // red plume
	glPushMatrix();
	glScalef(w, h, w);
	drawSphere(1.0);  // stretched sphere = feather
	glPopMatrix();
}

// Curved cheek wrap around back & sides of the head
void drawCheekWrap(const HumanDims& d) {
	useMetal();

	// Vertical position of the wrap (a bit below head center)
	const float y = -d.headR * 0.20f;

	// Radius just outside the head to avoid z-fighting with the dome
	const float radius = d.headR * 1.06f;

	// Arc that covers back and both cheeks, leaving the front open
	// Front is +Z in this scene; back is -Z. We leave a frontal gap.
	const float startDeg = 30.f;   // near right-front going toward back
	const float endDeg = 330.f;   // near left-front
	const int   segments = 14;

	// Plate sizing
	const float arcRad = (endDeg - startDeg) * (float)M_PI / 180.f;
	const float segArc = arcRad / segments;
	const float segW = radius * segArc * 1.15f;   // width along tangent
	const float segH = d.headR * 0.85f;          // vertical size
	const float segD0 = d.headR * 0.28f;          // radial thickness (bottom)
	const float segD1 = segD0 * 0.78f;            // slight taper
	const float segW0 = segW * 1.05f;             // slight width taper
	const float segW1 = segW * 0.90f;

	for (int i = 0; i < segments; ++i) {
		float t = (i + 0.5f) / segments;                       // center of segment
		float thetaDeg = startDeg + (endDeg - startDeg) * t;
		float theta = thetaDeg * (float)M_PI / 180.f;

		// Circle on XZ plane, Y up. Use (x=sin, z=cos) so theta=0 points to +Z (front).
		float cx = radius * sinf(theta);
		float cz = radius * cosf(theta);

		glPushMatrix();
		// Place each segment around the head ring
		glTranslatef(cx, y, cz);

		// Face each plate outward (local +Z points outward after this rotate)
		glRotatef(thetaDeg, 0.f, 1.f, 0.f);

		// Add a tiny downward cant so plates "hug" the cheekline
		glRotatef(8.f, 1.f, 0.f, 0.f);

		// Slight outward nudge to ensure no z-fighting with dome
		glTranslatef(0.f, 0.f, Z_EPS);

		// Trapezoid whose depth is radial; width runs along tangent
		drawTrapezoidBlock(
			/*bottomW*/ segW0, /*topW*/ segW1,
			/*h*/       segH,
			/*bottomD*/ segD0, /*topD*/ segD1
		);
		glPopMatrix();
	}
}


// --- New Helmet ---
void drawHelmet(const HumanDims& d) {
	// same neck pivot as head
	const float neckTopY = d.pelvisH * 0.5f + d.chestH + d.neckH;

	glPushMatrix();
	glTranslatef(0.f, neckTopY, 0.f);          // neck pivot
	if (g_mode == MODE_RIG) applyJointRot(J_NECK); // inherit head rotation
	glTranslatef(0.f, d.headR, 0.f);           // now we're at head center

	useMetal();

	// ---- Dome geometry (relative to head center) ----
	const float domeR = d.headR * 1.20f;    // hemisphere radius
	const float domeBaseY = d.headR * 0.20f;    // base ring above head center
	const float domeTopY = domeBaseY + domeR;

	// Dome cap (hemisphere sits above base ring)
	glPushMatrix();
	glTranslatef(0.f, domeBaseY, 0.f);
	drawHemisphere(domeR);
	glPopMatrix();

	// Cheek wrap (expects origin at head center)
	drawCheekWrap(d);

	// Rear neck guard (designed around head center)
	glPushMatrix();
	glTranslatef(0.f, -d.headR * 1.0f, -d.headR * 0.6f);
	glRotatef(-20.f, 1, 0, 0);
	drawTrapezoidBlock(d.headR * 2.0f, d.headR * 1.4f, d.headR * 0.6f,
		d.headR * 0.5f, d.headR * 0.3f);
	glPopMatrix();

	// Center plume + socket (relative to dome top)
	{
		const float sockH = d.headR * 1.f;
		const float socketBaseY = domeTopY - sockH * 0.05f; // tiny sink

		const float plumeH = d.headR * 1.f;
		const float plumeR0 = d.headR * 0.12f;
		const float tiltBackDeg = -10.f;

		glPushMatrix();
		glTranslatef(0.f, socketBaseY, 0.f);
		glRotatef(tiltBackDeg, 1.f, 0.f, 0.f);
		glTranslatef(0.f, sockH, 0.f);
		drawFeather(/*h*/ plumeH, /*w*/ plumeR0);
		glPopMatrix();
	}

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
	glTranslatef(0.f, -d.lowerLegL * 0.35f, 0.f);
	drawYCylinder(d.legLimbR * 1.15f, d.legLimbR * 1.25f, d.lowerLegL * 0.8f);
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

void drawSabaton(const HumanDims& d) {
	useMetal();
	glPushMatrix();
	glTranslatef(0.f, 0.f, d.footD * 0.35f);
	drawTrapezoidBlock(d.footW * 1.1f, d.footW * 1.1f,
		d.footH * 1.2f, d.footD * 1.0f, d.footD * 1.1f);
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
	useNeutral();
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
	useNeutral();
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
	useSkin();
	drawLimbSegment(d.neckR, d.neckR, d.neckH);
	glPopMatrix();
}

void drawEyebrows(const HumanDims& d) {
	const float R = d.headR;
	const float eyeOffX = R * 0.32f;
	const float eyeOffY = R * 0.10f;
	const float eyeOffZ = R * 0.85f;
	const float eyeR = R * 0.12f;

	// eyebrow size and placement
	const float browW = R * 0.36f;
	const float browH = R * 0.06f;
	const float browD = R * 0.10f;
	const float browY = eyeOffY + eyeR * 0.65f;          // a bit above eye
	const float browZ = eyeOffZ + eyeR + Z_EPS * 2.f;    // on skin surface

	// dark “hair” material
	GLfloat diff[4] = { 0.10f, 0.10f, 0.10f, 1.f };
	GLfloat spec[4] = { 0.05f, 0.05f, 0.05f, 1.f };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, diff);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 6.f);

	// Left brow: slight downward tilt toward nose
	glPushMatrix();
	glTranslatef(-eyeOffX + R * 0.03f, browY, browZ);
	glRotatef(-8.f, 0.f, 0.f, 1.f);  // angle toward nose
	glRotatef(-5.f, 1.f, 0.f, 0.f);  // small pitch
	drawBlock(browW, browH, browD);
	glPopMatrix();

	// Right brow: mirror tilt
	glPushMatrix();
	glTranslatef(+eyeOffX - R * 0.03f, browY, browZ);
	glRotatef(+8.f, 0.f, 0.f, 1.f);
	glRotatef(-5.f, 1.f, 0.f, 0.f);
	drawBlock(browW, browH, browD);
	glPopMatrix();
}

void drawEyes(const HumanDims& d) {
	const float R = d.headR;
	const float eyeR = R * 0.12f;   // sclera radius
	const float pupilR = R * 0.04f;   // pupil radius
	const float eyeOffX = R * 0.32f;   // L/R spacing
	const float eyeOffY = R * 0.10f;   // a bit above center
	const float eyeOffZ = R * 0.85f;   // front of head (+Z)
	const float surfaceZ = eyeOffZ + eyeR; // eye surface along +Z

	// --- sclera (actually white this time) ---
	{
		GLfloat diff[4] = { 0.98f, 0.98f, 0.98f, 1.f };
		GLfloat spec[4] = { 0.15f, 0.15f, 0.15f, 1.f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, diff);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 24.f);
	}

	// left sclera
	glPushMatrix();
	glTranslatef(-eyeOffX, eyeOffY, eyeOffZ);
	drawSphere(eyeR);
	glPopMatrix();

	// right sclera
	glPushMatrix();
	glTranslatef(+eyeOffX, eyeOffY, eyeOffZ);
	drawSphere(eyeR);
	glPopMatrix();

	// --- pupil material ---
	{
		GLfloat diff[4] = { 0.04f, 0.04f, 0.04f, 1.f };
		GLfloat spec[4] = { 0.07f, 0.07f, 0.07f, 1.f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, diff);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 8.f);
	}

	// Pupils: place ON the eye surface (eyeR forward) and flatten along Z
	const float pupilLift = Z_EPS;  // avoid z-fight with sclera
	const float pupilFlattenZ = 0.2f;     // 0.2 = “disc-like”

	// left pupil
	glPushMatrix();
	glTranslatef(-eyeOffX, eyeOffY, surfaceZ + pupilLift);
	glScalef(1.f, 1.f, pupilFlattenZ);
	drawSphere(pupilR);
	glPopMatrix();

	// right pupil
	glPushMatrix();
	glTranslatef(+eyeOffX, eyeOffY, surfaceZ + pupilLift);
	glScalef(1.f, 1.f, pupilFlattenZ);
	drawSphere(pupilR);
	glPopMatrix();
}


void drawHead(const HumanDims& d) {
	// pivot at the neck top (base of the head)
	const float neckTopY = d.pelvisH * 0.5f + d.chestH + d.neckH;

	glPushMatrix();
	glTranslatef(0.f, neckTopY, 0.f);          // move to neck pivot
	if (g_mode == MODE_RIG) applyJointRot(J_NECK); // rotate around neck
	glTranslatef(0.f, d.headR, 0.f);           // move up to head center

	useSkin();
	drawSphere(d.headR);                       // draw head centered at origin
	drawEyebrows(d);
	drawEyes(d);
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
	if (g_mode == MODE_RIG) {
		// shoulder: full XYZ
		applyJointRot(left ? J_SHOULDER_L : J_SHOULDER_R);
	}
	else {
		glRotatef(shoulder_rx_deg, 1.f, 0.f, 0.f);
	}
	if (g_armorOn) drawUpperArmPlate(d);
	useSkin();
	drawLimbSegment(d.armLimbR, d.armLimbR, d.upperArmL);

	// forearm + hand
	glTranslatef(0.f, -d.upperArmL, 0.f);
	if (g_mode == MODE_RIG) {
		// elbow: hinge (use X only)
		glRotatef(gJ[left ? J_ELBOW_L : J_ELBOW_R][0], 1, 0, 0);
	}
	else {
		glRotatef(elbow_rx_deg, 1.f, 0.f, 0.f);
	}

	// NEW: forearm bracer that follows elbow rotation
	if (g_armorOn) drawForearmBracer(d);
	useSkin();
	drawLimbSegment(d.armLimbR, d.armLimbR, d.forearmL);
	glTranslatef(0.f, -(d.forearmL), 0.f);
	// keep the hand centered as before (you had +handH*0.5f later for the block)
	if (g_mode == MODE_RIG) {
		applyJointRot(left ? J_WRIST_L : J_WRIST_R);
	}
	glTranslatef(0.f, -(d.handH * 0.5f), 0.f);
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

	if (g_mode == MODE_RIG) {
		applyJointRot(left ? J_HIP_L : J_HIP_R);    // full XYZ
	}
	else {
		glRotatef(-hip_rx_deg, 1.f, 0.f, 0.f);
	}
	useNeutral();
	drawLimbSegment(d.legLimbR, d.legLimbR, d.upperLegL);

	// knee
	glTranslatef(0.f, -d.upperLegL, 0.f);
	if (g_mode == MODE_RIG) {
		glRotatef(gJ[left ? J_KNEE_L : J_KNEE_R][0], 1, 0, 0);  // hinge X
	}
	else {
		glRotatef(-knee_rx_deg, 1.f, 0.f, 0.f);
	}

	// NEW: greave
	if (g_armorOn) drawShinGreave(d);
	useNeutral();

	// shin
	drawLimbSegment(d.legLimbR, d.legLimbR, d.lowerLegL);

	// move to ankle joint (top of foot), apply ankle, then place the foot block
	glTranslatef(0.f, -d.lowerLegL, 0.f);
	if (g_mode == MODE_RIG) {
		applyJointRot(left ? J_ANKLE_L : J_ANKLE_R); // full XYZ
	}

	// foot (drop to foot center and push slightly forward)
	glTranslatef(0.f, -d.footH * 0.5f, d.footD * 0.25f);
	if (g_armorOn) drawSabaton(d);
	useSkin();
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
	if (g_mode == MODE_RIG) applyJointRot(J_SPINE);

	// swing angles (for walk)
	float swing = walking ? sin(walkPhaseDeg * (float)M_PI / 180.0f) * 30.f : 0.f;

	// These are only used in non-RIG mode; RIG mode ignores them inside drawArm/drawLeg.
	float shL = -swing, elL = 0.f;
	float shR = swing, elR = 0.f;
	float hipL = swing, kneeL = -swing * 0.5f;
	float hipR = -swing, kneeR = swing * 0.5f;


	// Legs
	drawLeg(d, true, hipL, kneeL);   // left leg
	drawLeg(d, false, hipR, kneeR);   // right leg

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

	// Head (slight counter bob if wanted)
	drawHead(d);

	// NEW: helmet+plume
	if (g_armorOn) drawHelmet(d);

	// Arms
	drawArm(d, true, shL, elL);   // left arm
	drawArm(d, false, shR, elR);   // right arm

	glPopMatrix();
}

void lighting() {
	if (isLightOn) glEnable(GL_LIGHTING); else glDisable(GL_LIGHTING);

	// global ambient
	GLfloat globalAmb[4] = { ambientLevel, ambientLevel, ambientLevel, 1.0f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmb);

	// FILL (point)
	GLfloat fillPos[4] = { lightX, lightY, lightZ, 1.0f };
	GLfloat fillCol[4] = {
		fillColor[0] * fillIntensity,
		fillColor[1] * fillIntensity,
		fillColor[2] * fillIntensity, 1.0f
	};

	if (fillOn) {
		glEnable(GL_LIGHT1);
		glLightfv(GL_LIGHT1, GL_POSITION, fillPos);
		glLightfv(GL_LIGHT1, GL_DIFFUSE, fillCol);
		glLightfv(GL_LIGHT1, GL_SPECULAR, fillCol);
	}
	else {
		glDisable(GL_LIGHT1);
	}
}

void demo() {
	// Per-frame keyboard handling for modes
	handleInputPerFrame();

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

	// --- Visible light sources ("suns") ---
	if (showSuns) {
		// Fill-light sun at the actual point-light position (GL_LIGHT1)
		{
			float a[3] = { 0.f, 0.f, 0.f };
			float b[3] = { lightX, lightY, lightZ };
			drawLine(a, b); // line from origin to fill light (optional)
			// bright-ish color derived from your fill color
			drawUnlitSphere(lightX, lightY, lightZ, sunSizeFill,
				fillColor[0], fillColor[1], fillColor[2]);
		}
	}

	drawGround();
	HumanDims d;
	glPushMatrix();
	glTranslatef(hx, hy, hz);
	glRotatef(humanAngle, 0.f, 1.f, 0.f);
	drawHuman(d, walkPhase, isWalking);
	glPopMatrix();

	drawHUD();
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

	g_hWnd = hWnd;

	HDC hdc = GetDC(hWnd);

	//	initialize pixel format for the window
	initPixelFormat(hdc);

	//	get an openGL context
	HGLRC hglrc = wglCreateContext(hdc);

	//	make context current
	if (!wglMakeCurrent(hdc, hglrc)) return false;

	BuildFont(hdc, 18, "Consolas"); // or "Courier New" if you prefer

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

	KillFont();
	return true;
}
//--------------------------------------------------------------------