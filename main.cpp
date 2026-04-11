// ============================================================
// Bird Flying Over a Forest Scene
// Covers: Experiments 1–5
// Mac: g++ main.cpp OBJImporter.cpp -framework OpenGL -framework GLUT -o bird_scene && ./bird_scene
// ============================================================
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#include <GLUT/glut.h>
#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <cstring>

static constexpr float PI = 3.14159265f;

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
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(-1, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Top deep blue to mid sky blue
    glBegin(GL_QUADS);
    glColor3f(0.18f, 0.42f, 0.78f); glVertex2f(-1,  1.0f);
    glColor3f(0.18f, 0.42f, 0.78f); glVertex2f( 1,  1.0f);
    glColor3f(0.52f, 0.78f, 0.95f); glVertex2f( 1,  0.1f);
    glColor3f(0.52f, 0.78f, 0.95f); glVertex2f(-1,  0.1f);
    glEnd();

    // Mid sky to warm horizon haze
    glBegin(GL_QUADS);
    glColor3f(0.52f, 0.78f, 0.95f); glVertex2f(-1,  0.1f);
    glColor3f(0.52f, 0.78f, 0.95f); glVertex2f( 1,  0.1f);
    glColor3f(0.88f, 0.82f, 0.72f); glVertex2f( 1, -0.2f);
    glColor3f(0.88f, 0.82f, 0.72f); glVertex2f(-1, -0.2f);
    glEnd();

    // Horizon to ground
    glBegin(GL_QUADS);
    glColor3f(0.88f, 0.82f, 0.72f); glVertex2f(-1, -0.2f);
    glColor3f(0.88f, 0.82f, 0.72f); glVertex2f( 1, -0.2f);
    glColor3f(0.72f, 0.85f, 0.60f); glVertex2f( 1, -1.0f);
    glColor3f(0.72f, 0.85f, 0.60f); glVertex2f(-1, -1.0f);
    glEnd();

    glColor4f(1.0f, 0.96f, 0.92f, 0.28f);
    glPushMatrix();
    glTranslatef(0.0f, -0.05f, 0.0f);
    glScalef(1.15f, 0.22f, 1.0f);
    drawFilledCircle(1.0f, 64);
    glPopMatrix();

    glColor4f(1.0f, 1.0f, 1.0f, 0.14f);
    glPushMatrix();
    glTranslatef(-0.55f, 0.28f, 0.0f);
    glScalef(0.38f, 0.14f, 1.0f);
    drawFilledCircle(1.0f, 48);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.58f, 0.34f, 0.0f);
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
    glPushMatrix();
    glColor3f(1.0f, 0.94f, 0.2f);
    glTranslatef(-25.0f, 24.0f, -35.0f);
    glutSolidSphere(2.0, 24, 24);
    // soft glow ring
    glColor4f(1.0f, 0.92f, 0.3f, 0.18f);
    glutSolidSphere(2.8, 24, 24);
    glPopMatrix();
}

// ============================================================
// CLOUDS
// ============================================================

void drawCloud(float x, float y, float z, float scale) {
    glColor3f(0.98f, 0.98f, 1.0f);
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
    glPushMatrix();
    glTranslatef(14.0f, 0.01f, 14.0f);
    glRotatef(-90, 1, 0, 0);
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(0.22f, 0.55f, 0.78f); glVertex3f(0, 0, 0);
    for (int i = 0; i <= 28; i++) {
        float ang = i * PI * 2.0f / 28.0f;
        float blend = 0.45f + 0.55f * (0.5f + 0.5f * sinf(ang + 0.6f));
        glColor3f(0.08f + 0.16f * blend, 0.40f + 0.18f * blend, 0.62f + 0.18f * blend);
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
    glBegin(GL_TRIANGLES);
    glColor3f(0.54f, 0.60f, 0.67f); glVertex3f(-45, 0, -62);
    glColor3f(0.64f, 0.69f, 0.76f); glVertex3f(-28, 8, -70);
    glColor3f(0.54f, 0.60f, 0.67f); glVertex3f(-11, 0, -62);

    glColor3f(0.52f, 0.58f, 0.66f); glVertex3f(-18, 0, -62);
    glColor3f(0.66f, 0.71f, 0.78f); glVertex3f(2, 10, -72);
    glColor3f(0.52f, 0.58f, 0.66f); glVertex3f(22, 0, -62);

    glColor3f(0.50f, 0.57f, 0.63f); glVertex3f(10, 0, -62);
    glColor3f(0.63f, 0.69f, 0.74f); glVertex3f(30, 9, -69);
    glColor3f(0.50f, 0.57f, 0.63f); glVertex3f(50, 0, -62);
    glEnd();

    glColor3f(0.36f, 0.43f, 0.5f);
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
    // Base green field
    glColor3f(0.28f, 0.58f, 0.22f);
    glBegin(GL_QUADS);
    glVertex3f(-40,0,-40); glVertex3f(40,0,-40);
    glVertex3f(40,0, 40);  glVertex3f(-40,0, 40);
    glEnd();

    // Darker grass patches
    glColor3f(0.22f, 0.50f, 0.18f);
    glBegin(GL_QUADS);
    glVertex3f(-20,0.01f, 5); glVertex3f( -5,0.01f, 5);
    glVertex3f( -5,0.01f,20); glVertex3f(-20,0.01f,20);
    glEnd();
    glBegin(GL_QUADS);
    glVertex3f(8,0.01f,-8);  glVertex3f(18,0.01f,-8);
    glVertex3f(18,0.01f, 5); glVertex3f( 8,0.01f, 5);
    glEnd();

    // Bright clearing where kids play
    glColor3f(0.55f, 0.78f, 0.35f);
    glBegin(GL_QUADS);
    glVertex3f(-6,0.01f, 2); glVertex3f(6,0.01f, 2);
    glVertex3f( 6,0.01f,10); glVertex3f(-6,0.01f,10);
    glEnd();

    // Dirt path
    glColor3f(0.68f, 0.52f, 0.35f);
    glBegin(GL_QUADS);
    glVertex3f(-1,0.01f,-5); glVertex3f(1,0.01f,-5);
    glVertex3f( 1,0.01f,10); glVertex3f(-1,0.01f,10);
    glEnd();

    glColor3f(0.64f, 0.80f, 0.40f);
    glBegin(GL_QUADS);
    glVertex3f(-24,0.01f,-16); glVertex3f(-6,0.01f,-16);
    glVertex3f(-6,0.01f, -2); glVertex3f(-24,0.01f,-2);
    glEnd();

    glColor3f(0.20f, 0.46f, 0.20f);
    glBegin(GL_QUADS);
    glVertex3f(10,0.01f,8); glVertex3f(24,0.01f,8);
    glVertex3f(24,0.01f,22); glVertex3f(10,0.01f,22);
    glEnd();
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
        // Colorful parrot-like bird with more facial detail
        const float wingColors[5][3] = {
            {0.07f,0.82f,0.85f},{0.96f,0.55f,0.08f},
            {0.12f,0.75f,0.30f},{0.95f,0.17f,0.55f},{0.35f,0.65f,0.95f}
        };

        // Body
        glColor3f(0.90f,0.06f,0.15f);
        glPushMatrix(); glScalef(0.95f,0.72f,1.18f); glutSolidSphere(0.62,26,22); glPopMatrix();

        glColor3f(1.0f, 0.82f, 0.44f);
        glPushMatrix();
        glTranslatef(0.0f, -0.06f, -0.02f);
        glScalef(0.70f, 0.46f, 0.86f);
        glutSolidSphere(0.48f, 20, 18);
        glPopMatrix();

        // Head
        glColor3f(0.88f,0.05f,0.13f);
        glPushMatrix(); glTranslatef(0,0.42f,-0.76f); glScalef(0.74f,0.80f,0.74f); glutSolidSphere(0.42,22,18); glPopMatrix();

        glColor3f(0.30f, 0.82f, 0.90f);
        glPushMatrix();
        glTranslatef(-0.16f, 0.33f, -0.88f);
        glScalef(0.18f, 0.16f, 0.12f);
        glutSolidSphere(1.0f, 14, 12);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(0.16f, 0.33f, -0.88f);
        glScalef(0.18f, 0.16f, 0.12f);
        glutSolidSphere(1.0f, 14, 12);
        glPopMatrix();

        glColor3f(0.98f,0.72f,0.10f);
        glPushMatrix(); glTranslatef(0,0.43f,-1.09f); glRotatef(-90,1,0,0); glutSolidCone(0.12,0.34,12,2); glPopMatrix();
        glColor3f(0.90f,0.54f,0.10f);
        glPushMatrix(); glTranslatef(0,0.28f,-1.00f); glRotatef(90,1,0,0); glutSolidCone(0.075,0.22,10,2); glPopMatrix();

        glColor3f(0.98f, 0.98f, 0.98f);
        glPushMatrix(); glTranslatef(-0.19f,0.46f,-0.82f); glutSolidSphere(0.10,14,14); glPopMatrix();
        glPushMatrix(); glTranslatef( 0.19f,0.46f,-0.82f); glutSolidSphere(0.10,14,14); glPopMatrix();
        glColor3f(0.88f, 0.60f, 0.05f);
        glPushMatrix(); glTranslatef(-0.20f,0.45f,-0.90f); glutSolidSphere(0.055,12,12); glPopMatrix();
        glPushMatrix(); glTranslatef( 0.20f,0.45f,-0.90f); glutSolidSphere(0.055,12,12); glPopMatrix();
        glColor3f(0.04f,0.04f,0.10f);
        glPushMatrix(); glTranslatef(-0.21f,0.45f,-0.93f); glutSolidSphere(0.028,10,10); glPopMatrix();
        glPushMatrix(); glTranslatef( 0.21f,0.45f,-0.93f); glutSolidSphere(0.028,10,10); glPopMatrix();
        glColor3f(1.0f, 1.0f, 1.0f);
        glPushMatrix(); glTranslatef(-0.17f,0.49f,-0.90f); glutSolidSphere(0.015,8,8); glPopMatrix();
        glPushMatrix(); glTranslatef( 0.17f,0.49f,-0.90f); glutSolidSphere(0.015,8,8); glPopMatrix();

        // Crest feathers
        glColor3f(0.80f,0.18f,0.55f);
        for (int fe = 0; fe < 3; ++fe) {
            glPushMatrix();
            glTranslatef(-0.07f+fe*0.07f, 0.86f+fe*0.07f, -0.88f);
            glRotatef(-18.0f+fe*10.0f, 0,0,1);
            glRotatef(-8.0f, 1,0,0);
            glScalef(0.06f, 0.32f+fe*0.04f, 0.06f);
            drawBox(0.5f,0.5f,0.5f);
            glPopMatrix();
        }

        // Wings
        for (int side = -1; side <= 1; side += 2) {
            for (int i = 0; i < 5; ++i) {
                glColor3f(wingColors[i][0], wingColors[i][1], wingColors[i][2]);
                glPushMatrix();
                float offset = side * (0.58f + 0.19f*i*birdWingSpread);
                float zoff   = -0.08f + i*0.18f;
                float yoff   =  0.03f + i*0.02f;
                glTranslatef(offset, yoff, zoff);
                glRotatef(side*(18.0f+birdWingSpread*8.0f+flapAngle*0.55f+i*4.0f), 0,0,1);
                glRotatef(-birdWingLift*(8.0f+i*2.0f), 1,0,0);
                glScalef(0.36f,0.08f,0.27f);
                drawBox(0.5f,0.5f,0.5f);
                glPopMatrix();
            }
        }

        for (int side = -1; side <= 1; side += 2) {
            glColor3f(0.06f, 0.70f, 0.38f);
            glPushMatrix();
            glTranslatef(side * 0.48f, 0.16f, -0.04f);
            glRotatef(side * 18.0f, 0, 0, 1);
            glRotatef(side * 14.0f, 0, 1, 0);
            glScalef(0.28f, 0.14f, 0.48f);
            glutSolidSphere(0.58f, 16, 14);
            glPopMatrix();
        }

        // Tail feathers
        const float tailColors[4][3] = {
            {0.76f,0.12f,0.82f},{0.07f,0.82f,0.85f},
            {0.76f,0.12f,0.82f},{0.07f,0.82f,0.85f}
        };
        for (int i = 0; i < 4; ++i) {
            glColor3f(tailColors[i][0],tailColors[i][1],tailColors[i][2]);
            glPushMatrix();
            glTranslatef(-0.42f+i*0.28f, -0.46f, 0.62f+0.04f*i);
            glRotatef(-26.0f+i*6.0f, 1,0,0);
            glScalef(0.28f,0.07f,0.48f);
            drawBox(0.5f,0.5f,0.5f);
            glPopMatrix();
        }

        // Central tail
        glColor3f(0.20f,0.20f,0.24f);
        glPushMatrix(); glTranslatef(0,-0.22f,0.74f); glScalef(0.24f,0.08f,0.58f); drawBox(0.5f,0.5f,0.5f); glPopMatrix();

        glColor3f(0.42f, 0.42f, 0.46f);
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(side * 0.18f, -0.60f, 0.18f);
            glScalef(0.05f, 0.24f, 0.05f);
            drawBox(0.5f,0.5f,0.5f);
            glPopMatrix();

            glPushMatrix();
            glTranslatef(side * 0.18f, -0.72f, 0.26f);
            glRotatef(-18.0f, 1, 0, 0);
            glScalef(0.15f, 0.03f, 0.18f);
            drawBox(0.5f,0.5f,0.5f);
            glPopMatrix();
        }

    } else {
#ifdef USE_OBJ_BIRD
        // OBJ model bird (Experiment 5)
        static OBJImporter objBird("./models/bird.obj");
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

    drawCloud(-18.0f, 22.5f, -30.0f, 1.3f);
    drawCloud( 12.0f, 20.0f, -25.0f, 1.0f);
    drawCloud( 20.0f, 24.0f, -18.0f, 0.9f);
    drawCloud( -5.0f, 18.0f, -20.0f, 0.75f);
    drawCloud( -30.0f,19.0f, -15.0f, 1.1f);

    drawMountains();
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
    drawBuilding(-18,-25, 3.0f,3.0f, 8.0f, 0.60f,0.60f,0.65f);
    drawBuilding(-14,-24, 2.5f,2.5f,12.0f, 0.55f,0.55f,0.60f);
    drawBuilding(-10,-26, 4.0f,3.0f, 6.0f, 0.65f,0.60f,0.60f);
    drawBuilding( -6,-25, 2.0f,2.0f,15.0f, 0.50f,0.50f,0.58f);
    drawBuilding(  0,-27, 5.0f,3.5f, 9.0f, 0.60f,0.58f,0.62f);
    drawBuilding(  6,-25, 2.5f,2.5f,11.0f, 0.58f,0.62f,0.60f);
    drawBuilding( 12,-24, 3.0f,3.0f, 7.0f, 0.62f,0.60f,0.58f);
    drawBuilding( 17,-26, 2.0f,2.5f,13.0f, 0.55f,0.58f,0.65f);

    drawBirdShadow();

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

void drawHud() {
    if (!showHud) return;
    glMatrixMode(GL_PROJECTION);
    glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, winWidth, 0, winHeight);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix(); glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glColor3f(0.07f, 0.12f, 0.16f);

    const int startY  = winHeight - 22;
    const int lineStep = 18;
    char line[256];
    std::snprintf(line, sizeof(line),
        "Bird mode | drag: rotate | ctrl+drag: roll | shift+drag/wheel: zoom | auto-flight [%s] | HUD [h]",
        birdAutoFlight ? "on" : "off");
    drawBitmapText(14.0f, startY, line);
    drawBitmapText(14.0f, startY - lineStep,
        "Arrows/A,D: yaw-pitch | Q,E: roll | Z,X: zoom | W,S: speed | U,O: altitude | [,]: wings | ;,': flap lift");
    drawBitmapText(14.0f, startY - lineStep*2,
        "B: cycle bird look | F: follow cam | 1-6: cameras | 0: reset cam | R: reset bird | Space: flap | Esc: quit");

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
            float yawRad   = birdCamYaw   * PI / 180.0f;
            float pitchRad = birdCamPitch * PI / 180.0f;
            float focusX = birdX, focusY = birdY + 0.15f, focusZ = birdZ;
            float dXZ = birdZoom * cosf(pitchRad);
            float sX  = focusX + sinf(yawRad) * dXZ;
            float sY  = focusY + sinf(pitchRad) * birdZoom;
            float sZ  = focusZ + cosf(yawRad) * dXZ;
            gluLookAt(sX, sY, sZ, focusX, focusY, focusZ, 0,1,0);
            break;
        }
    }

    glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
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
        case 'r': resetBirdPose(); cameraMode = CAM_SHOWCASE; resetShowcaseCamera(); break;
        case 'z': birdZoom = fmaxf(3.5f, birdZoom-0.35f); cameraMode = CAM_SHOWCASE; break;
        case 'x': birdZoom = fminf(20.0f, birdZoom+0.35f); cameraMode = CAM_SHOWCASE; break;
        case '[': birdWingSpread = fmaxf(0.55f, birdWingSpread-0.08f); break;
        case ']': birdWingSpread = fminf(1.8f,  birdWingSpread+0.08f); break;
        case ';': birdWingLift   = fmaxf(0.2f,  birdWingLift  -0.08f); break;
        case '\'':birdWingLift   = fminf(1.8f,  birdWingLift  +0.08f); break;
        case 'p': birdAutoFlight = !birdAutoFlight;
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
        case 4: std::exit(0); break;
    }
    glutPostRedisplay();
}

void initMenu() {
    glutCreateMenu(menuAction);
    glutAddMenuEntry("Toggle wireframe  [1]", 0);
    glutAddMenuEntry("Toggle follow-cam [f]", 1);
    glutAddMenuEntry("Reset bird        [r]", 2);
    glutAddMenuEntry("Reset camera      [0]", 3);
    glutAddMenuEntry("Quit              [Esc]",4);
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
    resetBirdPose();
    resetShowcaseCamera();
    initMenu();
}

int main(int argc, char** argv) {
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
