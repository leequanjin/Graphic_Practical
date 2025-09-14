#define _USE_MATH_DEFINES
#include <math.h>
#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <stdio.h>
#include <stdint.h>

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
GLuint gTexBrownFabric = 0, gTexClothFabric = 0, gTexMetalLayered = 0, gTexRoofTile = 0, gTexShinyBlack = 0, gTexShinyMetal = 0, gTexWood = 0, gTexGold = 0;

int qNo = 1;

static const float GRID_HALF = 20.0f;

int noOfSides = 30; // for circle and cylinder

// --- Texture toggles/presets ---
bool g_texturesEnabled = true;   // V to toggle on/off
int  g_texPreset = 0;            // I to cycle
const int TEX_PRESETS = 4;

// What kind of texture are we asking for at each draw call?
enum TexSlot { TEX_WOOD = 0, TEX_METAL, TEX_SHINY, TEX_CLOTH, TEX_LEATHER, TEX_GOLD, TEX_SILVER, TEX_MAX
};

// Mapping: preset -> slot -> GL texture id
GLuint gTexPreset[TEX_PRESETS][TEX_MAX] = {};

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
const float CAM_Z_MIN = 5.0f;
const float CAM_Z_MAX = 50.0f;
bool isOrtho = false, isPerspective = true;

float camX = 0.f, camY = 3.f, camZ = 20.f; // camera position
float camRotX = 10.f, camRotY = -20.f, camRotZ = 0.f; // camera rotation
float camMoveSpeed = 0.2f;
float camRotSpeed = 2.0f;

// human position
float hx = 0.f, hy = 0.f, hz = 0.f;
float walkAnimSpeed = 3.f; // speed of swing
float humanSpeed = 0.04f; // speed of movement

// walk cycle
float walkPhase = 0.f;       // angle in degrees
bool isWalking = false;      // walking state
float walkDirX = 0.f, walkDirZ = 0.f; // direction
float humanAngle = 0.f; // facing direction in degrees

// Attack animation
bool  g_attackActive = false;
float g_attackT = 0.0f;   // 0..1 normalized time
float g_attackSpeed = 0.01f;  // advance per frame (tweak speed)


// ----- armor toggle (press U to show/hide) -----
bool g_armorOn = true;

int weaponType = 1;

// small offset so plates don't z-fight with the body
const float Z_EPS = 0.01f;

BITMAP BMP;				//bitmap structure
HBITMAP hBMP = NULL;	//bitmap handle

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

// --- Primitive tally (per frame) ---
struct PrimTally {
	long long points = 0, triangles = 0, tristrip = 0, trifan = 0,
		quads = 0, quadstrip = 0, polygon = 0, lines = 0, linestrip = 0, lineloop = 0;
} gPrim;

static inline void ResetPrimTally() { gPrim = PrimTally(); }


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

	float handH = unit * 0.5f;
	float handD = handH * 0.8;
	float handW = handH * 0.8f;

	// legs + feet = 4 (4/8 of total)
	float upperLegL = unit * 2.0f;
	float lowerLegL = unit * 1.6f;

	float footH = unit * 0.2f;
	float footD = footH * 6.f;
	float footW = footH * 2.f;
};

// Arrow-key state (used for camera/character movement per README)
bool g_keyUp = false;
bool g_keyDown = false;
bool g_keyLeft = false;
bool g_keyRight = false;

// Hold-to-run (character); also slightly speeds attack timebase
bool g_kShift = false;

static inline void ResetCameraDefaults() {
	// Camera position/rotation defaults
	camX = 0.f; camY = 3.f; camZ = 20.f;
	camRotX = 10.f; camRotY = -20.f; camRotZ = 0.f;

	// (Optional) snap projection back to perspective if you want:
	// isOrtho = false; isPerspective = true;
}

static inline void ResetCharacterDefaults() {
	// Character transform
	hx = 0.f; hy = 0.f; hz = 0.f;
	humanAngle = 0.f;

	// Locomotion & animation
	walkPhase = 0.f;
	isWalking = false;
	walkDirX = 0.f; walkDirZ = 0.f;

	// Cancel any attack in progress
	g_attackActive = false;
	g_attackT = 0.f;
}

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
		if (wParam == '1') { g_mode = MODE_CAM;  if (g_hWnd) SetWindowTextA(g_hWnd, "OpenGL Window  [Mode: CAMERA]"); break; }
		if (wParam == '2') { g_mode = MODE_CHAR; if (g_hWnd) SetWindowTextA(g_hWnd, "OpenGL Window  [Mode: CHARACTER]"); break; }
		if (wParam == '3') { g_mode = MODE_RIG;  if (g_hWnd) SetWindowTextA(g_hWnd, "OpenGL Window  [Mode: RIG]"); break; }

		// ---- Projection ----
		if (wParam == 'O') { isOrtho = true;  isPerspective = false; break; }
		if (wParam == 'P') { isOrtho = false; isPerspective = true;  break; }

		// ---- Camera rotation (always available) ----
		if (wParam == 'W') { camRotX += camRotSpeed; break; }
		if (wParam == 'S') { camRotX -= camRotSpeed; break; }
		if (wParam == 'A') { camRotY += camRotSpeed; break; }
		if (wParam == 'D') { camRotY -= camRotSpeed; break; }
		if (wParam == 'Q') { camRotZ += camRotSpeed; break; }
		if (wParam == 'E') { camRotZ -= camRotSpeed; break; }
		if (wParam == VK_HOME) { ResetCameraDefaults(); break; }

		// ---- Global toggles ----
		if (wParam == 'U') { g_armorOn = !g_armorOn; break; }
		if (wParam == VK_TAB) { weaponType = 1 - weaponType; break; }
		if (wParam == 'V') { g_texturesEnabled = !g_texturesEnabled; break; }
		if (wParam == 'I') { g_texPreset = (g_texPreset + 1) % TEX_PRESETS; break; }

		// ---- PageUp/PageDown -> camera dolly Z ----
		if (wParam == VK_PRIOR) {        // PageUp
			camZ -= camMoveSpeed * 5;
			if (camZ < CAM_Z_MIN) camZ = CAM_Z_MIN;
			break;
		}
		if (wParam == VK_NEXT) {         // PageDown
			camZ += camMoveSpeed * 5;
			if (camZ > CAM_Z_MAX) camZ = CAM_Z_MAX;
			break;
		}

		// ---- Character actions ----
		if (g_mode == MODE_CHAR) {
			if (wParam == VK_SPACE) { ResetCharacterDefaults(); break; }
			if (wParam == 'F') { if (!g_attackActive) { g_attackActive = true; g_attackT = 0.0f; } break; }
		}

		// ---- Lighting (global toggles) ----
		if (wParam == 'L') { fillOn = !fillOn; break; }
		if (wParam == 'J') { ambientLevel = max(0.0f, ambientLevel - 0.05f); break; }
		if (wParam == 'K') { ambientLevel = min(1.0f, ambientLevel + 0.05f); break; }
		if (wParam == 'C') {
			static int idx = 0;
			const float presets[][3] = { {1,1,1}, {1.0f,0.85f,0.65f}, {0.75f,0.85f,1.0f} };
			idx = (idx + 1) % 3;
			fillColor[0] = presets[idx][0]; fillColor[1] = presets[idx][1]; fillColor[2] = presets[idx][2];
			break;
		}
		if (wParam == 'B') { showSuns = !showSuns; break; }
		if (wParam == 'N') { sunSizeFill = max(0.1f, sunSizeFill - 0.1f); break; }
		if (wParam == 'M') { sunSizeFill += 0.1f; break; }

		// ---- Camera-only light positioning & intensity ----
		if (g_mode == MODE_CAM) {
			if (wParam == 'T') { lightY += lightMoveSpeed; break; }
			if (wParam == 'G') { lightY -= lightMoveSpeed; break; }
			if (wParam == 'F') { lightX -= lightMoveSpeed; break; }
			if (wParam == 'H') { lightX += lightMoveSpeed; break; }
			if (wParam == 'R') { lightZ -= lightMoveSpeed; break; }
			if (wParam == 'Y') { lightZ += lightMoveSpeed; break; }

			// scope +/- to camera mode to avoid conflict with rig step
			if (wParam == VK_OEM_PLUS) { fillIntensity = min(2.0f, fillIntensity + 0.05f); break; }
			if (wParam == VK_OEM_MINUS) { fillIntensity = max(0.0f, fillIntensity - 0.05f); break; }
		}

		// ---- RIG: selection, rotation, and step size ----
		if (g_mode == MODE_RIG) {
			// selection
			if (wParam == VK_OEM_4) { g_selJoint = (g_selJoint - 1 + J_MAX) % J_MAX; break; } // [
			if (wParam == VK_OEM_6) { g_selJoint = (g_selJoint + 1) % J_MAX; break; }        // ]
			if (wParam == 'X') { g_selAxis = 0; break; }
			if (wParam == 'Y') { g_selAxis = 1; break; }
			if (wParam == 'Z') { g_selAxis = 2; break; }
			if (wParam == VK_OEM_COMMA) { g_selAxis = (g_selAxis + 2) % 3; break; }  // ,
			if (wParam == VK_OEM_PERIOD) { g_selAxis = (g_selAxis + 1) % 3; break; }  // .

			// rotate selected axis
			if (wParam == VK_LEFT || wParam == VK_DOWN) { gJ[g_selJoint][g_selAxis] -= g_rigStep; break; }
			if (wParam == VK_RIGHT || wParam == VK_UP) { gJ[g_selJoint][g_selAxis] += g_rigStep; break; }

			// step size (scoped to rig)
			if (wParam == VK_OEM_MINUS) { g_rigStep = max(0.1f, g_rigStep - 0.5f); break; }
			if (wParam == VK_OEM_PLUS) { g_rigStep += 0.5f; break; }

			// reset
			if (wParam == '8') { gJ[g_selJoint][g_selAxis] = 0.f; break; }
			if (wParam == '9') { gJ[g_selJoint][0] = gJ[g_selJoint][1] = gJ[g_selJoint][2] = 0.f; break; }
			if (wParam == '0') { for (int j = 0; j < J_MAX; ++j) gJ[j][0] = gJ[j][1] = gJ[j][2] = 0.f; break; }
		}

		// ---- Arrow keys as stateful movement in CAM/CHAR ----
		if (g_mode != MODE_RIG) {
			if (wParam == VK_UP) { g_keyUp = true;    break; }
			if (wParam == VK_DOWN) { g_keyDown = true;  break; }
			if (wParam == VK_LEFT) { g_keyLeft = true;  break; }
			if (wParam == VK_RIGHT) { g_keyRight = true; break; }
			if (wParam == VK_SHIFT) { g_kShift = true; break; }
		}

		break;
	}
	case WM_KEYUP: {
		// release stateful arrows in CAM/CHAR
		if (g_mode != MODE_RIG) {
			if (wParam == VK_UP)    g_keyUp = false;
			if (wParam == VK_DOWN)  g_keyDown = false;
			if (wParam == VK_LEFT)  g_keyLeft = false;
			if (wParam == VK_RIGHT) g_keyRight = false;
			if (wParam == VK_SHIFT) g_kShift = false;
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

// Tiny BMP loader using WinAPI + GLU (expects 24-bit or 32-bit DIBs)
static GLuint LoadTextureBMP(LPCSTR filename) {
	BITMAP bmp = {};
	HBITMAP hBMP = (HBITMAP)LoadImageA(GetModuleHandle(NULL),
		filename, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);

	GLuint tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	// sane defaults
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	if (!hBMP) {
		// pink checker fallback (so missing files are obvious)
		const GLubyte fallback[4 * 3] = {
			255,105,180,  0,0,0,
			0,0,0,        255,105,180
		};
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0,
			GL_RGB, GL_UNSIGNED_BYTE, fallback);
		return tex;
	}

	GetObject(hBMP, sizeof(bmp), &bmp);
	// BMP pixels come as BGR in DIB sections
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB,
		bmp.bmWidth, bmp.bmHeight,
		GL_BGR_EXT, GL_UNSIGNED_BYTE, bmp.bmBits);

	DeleteObject(hBMP);
	return tex;
}

inline void UseSlot(TexSlot s) {
	if (!g_texturesEnabled) { glBindTexture(GL_TEXTURE_2D, 0); return; }
	glBindTexture(GL_TEXTURE_2D, gTexPreset[g_texPreset][s]);
}


static void LoadAllTextures() {
	glEnable(GL_TEXTURE_2D);

	gTexBrownFabric = LoadTextureBMP("brown_fabric.bmp");
	gTexClothFabric = LoadTextureBMP("cloth_fabric.bmp");
	gTexMetalLayered = LoadTextureBMP("metal_layered.bmp");
	gTexRoofTile = LoadTextureBMP("roof_tile.bmp");
	gTexShinyBlack = LoadTextureBMP("shiny_black.bmp");
	gTexShinyMetal = LoadTextureBMP("shiny_metal.bmp");
	gTexWood = LoadTextureBMP("wood.bmp");
	gTexGold = LoadTextureBMP("gold.bmp");

	// --- Preset 0: default ---
	gTexPreset[0][TEX_WOOD] = gTexWood;
	gTexPreset[0][TEX_METAL] = gTexMetalLayered;
	gTexPreset[0][TEX_SHINY] = gTexShinyMetal;
	gTexPreset[0][TEX_CLOTH] = gTexClothFabric;
	gTexPreset[0][TEX_LEATHER] = gTexBrownFabric;
	gTexPreset[0][TEX_GOLD] = gTexGold;
	gTexPreset[0][TEX_SILVER] = gTexShinyMetal;

	// --- Preset 1: alt look (darker metal & leather/cloth swapped) ---
	gTexPreset[1][TEX_WOOD] = gTexWood;
	gTexPreset[1][TEX_METAL] = gTexRoofTile;   // dark armor
	gTexPreset[1][TEX_SHINY] = gTexShinyBlack;   // keep shiny for accents
	gTexPreset[1][TEX_CLOTH] = gTexBrownFabric;  // cloth uses brown
	gTexPreset[1][TEX_LEATHER] = gTexClothFabric;  // leather uses cloth pattern
	gTexPreset[1][TEX_GOLD] = gTexGold;
	gTexPreset[1][TEX_SILVER] = gTexShinyMetal;

	// --- Preset 2: alt look (gold metal) ---
	gTexPreset[2][TEX_WOOD] = gTexWood;
	gTexPreset[2][TEX_METAL] = gTexMetalLayered;
	gTexPreset[2][TEX_SHINY] = gTexGold;
	gTexPreset[2][TEX_CLOTH] = gTexClothFabric;
	gTexPreset[2][TEX_LEATHER] = gTexBrownFabric;
	gTexPreset[2][TEX_GOLD] = gTexShinyMetal;
	gTexPreset[2][TEX_SILVER] = gTexShinyMetal;

	// --- Preset 3: alt look (gold metal) ---
	gTexPreset[3][TEX_WOOD] = gTexWood;
	gTexPreset[3][TEX_METAL] = gTexRoofTile;
	gTexPreset[3][TEX_SHINY] = gTexGold;
	gTexPreset[3][TEX_CLOTH] = gTexBrownFabric;
	gTexPreset[3][TEX_LEATHER] = gTexClothFabric;
	gTexPreset[3][TEX_GOLD] = gTexShinyBlack;
	gTexPreset[3][TEX_SILVER] = gTexShinyMetal;
}

static inline void BindTex(GLuint tex) {
	glBindTexture(GL_TEXTURE_2D, tex);
}

static inline void UnbindTex() {
	glBindTexture(GL_TEXTURE_2D, 0);
}

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
	const int w = rc.right - rc.left;
	const int h = rc.bottom - rc.top;

	glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT | GL_COLOR_BUFFER_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, w, 0, h, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glColor3f(1.f, 1.f, 1.f);

	int y = h - 20;
	auto row = [&](const char* s) {
		glRasterPos2i(10, y);
		glPrint("%s", s);
		y -= 18;
		};
	auto gap = [&]() { y -= 8; };

	// ===== Header =====
	{
		const char* modeName =
			(g_mode == MODE_CAM) ? "CAMERA" :
			(g_mode == MODE_CHAR) ? "CHARACTER" : "RIG";
		glRasterPos2i(10, y);
		glPrint("Mode: %s", modeName);
		y -= 18;

		glRasterPos2i(10, y);
		glPrint("Cam Pos: (%.2f, %.2f, %.2f)  Rot: (%.1f, %.1f, %.1f)",
			camX, camY, camZ, camRotX, camRotY, camRotZ);
		y -= 18;

		glRasterPos2i(10, y);
		glPrint("Char Pos: (%.2f, %.2f, %.2f)  Angle: %.1f  Walking: %s",
			hx, hy, hz, humanAngle, isWalking ? "YES" : "NO");
		y -= 18;
		gap();

		// --- RIG mode readout (selected joint/axis/step + angles) ---
		if (g_mode == MODE_RIG) {
			const int j = g_selJoint;
			const int a = g_selAxis;
			row("RIG:");
			glRasterPos2i(20, y);
			glPrint("Selected  Joint: %s   Axis: %c   Step: %.1f deg",
				jointName(j), axisName(a), g_rigStep);
			y -= 18;

			glRasterPos2i(20, y);
			glPrint("Angles   X: %.1f   Y: %.1f   Z: %.1f",
				gJ[j][0], gJ[j][1], gJ[j][2]);
			y -= 18;
			gap();
		}
	}

	// ===== Textures =====
	glRasterPos2i(10, y);
	glPrint("Textures: %s   Preset: %d",
		g_texturesEnabled ? "ON" : "OFF", g_texPreset);
	y -= 18;
	gap();

	// ===== Primitive Tally =====
	row("Primitives (this frame):");
	glRasterPos2i(10, y);
	glPrint("  Points:%lld  Tris:%lld  TriStrip:%lld  TriFan:%lld",
		gPrim.points, gPrim.triangles, gPrim.tristrip, gPrim.trifan);
	y -= 18;

	glRasterPos2i(10, y);
	glPrint("  Quads:%lld  QuadStrip:%lld  Polygon:%lld",
		gPrim.quads, gPrim.quadstrip, gPrim.polygon);
	y -= 18;

	glRasterPos2i(10, y);
	glPrint("  Lines:%lld  LineStrip:%lld  LineLoop:%lld",
		gPrim.lines, gPrim.linestrip, gPrim.lineloop);
	y -= 18;

	// Restore matrices/state
	glPopMatrix(); // model
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopAttrib();
}

static inline float smooth01(float t) { t = max(0.f, min(1.f, t)); return t * t * (3.f - 2.f * t); }

void setWire(bool on) {
	glPolygonMode(GL_FRONT_AND_BACK, on ? GL_LINE : GL_FILL);
}

void handleInputPerFrame() {
	// ---- CAMERA movement (Arrow keys move position; PageUp/Down handled on keydown) ----
	if (g_mode == MODE_CAM) {
		// Arrow Up/Down => Y ± ; Arrow Left/Right => X ±
		if (g_keyUp)    camY += camMoveSpeed;
		if (g_keyDown)  camY -= camMoveSpeed;
		if (g_keyLeft)  camX -= camMoveSpeed;
		if (g_keyRight) camX += camMoveSpeed;
		// NOTE: PageUp/PageDown (VK_PRIOR/VK_NEXT) already adjust camZ in WM_KEYDOWN.
	}

	// ---- CHARACTER movement (Arrow keys = 8-way walking; SHIFT to run) ----
	if (g_mode == MODE_CHAR) {
		float dx = 0.f, dz = 0.f;
		if (g_keyUp)    dz -= 1.f;
		if (g_keyDown)  dz += 1.f;
		if (g_keyLeft)  dx -= 1.f;
		if (g_keyRight) dx += 1.f;

		if (dx != 0.f || dz != 0.f) {
			float len = sqrtf(dx * dx + dz * dz);
			dx /= len; dz /= len;
			walkDirX = dx; walkDirZ = dz;
			// update facing NOW, even if we later reject the move
			humanAngle = atan2f(dx, dz) * 180.0f / (float)M_PI;
			isWalking = true;
		}
		else {
			isWalking = false;
		}
	}

	// ---- Attack animation timebase (unchanged; SHIFT gives tiny boost) ----
	if (g_mode == MODE_CHAR && g_attackActive) {
		g_attackT += g_attackSpeed * (g_kShift ? 3.0f : 1.0f);
		if (g_attackT >= 1.0f) { g_attackT = 0.0f; g_attackActive = false; }
	}
}


void drawSphere(double r) {
	const int slices = noOfSides;         // 30 by default
	const int stacks = noOfSides;         // keep symmetric; tweak if you like

	// equator at v=0.5, ranges [-pi/2, +pi/2]
	for (int i = 0; i < stacks; ++i) {
		double v0 = (double)i / stacks;
		double v1 = (double)(i + 1) / stacks;
		double phi0 = (M_PI * (v0 - 0.5));    // -pi/2 .. +pi/2
		double phi1 = (M_PI * (v1 - 0.5));

		glBegin(GL_TRIANGLE_STRIP);
		for (int j = 0; j <= slices; ++j) {
			double u = (double)j / slices;       // 0..1 around
			double th = u * (2.0 * M_PI);

			double c0 = cos(phi0), s0 = sin(phi0);
			double c1 = cos(phi1), s1 = sin(phi1);
			double ct = cos(th), st = sin(th);

			// ring i
			double nx0 = ct * c0, ny0 = s0, nz0 = st * c0;
			glNormal3d(nx0, ny0, nz0);
			glTexCoord2d(u, v0);
			glVertex3d(r * nx0, r * ny0, r * nz0);

			// ring i+1
			double nx1 = ct * c1, ny1 = s1, nz1 = st * c1;
			glNormal3d(nx1, ny1, nz1);
			glTexCoord2d(u, v1);
			glVertex3d(r * nx1, r * ny1, r * nz1);
		}
		glEnd();
		gPrim.tristrip += 1;
	}
}

void drawCylinder(double br, double tr, double h) {
	const int slices = noOfSides;

	// ---- side (QUAD STRIP) ----
	glBegin(GL_QUAD_STRIP);
	for (int j = 0; j <= slices; ++j) {          // repeat first ring to close seam
		double u = (double)j / slices;          // 0..1 around
		double th = u * (2.0 * M_PI);
		double ct = cos(th), st = sin(th);

		glNormal3d(ct, 0.0, st);
		glTexCoord2d(u, 0.0);                    // bottom ring
		glVertex3d(br * ct, 0.0, br * st);

		glNormal3d(ct, 0.0, st);
		glTexCoord2d(u, 1.0);                    // top ring
		glVertex3d(tr * ct, h, tr * st);
	}
	glEnd();
	gPrim.quadstrip += 1;

	// ---- bottom cap (fan) ----
	if (br > 0.0) {
		glBegin(GL_TRIANGLE_FAN);
		glNormal3d(0.0, -1.0, 0.0);
		glTexCoord2d(0.5, 0.5);
		glVertex3d(0.0, 0.0, 0.0);                // center
		for (int j = 0; j <= slices; ++j) {
			double u = (double)j / slices;
			double th = (1.0 - u) * (2.0 * M_PI); // CCW when viewed from below
			double ct = cos(th), st = sin(th);
			glTexCoord2d(0.5 + 0.5 * ct, 0.5 + 0.5 * st);
			glVertex3d(br * ct, 0.0, br * st);
		}
		glEnd();
		gPrim.trifan += 1;
	}

	// ---- top cap (fan) ----
	if (tr > 0.0) {
		glBegin(GL_TRIANGLE_FAN);
		glNormal3d(0.0, +1.0, 0.0);
		glTexCoord2d(0.5, 0.5);
		glVertex3d(0.0, h, 0.0);                  // center
		for (int j = 0; j <= slices; ++j) {
			double u = (double)j / slices;
			double th = u * (2.0 * M_PI);         // CCW when viewed from above
			double ct = cos(th), st = sin(th);
			glTexCoord2d(0.5 + 0.5 * ct, 0.5 + 0.5 * st);
			glVertex3d(tr * ct, h, tr * st);
		}
		glEnd();
		gPrim.trifan += 1;
	}
}

void drawYCylinder(double br, double tr, float h) {
	glPushMatrix();
	glRotatef(180.0f, 1.f, 0.f, 0.f);
	drawCylinder(br, tr, h);
	glPopMatrix();
}

void drawBlock(double w, double h, double d) {
	const float TILE = 1.0f;        // increase to repeat more
	double x = w * 0.5, y = h * 0.5, z = d * 0.5;

	glBegin(GL_QUADS);
	// Front (+Z): use X (u), Y (v)
	glNormal3f(0, 0, 1);
	glTexCoord2f(0, 0);         glVertex3f(-x, -y, z);
	glTexCoord2f(TILE, 0);      glVertex3f(x, -y, z);
	glTexCoord2f(TILE, TILE);   glVertex3f(x, y, z);
	glTexCoord2f(0, TILE);      glVertex3f(-x, y, z);

	// Back (-Z): X, Y (flip U to keep same orientation visually)
	glNormal3f(0, 0, -1);
	glTexCoord2f(TILE, 0);      glVertex3f(-x, -y, -z);
	glTexCoord2f(0, 0);         glVertex3f(x, -y, -z);
	glTexCoord2f(0, TILE);      glVertex3f(x, y, -z);
	glTexCoord2f(TILE, TILE);   glVertex3f(-x, y, -z);

	// Left (-X): use Z (u), Y (v)
	glNormal3f(-1, 0, 0);
	glTexCoord2f(0, 0);         glVertex3f(-x, -y, -z);
	glTexCoord2f(TILE, 0);      glVertex3f(-x, -y, z);
	glTexCoord2f(TILE, TILE);   glVertex3f(-x, y, z);
	glTexCoord2f(0, TILE);      glVertex3f(-x, y, -z);

	// Right (+X): Z, Y
	glNormal3f(1, 0, 0);
	glTexCoord2f(0, 0);         glVertex3f(x, -y, z);
	glTexCoord2f(TILE, 0);      glVertex3f(x, -y, -z);
	glTexCoord2f(TILE, TILE);   glVertex3f(x, y, -z);
	glTexCoord2f(0, TILE);      glVertex3f(x, y, z);

	// Top (+Y): X (u), Z (v)
	glNormal3f(0, 1, 0);
	glTexCoord2f(0, 0);         glVertex3f(-x, y, -z);
	glTexCoord2f(TILE, 0);      glVertex3f(x, y, -z);
	glTexCoord2f(TILE, TILE);   glVertex3f(x, y, z);
	glTexCoord2f(0, TILE);      glVertex3f(-x, y, z);

	// Bottom (-Y): X, Z
	glNormal3f(0, -1, 0);
	glTexCoord2f(0, 0);         glVertex3f(-x, -y, -z);
	glTexCoord2f(TILE, 0);      glVertex3f(x, -y, -z);
	glTexCoord2f(TILE, TILE);   glVertex3f(x, -y, z);
	glTexCoord2f(0, TILE);      glVertex3f(-x, -y, z);
	glEnd();

	gPrim.quads += 6;
}

// simple helpers (local)
static inline void nrmFromTri(double ax, double ay, double az,
	double bx, double by, double bz,
	double cx, double cy, double cz)
{
	// n = normalize( (b-a) x (c-a) )
	double ux = bx - ax, uy = by - ay, uz = bz - az;
	double vx = cx - ax, vy = cy - ay, vz = cz - az;
	double nx = uy * vz - uz * vy;
	double ny = uz * vx - ux * vz;
	double nz = ux * vy - uy * vx;
	double len = sqrt(nx * nx + ny * ny + nz * nz);
	if (len > 1e-12) { nx /= len; ny /= len; nz /= len; }
	glNormal3d(nx, ny, nz);
}

// Pyramid with rectangular base (baseW along X, baseD along Z), height h along +Y.
// Centered at origin: base at y=-h/2, apex at y=+h/2.
void drawPyramid(double baseW, double baseD, double h, bool capBase = true)
{
	const double bx = baseW * 0.5;
	const double bz = baseD * 0.5;
	const double y0 = -h * 0.5;  // base plane
	const double y1 = +h * 0.5;  // apex

	// base corners (CCW seen from +Y)
	const double x0 = -bx, x1 = +bx;
	const double z0 = -bz, z1 = +bz;

	// apex
	const double ax = 0.0, ay = y1, az = 0.0;

	// ---- 4 side faces (triangles), CCW seen from outside ----
	glBegin(GL_TRIANGLES);

	// +Z face: (x0,y0,z1) -> (x1,y0,z1) -> apex
	nrmFromTri(x0, y0, z1, x1, y0, z1, ax, ay, az);
	glTexCoord2d(0.0, 0.0); glVertex3d(x0, y0, z1);
	glTexCoord2d(1.0, 0.0); glVertex3d(x1, y0, z1);
	glTexCoord2d(0.5, 1.0); glVertex3d(ax, ay, az);

	// +X face: (x1,y0,z1) -> (x1,y0,z0) -> apex
	nrmFromTri(x1, y0, z1, x1, y0, z0, ax, ay, az);
	glTexCoord2d(0.0, 0.0); glVertex3d(x1, y0, z1);
	glTexCoord2d(1.0, 0.0); glVertex3d(x1, y0, z0);
	glTexCoord2d(0.5, 1.0); glVertex3d(ax, ay, az);

	// -Z face: (x1,y0,z0) -> (x0,y0,z0) -> apex
	nrmFromTri(x1, y0, z0, x0, y0, z0, ax, ay, az);
	glTexCoord2d(0.0, 0.0); glVertex3d(x1, y0, z0);
	glTexCoord2d(1.0, 0.0); glVertex3d(x0, y0, z0);
	glTexCoord2d(0.5, 1.0); glVertex3d(ax, ay, az);

	// -X face: (x0,y0,z0) -> (x0,y0,z1) -> apex
	nrmFromTri(x0, y0, z0, x0, y0, z1, ax, ay, az);
	glTexCoord2d(0.0, 0.0); glVertex3d(x0, y0, z0);
	glTexCoord2d(1.0, 0.0); glVertex3d(x0, y0, z1);
	glTexCoord2d(0.5, 1.0); glVertex3d(ax, ay, az);

	glEnd();

	gPrim.triangles += 4;

	// ---- optional base cap (quad), CCW seen from above ----
	if (capBase) {
		glBegin(GL_QUADS);
		glNormal3d(0.0, -1.0, 0.0);
		glTexCoord2d(0.0, 0.0); glVertex3d(x0, y0, z0);
		glTexCoord2d(1.0, 0.0); glVertex3d(x1, y0, z0);
		glTexCoord2d(1.0, 1.0); glVertex3d(x1, y0, z1);
		glTexCoord2d(0.0, 1.0); glVertex3d(x0, y0, z1);
		glEnd();
		gPrim.quads += 1;
	}
	
}


void drawTrapezoidBlock(double bottomW, double topW, double h, double bottomD, double topD)
{
	double bw = bottomW * 0.5, tw = topW * 0.5;
	double bh = h * 0.5, th = h * 0.5;
	double bd = bottomD * 0.5, td = topD * 0.5;

	glBegin(GL_QUADS);
	// Front (+Z): U across width, V across height
	glNormal3f(0, 0, 1);
	glTexCoord2f(0, 0); glVertex3f(-bw, -bh, bd);
	glTexCoord2f(1, 0); glVertex3f(bw, -bh, bd);
	glTexCoord2f(1, 1); glVertex3f(tw, th, td);
	glTexCoord2f(0, 1); glVertex3f(-tw, th, td);

	// Back (-Z)
	glNormal3f(0, 0, -1);
	glTexCoord2f(0, 0); glVertex3f(-bw, -bh, -bd);
	glTexCoord2f(1, 0); glVertex3f(bw, -bh, -bd);
	glTexCoord2f(1, 1); glVertex3f(tw, th, -td);
	glTexCoord2f(0, 1); glVertex3f(-tw, th, -td);

	// Left (-X): U depth, V height
	{
		// approximate normal already computed earlier – keep simple:
		glNormal3f(-1, 0, 0);
		glTexCoord2f(0, 0); glVertex3f(-bw, -bh, -bd);
		glTexCoord2f(1, 0); glVertex3f(-bw, -bh, bd);
		glTexCoord2f(1, 1); glVertex3f(-tw, th, td);
		glTexCoord2f(0, 1); glVertex3f(-tw, th, -td);
	}

	// Right (+X)
	glNormal3f(1, 0, 0);
	glTexCoord2f(0, 0); glVertex3f(bw, -bh, bd);
	glTexCoord2f(1, 0); glVertex3f(bw, -bh, -bd);
	glTexCoord2f(1, 1); glVertex3f(tw, th, -td);
	glTexCoord2f(0, 1); glVertex3f(tw, th, td);

	// Top (+Y): U across width, V across depth
	glNormal3f(0, 1, 0);
	glTexCoord2f(0, 0); glVertex3f(-tw, th, -td);
	glTexCoord2f(1, 0); glVertex3f(tw, th, -td);
	glTexCoord2f(1, 1); glVertex3f(tw, th, td);
	glTexCoord2f(0, 1); glVertex3f(-tw, th, td);

	// Bottom (-Y): U across width, V across depth
	glNormal3f(0, -1, 0);
	glTexCoord2f(0, 0); glVertex3f(-bw, -bh, -bd);
	glTexCoord2f(1, 0); glVertex3f(bw, -bh, -bd);
	glTexCoord2f(1, 1); glVertex3f(bw, -bh, bd);
	glTexCoord2f(0, 1); glVertex3f(-bw, -bh, bd);
	glEnd();

	gPrim.quads += 6;
}

// --- Helper: dome (hemisphere) ---
void drawHemisphere(float r, int slices = 30, int stacks = 15) {
	for (int i = 0; i < stacks; i++) {
		float v1 = (float)i / stacks;         // 0..1 from equator to top
		float v2 = (float)(i + 1) / stacks;
		float phi1 = (M_PI / 2.f) * v1;
		float phi2 = (M_PI / 2.f) * v2;
		glBegin(GL_TRIANGLE_STRIP);
		for (int j = 0; j <= slices; j++) {
			float u = (float)j / slices;      // 0..1 around
			float th = 2.f * (float)M_PI * u;

			float x1 = cosf(th) * cosf(phi1), y1 = sinf(phi1), z1 = sinf(th) * cosf(phi1);
			float x2 = cosf(th) * cosf(phi2), y2 = sinf(phi2), z2 = sinf(th) * cosf(phi2);

			glNormal3f(x1, y1, z1); glTexCoord2f(u, v1); glVertex3f(r * x1, r * y1, r * z1);
			glNormal3f(x2, y2, z2); glTexCoord2f(u, v2); glVertex3f(r * x2, r * y2, r * z2);
		}
		glEnd();
	}

	gPrim.tristrip += stacks;
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
	gPrim.lines += 1;
}


// --- Helper: feather plume ---
void drawFeather(float h = 1.5f, float w = 0.1f) {
	useCloth(0.9f, 0.1f, 0.1f); // red plume
	UseSlot(TEX_CLOTH);
	glPushMatrix();
	glScalef(w, h, w);
	drawSphere(1.0);  // stretched sphere = feather
	glPopMatrix();
	UnbindTex();
}

// Curved cheek wrap around back & sides of the head
void drawCheekWrap(const HumanDims& d) {
	useMetal();
	UseSlot(TEX_METAL);

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
	UnbindTex();
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
	UseSlot(TEX_SHINY);
	// ---- Dome geometry (relative to head center) ----
	const float domeR = d.headR * 1.20f;    // hemisphere radius
	const float domeBaseY = d.headR * 0.20f;    // base ring above head center
	const float domeTopY = domeBaseY + domeR;

	// Dome cap (hemisphere sits above base ring)
	glPushMatrix();
	glTranslatef(0.f, domeBaseY, 0.f);
	drawHemisphere(domeR);
	glPopMatrix();
	UnbindTex();

	// Cheek wrap (expects origin at head center)
	drawCheekWrap(d);
	

	// Rear neck guard (designed around head center)
	UseSlot(TEX_SHINY);
	glPushMatrix();
	glTranslatef(0.f, -d.headR * 1.0f, -d.headR * 0.6f);
	glRotatef(-20.f, 1, 0, 0);
	drawTrapezoidBlock(d.headR * 2.0f, d.headR * 1.4f, d.headR * 0.6f,
		d.headR * 0.5f, d.headR * 0.3f);
	glPopMatrix();
	UnbindTex();

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
	UseSlot(TEX_SHINY);
	drawShoulderPadPiece(d.shoulderW, d.chestH * 0.18f, d.chestD);
	UnbindTex();
	glTranslatef(0, d.chestH * 0.12f, 0);
	UseSlot(TEX_METAL);
	drawShoulderPadPiece(d.shoulderW * 0.9, d.chestH * 0.14f, d.chestD * 0.9);
	UnbindTex();
	glPopMatrix();
}

// ---------- Chest/back plate + belt ----------
void drawCuirass(const HumanDims& d) {
	useMetal();
	UseSlot(TEX_METAL);
	const float chestCenterY = d.pelvisH * 0.5f + d.chestH * 0.5f;

	glPushMatrix();
	glTranslatef(0.f, chestCenterY, d.chestD * 0.52f + Z_EPS);
	drawTrapezoidBlock(d.shoulderW * 0.80f, d.shoulderW * 0.95f, d.chestH * 0.98f, d.chestD * 0.02f, d.chestD * 0.02f);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.f, chestCenterY, -(d.chestD * 0.52f + Z_EPS));
	drawTrapezoidBlock(d.shoulderW * 0.80f, d.shoulderW * 0.95f, d.chestH * 0.98f, d.chestD * 0.02f, d.chestD * 0.02f);
	glPopMatrix();
	UnbindTex();                              

	useMetal();
	UseSlot(TEX_SHINY);
	glPushMatrix();
	const float beltY = d.pelvisH * 0.25f;
	glTranslatef(0.f, beltY, 0.f);
	drawBlock(d.pelvisW * 0.84f, d.pelvisH * 0.18f, d.pelvisD * 1.22f);
	// --- decorative belt holes (points) on the FRONT face ---
	{
		const float w = d.pelvisW * 0.8f;     // same as belt width
		const float h = d.pelvisH * 0.18f;    // same as belt height
		const float dep = d.pelvisD * 1.22f;    // same as belt depth

		const float yRow = 0.05f * h;         // slightly above belt center
		const float zFront = 0.5f * dep + Z_EPS * 2.f; // sit on front surface (+ tiny lift)

		const int   holes = 9;                 // number of holes
		const float margin = 0.12f * w;         // inset from belt edges
		const float usable = w - 2.f * margin;

		glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT | GL_TEXTURE_BIT | GL_CURRENT_BIT | GL_POINT_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_LIGHTING);
		glBindTexture(GL_TEXTURE_2D, 0);
		glEnable(GL_DEPTH_TEST);

		glPointSize(5.5f);
		glColor3f(0.08f, 0.08f, 0.08f);        // dark holes

		glBegin(GL_POINTS);
		int placed = 0;
		for (int i = 0; i < holes; ++i) {
			float t = (holes == 1) ? 0.5f : (float)i / (float)(holes - 1);
			float x = -0.5f * w + margin + usable * t;
			glVertex3f(x, yRow, zFront);
			++placed;
		}
		glEnd();
		glPopAttrib();

		gPrim.points += placed;
	}
	glPopMatrix();
	UnbindTex();
}

// ---------- Forearm bracer (call inside right/left arm local space after elbow) ----------
void drawForearmBracer(const HumanDims& d) {
	useMetal();
	UseSlot(TEX_SHINY);
	glPushMatrix();
	glTranslatef(0.f, -d.forearmL * 0.35f, 0.f);

	// ---- original bracer ----
	const float h = d.forearmL * 0.7f;           // length
	const float r0 = d.armLimbR * 1.15f;          // base radius (near elbow)
	const float r1 = d.armLimbR * 1.10f;          // tip radius (toward wrist)
	drawYCylinder(r0, r1, h);

	// ---- decorative overlays: line strips + points ----
	{
		const int   slices = noOfSides;
		const float lift = d.armLimbR * 0.02f;  // tiny radial lift to avoid z-fight
		const float thStep = float(2.0 * M_PI) / float(slices);

		// unlit, no texture, darker color
		glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT | GL_TEXTURE_BIT | GL_CURRENT_BIT | GL_LINE_BIT | GL_POINT_BIT);
		glDisable(GL_LIGHTING);
		glBindTexture(GL_TEXTURE_2D, 0);
		glColor3f(0.08f, 0.08f, 0.08f);

		// Helper to get radius along Y (linear taper)
		auto rAt = [&](float t) -> float {
			return (1.0f - t) * r0 + t * r1 + lift;
			};

		// ---- circumferential ring near the top ----
		{
			const float t = 0.2f; // 0..1 along +Y
			const float y = t * h;
			const float rr = rAt(t);

			glLineWidth(10.0f);
			glBegin(GL_LINE_STRIP);
			for (int j = 0; j <= slices; ++j) {
				float th = j * thStep;
				glVertex3f(rr * cosf(th), y, rr * sinf(th));
			}
			glEnd();
			gPrim.linestrip += 1;
		}

		// ---- circumferential ring near the bottom ----
		{
			const float t = 0.15f;
			const float y = t * h;
			const float rr = rAt(t);

			glBegin(GL_LINE_STRIP);
			for (int j = 0; j <= slices; ++j) {
				float th = j * thStep;
				glVertex3f(rr * cosf(th), y, rr * sinf(th));
			}
			glEnd();
			gPrim.linestrip += 1;
		}

		glPopAttrib();
	}

	glPopMatrix();
	UnbindTex();
}


// ---------- Greave + knee plate (call inside leg local space after knee) ----------
void drawShinGreave(const HumanDims& d) {
	useMetal();
	UseSlot(TEX_SHINY);
	// shin tube
	glPushMatrix();
	glTranslatef(0.f, -d.lowerLegL * 0.35f, 0.f);
	drawYCylinder(d.legLimbR * 1.15f, d.legLimbR * 1.25f, d.lowerLegL * 0.8f);
	glPopMatrix();
	// knee cap
	glPushMatrix();
	drawSphere(d.legLimbR * 0.65f);
	glPopMatrix();
	UnbindTex();
}

// ---------- Layered skirt / tassets (front/back) ----------
void drawSkirtArmor(const HumanDims& d) {
	const float waistY = d.pelvisH * 0.5f;

	// cloth front / back
	useMetal();
	UseSlot(TEX_METAL);
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
	UnbindTex();
}

void drawCenterTasset(const HumanDims& d) {
	useMetal();
	UseSlot(TEX_SHINY);

	const float waistY = d.pelvisH * 0.5f;

	glPushMatrix();
	const float tz = d.pelvisD * 0.06f;
	glTranslatef(0.f, waistY - d.pelvisH * 0.25f, 0.61f * d.pelvisD + 0.5f * tz + Z_EPS);

	// --- symmetric footprint (centered) ---
	const float plateH = d.upperLegL * 0.55f;
	const float plateBottomW = d.pelvisW * 0.45f;
	const float plateTopW = d.pelvisW * 0.28f;
	const float plateW = 0.5f * (plateBottomW + plateTopW); // symmetric width

	// old ellipse radii (kept only to choose the limiting dimension)
	const float rx = plateW * 0.48f;
	const float ry = plateH * 0.48f;

	// Use a single radius so the octagon is regular (equal side lengths)
	const float R = (rx < ry ? rx : ry);         // or std::fmin(rx, ry)

	// thickness so it has depth (extrusion along +Z/-Z locally)
	const float zBack = -0.5f * tz;
	const float zFront = +0.5f * tz;

	// -------- regular-octagon vertices (centered, flats horizontal) --------
	double vx[8], vy[8];
	{
		const double th0 = M_PI / 8.0;           // start so flats are horizontal
		const double dth = M_PI / 4.0;           // 45° steps
		for (int i = 0; i < 8; ++i) {
			double th = th0 + i * dth;
			vx[i] = R * cos(th);
			vy[i] = R * sin(th);
		}
	}

	// ---- front face (polygon) ----
	glNormal3f(0.f, 0.f, 1.f);
	glBegin(GL_POLYGON);
	for (int i = 0; i < 8; ++i) {
		// circular UV so it doesn’t skew when rx≠ry
		double u = 0.5 + 0.5 * (vx[i] / R);
		double v = 0.5 + 0.5 * (vy[i] / R);
		glTexCoord2d(u, v);
		glVertex3d(vx[i], vy[i], zFront);
	}
	glEnd();
	gPrim.polygon += 1;

	// ---- back face (polygon) ----
	glNormal3f(0.f, 0.f, -1.f);
	glBegin(GL_POLYGON);
	for (int i = 7; i >= 0; --i) { // reverse winding so the normal points outward
		double u = 0.5 + 0.5 * (vx[i] / R);
		double v = 0.5 + 0.5 * (vy[i] / R);
		glTexCoord2d(u, v);
		glVertex3d(vx[i], vy[i], zBack);
	}
	glEnd();
	gPrim.polygon += 1;

	// ---- side wall quads to close the extrusion ----
	glBegin(GL_QUADS);
	for (int i = 0; i < 8; ++i) {
		int j = (i + 1) & 7;
		// face-normal for the side (approx: outward in XY, no Z)
		double nx = (vy[j] - vy[i]);
		double ny = -(vx[j] - vx[i]);
		double len = sqrt(nx * nx + ny * ny);
		if (len > 1e-9) { nx /= len; ny /= len; }
		glNormal3d(nx, ny, 0.0);

		glTexCoord2d(0, 0); glVertex3d(vx[i], vy[i], zFront);
		glTexCoord2d(1, 0); glVertex3d(vx[j], vy[j], zFront);
		glTexCoord2d(1, 1); glVertex3d(vx[j], vy[j], zBack);
		glTexCoord2d(0, 1); glVertex3d(vx[i], vy[i], zBack);
	}
	glEnd();
	UnbindTex();
	gPrim.quads += 8;


	// ---- near-edge outline (line loop) on the FRONT face ----
	{
		const float outlineScale = 0.96f;        // closer to 1 => nearer the edge
		const float Ro = R * outlineScale;

		glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_LIGHTING_BIT | GL_LINE_BIT);
		glDisable(GL_LIGHTING);
		glBindTexture(GL_TEXTURE_2D, 0);
		glLineWidth(5.0f);
		glColor3f(0.3f, 0.3f, 0.3f);

		glBegin(GL_LINE_LOOP);
		for (int i = 0; i < 8; ++i) {
			double th = M_PI / 8.0 + i * (M_PI / 4.0);
			glVertex3d(Ro * cos(th), Ro * sin(th), zFront + 0.001); // tiny lift to avoid z-fight
		}
		glEnd();
		glPopAttrib();

		gPrim.lineloop += 1;
	}

	glPopMatrix();
	
}

void drawUpperArmPlate(const HumanDims& d) {
	useCloth();
	UseSlot(TEX_CLOTH);
	glPushMatrix();              
	drawYCylinder(d.armLimbR * 1.10f, d.armLimbR * 1.5f, d.upperArmL * 1.f);
	glPopMatrix();
	UnbindTex();
}

void drawSabaton(const HumanDims& d) {
	useMetal();
	UseSlot(TEX_SHINY);
	glPushMatrix();
	glTranslatef(0.f, 0.f, d.footD * 0.35f);
	drawTrapezoidBlock(d.footW * 1.1f, d.footW * 1.1f,
		d.footH * 1.2f, d.footD * 1.0f, d.footD * 1.1f);
	glPopMatrix();
	UnbindTex();
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
	useCloth();
	UseSlot(TEX_CLOTH);
	drawTrapezoidBlock(
		d.pelvisW,                 // bottom width
		d.pelvisW * 0.7f,          // top width (slimmer waist)
		d.pelvisH,                 // height
		d.pelvisD,                 // bottom depth
		d.pelvisD * 0.8f           // top depth
	);
	UnbindTex();

	// chest center = pelvis_full/2 + chest_half
	float chestCenterY = (d.pelvisH + d.chestH) * 0.5f;
	glPushMatrix();
	glTranslatef(0.f, chestCenterY, 0.f);
	// chest tapers opposite: top wider than bottom (shoulders broader)
	useCloth();
	UseSlot(TEX_CLOTH);
	drawTrapezoidBlock(
		d.shoulderW * 0.7f,    // bottom width (narrower near waist)
		d.shoulderW,           // top width (broad shoulders)
		d.chestH,              // height
		d.chestD * 0.7f,       // bottom depth
		d.chestD               // top depth
	);
	UnbindTex();
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
	glRotatef(90.f, 0.f, 1.f, 0.f);
	glTranslatef(0.f, 0.5f, 0.0f);

	// ------- CROSSGUARD (sits above the grip, top at y=guardH) -------
	useMetal(0.82f, 0.82f, 0.87f);
	UseSlot(TEX_SHINY);
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
	UnbindTex();

	// ------- GRIP -------
	useCloth(0.25f, 0.12f, 0.08f);
	UseSlot(TEX_LEATHER);
	glPushMatrix();
	glTranslatef(0.f, 0.0f, 0.f);         // base at -handleL
	drawYCylinder(gripR, gripR, handleL + EPS);      // reaches (slightly past) y=0
	glPopMatrix();
	UnbindTex();


	// small metal rings on grip
	useMetal(0.78f, 0.78f, 0.83f);
	UseSlot(TEX_SHINY);
	glPushMatrix(); glTranslatef(0.f, -0.15f, 0.f);
	drawYCylinder(gripR * 1.07f, gripR * 1.07f, 0.03f);
	glPopMatrix();
	glPushMatrix(); glTranslatef(0.f, -handleL + 0.15f, 0.f);
	drawYCylinder(gripR * 1.07f, gripR * 1.07f, 0.03f);
	glPopMatrix();
	UnbindTex();

	// ------- POMMEL -------
	UseSlot(TEX_SHINY);
	glPushMatrix();
	glTranslatef(0.f, -handleL, 0.f);
	drawYCylinder(pommelR * 0.85f, pommelR, pommelH + EPS);
	glPopMatrix();
	UnbindTex();

	// ------- BLADE -------
	useMetal(0.86f, 0.86f, 0.91f);
	UseSlot(TEX_SILVER);

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

	glPopMatrix();
	UnbindTex();
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
		UseSlot(TEX_WOOD);
		drawYCylinder(shaftR, shaftR, shaftL);   // runs to the tip
	glPopMatrix();
	UnbindTex();

	// Butt cap (little metal piece at the very end)
	useMetal(0.7f, 0.7f, 0.75f);
	UseSlot(TEX_SHINY);
	glPushMatrix();
		glTranslatef(0.f, buttY - 0.02f, 0.f);
		drawYCylinder(shaftR * 1.05f, shaftR * 1.05f, 0.12f);
	glPopMatrix();
	UnbindTex();

	// Hand wrap centered on the grip (origin)
	useCloth(0.35f, 0.18f, 0.12f); // leather-ish
	UseSlot(TEX_LEATHER);
	glPushMatrix();
		glTranslatef(0.f, -0.15f, 0.f);
		drawYCylinder(shaftR * 1.1f, shaftR * 1.1f, 2.0f);
	glPopMatrix();
	UnbindTex();

	glPushMatrix();
	glRotatef(180.f, 1.f, 0.f, 0.f);

	// Ferrule under the head
	useMetal();
	UseSlot(TEX_SHINY);
	glPushMatrix();
	glTranslatef(0.f, tipY - 0.45f, 0.f);
	drawYCylinder(shaftR * 1.12f, shaftR * 1.12f, 0.22f);
	glPopMatrix();
	UnbindTex();

	// Small square collar
	UseSlot(TEX_SHINY);
	glPushMatrix();
	glTranslatef(0.f, tipY - 0.05f, 0.f);
	drawBlock(shaftR * 3.0f, 0.15f, shaftR * 3.0f);
	glPopMatrix();
	UnbindTex();

	// --- SPEAR HEAD (leaf/diamond) ---
	useMetal(0.85f, 0.85f, 0.90f);
	UseSlot(TEX_SILVER);
	// main leaf to the tip
	const float bladeH = 1.9f;
	glPushMatrix();
	glTranslatef(0.f, tipY + bladeH * 0.5f, 0.f); // bottom sits at tipY
	drawPyramid(shaftR * 2.6, shaftR * 1.2, bladeH, true);
	glPopMatrix();
	UnbindTex();

	glPopMatrix();
	glPopMatrix();
}

void drawArm(const HumanDims& d, bool left,
	float shx = 0.f, float shy = 0.f, float shz = 0.f,
	float elx = 0.f,
	float wristx = 0.f, float wristy = 0.f, float wristz = 0.f)
{
	float side = left ? -1.f : 1.f;
	float shoulderX = side * (d.shoulderW * 0.5f + d.armJointR * 0.5);
	float shoulderY = (d.pelvisH * 0.5f) + d.chestH - d.armJointR * 0.5;

	glPushMatrix();
	glTranslatef(shoulderX, shoulderY, 0.f);

	if (g_mode == MODE_RIG) {
		applyJointRot(left ? J_SHOULDER_L : J_SHOULDER_R);
	}
	else {
		glRotatef(shx, 1.f, 0.f, 0.f);
		glRotatef(shy, 0.f, 1.f, 0.f);
		glRotatef(shz, 0.f, 0.f, 1.f);  // ← shoulder “raise horizontally outwards”
	}

	if (g_armorOn) drawUpperArmPlate(d);
	useCloth();
	UseSlot(TEX_CLOTH);
	drawLimbSegment(d.armLimbR, d.armLimbR, d.upperArmL);
	UnbindTex();

	// elbow
	glTranslatef(0.f, -d.upperArmL, 0.f);
	if (g_mode == MODE_RIG) {
		applyJointRot(left ? J_ELBOW_L : J_ELBOW_R); // full X,Y,Z
	}
	else {
		glRotatef(elx, 1.f, 0.f, 0.f);
	}

	if (g_armorOn) drawForearmBracer(d);
	useSkin();
	drawLimbSegment(d.armLimbR, d.armLimbR, d.forearmL);

	// wrist + hand
	glTranslatef(0.f, -(d.forearmL), 0.f);
	if (g_mode == MODE_RIG) {
		applyJointRot(left ? J_WRIST_L : J_WRIST_R);
	}
	else {
		glRotatef(wristx, 1.f, 0.f, 0.f);
		glRotatef(wristy, 0.f, 1.f, 0.f);
		glRotatef(wristz, 0.f, 0.f, 1.f);
	}
	glTranslatef(0.f, -(d.handH * 0.5f), 0.f);
	drawBlock(d.handW, d.handH, d.handD);

	if (!left) {
		glPushMatrix();
		glTranslatef(0.f, -d.handH * 0.5f, 0.f);
		glRotatef(-90.f, 1.f, 0.f, 0.f);
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
	useCloth();
	UseSlot(TEX_CLOTH);
	drawLimbSegment(d.legLimbR, d.legLimbR, d.upperLegL);

	// knee
	glTranslatef(0.f, -d.upperLegL, 0.f);
	if (g_mode == MODE_RIG) {
		applyJointRot(left ? J_KNEE_L : J_KNEE_R); // full X,Y,Z
	}
	else {
		glRotatef(-knee_rx_deg, 1.f, 0.f, 0.f);
	}
	UnbindTex();

	// NEW: greave
	if (g_armorOn) drawShinGreave(d);
	useNeutral();

	// shin
	useCloth();
	UseSlot(TEX_CLOTH);
	drawLimbSegment(d.legLimbR, d.legLimbR, d.lowerLegL);

	// move to ankle joint (top of foot), apply ankle, then place the foot block
	glTranslatef(0.f, -d.lowerLegL, 0.f);
	if (g_mode == MODE_RIG) {
		applyJointRot(left ? J_ANKLE_L : J_ANKLE_R); // full XYZ
	}
	UnbindTex();

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
		gPrim.lines += 1;
		glVertex3f(t, 0.f, -size); glVertex3f(t, 0.f, size);
		gPrim.lines += 1;
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
	// Arms
	if (g_mode == MODE_CHAR && g_attackActive) {
		float t = g_attackT;
		const float raiseEnd = 0.45f;

		if (weaponType == 1) {
			// ---- SPEAR ATTACK ----
			float shx = 0.f, shy = 0.f, shz = 0.f;
			float elx = 0.f;
			float wx = 0.f, wy = 0.f, wz = 0.f;

			const float raiseEnd = 0.45f;  // already used
			const float thrustEnd = 0.80f;  // end of forward thrust, start of recovery

			// keyframes (X only)
			const float shx_raise = +80.f;  // shoulder at end of raise
			const float elx_raise = -80.f;  // elbow at end of raise
			const float wx_raise = 0.f;

			const float shx_thrust = -85.f; // forward reach
			const float elx_thrust = 0.f;   // straight arm
			const float wx_thrust = +90.f; // spear vertical (adjust sign if needed)

			if (t < raiseEnd) {
				// RAISE
				float u = smooth01(t / raiseEnd);
				shx = shx_raise * u;
				elx = elx_raise * u;
				wx = wx_raise * u;  // stays 0 anyway
			}
			else if (t < thrustEnd) {
				// THRUST
				float u = smooth01((t - raiseEnd) / (thrustEnd - raiseEnd));
				shx = shx_raise + (shx_thrust - shx_raise) * u;
				elx = elx_raise + (elx_thrust - elx_raise) * u;
				wx = wx_raise + (wx_thrust - wx_raise) * u;
			}
			else {
				// RECOVER back to neutral (0) to avoid snapping when the anim ends
				float u = smooth01((t - thrustEnd) / (1.0f - thrustEnd));
				shx = shx_thrust * (1.0f - u);   // -> 0
				elx = elx_thrust * (1.0f - u);   // -> 0
				wx = wx_thrust * (1.0f - u);   // -> 0
			}

			// left arm idle, right arm performs the attack
			drawArm(d, true, -0.f, 0.f, 0.f, 0.f);
			drawArm(d, false, shx, /*shy*/0.f, /*shz*/0.f, elx, /*wristX*/wx, /*wY*/0.f, /*wZ*/0.f);
			glPopMatrix();
			return;
		}
		else {
			// ---- SWORD (X-only with smooth recovery) ----
			float shx = 0.f, elx = 0.f;

			const float windupEnd = 0.35f;   // end of windup
			const float swingEnd = 0.70f;   // end of strike, start of recovery

			// Key poses (X axis only)
			const float shx_windup = -160.f; // big windup back
			const float elx_windup = -30.f; // slight bend

			const float shx_hit = +40.f; // follow-through forward (tweak sign if needed)
			const float elx_hit = -10.f; // small bend at impact

			if (t < windupEnd) {
				// WINDUP: neutral -> windup
				float u = smooth01(t / windupEnd);
				shx = (1.0f - u) * 0.f + u * shx_windup;
				elx = (1.0f - u) * 0.f + u * elx_windup;
			}
			else if (t < swingEnd) {
				// STRIKE: windup -> hit
				float u = smooth01((t - windupEnd) / (swingEnd - windupEnd));
				shx = shx_windup + (shx_hit - shx_windup) * u;
				elx = elx_windup + (elx_hit - elx_windup) * u;
			}
			else {
				// RECOVER: hit -> neutral (0) to avoid snapping when the anim flag resets
				float u = smooth01((t - swingEnd) / (1.0f - swingEnd));
				shx = shx_hit * (1.0f - u);   // -> 0
				elx = elx_hit * (1.0f - u);   // -> 0
			}

			// left idle, right swings (X only; Y/Z stay 0)
			drawArm(d, true, -0.f, 0.f, 0.f, 0.f);
			drawArm(d, false, shx, 0.f, 0.f, elx);
			glPopMatrix();
			return;
		}
	}

	// (no attack) – fallback:
	drawArm(d, true, shL, 0.f, 0.f, elL);
	drawArm(d, false, shR, 0.f, 0.f, elR);

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
	ResetPrimTally();
	if (g_texturesEnabled) glEnable(GL_TEXTURE_2D);
	else                   glDisable(GL_TEXTURE_2D);
	// Per-frame keyboard handling for modes
	handleInputPerFrame();

	static const float GRID_HALF = 20.0f; // 40×40 square => x,z in [-20, 20]

	if (isWalking) {
		float walkSpeed = walkAnimSpeed * (g_kShift ? 4.f : 1.0f);
		walkPhase += walkSpeed;
		if (walkPhase > 360.f) walkPhase -= 360.f;

		float speed = humanSpeed * (g_kShift ? 3.f : 1.0f);

		// propose
		float nx = hx + walkDirX * speed;
		float nz = hz + walkDirZ * speed;

		// accept or block
		if (fabsf(nx) <= GRID_HALF && fabsf(nz) <= GRID_HALF) {
			hx = nx; hz = nz;
		}
		else {
			isWalking = false; // blocked: stop animation, but facing already updated
		}
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
		glOrtho(-10, 10, -10, 10, PNear, PFar);
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
	LoadAllTextures();
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