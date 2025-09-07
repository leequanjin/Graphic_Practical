#include <Windows.h>
#include <wingdi.h>   // BITMAP, HBITMAP, LoadImage, GetObject, DeleteObject
#include <gl/GL.h>
#include <gl/GLU.h>
#include <vector>
#include <cmath>
#include <string>   

#pragma comment (lib, "OpenGL32.lib")
#pragma comment (lib, "Glu32.lib")

#define WINDOW_TITLE "OpenGL Window (Q1 + Q2)"

static float rotX = 20.0f;
static float rotY = -30.0f;
static float rotZ = 0.0f;
static float zoom = -4.0f;

static bool  g_glInited = false;

enum class AppMode { Q1_PYRAMID, Q2_CUBE };
static AppMode g_mode = AppMode::Q1_PYRAMID;

static GLuint g_texBrick = 0;
static GLuint g_texWood = 0;
static GLuint g_texMetal = 0;

enum class CubeTex { WOOD, METAL, BRICK };
static CubeTex g_cubeTex = CubeTex::WOOD;

#ifndef GL_BGR_EXT
#define GL_BGR_EXT  0x80E0
#endif
#ifndef GL_BGRA_EXT
#define GL_BGRA_EXT 0x80E1
#endif

static std::string exeDir()
{
    char buf[MAX_PATH]{};
    GetModuleFileNameA(GetModuleHandle(NULL), buf, MAX_PATH);
    std::string s(buf);
    size_t p = s.find_last_of("\\/");
    return (p == std::string::npos) ? std::string("") : s.substr(0, p + 1);
}

static bool loadTextureBMP(const char* fileName, GLuint* outTex)
{
    if (!outTex) return false;
    if (*outTex == 0) glGenTextures(1, outTex);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    std::string full = fileName;
    if (full.find(':') == std::string::npos && full.find('\\') == std::string::npos &&
        full.find('/') == std::string::npos)
        full = exeDir() + full;

    HBITMAP hBMP = (HBITMAP)LoadImageA(
        GetModuleHandle(NULL), full.c_str(), IMAGE_BITMAP, 0, 0,
        LR_CREATEDIBSECTION | LR_LOADFROMFILE);

    if (!hBMP) {
        std::string msg = "Failed to load BMP:\n" + full +
            "\nPlace the .bmp next to the .exe or set Copy to Output Directory.";
        MessageBoxA(NULL, msg.c_str(), "Texture Load Error", MB_OK | MB_ICONERROR);
        return false;
    }

    BITMAP bmp{};
    GetObject(hBMP, sizeof(BITMAP), &bmp);

    GLenum srcFormat = (bmp.bmBitsPixel == 32) ? GL_BGRA_EXT : GL_BGR_EXT;
    GLint  internal = (bmp.bmBitsPixel == 32) ? GL_RGBA : GL_RGB;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, *outTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, internal,
        bmp.bmWidth, bmp.bmHeight, 0,
        srcFormat, GL_UNSIGNED_BYTE, bmp.bmBits);

    glDisable(GL_TEXTURE_2D);
    DeleteObject(hBMP);
    return true;
}

static void drawTexturedPyramid()
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, g_texBrick);

    const float s = 1.0f;
    const float h = 1.2f;

    // Base
    glBegin(GL_TRIANGLES);
    glNormal3f(0, -1, 0);
    glTexCoord2f(0, 0); glVertex3f(-s, 0, -s);
    glTexCoord2f(1, 0); glVertex3f(s, 0, -s);
    glTexCoord2f(1, 1); glVertex3f(s, 0, s);

    glTexCoord2f(0, 0); glVertex3f(-s, 0, -s);
    glTexCoord2f(1, 1); glVertex3f(s, 0, s);
    glTexCoord2f(0, 1); glVertex3f(-s, 0, s);
    glEnd();

    // Sides
    glBegin(GL_TRIANGLES);
    // +Z
    glNormal3f(0, h, s);
    glTexCoord2f(0, 0);   glVertex3f(-s, 0, s);
    glTexCoord2f(1, 0);   glVertex3f(s, 0, s);
    glTexCoord2f(0.5f, 1); glVertex3f(0, h, 0);

    // -Z
    glNormal3f(0, h, -s);
    glTexCoord2f(0, 0);   glVertex3f(s, 0, -s);
    glTexCoord2f(1, 0);   glVertex3f(-s, 0, -s);
    glTexCoord2f(0.5f, 1); glVertex3f(0, h, 0);

    // +X
    glNormal3f(s, h, 0);
    glTexCoord2f(0, 0);   glVertex3f(s, 0, s);
    glTexCoord2f(1, 0);   glVertex3f(s, 0, -s);
    glTexCoord2f(0.5f, 1); glVertex3f(0, h, 0);

    // -X
    glNormal3f(-s, h, 0);
    glTexCoord2f(0, 0);   glVertex3f(-s, 0, -s);
    glTexCoord2f(1, 0);   glVertex3f(-s, 0, s);
    glTexCoord2f(0.5f, 1); glVertex3f(0, h, 0);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

static GLuint currentCubeTexture()
{
    switch (g_cubeTex)
    {
    case CubeTex::WOOD:  return g_texWood;
    case CubeTex::METAL: return g_texMetal;
    default:             return g_texBrick;
    }
}

static void drawTexturedCube()
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, currentCubeTexture());

    const float s = 1.0f;

    glBegin(GL_QUADS);
    // +Z (front)
    glNormal3f(0, 0, 1);
    glTexCoord2f(0, 0); glVertex3f(-s, -s, s);
    glTexCoord2f(1, 0); glVertex3f(s, -s, s);
    glTexCoord2f(1, 1); glVertex3f(s, s, s);
    glTexCoord2f(0, 1); glVertex3f(-s, s, s);

    // -Z (back)
    glNormal3f(0, 0, -1);
    glTexCoord2f(0, 0); glVertex3f(s, -s, -s);
    glTexCoord2f(1, 0); glVertex3f(-s, -s, -s);
    glTexCoord2f(1, 1); glVertex3f(-s, s, -s);
    glTexCoord2f(0, 1); glVertex3f(s, s, -s);

    // +X (right)
    glNormal3f(1, 0, 0);
    glTexCoord2f(0, 0); glVertex3f(s, -s, s);
    glTexCoord2f(1, 0); glVertex3f(s, -s, -s);
    glTexCoord2f(1, 1); glVertex3f(s, s, -s);
    glTexCoord2f(0, 1); glVertex3f(s, s, s);

    // -X (left)
    glNormal3f(-1, 0, 0);
    glTexCoord2f(0, 0); glVertex3f(-s, -s, -s);
    glTexCoord2f(1, 0); glVertex3f(-s, -s, s);
    glTexCoord2f(1, 1); glVertex3f(-s, s, s);
    glTexCoord2f(0, 1); glVertex3f(-s, s, -s);

    // +Y (top)
    glNormal3f(0, 1, 0);
    glTexCoord2f(0, 0); glVertex3f(-s, s, s);
    glTexCoord2f(1, 0); glVertex3f(s, s, s);
    glTexCoord2f(1, 1); glVertex3f(s, s, -s);
    glTexCoord2f(0, 1); glVertex3f(-s, s, -s);

    // -Y (bottom)
    glNormal3f(0, -1, 0);
    glTexCoord2f(0, 0); glVertex3f(-s, -s, -s);
    glTexCoord2f(1, 0); glVertex3f(s, -s, -s);
    glTexCoord2f(1, 1); glVertex3f(s, -s, s);
    glTexCoord2f(0, 1); glVertex3f(-s, -s, s);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

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

    int n = ChoosePixelFormat(hdc, &pfd);
    return SetPixelFormat(hdc, n, &pfd) == TRUE;
}

static void resetView() { rotX = 20.0f; rotY = -30.0f; rotZ = 0.0f; zoom = -4.0f; }

LRESULT WINAPI WindowProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_SIZE:
        if (g_glInited)
        {
            int w = LOWORD(lParam), h = HIWORD(lParam);
            if (h == 0) h = 1;
            glViewport(0, 0, w, h);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            gluPerspective(60.0, (double)w / (double)h, 0.1, 100.0);
            glMatrixMode(GL_MODELVIEW);
        }
        break;

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) PostQuitMessage(0);

        // rotate all axes
        if (wParam == VK_LEFT)  rotY -= 5.0f;
        if (wParam == VK_RIGHT) rotY += 5.0f;
        if (wParam == VK_UP)    rotX -= 5.0f;
        if (wParam == VK_DOWN)  rotX += 5.0f;
        if (wParam == 'Q')      rotZ -= 5.0f;
        if (wParam == 'E')      rotZ += 5.0f;

        // zoom
        if (wParam == VK_OEM_PLUS || wParam == '=') zoom += 0.2f;
        if (wParam == VK_OEM_MINUS || wParam == '-') zoom -= 0.2f;

        // reset
        if (wParam == 'R') resetView();

        // switch model (clear distinction Q1 vs Q2)
        if (wParam == '1') { g_mode = AppMode::Q1_PYRAMID; resetView(); }
        if (wParam == '2') { g_mode = AppMode::Q2_CUBE;    resetView(); }

        // Q2: switch cube texture
        if (wParam == 'W') g_cubeTex = CubeTex::WOOD;
        if (wParam == 'M') g_cubeTex = CubeTex::METAL;
        if (wParam == 'B') g_cubeTex = CubeTex::BRICK;
        if (wParam == 'T')
        {
            g_cubeTex = (g_cubeTex == CubeTex::WOOD) ? CubeTex::METAL :
                (g_cubeTex == CubeTex::METAL) ? CubeTex::BRICK : CubeTex::WOOD;
        }
        break;

    default: break;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

void display()
{
    if (!g_glInited) return;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0.0f, -0.1f, zoom);
    glRotatef(rotX, 1, 0, 0);
    glRotatef(rotY, 0, 1, 0);
    glRotatef(rotZ, 0, 0, 1);

    if (g_mode == AppMode::Q1_PYRAMID)      drawTexturedPyramid(); // ===== Q1 =====
    else                                     drawTexturedCube();    // ===== Q2 =====
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)
{
    WNDCLASSEX wc; ZeroMemory(&wc, sizeof(WNDCLASSEX));
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpfnWndProc = WindowProcedure;
    wc.lpszClassName = WINDOW_TITLE;
    wc.style = CS_HREDRAW | CS_VREDRAW;

    if (!RegisterClassEx(&wc)) return false;

    HWND hWnd = CreateWindow(
        WINDOW_TITLE, WINDOW_TITLE, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 900, 650, NULL, NULL, wc.hInstance, NULL);

    HDC hdc = GetDC(hWnd);
    if (!initPixelFormat(hdc)) return false;
    HGLRC hglrc = wglCreateContext(hdc);
    if (!wglMakeCurrent(hdc, hglrc)) return false;

    g_glInited = true;
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);

    RECT rc; GetClientRect(hWnd, &rc);
    SendMessage(hWnd, WM_SIZE, 0, MAKELPARAM(rc.right - rc.left, rc.bottom - rc.top));

    bool okB = loadTextureBMP("brick.bmp", &g_texBrick);
    if (!okB) okB = loadTextureBMP("Box.bmp", &g_texBrick);

    bool okW = loadTextureBMP("Box.bmp", &g_texWood);
    if (!okW) okW = loadTextureBMP("Box.bmp", &g_texWood);

    bool okM = loadTextureBMP("metal.bmp", &g_texMetal);
    if (!okM) okM = loadTextureBMP("Box.bmp", &g_texMetal);

    if (!okW) g_texWood = g_texBrick;
    if (!okM) g_texMetal = g_texBrick;

    ShowWindow(hWnd, nCmdShow);

    MSG msg; ZeroMemory(&msg, sizeof(msg));
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

    if (g_texBrick) glDeleteTextures(1, &g_texBrick);
    if (g_texWood)  glDeleteTextures(1, &g_texWood);
    if (g_texMetal) glDeleteTextures(1, &g_texMetal);

    UnregisterClass(WINDOW_TITLE, wc.hInstance);
    return true;
}
