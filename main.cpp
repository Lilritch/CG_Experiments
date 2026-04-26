// ============================================================
// Bird Flying Over a Forest Scene
// Covers: Experiments 1–5
// Mac: g++ main.cpp OBJImporter.cpp -framework OpenGL -framework GLUT -o bird_scene && ./bird_scene
// ============================================================
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#include "gl_headers.h"
#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string>

static constexpr float PI = 3.14159265f;
static constexpr int NUM_COLLECTIBLES = 8;

#define USE_OBJ_BIRD
#ifdef USE_OBJ_BIRD
#include "OBJImporter.h"
#endif

// ============================================================
// GLOBALS
// ============================================================

int winWidth = 1000, winHeight = 1000;

float simTime   = 0.0f;
float deltaTime = 0.016f;

float birdX = 0.0f, birdY = 2.0f, birdZ = 0.0f;
float birdYaw = -28.0f;
float birdPitch = 14.0f;
float birdRoll = 8.0f;
float birdSpeed = 0.0f;
int birdLook = 2;
float birdScale = 1.4f;
float birdWingSpread = 1.0f;
float birdWingLift = 1.0f;
float birdBobAmount = 0.22f;
float birdTurnRate = 3.0f;
bool birdAutoFlight = false;
bool birdAutoFlap = true;

enum ControlMode { BirdControl, CameraControl };
ControlMode controlMode = BirdControl;

enum CameraMode { CAM_SHOWCASE, CAM_FOLLOW, CAM_FRONT, CAM_BACK, CAM_LEFT, CAM_RIGHT, CAM_TOP, CAM_FREE };
CameraMode cameraMode = CAM_SHOWCASE;

float camX=0.0f, camY=4.5f, camZ=10.0f;
float camLookX=0.0f, camLookY=2.0f, camLookZ=0.0f;
float camUpX=0.0f, camUpY=1.0f, camUpZ=0.0f;
bool followCam = false;
float birdZoom = 8.0f;
float birdCamYaw = 18.0f;
float birdCamPitch = 16.0f;
bool showHud = true;
bool showControls = true;
bool showTitleCard = true;
bool paused = true;
bool autoShowcase = true;
float worldTime = 0.22f;
float worldTimeScale = 0.012f;
float windStrength = 0.45f;
float travelDistance = 0.0f;
int score = 0;
float lastBirdX = 0.0f, lastBirdZ = 0.0f;
bool collectibleTaken[NUM_COLLECTIBLES] = {false};
float collectibleSpin = 0.0f;
std::string executableDir = ".";

enum WorldTheme { THEME_MEADOW, THEME_TROPICAL, THEME_SUNSET, THEME_DUSK };
enum WeatherMode { WEATHER_CLEAR, WEATHER_BREEZY, WEATHER_RAIN, WEATHER_MAGIC };
WorldTheme worldTheme = THEME_MEADOW;
WeatherMode weatherMode = WEATHER_CLEAR;

struct ThemePalette {
    GLfloat skyTop[3];
    GLfloat skyMid[3];
    GLfloat skyHorizon[3];
    GLfloat groundA[3];
    GLfloat groundB[3];
    GLfloat haze[3];
    GLfloat water[3];
};

const ThemePalette kThemePalettes[] = {
    {{0.18f,0.48f,0.84f},{0.52f,0.82f,0.98f},{0.95f,0.87f,0.72f},{0.30f,0.62f,0.26f},{0.50f,0.80f,0.34f},{0.92f,0.95f,0.98f},{0.14f,0.50f,0.78f}},
    {{0.12f,0.56f,0.88f},{0.40f,0.84f,0.92f},{0.98f,0.86f,0.64f},{0.18f,0.54f,0.24f},{0.48f,0.76f,0.30f},{0.86f,0.94f,0.88f},{0.10f,0.62f,0.72f}},
    {{0.32f,0.22f,0.54f},{0.92f,0.54f,0.36f},{0.99f,0.80f,0.46f},{0.34f,0.48f,0.22f},{0.70f,0.64f,0.28f},{0.98f,0.82f,0.72f},{0.30f,0.34f,0.62f}},
    {{0.05f,0.10f,0.28f},{0.18f,0.24f,0.46f},{0.40f,0.32f,0.56f},{0.12f,0.26f,0.18f},{0.18f,0.34f,0.24f},{0.70f,0.74f,0.92f},{0.16f,0.24f,0.38f}}
};

const float kCollectiblePositions[NUM_COLLECTIBLES][3] = {
    {-9.0f, 3.0f, -4.0f}, {-4.0f, 4.2f, 7.5f}, {1.5f, 5.3f, -8.0f}, {7.0f, 3.7f, 9.5f},
    {13.0f, 4.5f, -3.0f}, {-13.5f, 5.0f, 3.5f}, {0.0f, 6.0f, 15.0f}, {11.5f, 5.2f, 13.5f}
};

int lastMouseX=0, lastMouseY=0;
bool mouseDown = false;
enum MouseDragMode { DragNone, DragBirdRotate, DragBirdRoll, DragZoomCamera, DragFreeCamera };
MouseDragMode mouseDragMode = DragNone;

enum TransMode { Rotate, Translate, Scale };
TransMode transMode = Rotate;
bool drawLines = false;

GLfloat objXform[4][4] = {
    {1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}
};

float mynear = 0.5f, myfar = 200.0f;

float clamp01(float v) {
    return fmaxf(0.0f, fminf(1.0f, v));
}

float lerpf(float a, float b, float t) {
    return a + (b - a) * t;
}

void colorLerp(const GLfloat a[3], const GLfloat b[3], float t, GLfloat out[3]) {
    out[0] = lerpf(a[0], b[0], t);
    out[1] = lerpf(a[1], b[1], t);
    out[2] = lerpf(a[2], b[2], t);
}

float getDayBlend() {
    return 0.5f + 0.5f * sinf(worldTime * 2.0f * PI - PI * 0.5f);
}

float getSunHeight() {
    return sinf(worldTime * 2.0f * PI - PI * 0.5f);
}

void applyTimeOfDayToPalette(const ThemePalette& base, GLfloat skyTop[3], GLfloat skyMid[3], GLfloat skyHorizon[3]) {
    const GLfloat nightTop[3] = {0.03f, 0.05f, 0.14f};
    const GLfloat nightMid[3] = {0.10f, 0.13f, 0.26f};
    const GLfloat nightHorizon[3] = {0.24f, 0.18f, 0.34f};
    float dayBlend = clamp01(getDayBlend() * 1.12f);
    colorLerp(nightTop, base.skyTop, dayBlend, skyTop);
    colorLerp(nightMid, base.skyMid, dayBlend, skyMid);
    colorLerp(nightHorizon, base.skyHorizon, dayBlend, skyHorizon);
}

const char* themeName() {
    switch (worldTheme) {
        case THEME_TROPICAL: return "Tropical";
        case THEME_SUNSET:   return "Sunset";
        case THEME_DUSK:     return "Dusk";
        case THEME_MEADOW:
        default:             return "Meadow";
    }
}

const char* weatherName() {
    switch (weatherMode) {
        case WEATHER_BREEZY: return "Breezy";
        case WEATHER_RAIN:   return "Rain";
        case WEATHER_MAGIC:  return "Magic";
        case WEATHER_CLEAR:
        default:             return "Clear";
    }
}

std::string findAssetPath(const std::string& relativePath) {
    namespace fs = std::filesystem;
    const fs::path rel(relativePath);
    const fs::path candidates[] = {
        fs::current_path() / rel,
        fs::path(executableDir) / rel,
        fs::path(executableDir) / ".." / rel,
        fs::path(executableDir) / ".." / ".." / rel
    };
    for (const auto& candidate : candidates) {
        std::error_code ec;
        if (fs::exists(candidate, ec)) {
            return candidate.lexically_normal().string();
        }
    }
    return relativePath;
}

void drawFilledCircle(float radius, int segments) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 0.0f, 0.0f);
    for (int i = 0; i <= segments; ++i) {
        float ang = (2.0f * PI * i) / segments;
        glVertex3f(cosf(ang) * radius, sinf(ang) * radius, 0.0f);
    }
    glEnd();
}

// ============================================================
// RESET HELPERS
// ============================================================

void resetBirdPose() {
    birdX = 0.0f; birdY = 2.0f; birdZ = 0.0f;
    birdYaw = -28.0f; birdPitch = 14.0f; birdRoll = 8.0f;
    birdSpeed = 0.0f; birdScale = 1.4f;
    birdWingSpread = 1.0f; birdWingLift = 1.0f;
    birdBobAmount = 0.22f; birdTurnRate = 3.0f;
    travelDistance = 0.0f;
    score = 0;
    for (int i = 0; i < NUM_COLLECTIBLES; ++i) collectibleTaken[i] = false;
    lastBirdX = birdX;
    lastBirdZ = birdZ;
}

void resetShowcaseCamera() {
    birdZoom = 8.0f; birdCamYaw = 18.0f; birdCamPitch = 16.0f;
    camX = 0.0f; camY = 4.5f; camZ = 10.0f;
    camLookX = 0.0f; camLookY = 2.0f; camLookZ = 0.0f;
    camUpX = 0.0f; camUpY = 1.0f; camUpZ = 0.0f;
}

// ============================================================
// GEOMETRY HELPERS
// ============================================================

void drawBox(float hx, float hy, float hz) {
    GLfloat verts[8][3] = {
        {-hx,-hy,-hz},{hx,-hy,-hz},{hx,hy,-hz},{-hx,hy,-hz},
        {-hx,-hy, hz},{hx,-hy, hz},{hx,hy, hz},{-hx,hy, hz}
    };
    auto face = [&](int a,int b,int c,int d, float r,float g,float bl){
        if (drawLines) { glColor3f(1,1,1); glBegin(GL_LINE_LOOP); }
        else           { glColor3f(r,g,bl); glBegin(GL_POLYGON); }
        glVertex3fv(verts[a]); glVertex3fv(verts[b]);
        glVertex3fv(verts[c]); glVertex3fv(verts[d]);
        glEnd();
    };
    face(1,0,3,2, 0.7f,0.5f,0.3f);
    face(4,5,6,7, 0.7f,0.5f,0.3f);
    face(0,4,7,3, 0.6f,0.4f,0.2f);
    face(5,1,2,6, 0.6f,0.4f,0.2f);
    face(3,7,6,2, 0.8f,0.6f,0.4f);
    face(0,1,5,4, 0.5f,0.35f,0.2f);
}

// ============================================================
// SKY
// ============================================================

void drawSky() {
    const ThemePalette& palette = kThemePalettes[worldTheme];
    GLfloat skyTop[3], skyMid[3], skyHorizon[3];
    applyTimeOfDayToPalette(palette, skyTop, skyMid, skyHorizon);
    float sunHeight = getSunHeight();
    float daylight = clamp01(getDayBlend() * 1.15f);

    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(-1, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glBegin(GL_QUADS);
    glColor3f(skyTop[0], skyTop[1], skyTop[2]); glVertex2f(-1,  1.0f);
    glColor3f(skyTop[0], skyTop[1], skyTop[2]); glVertex2f( 1,  1.0f);
    glColor3f(skyMid[0], skyMid[1], skyMid[2]); glVertex2f( 1,  0.1f);
    glColor3f(skyMid[0], skyMid[1], skyMid[2]); glVertex2f(-1,  0.1f);
    glEnd();

    glBegin(GL_QUADS);
    glColor3f(skyMid[0], skyMid[1], skyMid[2]); glVertex2f(-1,  0.1f);
    glColor3f(skyMid[0], skyMid[1], skyMid[2]); glVertex2f( 1,  0.1f);
    glColor3f(skyHorizon[0], skyHorizon[1], skyHorizon[2]); glVertex2f( 1, -0.2f);
    glColor3f(skyHorizon[0], skyHorizon[1], skyHorizon[2]); glVertex2f(-1, -0.2f);
    glEnd();

    glBegin(GL_QUADS);
    glColor3f(skyHorizon[0], skyHorizon[1], skyHorizon[2]); glVertex2f(-1, -0.2f);
    glColor3f(skyHorizon[0], skyHorizon[1], skyHorizon[2]); glVertex2f( 1, -0.2f);
    glColor3f(palette.groundB[0], palette.groundB[1], palette.groundB[2]); glVertex2f( 1, -1.0f);
    glColor3f(palette.groundB[0], palette.groundB[1], palette.groundB[2]); glVertex2f(-1, -1.0f);
    glEnd();

    if (daylight < 0.42f) {
        glPointSize(2.0f);
        glBegin(GL_POINTS);
        for (int i = 0; i < 48; ++i) {
            float fx = -0.96f + fmodf(i * 0.173f, 1.92f);
            float fy = -0.02f + fmodf(i * 0.347f, 1.02f);
            float twinkle = 0.45f + 0.55f * sinf(simTime * 2.8f + i * 1.37f);
            glColor4f(1.0f, 1.0f, 0.92f, (1.0f - daylight) * twinkle);
            glVertex2f(fx, fy);
        }
        glEnd();
    }

    glColor4f(1.0f, 0.96f, 0.92f, 0.20f + daylight * 0.12f);
    glPushMatrix();
    glTranslatef(0.0f, -0.05f, 0.0f);
    glScalef(1.15f, 0.22f, 1.0f);
    drawFilledCircle(1.0f, 64);
    glPopMatrix();

    GLfloat orb[3] = {1.0f, 0.92f, 0.26f};
    if (daylight < 0.35f) {
        orb[0] = 0.92f; orb[1] = 0.96f; orb[2] = 1.0f;
    }
    glColor4f(orb[0], orb[1], orb[2], 0.85f);
    drawFilledCircle(1.0f, 48);
    glPushMatrix();
    glTranslatef(-0.75f + worldTime * 1.5f, 0.58f * sunHeight + 0.18f, 0.0f);
    glScalef(0.12f, 0.12f, 1.0f);
    drawFilledCircle(1.0f, 48);
    glPopMatrix();

    glColor4f(1.0f, 1.0f, 1.0f, 0.14f);
    float cloudDrift = fmodf(simTime * 0.01f * (0.5f + windStrength), 2.0f);
    glPushMatrix();
    glTranslatef(-0.55f + cloudDrift, 0.28f, 0.0f);
    glScalef(0.38f, 0.14f, 1.0f);
    drawFilledCircle(1.0f, 48);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.58f - cloudDrift * 0.7f, 0.34f, 0.0f);
    glScalef(0.32f, 0.12f, 1.0f);
    drawFilledCircle(1.0f, 48);
    glPopMatrix();

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
}

// ============================================================
// SUN
// ============================================================

void drawSun() {
    float sunHeight = getSunHeight();
    float daylight = clamp01(getDayBlend() * 1.08f);
    GLfloat orb[3] = {1.0f, 0.94f, 0.2f};
    if (daylight < 0.36f) {
        orb[0] = 0.92f; orb[1] = 0.96f; orb[2] = 1.0f;
    }
    glPushMatrix();
    glColor3f(orb[0], orb[1], orb[2]);
    glTranslatef(-28.0f + worldTime * 48.0f, 20.0f + sunHeight * 14.0f, -35.0f);
    glutSolidSphere(2.0, 24, 24);
    glColor4f(orb[0], orb[1], orb[2], 0.18f);
    glutSolidSphere(2.8, 24, 24);
    glPopMatrix();
}

// ============================================================
// CLOUDS
// ============================================================

void drawCloud(float x, float y, float z, float scale) {
    float daylight = clamp01(getDayBlend() * 1.05f);
    glColor3f(0.85f + daylight * 0.13f, 0.87f + daylight * 0.11f, 0.94f + daylight * 0.06f);
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(scale, scale * 0.6f, scale);
    glutSolidSphere(1.0, 14, 10);
    glTranslatef(0.9f, 0.2f, 0.1f);
    glColor3f(1.0f, 1.0f, 1.0f);
    glutSolidSphere(0.85, 12, 8);
    glTranslatef(-1.8f, 0.1f, 0.0f);
    glutSolidSphere(0.9, 12, 8);
    glTranslatef(0.9f, 0.3f, 0.0f);
    glutSolidSphere(0.7, 10, 8);
    glPopMatrix();
}

// ============================================================
// POND
// ============================================================

void drawPond() {
    const ThemePalette& palette = kThemePalettes[worldTheme];
    glPushMatrix();
    glTranslatef(14.0f, 0.01f, 14.0f);
    glRotatef(-90, 1, 0, 0);
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(palette.water[0], palette.water[1], palette.water[2]); glVertex3f(0, 0, 0);
    for (int i = 0; i <= 28; i++) {
        float ang = i * PI * 2.0f / 28.0f;
        float blend = 0.45f + 0.55f * (0.5f + 0.5f * sinf(ang + 0.6f + simTime * 1.2f));
        glColor3f(palette.water[0] * (0.72f + 0.22f * blend),
                  palette.water[1] * (0.72f + 0.18f * blend),
                  palette.water[2] * (0.84f + 0.16f * blend));
        glVertex3f(cosf(ang) * 6.0f, sinf(ang) * 4.0f, 0);
    }
    glEnd();

    glColor4f(1.0f, 1.0f, 1.0f, 0.22f);
    glPushMatrix();
    glTranslatef(-1.1f, 1.0f, 0.0f);
    glScalef(2.3f, 0.7f, 1.0f);
    drawFilledCircle(1.0f, 40);
    glPopMatrix();
    glPopMatrix();
}

// ============================================================
// MOUNTAINS
// ============================================================

void drawMountains() {
    float dayBlend = clamp01(getDayBlend() * 1.05f);
    float tint = 0.72f + dayBlend * 0.38f;
    glBegin(GL_TRIANGLES);
    glColor3f(0.54f * tint, 0.60f * tint, 0.67f * tint); glVertex3f(-45, 0, -62);
    glColor3f(0.64f * tint, 0.69f * tint, 0.76f * tint); glVertex3f(-28, 8, -70);
    glColor3f(0.54f * tint, 0.60f * tint, 0.67f * tint); glVertex3f(-11, 0, -62);

    glColor3f(0.52f * tint, 0.58f * tint, 0.66f * tint); glVertex3f(-18, 0, -62);
    glColor3f(0.66f * tint, 0.71f * tint, 0.78f * tint); glVertex3f(2, 10, -72);
    glColor3f(0.52f * tint, 0.58f * tint, 0.66f * tint); glVertex3f(22, 0, -62);

    glColor3f(0.50f * tint, 0.57f * tint, 0.63f * tint); glVertex3f(10, 0, -62);
    glColor3f(0.63f * tint, 0.69f * tint, 0.74f * tint); glVertex3f(30, 9, -69);
    glColor3f(0.50f * tint, 0.57f * tint, 0.63f * tint); glVertex3f(50, 0, -62);
    glEnd();

    glColor3f(0.36f * tint, 0.43f * tint, 0.50f * tint);
    glBegin(GL_TRIANGLES);
    glVertex3f(-35, 0, -40); glVertex3f(-18, 11, -50); glVertex3f(-1,  0, -40);
    glVertex3f(-28, 0, -40); glVertex3f(-13, 8.5f,-52); glVertex3f( 2,  0, -40);
    glVertex3f(  3, 0, -40); glVertex3f( 18, 12, -52); glVertex3f(33,  0, -40);
    glEnd();
    // Snow caps
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLES);
    glVertex3f(-20,11,-50); glVertex3f(-18,13,-50); glVertex3f(-16,11,-50);
    glVertex3f( -2, 9,-52); glVertex3f(  0,11,-52); glVertex3f(  2, 9,-52);
    glVertex3f( 16,12,-52); glVertex3f( 18,14,-52); glVertex3f( 20,12,-52);
    glEnd();
}

void drawTreeLine() {
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < 14; ++i) {
        float baseX = -38.0f + i * 6.0f;
        float width = 3.8f + (i % 3) * 0.8f;
        float height = 3.0f + (i % 4) * 1.0f;
        glColor3f(0.11f, 0.30f, 0.12f);
        glVertex3f(baseX - width, 0.0f, -30.0f - (i % 2) * 2.0f);
        glColor3f(0.18f, 0.40f, 0.16f);
        glVertex3f(baseX, height, -31.5f - (i % 2) * 2.0f);
        glColor3f(0.11f, 0.30f, 0.12f);
        glVertex3f(baseX + width, 0.0f, -30.0f - (i % 2) * 2.0f);
    }
    glEnd();
}

// ============================================================
// GROUND
// ============================================================

void drawGround() {
    const ThemePalette& palette = kThemePalettes[worldTheme];
    const float baseY = 0.0f;
    const float detailY = 0.03f;
    const float accentY = 0.05f;

    glColor3f(palette.groundA[0], palette.groundA[1], palette.groundA[2]);
    glBegin(GL_QUADS);
    glVertex3f(-40,baseY,-40); glVertex3f(40,baseY,-40);
    glVertex3f(40,baseY, 40);  glVertex3f(-40,baseY, 40);
    glEnd();

    glColor3f(palette.groundA[0] * 0.78f, palette.groundA[1] * 0.88f, palette.groundA[2] * 0.82f);
    glBegin(GL_QUADS);
    glVertex3f(-20,detailY, 5); glVertex3f( -5,detailY, 5);
    glVertex3f( -5,detailY,20); glVertex3f(-20,detailY,20);
    glEnd();
    glBegin(GL_QUADS);
    glVertex3f(8,detailY,-8);  glVertex3f(18,detailY,-8);
    glVertex3f(18,detailY, 5); glVertex3f( 8,detailY, 5);
    glEnd();

    glColor3f(palette.groundB[0], palette.groundB[1], palette.groundB[2]);
    glBegin(GL_QUADS);
    glVertex3f(-6,detailY, 2); glVertex3f(6,detailY, 2);
    glVertex3f( 6,detailY,10); glVertex3f(-6,detailY,10);
    glEnd();

    glColor3f(0.72f, 0.58f, 0.35f);
    glBegin(GL_QUADS);
    glVertex3f(-1,accentY,2.0f); glVertex3f(1,accentY,2.0f);
    glVertex3f( 1,accentY,10);   glVertex3f(-1,accentY,10);
    glEnd();

    glColor3f(palette.groundB[0] * 1.02f, palette.groundB[1] * 1.02f, palette.groundB[2] * 0.95f);
    glBegin(GL_QUADS);
    glVertex3f(-24,detailY,-16); glVertex3f(-6,detailY,-16);
    glVertex3f(-6,detailY, -2); glVertex3f(-24,detailY,-2);
    glEnd();

    glColor3f(palette.groundA[0] * 0.72f, palette.groundA[1] * 0.84f, palette.groundA[2] * 0.82f);
    glBegin(GL_QUADS);
    glVertex3f(10,detailY,8); glVertex3f(24,detailY,8);
    glVertex3f(24,detailY,22); glVertex3f(10,detailY,22);
    glEnd();

    // Meadow bands add depth without relying on nearly coplanar translucent overlays.
    glColor3f(palette.groundA[0] * 0.90f, palette.groundA[1] * 0.96f, palette.groundA[2] * 0.88f);
    for (int i = 0; i < 5; ++i) {
        float x = -28.0f + i * 13.0f;
        glBegin(GL_QUADS);
        glVertex3f(x, accentY, -40.0f);
        glVertex3f(x + 4.5f, accentY, -40.0f);
        glVertex3f(x + 11.0f, accentY,  40.0f);
        glVertex3f(x + 6.5f, accentY,  40.0f);
        glEnd();
    }
}

// ============================================================
// FLOWERS
// ============================================================

void drawFlowers(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0, z);

    // Stem
    glColor3f(0.2f, 0.55f, 0.1f);
    GLUquadric* q = gluNewQuadric();
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    gluCylinder(q, 0.02f, 0.02f, 0.25f, 4, 1);
    glPopMatrix();
    gluDeleteQuadric(q);

    // Centre
    glTranslatef(0, 0.25f, 0);
    glColor3f(1.0f, 0.85f, 0.1f);
    glutSolidSphere(0.07f, 6, 4);

    // Petals
    glColor3f(1.0f, 0.3f, 0.3f);
    for (int p = 0; p < 6; p++) {
        float a = p * PI / 3.0f;
        glPushMatrix();
        glTranslatef(cosf(a)*0.09f, 0, sinf(a)*0.09f);
        glutSolidSphere(0.05f, 5, 4);
        glPopMatrix();
    }
    glPopMatrix();
}

// ============================================================
// FENCE
// ============================================================

void drawFence(float x1, float z1, float x2, float z2) {
    glColor3f(0.72f, 0.58f, 0.38f);
    // Posts at each end
    for (int i = 0; i <= 1; i++) {
        glPushMatrix();
        float px = x1 + (x2-x1)*i;
        float pz = z1 + (z2-z1)*i;
        glTranslatef(px, 0.4f, pz);
        glScalef(0.08f, 0.8f, 0.08f);
        glutSolidCube(1.0);
        glPopMatrix();
    }
    // Middle post
    glPushMatrix();
    glTranslatef((x1+x2)*0.5f, 0.4f, (z1+z2)*0.5f);
    glScalef(0.08f, 0.8f, 0.08f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Top rail
    glBegin(GL_QUADS);
    glVertex3f(x1, 0.58f, z1); glVertex3f(x2, 0.58f, z2);
    glVertex3f(x2, 0.52f, z2); glVertex3f(x1, 0.52f, z1);
    glEnd();

    // Bottom rail
    glBegin(GL_QUADS);
    glVertex3f(x1, 0.28f, z1); glVertex3f(x2, 0.28f, z2);
    glVertex3f(x2, 0.22f, z2); glVertex3f(x1, 0.22f, z1);
    glEnd();
}

// ============================================================
// KID FIGURE
// ============================================================

void drawKid(float x, float z,
             float shirtR, float shirtG, float shirtB,
             float armAngle, float legAngle) {
    glPushMatrix();
    glTranslatef(x, 0, z);

    // Legs
    glColor3f(0.25f, 0.35f, 0.7f);
    glPushMatrix();
    glTranslatef(-0.08f, 0.2f, 0);
    glRotatef(legAngle, 1, 0, 0);
    glScalef(0.1f, 0.4f, 0.1f);
    glutSolidCube(1.0);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.08f, 0.2f, 0);
    glRotatef(-legAngle, 1, 0, 0);
    glScalef(0.1f, 0.4f, 0.1f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Shoes
    glColor3f(0.15f, 0.1f, 0.08f);
    glPushMatrix();
    glTranslatef(-0.08f, 0.02f, 0.04f);
    glScalef(0.12f, 0.07f, 0.18f);
    glutSolidCube(1.0);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.08f, 0.02f, 0.04f);
    glScalef(0.12f, 0.07f, 0.18f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Body / shirt
    glColor3f(shirtR, shirtG, shirtB);
    glPushMatrix();
    glTranslatef(0, 0.55f, 0);
    glScalef(0.28f, 0.38f, 0.18f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Left arm
    glColor3f(shirtR, shirtG, shirtB);
    glPushMatrix();
    glTranslatef(-0.20f, 0.58f, 0);
    glRotatef(-armAngle, 1, 0, 0);
    glScalef(0.09f, 0.35f, 0.09f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Right arm
    glPushMatrix();
    glTranslatef(0.20f, 0.58f, 0);
    glRotatef(armAngle, 1, 0, 0);
    glScalef(0.09f, 0.35f, 0.09f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Head
    glColor3f(0.95f, 0.78f, 0.60f);
    glPushMatrix();
    glTranslatef(0, 0.88f, 0);
    glutSolidSphere(0.16f, 10, 8);
    glPopMatrix();

    // Hair
    glColor3f(0.22f, 0.13f, 0.04f);
    glPushMatrix();
    glTranslatef(0, 0.97f, 0);
    glutSolidSphere(0.12f, 8, 6);
    glPopMatrix();

    glPopMatrix();
}

// ============================================================
// BALL
// ============================================================

void drawBall(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glColor3f(0.9f, 0.15f, 0.15f);
    glutSolidSphere(0.18f, 12, 10);
    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glScalef(1.0f, 0.15f, 1.0f);
    glutWireSphere(0.19f, 8, 2);
    glPopMatrix();
    glPopMatrix();
}

// ============================================================
// KIDS SCENE
// ============================================================

void drawKidsScene() {
    // Fence around the clearing
    drawFence(-6, 2,   6, 2);
    drawFence( 6, 2,   6, 10);
    drawFence( 6, 10, -6, 10);
    drawFence(-6, 10, -6, 2);

    // Flowers around fence line
    float fp[][2] = {
        {-5,2.5f},{-4.5f,5},{-5,7.5f},{-5,9.5f},
        { 5,3.0f},{ 4.8f,6},{ 5,8.5f},
        {-2,1.8f},{ 1,1.8f},{ 3,1.8f},
        { 4,1.8f},{-4,1.8f}
    };
    for (auto& f : fp)
        drawFlowers(f[0], f[1]);

    // Walking animation
    float walk = sinf(simTime * 3.0f) * 22.0f;

    // Kid 1 — red shirt, running left to right
    drawKid(-2.5f, 5.5f,  0.9f, 0.15f, 0.15f,  walk, walk);

    // Kid 2 — yellow shirt, arms raised to catch
    drawKid( 2.5f, 6.2f,  0.95f, 0.85f, 0.1f,  -38.0f, 12.0f);

    // Kid 3 — green shirt, standing watching
    drawKid(-0.5f, 8.5f,  0.15f, 0.72f, 0.22f,  8.0f, 0.0f);

    // Kid 4 — purple shirt, sitting (small scale)
    glPushMatrix();
    glTranslatef(3.5f, 0, 4.0f);
    glScalef(0.85f, 0.85f, 0.85f);
    glPopMatrix();
    drawKid( 3.5f, 4.0f,  0.65f, 0.2f, 0.85f,  20.0f, -5.0f);

    // Ball arcing between kid 1 and kid 2
    float ballArc = fabsf(sinf(simTime * 2.8f));
    drawBall(0.0f, 0.35f + ballArc * 2.2f, 5.8f);
}

// ============================================================
// TREE
// ============================================================

void drawCone(float base, float height, int slices) {
    GLUquadric* q = gluNewQuadric();
    gluQuadricNormals(q, GLU_SMOOTH);
    gluCylinder(q, base, 0.0f, height, slices, 1);
    glPushMatrix();
    glRotatef(180,1,0,0);
    gluDisk(q, 0, base, slices, 1);
    glPopMatrix();
    gluDeleteQuadric(q);
}

void drawTree(float x, float z, float scale) {
    glPushMatrix();
    glTranslatef(x, 0, z);
    glRotatef(sinf(simTime * (1.2f + scale * 0.25f) + x * 0.4f + z * 0.2f) * windStrength * 2.2f, 0, 0, 1);
    glScalef(scale, scale, scale);

    // Trunk
    glColor3f(0.42f, 0.26f, 0.10f);
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    GLUquadric* q = gluNewQuadric();
    gluCylinder(q, 0.15f, 0.09f, 1.3f, 8, 1);
    gluDeleteQuadric(q);
    glPopMatrix();

    // Three cone tiers — slightly varied greens
    float tierData[3][3] = {
        {0.8f, 1.0f, 0.55f},  // yOff, radius, height factor
        {1.5f, 0.75f, 0.48f},
        {2.1f, 0.52f, 0.42f}
    };
    float tierGreen[3] = {0.52f, 0.60f, 0.68f};
    for (int t = 0; t < 3; t++) {
        glColor3f(0.08f, tierGreen[t], 0.12f);
        glPushMatrix();
        glTranslatef(0, tierData[t][0], 0);
        glRotatef(-90, 1, 0, 0);
        drawCone(tierData[t][1], 1.1f - t*0.08f, 10);
        glPopMatrix();
    }
    glPopMatrix();
}

void drawFloatingIslands() {
    const ThemePalette& palette = kThemePalettes[worldTheme];
    for (int i = 0; i < 3; ++i) {
        float x = -26.0f + i * 24.0f + sinf(simTime * 0.18f + i) * 1.5f;
        float y = 12.0f + i * 2.8f + sinf(simTime * 0.9f + i * 0.7f) * 0.6f;
        float z = -24.0f - i * 7.0f;
        glPushMatrix();
        glTranslatef(x, y, z);
        glColor3f(palette.groundB[0], palette.groundB[1], palette.groundB[2]);
        glScalef(2.7f + i * 0.5f, 0.35f, 1.6f + i * 0.3f);
        glutSolidCube(1.0f);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(x, y - 0.55f, z);
        glColor3f(0.68f, 0.56f, 0.40f);
        glScalef(1.4f + i * 0.25f, 0.7f, 0.9f + i * 0.14f);
        glutSolidCone(0.7f, 1.2f, 12, 2);
        glPopMatrix();
    }
}

void drawDistantFlock() {
    glColor4f(0.10f, 0.12f, 0.16f, 0.65f);
    for (int i = 0; i < 6; ++i) {
        float x = -24.0f + i * 8.0f + fmodf(simTime * 2.0f + i * 1.7f, 34.0f);
        float y = 16.0f + sinf(simTime * 0.9f + i) * 1.1f;
        float z = -26.0f - i * 1.8f;
        glPushMatrix();
        glTranslatef(x, y, z);
        glRotatef(sinf(simTime * 7.0f + i) * 18.0f, 0, 0, 1);
        glBegin(GL_LINE_STRIP);
        glVertex3f(-0.35f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.14f, 0.0f);
        glVertex3f(0.35f, 0.0f, 0.0f);
        glEnd();
        glPopMatrix();
    }
}

void drawCollectibles() {
    for (int i = 0; i < NUM_COLLECTIBLES; ++i) {
        if (collectibleTaken[i]) continue;
        float bob = sinf(simTime * 2.4f + i) * 0.18f;
        glPushMatrix();
        glTranslatef(kCollectiblePositions[i][0], kCollectiblePositions[i][1] + bob, kCollectiblePositions[i][2]);
        glRotatef(collectibleSpin + i * 25.0f, 0, 1, 0);

        glColor4f(1.0f, 0.88f, 0.20f, 0.9f);
        glutSolidTorus(0.05f, 0.22f, 10, 22);

        glColor4f(1.0f, 0.98f, 0.74f, 0.35f);
        glutWireSphere(0.34f, 10, 10);
        glPopMatrix();
    }
}

void drawWeatherEffects() {
    if (weatherMode == WEATHER_RAIN) {
        glColor4f(0.72f, 0.84f, 1.0f, 0.34f);
        glBegin(GL_LINES);
        for (int i = 0; i < 70; ++i) {
            float x = -28.0f + fmodf(i * 1.7f + simTime * 16.0f, 56.0f);
            float z = -28.0f + fmodf(i * 2.3f + simTime * 9.0f, 56.0f);
            float y = 10.0f - fmodf(i * 0.6f + simTime * 20.0f, 10.0f);
            glVertex3f(x, y, z);
            glVertex3f(x - 0.5f, y - 1.4f, z + 0.2f);
        }
        glEnd();
    } else if (weatherMode == WEATHER_MAGIC) {
        glPointSize(4.0f);
        glBegin(GL_POINTS);
        for (int i = 0; i < 50; ++i) {
            float x = birdX - 10.0f + fmodf(i * 2.1f + simTime * 0.9f, 20.0f);
            float y = 1.2f + fmodf(i * 0.8f + simTime * 1.8f, 7.5f);
            float z = birdZ - 10.0f + fmodf(i * 3.2f + simTime * 0.7f, 20.0f);
            float glow = 0.55f + 0.45f * sinf(simTime * 4.0f + i * 0.7f);
            glColor4f(0.80f, 0.62f + glow * 0.25f, 1.0f, 0.25f + glow * 0.5f);
            glVertex3f(x, y, z);
        }
        glEnd();
    } else if (weatherMode == WEATHER_BREEZY) {
        glColor4f(1.0f, 1.0f, 1.0f, 0.18f);
        glBegin(GL_LINES);
        for (int i = 0; i < 18; ++i) {
            float x = -26.0f + fmodf(i * 3.4f + simTime * 6.0f, 54.0f);
            float y = 2.0f + fmodf(i * 0.7f + simTime * 1.3f, 12.0f);
            float z = -12.0f + sinf(simTime * 0.8f + i) * 10.0f;
            glVertex3f(x, y, z);
            glVertex3f(x + 1.6f, y + 0.1f, z);
        }
        glEnd();
    }
}

// ============================================================
// BUILDING
// ============================================================

void drawBuilding(float x, float z, float w, float d, float h,
                  float r, float g, float b) {
    // Main body
    glPushMatrix();
    glTranslatef(x, h*0.5f, z);
    glColor3f(r, g, b);
    glScalef(w, h, d);
    glutSolidCube(1.0);
    glPopMatrix();

    // Windows — simple grid of slightly lighter quads
    glColor3f(r+0.15f, g+0.15f, b+0.2f);
    int floors = (int)(h / 2.0f);
    for (int fl = 0; fl < floors; fl++) {
        float wy = 1.0f + fl * 2.0f;
        glBegin(GL_QUADS);
        glVertex3f(x - w*0.5f+0.2f, wy+0.3f, z - d*0.5f - 0.01f);
        glVertex3f(x - w*0.5f+0.6f, wy+0.3f, z - d*0.5f - 0.01f);
        glVertex3f(x - w*0.5f+0.6f, wy+0.8f, z - d*0.5f - 0.01f);
        glVertex3f(x - w*0.5f+0.2f, wy+0.8f, z - d*0.5f - 0.01f);
        glEnd();
        glBegin(GL_QUADS);
        glVertex3f(x + w*0.5f-0.6f, wy+0.3f, z - d*0.5f - 0.01f);
        glVertex3f(x + w*0.5f-0.2f, wy+0.3f, z - d*0.5f - 0.01f);
        glVertex3f(x + w*0.5f-0.2f, wy+0.8f, z - d*0.5f - 0.01f);
        glVertex3f(x + w*0.5f-0.6f, wy+0.8f, z - d*0.5f - 0.01f);
        glEnd();
    }

    // Roof
    glPushMatrix();
    glTranslatef(x, h + 0.06f, z);
    glColor3f(r*0.55f, g*0.55f, b*0.55f);
    glScalef(w+0.12f, 0.12f, d+0.12f);
    glutSolidCube(1.0);
    glPopMatrix();
}

void drawStreetLamp(float x, float z, float height) {
    glPushMatrix();
    glTranslatef(x, 0.0f, z);

    glColor3f(0.22f, 0.24f, 0.28f);
    glPushMatrix();
    glTranslatef(0.0f, height * 0.5f, 0.0f);
    glScalef(0.08f, height, 0.08f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, height + 0.05f, 0.0f);
    glScalef(0.42f, 0.08f, 0.08f);
    glutSolidCube(1.0f);
    glPopMatrix();

    float glow = 0.72f + 0.18f * clamp01(getDayBlend());
    glColor3f(1.0f, 0.92f * glow, 0.58f * glow);
    glPushMatrix();
    glTranslatef(0.16f, height, 0.0f);
    glutSolidSphere(0.11f, 8, 8);
    glPopMatrix();
    glPopMatrix();
}

void drawCar(float x, float z, float heading, float bodyR, float bodyG, float bodyB) {
    glPushMatrix();
    glTranslatef(x, 0.10f, z);
    glRotatef(heading, 0, 1, 0);

    glColor3f(bodyR, bodyG, bodyB);
    glPushMatrix();
    glTranslatef(0.0f, 0.18f, 0.0f);
    glScalef(0.85f, 0.22f, 1.45f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glColor3f(bodyR * 0.82f, bodyG * 0.84f, bodyB * 0.88f);
    glPushMatrix();
    glTranslatef(0.0f, 0.40f, -0.06f);
    glScalef(0.62f, 0.20f, 0.72f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glColor3f(0.78f, 0.88f, 0.96f);
    glBegin(GL_QUADS);
    glVertex3f(-0.22f, 0.46f, -0.34f); glVertex3f(0.22f, 0.46f, -0.34f);
    glVertex3f(0.22f, 0.34f, -0.56f);  glVertex3f(-0.22f, 0.34f, -0.56f);
    glEnd();

    glColor3f(0.10f, 0.10f, 0.12f);
    const float wheelX[2] = {-0.34f, 0.34f};
    const float wheelZ[2] = {-0.50f, 0.50f};
    for (float wx : wheelX) {
        for (float wz : wheelZ) {
            glPushMatrix();
            glTranslatef(wx, 0.06f, wz);
            glRotatef(90.0f, 0, 1, 0);
            glutSolidTorus(0.05f, 0.11f, 8, 10);
            glPopMatrix();
        }
    }
    glPopMatrix();
}

void drawCityWalker(float x, float z, float shirtR, float shirtG, float shirtB, float phase) {
    float swing = sinf(simTime * 2.6f + phase) * 18.0f;
    glPushMatrix();
    glTranslatef(x, 0.0f, z);
    glScalef(0.55f, 0.55f, 0.55f);
    drawKid(0.0f, 0.0f, shirtR, shirtG, shirtB, swing, swing * 0.8f);
    glPopMatrix();
}

void drawTrafficLight(float x, float z, bool verticalRoadGreen) {
    glPushMatrix();
    glTranslatef(x, 0.0f, z);

    glColor3f(0.18f, 0.20f, 0.23f);
    glPushMatrix();
    glTranslatef(0.0f, 1.15f, 0.0f);
    glScalef(0.09f, 2.3f, 0.09f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 2.05f, 0.0f);
    glScalef(0.22f, 0.55f, 0.20f);
    glutSolidCube(1.0f);
    glPopMatrix();

    const GLfloat brightRed[3] = {0.95f, 0.16f, 0.12f};
    const GLfloat dimRed[3] = {0.32f, 0.08f, 0.08f};
    const GLfloat brightYellow[3] = {0.95f, 0.78f, 0.12f};
    const GLfloat dimYellow[3] = {0.34f, 0.26f, 0.07f};
    const GLfloat brightGreen[3] = {0.16f, 0.88f, 0.24f};
    const GLfloat dimGreen[3] = {0.08f, 0.24f, 0.08f};

    const GLfloat* red = verticalRoadGreen ? dimRed : brightRed;
    const GLfloat* yellow = (fmodf(simTime, 6.0f) > 4.6f && fmodf(simTime, 6.0f) < 5.2f) ? brightYellow : dimYellow;
    const GLfloat* green = verticalRoadGreen ? brightGreen : dimGreen;
    const float lensY[3] = {2.22f, 2.05f, 1.88f};
    const GLfloat* colors[3] = {red, yellow, green};

    for (int i = 0; i < 3; ++i) {
        glColor3f(colors[i][0], colors[i][1], colors[i][2]);
        glPushMatrix();
        glTranslatef(0.0f, lensY[i], 0.11f);
        glutSolidSphere(0.05f, 8, 8);
        glPopMatrix();
    }
    glPopMatrix();
}

void drawBusStop(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0.0f, z);

    glColor3f(0.26f, 0.28f, 0.32f);
    const float postX[2] = {-0.7f, 0.7f};
    for (float px : postX) {
        glPushMatrix();
        glTranslatef(px, 0.6f, 0.0f);
        glScalef(0.08f, 1.2f, 0.08f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    glColor3f(0.90f, 0.84f, 0.26f);
    glPushMatrix();
    glTranslatef(0.0f, 1.26f, 0.0f);
    glScalef(1.6f, 0.08f, 0.55f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glColor4f(0.72f, 0.88f, 0.98f, 0.72f);
    glBegin(GL_QUADS);
    glVertex3f(-0.62f, 1.02f, 0.20f); glVertex3f(0.62f, 1.02f, 0.20f);
    glVertex3f(0.62f, 0.24f, 0.20f);  glVertex3f(-0.62f, 0.24f, 0.20f);
    glEnd();

    glColor3f(0.46f, 0.28f, 0.12f);
    glPushMatrix();
    glTranslatef(0.0f, 0.28f, -0.05f);
    glScalef(1.0f, 0.08f, 0.22f);
    glutSolidCube(1.0f);
    glPopMatrix();
    glPopMatrix();
}

void drawStorefront(float x, float z, float width, float height, float awningR, float awningG, float awningB) {
    glColor3f(0.84f, 0.80f, 0.72f);
    glPushMatrix();
    glTranslatef(x, height * 0.5f, z);
    glScalef(width, height, 1.5f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glColor3f(awningR, awningG, awningB);
    glPushMatrix();
    glTranslatef(x, height + 0.15f, z + 0.55f);
    glScalef(width + 0.12f, 0.12f, 0.55f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glColor3f(0.68f, 0.86f, 0.96f);
    glBegin(GL_QUADS);
    glVertex3f(x - width * 0.32f, 1.15f, z + 0.76f);
    glVertex3f(x + width * 0.32f, 1.15f, z + 0.76f);
    glVertex3f(x + width * 0.32f, 0.45f, z + 0.76f);
    glVertex3f(x - width * 0.32f, 0.45f, z + 0.76f);
    glEnd();
}

void drawCityScene() {
    const float roadY = 0.04f;
    const float lineY = 0.055f;
    const float walkY = 0.05f;

    // Main roads
    glColor3f(0.18f, 0.19f, 0.22f);
    glBegin(GL_QUADS);
    glVertex3f(-24.0f, roadY, -28.5f); glVertex3f(24.0f, roadY, -28.5f);
    glVertex3f(24.0f, roadY, -21.5f);  glVertex3f(-24.0f, roadY, -21.5f);
    glEnd();
    glBegin(GL_QUADS);
    glVertex3f(-2.8f, roadY, -34.0f); glVertex3f(2.8f, roadY, -34.0f);
    glVertex3f(2.8f, roadY, -16.0f);  glVertex3f(-2.8f, roadY, -16.0f);
    glEnd();

    // Sidewalks
    glColor3f(0.56f, 0.56f, 0.60f);
    glBegin(GL_QUADS);
    glVertex3f(-24.0f, walkY, -30.0f); glVertex3f(24.0f, walkY, -30.0f);
    glVertex3f(24.0f, walkY, -28.5f);  glVertex3f(-24.0f, walkY, -28.5f);
    glVertex3f(-24.0f, walkY, -21.5f); glVertex3f(24.0f, walkY, -21.5f);
    glVertex3f(24.0f, walkY, -20.0f);  glVertex3f(-24.0f, walkY, -20.0f);
    glVertex3f(-4.2f, walkY, -34.0f);  glVertex3f(-2.8f, walkY, -34.0f);
    glVertex3f(-2.8f, walkY, -16.0f);  glVertex3f(-4.2f, walkY, -16.0f);
    glVertex3f(2.8f, walkY, -34.0f);   glVertex3f(4.2f, walkY, -34.0f);
    glVertex3f(4.2f, walkY, -16.0f);   glVertex3f(2.8f, walkY, -16.0f);
    glEnd();

    // Lane markings
    glColor3f(0.96f, 0.92f, 0.55f);
    for (int i = 0; i < 6; ++i) {
        float x = -21.0f + i * 8.0f;
        glBegin(GL_QUADS);
        glVertex3f(x, lineY, -25.18f); glVertex3f(x + 3.6f, lineY, -25.18f);
        glVertex3f(x + 3.6f, lineY, -24.82f); glVertex3f(x, lineY, -24.82f);
        glEnd();
    }
    for (int i = 0; i < 4; ++i) {
        float z = -32.0f + i * 4.5f;
        glBegin(GL_QUADS);
        glVertex3f(-0.18f, lineY, z); glVertex3f(0.18f, lineY, z);
        glVertex3f(0.18f, lineY, z + 2.2f); glVertex3f(-0.18f, lineY, z + 2.2f);
        glEnd();
    }

    // Crosswalk
    glColor3f(0.94f, 0.94f, 0.96f);
    for (int i = 0; i < 7; ++i) {
        float x = -3.0f + i * 0.9f;
        glBegin(GL_QUADS);
        glVertex3f(x, lineY, -27.2f); glVertex3f(x + 0.5f, lineY, -27.2f);
        glVertex3f(x + 0.5f, lineY, -22.8f); glVertex3f(x, lineY, -22.8f);
        glEnd();
    }

    // Buildings
    drawBuilding(-20,-18.0f, 3.5f,3.0f, 8.5f, 0.60f,0.60f,0.65f);
    drawBuilding(-15,-17.2f, 2.6f,2.6f,12.5f, 0.55f,0.55f,0.60f);
    drawBuilding(-10,-18.5f, 4.2f,3.2f, 6.8f, 0.65f,0.60f,0.60f);
    drawBuilding( -5,-17.6f, 2.2f,2.2f,15.5f, 0.50f,0.50f,0.58f);
    drawBuilding(  1,-19.0f, 5.2f,3.5f, 9.6f, 0.60f,0.58f,0.62f);
    drawBuilding(  8,-17.5f, 2.8f,2.6f,11.8f, 0.58f,0.62f,0.60f);
    drawBuilding( 14,-17.0f, 3.0f,3.0f, 7.5f, 0.62f,0.60f,0.58f);
    drawBuilding( 19,-18.3f, 2.4f,2.6f,13.2f, 0.55f,0.58f,0.65f);

    // Storefront row closer to the street
    drawStorefront(-18.0f, -31.8f, 2.8f, 1.9f, 0.82f, 0.30f, 0.24f);
    drawStorefront(-13.5f, -31.6f, 2.2f, 1.8f, 0.24f, 0.60f, 0.88f);
    drawStorefront(10.5f, -31.8f, 2.6f, 1.9f, 0.22f, 0.68f, 0.34f);
    drawStorefront(15.0f, -31.5f, 2.3f, 1.7f, 0.88f, 0.72f, 0.18f);

    // Small plaza kiosk
    glColor3f(0.72f, 0.42f, 0.28f);
    glPushMatrix();
    glTranslatef(-8.0f, 0.55f, -31.6f);
    glScalef(1.5f, 1.1f, 1.2f);
    glutSolidCube(1.0f);
    glPopMatrix();
    glColor3f(0.95f, 0.82f, 0.28f);
    glPushMatrix();
    glTranslatef(-8.0f, 1.25f, -31.6f);
    glScalef(1.9f, 0.16f, 1.5f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Street lamps
    const float lampXs[] = {-18.0f, -9.5f, -1.0f, 7.5f, 16.0f};
    for (float x : lampXs) {
        drawStreetLamp(x, -20.8f, 1.8f);
        drawStreetLamp(x, -29.2f, 1.8f);
    }

    // Traffic control at the intersection
    drawTrafficLight(-4.6f, -21.0f, true);
    drawTrafficLight( 4.6f, -21.0f, true);
    drawTrafficLight(-4.6f, -29.0f, false);
    drawTrafficLight( 4.6f, -29.0f, false);

    // Bus stop
    drawBusStop(13.6f, -30.2f);

    // Cars
    float carA = -22.0f + fmodf(simTime * 6.0f, 44.0f);
    float carB = 22.0f - fmodf(simTime * 4.8f, 44.0f);
    float carC = -31.5f + fmodf(simTime * 5.2f, 16.0f);
    float carD = -22.0f + fmodf(simTime * 3.9f + 11.0f, 44.0f);
    drawCar(carA, -26.3f, 90.0f, 0.88f, 0.20f, 0.16f);
    drawCar(carB, -23.7f, -90.0f, 0.18f, 0.42f, 0.88f);
    drawCar(1.4f, carC, 180.0f, 0.94f, 0.82f, 0.20f);
    drawCar(carD, -26.1f, 90.0f, 0.22f, 0.72f, 0.68f);

    // People and activity
    drawCityWalker(-11.4f, -30.8f, 0.90f, 0.28f, 0.22f, 0.0f);
    drawCityWalker(-6.8f, -30.5f, 0.18f, 0.66f, 0.84f, 1.5f);
    drawCityWalker(6.2f, -30.7f, 0.92f, 0.70f, 0.16f, 2.6f);
    drawCityWalker(11.6f, -30.4f, 0.66f, 0.36f, 0.88f, 3.7f);
    drawCityWalker(3.6f, -19.2f, 0.20f, 0.78f, 0.34f, 4.4f);
    drawCityWalker(14.3f, -30.2f, 0.18f, 0.46f, 0.88f, 5.1f);
    drawCityWalker(16.0f, -30.8f, 0.86f, 0.42f, 0.18f, 5.8f);

    // Bench and street activity near the kiosk
    glColor3f(0.46f, 0.28f, 0.12f);
    glPushMatrix();
    glTranslatef(-5.8f, 0.28f, -31.0f);
    glScalef(0.9f, 0.08f, 0.28f);
    glutSolidCube(1.0f);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-6.15f, 0.16f, -31.0f);
    glScalef(0.08f, 0.32f, 0.08f);
    glutSolidCube(1.0f);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-5.45f, 0.16f, -31.0f);
    glScalef(0.08f, 0.32f, 0.08f);
    glutSolidCube(1.0f);
    glPopMatrix();

    drawBall(-7.4f + sinf(simTime * 1.6f) * 0.6f, 0.30f, -30.8f + cosf(simTime * 1.6f) * 0.3f);

    // Cafe tables
    glColor3f(0.78f, 0.78f, 0.82f);
    for (int i = 0; i < 2; ++i) {
        float tableX = 6.5f + i * 2.0f;
        glPushMatrix();
        glTranslatef(tableX, 0.42f, -31.2f);
        glScalef(0.55f, 0.05f, 0.55f);
        glutSolidCube(1.0f);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(tableX, 0.20f, -31.2f);
        glScalef(0.08f, 0.40f, 0.08f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }
}

// ============================================================
// BIRD SHADOW
// ============================================================

void drawBirdShadow() {
    glPushMatrix();
    glTranslatef(birdX, 0.02f, birdZ);
    glRotatef(birdYaw, 0, 1, 0);
    glColor4f(0.12f, 0.06f, 0.06f, 0.20f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0,0,0);
    for (int i = 0; i <= 40; ++i) {
        float ang = (2.0f * PI * i) / 40.0f;
        glVertex3f(cosf(ang)*1.6f*birdScale, 0, sinf(ang)*1.0f*birdScale);
    }
    glEnd();
    glPopMatrix();
}

// ============================================================
// BIRD
// ============================================================

void drawBird(float flapAngle) {
    glPushMatrix();
    glTranslatef(birdX, birdY, birdZ);
    glRotatef(birdYaw,   0, 1, 0);
    glRotatef(birdPitch, 1, 0, 0);
    glRotatef(birdRoll,  0, 0, 1);
    glScalef(birdScale, birdScale, birdScale);

    if (birdLook == 0) {
        // Simple boxy bird
        glColor3f(0.3f, 0.3f, 0.35f);
        glPushMatrix(); glScalef(0.5f,0.2f,0.8f); glutSolidSphere(0.5,12,10); glPopMatrix();
        glColor3f(0.35f,0.35f,0.4f);
        glPushMatrix(); glTranslatef(0,0.1f,-0.45f); glutSolidSphere(0.18f,8,6); glPopMatrix();
        glColor3f(0.9f,0.7f,0.1f);
        glPushMatrix(); glTranslatef(0,0.05f,-0.65f); glScalef(0.1f,0.08f,0.25f); glutSolidCone(1,1,6,1); glPopMatrix();
        glColor3f(0.25f,0.25f,0.3f);
        glPushMatrix(); glTranslatef(-0.25f,0,0); glRotatef(flapAngle,0,0,1); glScalef(0.8f,0.08f,0.5f); drawBox(0.5f,0.5f,0.5f); glPopMatrix();
        glPushMatrix(); glTranslatef(0.25f,0,0); glRotatef(-flapAngle,0,0,1); glScalef(0.8f,0.08f,0.5f); drawBox(0.5f,0.5f,0.5f); glPopMatrix();
        glColor3f(0.2f,0.2f,0.25f);
        glPushMatrix(); glTranslatef(0,-0.05f,0.45f); glScalef(0.3f,0.08f,0.3f); drawBox(0.5f,0.5f,0.5f); glPopMatrix();

    } else if (birdLook == 1) {
        // Sleek glider
        glColor3f(0.22f,0.28f,0.33f);
        glPushMatrix(); glScalef(0.4f,0.16f,1.0f); drawBox(0.75f,0.5f,0.5f); glPopMatrix();
        glColor3f(0.15f,0.15f,0.18f);
        glPushMatrix(); glTranslatef(-0.05f,0.05f,-0.4f); glRotatef(5,1,0,0); glScalef(0.18f,0.18f,0.18f); glutSolidSphere(0.6,10,8); glPopMatrix();
        glColor3f(0.24f,0.27f,0.32f);
        glPushMatrix(); glTranslatef(-0.1f,0,0.2f); glRotatef(15,0,1,0); glScalef(1.5f,0.04f,0.25f); drawBox(0.5f,0.5f,0.5f); glPopMatrix();
        glPushMatrix(); glTranslatef(0.1f,0,0.2f); glRotatef(-15,0,1,0); glScalef(1.5f,0.04f,0.25f); drawBox(0.5f,0.5f,0.5f); glPopMatrix();

    } else if (birdLook == 2) {
        // Cartoon bird inspired by the reference image
        const GLfloat bodyRed[3]    = {0.96f, 0.05f, 0.18f};
        const GLfloat bellyCream[3] = {0.88f, 0.86f, 0.80f};
        const GLfloat beakYellow[3] = {0.96f, 0.84f, 0.12f};
        const GLfloat eyeBlack[3]   = {0.08f, 0.08f, 0.10f};
        const GLfloat crestPink[3]  = {0.80f, 0.14f, 0.58f};
        const GLfloat tailPurple[3] = {0.60f, 0.18f, 0.88f};
        const GLfloat greenBase[3]  = {0.12f, 0.78f, 0.22f};
        const GLfloat orangeTop[3]  = {0.98f, 0.62f, 0.10f};
        const GLfloat cyanTip[3]    = {0.12f, 0.84f, 0.90f};
        const GLfloat magentaAlt[3] = {0.78f, 0.18f, 0.94f};

        // Main body
        glColor3f(bodyRed[0], bodyRed[1], bodyRed[2]);
        glPushMatrix();
        glTranslatef(0.0f, 0.02f, 0.05f);
        glScalef(1.20f, 0.82f, 1.75f);
        glutSolidSphere(0.52f, 30, 24);
        glPopMatrix();

        // Soft belly
        glColor3f(bellyCream[0], bellyCream[1], bellyCream[2]);
        glPushMatrix();
        glTranslatef(0.0f, -0.34f, 0.12f);
        glScalef(0.88f, 0.42f, 1.05f);
        glutSolidSphere(0.48f, 24, 20);
        glPopMatrix();

        // Head
        glColor3f(bodyRed[0], bodyRed[1], bodyRed[2]);
        glPushMatrix();
        glTranslatef(0.0f, 0.34f, -0.88f);
        glScalef(0.66f, 0.66f, 0.66f);
        glutSolidSphere(0.42f, 24, 20);
        glPopMatrix();

        // Beak front disk
        glColor3f(beakYellow[0], beakYellow[1], beakYellow[2]);
        glPushMatrix();
        glTranslatef(0.0f, 0.27f, -1.18f);
        glRotatef(-90.0f, 1, 0, 0);
        glScalef(0.22f, 0.22f, 0.14f);
        glutSolidCone(1.0f, 1.0f, 24, 4);
        glPopMatrix();

        // Beak center
        glColor3f(0.92f, 0.72f, 0.06f);
        glPushMatrix();
        glTranslatef(0.0f, 0.29f, -1.10f);
        glScalef(0.17f, 0.17f, 0.20f);
        glutSolidSphere(1.0f, 18, 14);
        glPopMatrix();

        // Eyes
        glColor3f(eyeBlack[0], eyeBlack[1], eyeBlack[2]);
        glPushMatrix();
        glTranslatef(-0.19f, 0.34f, -1.00f);
        glutSolidSphere(0.09f, 14, 12);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(0.19f, 0.34f, -1.00f);
        glutSolidSphere(0.09f, 14, 12);
        glPopMatrix();

        glColor3f(1.0f, 1.0f, 1.0f);
        glPushMatrix();
        glTranslatef(-0.16f, 0.38f, -1.05f);
        glutSolidSphere(0.025f, 8, 8);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(0.16f, 0.38f, -1.05f);
        glutSolidSphere(0.025f, 8, 8);
        glPopMatrix();

        // Crest
        glColor3f(crestPink[0], crestPink[1], crestPink[2]);
        for (int c = 0; c < 2; ++c) {
            glPushMatrix();
            glTranslatef(-0.03f + c * 0.06f, 0.68f + c * 0.02f, -0.88f);
            glRotatef(-12.0f + c * 16.0f, 0, 0, 1);
            glRotatef(-18.0f, 1, 0, 0);
            glScalef(0.06f, 0.26f, 0.08f);
            drawBox(0.5f, 0.5f, 0.5f);
            glPopMatrix();
        }

        // Tail nub
        glColor3f(tailPurple[0], tailPurple[1], tailPurple[2]);
        glPushMatrix();
        glTranslatef(0.0f, 0.02f, 1.12f);
        glRotatef(22.0f, 1, 0, 0);
        glScalef(0.14f, 0.08f, 0.34f);
        drawBox(0.5f, 0.5f, 0.5f);
        glPopMatrix();

        // Blocky rainbow wings
        for (int side = -1; side <= 1; side += 2) {
            float sf = float(side);
            float sideFlap = sf * (flapAngle * 0.65f + 10.0f);

            glPushMatrix();
            glTranslatef(sf * 0.46f, -0.02f, 0.02f);
            glRotatef(sideFlap, 0, 0, 1);
            glRotatef(-12.0f, 1, 0, 0);

            for (int i = 0; i < 6; ++i) {
                bool tip = i >= 4;
                bool alt = (i % 2) == 0;
                const GLfloat* topColor = tip ? cyanTip : (alt ? orangeTop : magentaAlt);

                glColor3f(greenBase[0], greenBase[1], greenBase[2]);
                glPushMatrix();
                glTranslatef(sf * (0.14f + i * 0.18f), -0.10f - i * 0.02f, -0.02f + i * 0.01f);
                glScalef(0.18f, 0.05f, 0.30f);
                drawBox(0.5f, 0.5f, 0.5f);
                glPopMatrix();

                glColor3f(topColor[0], topColor[1], topColor[2]);
                glPushMatrix();
                glTranslatef(sf * (0.14f + i * 0.18f), -0.04f - i * 0.02f, -0.01f + i * 0.01f);
                glScalef(0.22f, 0.07f, 0.32f);
                drawBox(0.5f, 0.5f, 0.5f);
                glPopMatrix();
            }
            glPopMatrix();
        }

        // Small feet tucked under the body
        for (int side = -1; side <= 1; side += 2) {
            float sf = float(side);
            glColor3f(0.92f, 0.54f, 0.10f);
            glPushMatrix();
            glTranslatef(sf * 0.14f, -0.54f, 0.16f);
            glScalef(0.05f, 0.14f, 0.05f);
            drawBox(0.5f, 0.5f, 0.5f);
            glPopMatrix();

            glPushMatrix();
            glTranslatef(sf * 0.14f, -0.62f, 0.22f);
            glScalef(0.16f, 0.03f, 0.10f);
            drawBox(0.5f, 0.5f, 0.5f);
            glPopMatrix();
        }

    } else {
#ifdef USE_OBJ_BIRD
        // OBJ model bird (Experiment 5)
        static OBJImporter objBird(findAssetPath("models/bird.obj"));
        glEnable(GL_NORMALIZE);
        static const GLfloat lpos[] = {5,10,5,0};
        glLightfv(GL_LIGHT0, GL_POSITION, lpos);
        glEnable(GL_LIGHT0);
        glEnable(GL_LIGHTING);
        for (auto& f : objBird.faceList) {
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   objBird.materialTable[f.mtl_name].ambient);
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   objBird.materialTable[f.mtl_name].diffuse);
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  objBird.materialTable[f.mtl_name].specular);
            glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, objBird.materialTable[f.mtl_name].shininess);
            glBegin(GL_TRIANGLES);
            glNormal3f(objBird.normalList[f.na].x, objBird.normalList[f.na].y, objBird.normalList[f.na].z);
            glVertex3f(objBird.vertexList[f.a].x,  objBird.vertexList[f.a].y,  objBird.vertexList[f.a].z);
            glNormal3f(objBird.normalList[f.nb].x, objBird.normalList[f.nb].y, objBird.normalList[f.nb].z);
            glVertex3f(objBird.vertexList[f.b].x,  objBird.vertexList[f.b].y,  objBird.vertexList[f.b].z);
            glNormal3f(objBird.normalList[f.nc].x, objBird.normalList[f.nc].y, objBird.normalList[f.nc].z);
            glVertex3f(objBird.vertexList[f.c].x,  objBird.vertexList[f.c].y,  objBird.vertexList[f.c].z);
            glEnd();
        }
        glDisable(GL_LIGHTING);
#endif
    }
    glPopMatrix();
}

// ============================================================
// DRAW SCENE
// ============================================================

void drawScene() {
    drawSky();
    drawGround();
    drawSun();

    float cloudShift = simTime * (weatherMode == WEATHER_BREEZY ? 1.4f : 0.6f);
    drawCloud(-18.0f + fmodf(cloudShift, 70.0f), 22.5f, -30.0f, 1.3f);
    drawCloud( 12.0f - fmodf(cloudShift * 0.7f, 68.0f), 20.0f, -25.0f, 1.0f);
    drawCloud( 20.0f - fmodf(cloudShift * 0.5f, 66.0f), 24.0f, -18.0f, 0.9f);
    drawCloud( -5.0f + fmodf(cloudShift * 0.9f, 64.0f), 18.0f, -20.0f, 0.75f);
    drawCloud( -30.0f + fmodf(cloudShift * 0.8f, 62.0f),19.0f, -15.0f, 1.1f);

    drawMountains();
    drawFloatingIslands();
    drawDistantFlock();
    drawTreeLine();
    drawPond();
    drawKidsScene();

    // Forest
    float positions[][2] = {
        {-8,-5},{-5,-9},{0,-10},{5,-9},{8,-5},
        {-10,0},{-10,5},{-8,9},{-5,11},
        {10,0}, {10,5}, {8,9}, {5,11},
        {-3,-15},{3,-15},{0,-18},
        {-15,-2},{15,-2}
    };
    float scales[] = {1.2f,1.0f,1.3f,0.9f,1.1f,1.0f,1.2f,0.8f,1.0f,
                      1.0f,1.3f,0.9f,1.1f,0.8f,1.2f,1.0f,1.1f,0.9f};
    int numTrees = sizeof(positions)/sizeof(positions[0]);
    for (int i = 0; i < numTrees; i++)
        drawTree(positions[i][0], positions[i][1], scales[i]);

    // City
    drawCityScene();

    drawBirdShadow();
    drawCollectibles();
    drawWeatherEffects();

    float flapCycle = birdAutoFlap ? sinf(simTime * 4.0f) : 0.0f;
    float flapAngle = flapCycle * (24.0f + birdWingLift * 12.0f);
    drawBird(flapAngle);
}

// ============================================================
// HUD
// ============================================================

void drawBitmapText(float x, float y, const char* text) {
    glRasterPos2f(x, y);
    for (const char* c = text; *c; ++c)
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);
}

void drawOverlayPanel(float x1, float y1, float x2, float y2, float r, float g, float b, float a) {
    glColor4f(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex2f(x1, y1); glVertex2f(x2, y1);
    glVertex2f(x2, y2); glVertex2f(x1, y2);
    glEnd();
}

void drawHud() {
    if (!showHud) return;
    glMatrixMode(GL_PROJECTION);
    glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, winWidth, 0, winHeight);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix(); glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glColor3f(0.07f, 0.12f, 0.16f);

    const int startY  = winHeight - 22;
    const int lineStep = 18;
    char line[256];
    drawOverlayPanel(10.0f, winHeight - 86.0f, 730.0f, winHeight - 8.0f, 0.96f, 0.98f, 1.0f, 0.68f);
    std::snprintf(line, sizeof(line),
        "Theme: %s | Weather: %s | Score: %d/%d | Distance: %.1f | Camera: %d | Auto-flight [%s]",
        themeName(), weatherName(), score, NUM_COLLECTIBLES, travelDistance, int(cameraMode), birdAutoFlight ? "on" : "off");
    drawBitmapText(20.0f, startY, line);
    if (showControls) {
        drawBitmapText(20.0f, startY - lineStep,
            "Enter: start | P: pause | G: auto-flight | T: theme | Y: weather | M: demo camera | H: HUD | K: controls");
        drawBitmapText(20.0f, startY - lineStep*2,
            "Arrows/A,D: yaw-pitch | Q,E: roll | Z,X: zoom | W,S: speed | U,O: altitude | F: follow | 0-6: cameras");
    } else {
        drawBitmapText(20.0f, startY - lineStep,
            "K toggles detailed controls. Explore the scene, collect rings, and use T/Y to cycle showcase looks.");
    }

    if (paused) {
        drawOverlayPanel(winWidth * 0.28f, winHeight * 0.42f, winWidth * 0.72f, winHeight * 0.58f, 0.05f, 0.08f, 0.14f, 0.74f);
        glColor3f(0.96f, 0.98f, 1.0f);
        drawBitmapText(winWidth * 0.40f, winHeight * 0.53f, "Paused");
        drawBitmapText(winWidth * 0.33f, winHeight * 0.48f, "Press P to resume the showcase");
    }

    if (showTitleCard) {
        drawOverlayPanel(winWidth * 0.17f, winHeight * 0.26f, winWidth * 0.83f, winHeight * 0.66f, 0.04f, 0.08f, 0.15f, 0.76f);
        glColor3f(0.98f, 0.99f, 1.0f);
        drawBitmapText(winWidth * 0.36f, winHeight * 0.60f, "SkyBird Showcase");
        drawBitmapText(winWidth * 0.24f, winHeight * 0.55f, "Stylized flying-bird demo with themes, weather, collectibles, and camera modes");
        drawBitmapText(winWidth * 0.27f, winHeight * 0.49f, "Press Enter to begin, P to pause, T for theme, Y for weather, and M for auto-demo");
        drawBitmapText(winWidth * 0.31f, winHeight * 0.43f, "Use drag and keyboard controls to pose and fly the bird through the scene");
    }

    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// ============================================================
// DISPLAY
// ============================================================

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = float(winWidth) / float(winHeight ? winHeight : 1);
    gluPerspective(60.0f, aspect, mynear, myfar);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    switch (cameraMode) {
        case CAM_FOLLOW: {
            float rad = birdYaw * PI / 180.0f;
            float cx  = birdX + sinf(rad) * 3.0f;
            float cz  = birdZ + cosf(rad) * 3.0f;
            gluLookAt(cx, birdY+1.5f, cz, birdX, birdY, birdZ, 0,1,0);
            break;
        }
        case CAM_FRONT:
            gluLookAt(birdX, birdY+2.5f, birdZ+20.0f, birdX, birdY, birdZ, 0,1,0); break;
        case CAM_BACK:
            gluLookAt(birdX, birdY+2.5f, birdZ-20.0f, birdX, birdY, birdZ, 0,1,0); break;
        case CAM_LEFT:
            gluLookAt(birdX-20.0f, birdY+2.5f, birdZ,  birdX, birdY, birdZ, 0,1,0); break;
        case CAM_RIGHT:
            gluLookAt(birdX+20.0f, birdY+2.5f, birdZ,  birdX, birdY, birdZ, 0,1,0); break;
        case CAM_TOP:
            gluLookAt(birdX, birdY+25.0f, birdZ, birdX, birdY, birdZ, 0,0,-1); break;
        case CAM_FREE:
            gluLookAt(camX, camY, camZ, camLookX, camLookY, camLookZ, camUpX, camUpY, camUpZ); break;
        case CAM_SHOWCASE:
        default: {
            float showcaseYaw = birdCamYaw;
            float showcasePitch = birdCamPitch;
            float showcaseZoom = birdZoom;
            if (autoShowcase) {
                showcaseYaw += simTime * 10.0f;
                showcasePitch = 12.0f + sinf(simTime * 0.45f) * 6.0f;
                showcaseZoom = 7.8f + sinf(simTime * 0.37f) * 1.1f;
            }
            float yawRad   = showcaseYaw   * PI / 180.0f;
            float pitchRad = showcasePitch * PI / 180.0f;
            float focusX = birdX, focusY = birdY + 0.15f, focusZ = birdZ;
            float dXZ = showcaseZoom * cosf(pitchRad);
            float sX  = focusX + sinf(yawRad) * dXZ;
            float sY  = focusY + sinf(pitchRad) * showcaseZoom;
            float sZ  = focusZ + cosf(yawRad) * dXZ;
            gluLookAt(sX, sY, sZ, focusX, focusY, focusZ, 0,1,0);
            break;
        }
    }

    GLfloat bgTop[3], bgMid[3], bgHorizon[3];
    applyTimeOfDayToPalette(kThemePalettes[worldTheme], bgTop, bgMid, bgHorizon);
    glClearColor(bgMid[0], bgMid[1], bgMid[2], 1.0f);
    drawScene();
    drawHud();
    glutSwapBuffers();
}

// ============================================================
// RESHAPE
// ============================================================

void reshape(int w, int h) {
    winWidth = w; winHeight = h;
    glViewport(0, 0, w, h);
}

// ============================================================
// UPDATE
// ============================================================

void update() {
    simTime += deltaTime;
    if (!paused) {
        worldTime += deltaTime * worldTimeScale;
        if (worldTime > 1.0f) worldTime -= 1.0f;
        collectibleSpin += 70.0f * deltaTime;
    }

    if (paused) {
        glutPostRedisplay();
        glutTimerFunc(16, [](int){ update(); }, 0);
        return;
    }

    if (birdAutoFlight) {
        float rad = birdYaw * PI / 180.0f;
        birdX -= sinf(rad) * birdSpeed;
        birdZ -= cosf(rad) * birdSpeed;
    }

    float baseH = birdAutoFlight ? 2.2f : 2.0f;
    birdY = baseH + sinf(simTime * 1.5f) * birdBobAmount;

    if (birdX >  30) birdX = -30;
    if (birdX < -30) birdX =  30;
    if (birdZ >  30) birdZ = -30;
    if (birdZ < -30) birdZ =  30;

    float dx = birdX - lastBirdX;
    float dz = birdZ - lastBirdZ;
    travelDistance += sqrtf(dx * dx + dz * dz);
    lastBirdX = birdX;
    lastBirdZ = birdZ;

    for (int i = 0; i < NUM_COLLECTIBLES; ++i) {
        if (collectibleTaken[i]) continue;
        float cx = kCollectiblePositions[i][0];
        float cy = kCollectiblePositions[i][1];
        float cz = kCollectiblePositions[i][2];
        float ddx = birdX - cx;
        float ddy = birdY - cy;
        float ddz = birdZ - cz;
        if (ddx * ddx + ddy * ddy + ddz * ddz < 2.0f) {
            collectibleTaken[i] = true;
            score++;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, [](int){ update(); }, 0);
}

// ============================================================
// KEYBOARD
// ============================================================

void keyboard(unsigned char key, int x, int y) {
    float camDirX = camLookX - camX;
    float camDirY = camLookY - camY;
    float camDirZ = camLookZ - camZ;
    float camLen  = sqrtf(camDirX*camDirX + camDirY*camDirY + camDirZ*camDirZ);
    if (camLen > 0.0001f) { camDirX/=camLen; camDirY/=camLen; camDirZ/=camLen; }
    float camRightX =  camDirZ;
    float camRightZ = -camDirX;
    float moveSpeed = 0.35f;

    switch (key) {
        case 13: showTitleCard = false; paused = false; break;
        case 'e': birdRoll += birdTurnRate; break;
        case 'q': birdRoll -= birdTurnRate; break;
        case 'b': birdLook = (birdLook + 1) % 4; printf("Bird look: %d\n", birdLook); break;
        case '+': case '=': birdScale = fminf(birdScale+0.1f, 3.0f); break;
        case '-': case '_': birdScale = fmaxf(birdScale-0.1f, 0.6f); break;
        case '1': cameraMode = CAM_FRONT;    break;
        case '2': cameraMode = CAM_BACK;     break;
        case '3': cameraMode = CAM_LEFT;     break;
        case '4': cameraMode = CAM_RIGHT;    break;
        case '5': cameraMode = CAM_TOP;      break;
        case '6': cameraMode = CAM_FREE;     break;
        case 'f':
            cameraMode = (cameraMode == CAM_FOLLOW) ? CAM_SHOWCASE : CAM_FOLLOW;
            followCam  = (cameraMode == CAM_FOLLOW);
            printf("Follow cam: %s\n", followCam ? "ON" : "OFF");
            break;
        case '0': cameraMode = CAM_SHOWCASE; resetShowcaseCamera(); followCam = false; break;
        case ' ': birdAutoFlap = !birdAutoFlap; break;
        case 'h': showHud = !showHud; break;
        case 'k': showControls = !showControls; break;
        case 'm': autoShowcase = !autoShowcase; cameraMode = CAM_SHOWCASE; break;
        case 't': worldTheme = WorldTheme((worldTheme + 1) % 4); break;
        case 'y': weatherMode = WeatherMode((weatherMode + 1) % 4); break;
        case 'r': resetBirdPose(); cameraMode = CAM_SHOWCASE; resetShowcaseCamera(); break;
        case 'z': birdZoom = fmaxf(3.5f, birdZoom-0.35f); cameraMode = CAM_SHOWCASE; break;
        case 'x': birdZoom = fminf(20.0f, birdZoom+0.35f); cameraMode = CAM_SHOWCASE; break;
        case '[': birdWingSpread = fmaxf(0.55f, birdWingSpread-0.08f); break;
        case ']': birdWingSpread = fminf(1.8f,  birdWingSpread+0.08f); break;
        case ';': birdWingLift   = fmaxf(0.2f,  birdWingLift  -0.08f); break;
        case '\'':birdWingLift   = fminf(1.8f,  birdWingLift  +0.08f); break;
        case 'p': paused = !paused; showTitleCard = false; break;
        case 'g': birdAutoFlight = !birdAutoFlight;
                  if (birdAutoFlight && birdSpeed < 0.03f) birdSpeed = 0.05f; break;
        case 'c':
            controlMode = (controlMode == BirdControl ? CameraControl : BirdControl);
            if (controlMode == CameraControl) cameraMode = CAM_FREE;
            printf("Control mode: %s\n", controlMode == BirdControl ? "BIRD" : "CAMERA");
            break;
        case 27: std::exit(0); break;
    }

    if (controlMode == BirdControl) {
        switch (key) {
            case 'a': birdYaw   += birdTurnRate; break;
            case 'd': birdYaw   -= birdTurnRate; break;
            case 'w': birdSpeed  = fminf(birdSpeed+0.01f, 0.4f); break;
            case 's': birdSpeed  = fmaxf(birdSpeed-0.01f, 0.0f); break;
            case 'u': birdY     += 0.15f; break;
            case 'o': birdY      = fmaxf(0.5f, birdY-0.15f); break;
        }
    } else if (controlMode == CameraControl && cameraMode == CAM_FREE) {
        switch (key) {
            case 'w':
                camX+=camDirX*moveSpeed; camY+=camDirY*moveSpeed; camZ+=camDirZ*moveSpeed;
                camLookX+=camDirX*moveSpeed; camLookY+=camDirY*moveSpeed; camLookZ+=camDirZ*moveSpeed; break;
            case 's':
                camX-=camDirX*moveSpeed; camY-=camDirY*moveSpeed; camZ-=camDirZ*moveSpeed;
                camLookX-=camDirX*moveSpeed; camLookY-=camDirY*moveSpeed; camLookZ-=camDirZ*moveSpeed; break;
            case 'a':
                camX-=camRightX*moveSpeed; camZ-=camRightZ*moveSpeed;
                camLookX-=camRightX*moveSpeed; camLookZ-=camRightZ*moveSpeed; break;
            case 'd':
                camX+=camRightX*moveSpeed; camZ+=camRightZ*moveSpeed;
                camLookX+=camRightX*moveSpeed; camLookZ+=camRightZ*moveSpeed; break;
            case 'u': camY+=0.3f; camLookY+=0.3f; break;
            case 'o': camY-=0.3f; camLookY-=0.3f; break;
        }
    }
    glutPostRedisplay();
}

void specialKeys(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_LEFT:  if (controlMode==BirdControl) birdYaw  += birdTurnRate; break;
        case GLUT_KEY_RIGHT: if (controlMode==BirdControl) birdYaw  -= birdTurnRate; break;
        case GLUT_KEY_UP:    if (controlMode==BirdControl) birdPitch = fminf(55.0f, birdPitch+2.0f); break;
        case GLUT_KEY_DOWN:  if (controlMode==BirdControl) birdPitch = fmaxf(-55.0f,birdPitch-2.0f); break;
    }
    glutPostRedisplay();
}

// ============================================================
// MOUSE
// ============================================================

void mouseButton(int button, int state, int x, int y) {
    if (button == 3 && state == GLUT_DOWN) {
        birdZoom = fmaxf(3.5f, birdZoom-0.4f); cameraMode = CAM_SHOWCASE;
        glutPostRedisplay(); return;
    }
    if (button == 4 && state == GLUT_DOWN) {
        birdZoom = fminf(20.0f, birdZoom+0.4f); cameraMode = CAM_SHOWCASE;
        glutPostRedisplay(); return;
    }
    if (button == GLUT_LEFT_BUTTON) {
        mouseDown = (state == GLUT_DOWN);
        lastMouseX = x; lastMouseY = y;
        if (state == GLUT_DOWN) {
            int mod = glutGetModifiers();
            if (controlMode == BirdControl) {
                if      (mod & GLUT_ACTIVE_CTRL)  mouseDragMode = DragBirdRoll;
                else if (mod & GLUT_ACTIVE_SHIFT) mouseDragMode = DragZoomCamera;
                else                              mouseDragMode = DragBirdRotate;
                if (cameraMode == CAM_FREE) cameraMode = CAM_SHOWCASE;
            } else {
                mouseDragMode = DragFreeCamera;
            }
        } else {
            mouseDragMode = DragNone;
        }
    }
}

void mouseMotion(int x, int y) {
    if (!mouseDown) return;
    float dx = float(x - lastMouseX);
    float dy = float(y - lastMouseY);

    if (mouseDragMode == DragBirdRotate) {
        birdYaw   -= dx * 0.35f;
        birdPitch  = fmaxf(-55.0f, fminf(55.0f, birdPitch - dy*0.25f));
    } else if (mouseDragMode == DragBirdRoll) {
        birdRoll  += dx * 0.35f;
    } else if (mouseDragMode == DragZoomCamera) {
        birdZoom   = fmaxf(3.5f, fminf(20.0f, birdZoom + dy*0.03f));
        cameraMode = CAM_SHOWCASE;
    } else {
        float sensitivity = 0.3f;
        float yaw   = dx * sensitivity * PI / 180.0f;
        float pitch = dy * sensitivity * PI / 180.0f;
        float toX = camX-camLookX, toY = camY-camLookY, toZ = camZ-camLookZ;
        float dist = sqrtf(toX*toX + toY*toY + toZ*toZ);
        float newToX = toX*cosf(yaw) + toZ*sinf(yaw);
        float newToZ = -toX*sinf(yaw) + toZ*cosf(yaw);
        toX = newToX; toZ = newToZ;
        float el = asinf(fmaxf(-1.0f, fminf(1.0f, toY/dist)));
        el = fmaxf(-1.4f, fminf(1.4f, el - pitch));
        float horiz  = cosf(el) * dist;
        toY = sinf(el) * dist;
        float planar = sqrtf(toX*toX + toZ*toZ);
        if (planar < 0.0001f) planar = 0.0001f;
        toX = toX/planar * horiz;
        toZ = toZ/planar * horiz;
        camX = camLookX + toX;
        camY = camLookY + toY;
        camZ = camLookZ + toZ;
    }
    lastMouseX = x; lastMouseY = y;
    glutPostRedisplay();
}

// ============================================================
// MENU
// ============================================================

void menuAction(int val) {
    switch (val) {
        case 0: drawLines  = !drawLines; break;
        case 1: followCam  = !followCam; cameraMode = followCam ? CAM_FOLLOW : CAM_SHOWCASE; break;
        case 2: resetBirdPose(); break;
        case 3: resetShowcaseCamera(); cameraMode = CAM_SHOWCASE; break;
        case 4: worldTheme = WorldTheme((worldTheme + 1) % 4); break;
        case 5: weatherMode = WeatherMode((weatherMode + 1) % 4); break;
        case 6: autoShowcase = !autoShowcase; cameraMode = CAM_SHOWCASE; break;
        case 7: std::exit(0); break;
    }
    glutPostRedisplay();
}

void initMenu() {
    glutCreateMenu(menuAction);
    glutAddMenuEntry("Toggle wireframe  [1]", 0);
    glutAddMenuEntry("Toggle follow-cam [f]", 1);
    glutAddMenuEntry("Reset bird        [r]", 2);
    glutAddMenuEntry("Reset camera      [0]", 3);
    glutAddMenuEntry("Cycle theme       [t]", 4);
    glutAddMenuEntry("Cycle weather     [y]", 5);
    glutAddMenuEntry("Toggle demo cam   [m]", 6);
    glutAddMenuEntry("Quit              [Esc]",7);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

// ============================================================
// INIT + MAIN
// ============================================================

void init() {
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    resetBirdPose();
    resetShowcaseCamera();
    initMenu();
}

int main(int argc, char** argv) {
    if (argc > 0 && argv[0] && argv[0][0] != '\0') {
        std::filesystem::path exePath(argv[0]);
        if (exePath.has_parent_path()) {
            executableDir = exePath.parent_path().string();
        }
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1000, 1000);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Bird Scene — CG Experiments");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);

    glutTimerFunc(16, [](int){ update(); }, 0);
    glutMainLoop();
    return 0;
}

#pragma clang diagnostic pop
