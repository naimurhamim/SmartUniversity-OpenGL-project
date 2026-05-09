#include <windows.h>
#include <GL/glut.h>
#include <GL/glu.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define PI 3.1416f

///===================///
///*** Global Vars ***///
///===================///
GLint i, j, k;
GLfloat sun_spin       = 0;
GLfloat cloud_ax = 0, cloud_bx = 0, cloud_cx = 0, cloud_dx = 0;
GLfloat spin           = 0.0f;   // rotor / windmill
GLfloat heli_x         = -200.0f;
GLfloat flag_wave      = 0.0f;
bool    startProject   = false;
float animSpeed  = 1.0f;   // 0.5=slow, 1=normal, 2=fast
bool  animPaused = false;

// Scene management (0-5)
int currentScene = 0;

// Scene 1 – entrance / sliding door
GLfloat doorOpen       = 0.0f;   // 0 = closed, 1 = open
bool    doorOpening    = false;
GLfloat personX        = 200.0f; // person walking toward door
bool    personWalking  = true;

// Scene 2 – attendance
int     attendanceStep = 0;       // 0=none,1=Naimur shown,2=Sourav shown
GLfloat scanAnim       = 0.0f;
bool    scanDone[2]    = {false, false};
GLfloat studentX[2]    = {100.0f, 200.0f}; // approach positions
bool    studentEntered[2] = {false, false};

// Scene 3 – classroom
GLfloat markerX        = 0.0f;   // teacher marker on board
bool    markerDir      = true;

// Scene 4 – rooftop helipad
struct HeliStudent { GLfloat x; GLfloat y; bool pickedUp; GLfloat flyY; };
HeliStudent roofHeli[3];
int     roofHeliPhase  = 0;      // which heli is active
GLfloat roofHeliX      = -200.0f;
GLfloat roofHeliY      = 460.0f;
bool    pickingUp      = false;
int     studentsPicked = 0;
GLfloat roofSpin       = 0.0f;
int   roofPhase    = 0;    // 0=coming, 1=landing, 2=boarding, 3=takeoff
float boardingTimer = 0.0f;
float landedY      = 260.0f;  // helipad এর Y position

// Scene 5 – funny text scroll
GLfloat textScrollY    = -50.0f;
int     funnyLine      = 0;
GLfloat blinkTimer     = 0.0f;
float waitX_anim[3] = {300.0f, 680.0f, 150.0f};
///=======================///
///*** Text Rendering  ***///
///=======================///
void displayRasterText(float x, float y, float z, const char* str, void* font = GLUT_BITMAP_TIMES_ROMAN_24)
{
    glRasterPos3f(x, y, z);
    for (const char* c = str; *c != '\0'; c++)
        glutBitmapCharacter(font, *c);
}

void displayStrokeText(float x, float y, float scale, const char* str)
{
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(scale, scale, 1.0f);
    for (const char* c = str; *c != '\0'; c++)
        glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
    glPopMatrix();
}

///====================///
///*** Init / Setup  ***///
///====================///
void init(void)
{
    glClearColor(0.53f, 0.81f, 0.98f, 0.0f);
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0.0, 1000.0, 0.0, 500.0);
}

///====================///
///***  Intro Screen ***///
///====================///
void introScreen()
{
    glClearColor(0.05f, 0.05f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(1.0f, 0.85f, 0.0f);
    displayRasterText(285, 440, 0, "UNIVERSITY OF FRONTIER TECHNOLOGY, BANGLADESH", GLUT_BITMAP_TIMES_ROMAN_24);

    glColor3f(0.7f, 1.0f, 1.0f);
    displayRasterText(300, 415, 0, "Department of Internet of Things and Robotics Engineering", GLUT_BITMAP_HELVETICA_18);

    glColor3f(0.3f, 0.6f, 1.0f);
    glBegin(GL_LINES);
    glVertex2f(100, 405); glVertex2f(900, 405);
    glEnd();

    glColor3f(0.5f, 0.9f, 1.0f);
    displayRasterText(270, 385, 0, "Course: IRE 404 - Computer Graphics and Animation Sessional", GLUT_BITMAP_HELVETICA_18);

    glColor3f(0.2f, 0.8f, 1.0f);
    displayRasterText(380, 355, 0, "Project: SMART UNIVERSITY", GLUT_BITMAP_TIMES_ROMAN_24);

    glColor3f(1.0f, 0.4f, 0.4f);
    displayRasterText(120, 300, 0, "SUBMITTED BY:", GLUT_BITMAP_TIMES_ROMAN_24);
    glColor3f(1.0f, 1.0f, 1.0f);
    displayRasterText(120, 272, 0, "MD NAIMUR RASHID (2101042)", GLUT_BITMAP_HELVETICA_18);
    displayRasterText(120, 250, 0, "SOURAV CHAKRABORTY (2101031)", GLUT_BITMAP_HELVETICA_18);

    glColor3f(1.0f, 0.4f, 0.4f);
    displayRasterText(620, 300, 0, "SUBMITTED TO:", GLUT_BITMAP_TIMES_ROMAN_24);
    glColor3f(1.0f, 1.0f, 1.0f);
    displayRasterText(620, 272, 0, "SAURAV CHANDRA DAS", GLUT_BITMAP_HELVETICA_18);
    displayRasterText(620, 250, 0, "Lecturer, IRE, UFTB", GLUT_BITMAP_HELVETICA_18);

    glColor3f(1.0f, 0.6f, 0.2f);
    displayRasterText(390, 180, 0, "Academic Year 2025-2026", GLUT_BITMAP_TIMES_ROMAN_24);

    glColor3f(0.4f, 1.0f, 0.4f);
    displayRasterText(335, 100, 0, ">> Press ENTER to Start  |  Right Click = Next Scene <<", GLUT_BITMAP_TIMES_ROMAN_24);

    glutSwapBuffers();
}

///===========================///
///*** Primitive: Circle   ***///
///===========================///
void circle(GLdouble rad)
{
    GLint pts = 60;
    GLdouble dT = (2.0 * PI) / pts;
    GLdouble theta = 0.0;
    glBegin(GL_POLYGON);
    for (i = 0; i <= pts; i++, theta += dT)
        glVertex2f((float)(rad * cos(theta)), (float)(rad * sin(theta)));
    glEnd();
}

void rect(float x1, float y1, float x2, float y2)
{
    glBegin(GL_POLYGON);
    glVertex2f(x1,y1); glVertex2f(x2,y1);
    glVertex2f(x2,y2); glVertex2f(x1,y2);
    glEnd();
}

///=====================///
///***    Sky / Sun   ***///
///=====================///
void drawSky()
{
    glBegin(GL_POLYGON);
    glColor3f(0.40f, 0.70f, 1.0f);  glVertex2f(0, 500);
    glColor3f(0.40f, 0.70f, 1.0f);  glVertex2f(1000, 500);
    glColor3f(0.65f, 0.88f, 1.0f);  glVertex2f(1000, 230);
    glColor3f(0.65f, 0.88f, 1.0f);  glVertex2f(0, 230);
    glEnd();
}

void Sun_Model()
{
    glPushMatrix();
    glRotatef(-sun_spin, 0, 0, 1);
    glTranslatef(870, 420, 0);
    glColor4f(1.0f, 1.0f, 0.5f, 0.3f);
    circle(36);
    glColor3f(1.0f, 0.95f, 0.2f);
    circle(26);
    glPopMatrix();
}

///====================///
///***    Clouds     ***///
///====================///
void cloudPuff(float x, float y, float r)
{
    glPushMatrix();
    glTranslatef(x, y, 0);
    circle(r);
    glPopMatrix();
}

void cloud_model(float ox, float oy, float scale)
{
    float s = scale;
    cloudPuff(ox,        oy,        14*s);
    cloudPuff(ox+18*s,   oy+8*s,    18*s);
    cloudPuff(ox+38*s,   oy+4*s,    16*s);
    cloudPuff(ox+54*s,   oy,        12*s);
    cloudPuff(ox+20*s,   oy-4*s,    13*s);
    cloudPuff(ox+36*s,   oy-4*s,    13*s);
}

void drawClouds()
{
    glColor3f(0.95f, 0.97f, 1.0f);
    cloud_model(cloud_ax,        380, 1.0f);
    cloud_model(cloud_bx + 300,  420, 0.8f);
    cloud_model(cloud_cx + 600,  395, 1.1f);
    cloud_model(cloud_dx + 100,  450, 0.7f);
    cloud_model(cloud_ax + 700,  355, 0.9f);
}

///====================///
///***  Ground       ***///
///====================///
void drawGround()
{
    glBegin(GL_POLYGON);
    glColor3f(0.22f, 0.62f, 0.18f);
    glVertex2f(0, 0);  glVertex2f(1000, 0);
    glVertex2f(1000, 235); glVertex2f(0, 235);
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(0.40f, 0.40f, 0.42f);
    glVertex2f(300, 110); glVertex2f(700, 110);
    glVertex2f(700, 135); glVertex2f(300, 135);
    glEnd();

    glColor3f(1.0f, 1.0f, 0.5f);
    for (int rx = 350; rx < 680; rx += 50)
    {
        glBegin(GL_POLYGON);
        glVertex2f(rx, 119); glVertex2f(rx+25, 119);
        glVertex2f(rx+25, 125); glVertex2f(rx, 125);
        glEnd();
    }

    glBegin(GL_POLYGON);
    glColor3f(0.28f, 0.72f, 0.22f);
    glVertex2f(310, 135); glVertex2f(690, 135);
    glVertex2f(690, 165); glVertex2f(310, 165);
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(0.20f, 0.55f, 0.80f);
    glVertex2f(20, 140); glVertex2f(180, 140);
    glVertex2f(190, 165); glVertex2f(10, 165);
    glEnd();
    glBegin(GL_POLYGON);
    glColor3f(0.25f, 0.62f, 0.88f);
    glVertex2f(30, 148); glVertex2f(170, 148);
    glVertex2f(170, 158); glVertex2f(30, 158);
    glEnd();
}

///====================///
///***    Trees       ***///
///====================///
void treeAt(float x, float y, float h, float r)
{
    glColor3f(0.42f, 0.27f, 0.10f);
    rect(x-3, y, x+3, y+h);
    glColor3f(0.18f, 0.58f, 0.15f);
    glPushMatrix(); glTranslatef(x, y+h, 0); circle(r); glPopMatrix();
    glColor3f(0.22f, 0.70f, 0.20f);
    glPushMatrix(); glTranslatef(x, y+h+r*0.5f, 0); circle(r*0.75f); glPopMatrix();
    glColor3f(0.28f, 0.80f, 0.25f);
    glPushMatrix(); glTranslatef(x, y+h+r*0.9f, 0); circle(r*0.50f); glPopMatrix();
}

void drawTrees()
{
    treeAt(40,  165, 20, 14); treeAt(70,  165, 18, 12);
    treeAt(100, 165, 22, 15); treeAt(130, 165, 17, 11);
    treeAt(860, 165, 20, 14); treeAt(890, 165, 22, 15);
    treeAt(920, 165, 18, 12); treeAt(950, 165, 21, 14);
    treeAt(305, 135, 14,  9); treeAt(695, 135, 14,  9);
    treeAt(320, 165, 16, 10); treeAt(680, 165, 16, 10);
    treeAt(200, 165, 16, 11); treeAt(780, 165, 18, 13);
    treeAt(240, 160, 15, 10); treeAt(750, 162, 17, 11);
}

///==============================///
///*** University Buildings    ***///
///==============================///
void windowRow(float startX, float startY, float wW, float wH, float gap, int count, float r, float g, float b)
{
    glColor3f(r, g, b);
    for (int w = 0; w < count; w++)
    {
        float wx = startX + w * (wW + gap);
        rect(wx, startY, wx+wW, startY+wH);
    }
}

void drawTower(float bx, float by, float w, float h, float r, float g, float b)
{
    glColor3f(r*0.7f, g*0.7f, b*0.7f);
    rect(bx+w*0.85f, by, bx+w, by+h);
    glColor3f(r, g, b);
    rect(bx, by, bx+w*0.85f, by+h);
    glColor3f(r*0.85f, g*0.85f, b*0.9f);
    rect(bx, by+h, bx+w, by+h+8);
    float fl_h = 18.0f;
    float numFloors = (int)(h / fl_h);
    for (int fl = 0; fl < numFloors; fl++)
    {
        float wy = by + 5 + fl*fl_h;
        windowRow(bx+4, wy, 5, 10, 4, 5, 0.55f, 0.78f, 0.90f);
    }
}

void drawBlock(float bx, float by, float w, float h, float r, float g, float b)
{
    glColor3f(r*0.8f, g*0.8f, b*0.8f);
    rect(bx+w*0.9f, by, bx+w, by+h);
    glColor3f(r, g, b);
    rect(bx, by, bx+w*0.9f, by+h);
    glColor3f(r*0.7f, g*0.7f, b*0.7f);
    rect(bx, by+h, bx+w, by+h+5);
    int floors = (int)(h / 25);
    for (int fl = 0; fl < floors; fl++)
    {
        float wy = by + 5 + fl*25;
        windowRow(bx+6, wy, 8, 12, 6, 4, 0.55f, 0.78f, 0.92f);
    }
}

void drawMainBuilding()
{
    drawBlock(370, 165, 260, 100, 0.88f, 0.88f, 0.90f);

    glColor3f(0.60f, 0.60f, 0.65f);
    rect(478, 165, 495, 200);
    rect(505, 165, 522, 200);
    glColor3f(0.55f, 0.55f, 0.60f);
    rect(478, 198, 522, 205);
    glColor3f(0.20f, 0.22f, 0.28f);
    rect(481, 165, 519, 198);

    glColor3f(0.72f, 0.74f, 0.78f);
    rect(483, 205, 517, 240);
    glColor3f(0.92f, 0.92f, 0.95f);
    glPushMatrix(); glTranslatef(500, 228, 0); circle(8); glPopMatrix();
    glColor3f(0.15f, 0.15f, 0.20f);
    glPushMatrix(); glTranslatef(500, 228, 0); circle(1); glPopMatrix();
    glColor3f(0.10f, 0.10f, 0.15f);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex2f(500, 228); glVertex2f(500, 235);
    glVertex2f(500, 228); glVertex2f(505, 228);
    glEnd();

    drawBlock(280, 165, 95, 75, 0.84f, 0.84f, 0.88f);
    drawBlock(625, 165, 95, 75, 0.84f, 0.84f, 0.88f);

    glColor3f(0.90f, 0.50f, 0.10f);
    displayRasterText(450, 252, 0, "UFTB", GLUT_BITMAP_TIMES_ROMAN_24);
}

void drawUniversity()
{
    drawTower(340, 165, 60, 155, 0.82f, 0.84f, 0.88f);
    glColor3f(0.90f, 0.50f, 0.10f);
    displayRasterText(348, 326, 0, "UFTB", GLUT_BITMAP_HELVETICA_12);

    drawTower(600, 165, 60, 155, 0.82f, 0.84f, 0.88f);
    glColor3f(0.90f, 0.50f, 0.10f);
    displayRasterText(608, 326, 0, "UFTB", GLUT_BITMAP_HELVETICA_12);

    drawMainBuilding();

    drawBlock(160, 165, 80, 60, 0.78f, 0.65f, 0.50f);
    glColor3f(0.90f, 0.50f, 0.10f);
    displayRasterText(172, 230, 0, "UFTB", GLUT_BITMAP_HELVETICA_12);

    drawBlock(760, 165, 80, 60, 0.78f, 0.65f, 0.50f);
    glColor3f(0.90f, 0.50f, 0.10f);
    displayRasterText(772, 230, 0, "UFTB", GLUT_BITMAP_HELVETICA_12);

    glColor3f(0.10f, 0.10f, 0.30f);
    rect(355, 140, 645, 155);
    glColor3f(1.0f, 0.95f, 0.30f);
    displayRasterText(363, 144, 0, "UNIVERSITY OF FRONTIER TECHNOLOGY, BANGLADESH", GLUT_BITMAP_HELVETICA_10);
}

///================================///
///***   People (small figures)  ***///
///================================///
void drawPerson(float x, float y, float scale, float r=0.20f, float g=0.40f, float b=0.80f)
{
    glColor3f(0.95f, 0.80f, 0.65f);
    glPushMatrix(); glTranslatef(x, y+10*scale, 0); circle(3*scale); glPopMatrix();
    glColor3f(r, g, b);
    rect(x-2*scale, y+2*scale, x+2*scale, y+8*scale);
    glColor3f(0.15f, 0.15f, 0.35f);
    rect(x-2*scale, y, x-0.5f*scale, y+3*scale);
    rect(x+0.5f*scale, y, x+2*scale, y+3*scale);
}

void drawPeople()
{
    drawPerson(430, 110, 1.0f);
    drawPerson(460, 112, 1.0f);
    drawPerson(510, 109, 1.0f);
    drawPerson(540, 111, 1.0f);
    drawPerson(570, 110, 1.0f);
    drawPerson(340, 113, 0.9f);
    drawPerson(655, 112, 0.9f);
    drawPerson(400, 145, 0.85f);
    drawPerson(580, 148, 0.85f);
    drawPerson(500, 150, 0.85f);
}

///================================///
///***      Helicopter           ***///
///================================///
void drawFlag(float hx, float hy)
{
    glColor3f(0.55f, 0.45f, 0.30f);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex2f(hx - 5, hy + 14);
    glVertex2f(hx + 100, hy + 14);
    glEnd();

    float fx = hx + 5;
    float fy = hy + 14;
    int segments = 12;
    float seg_w = 90.0f / segments;
    float amp = 6.0f;

    glBegin(GL_TRIANGLE_STRIP);
    for (int s = 0; s <= segments; s++)
    {
        float prog = (float)s / segments;
        float wave = (float)sin((prog * 3.14f * 2.0f) + flag_wave) * amp * prog;
        glColor3f(0.0f, 0.40f, 0.10f);
        glVertex2f(fx + s*seg_w, fy + wave + 18);
        glColor3f(0.0f, 0.50f, 0.15f);
        glVertex2f(fx + s*seg_w, fy + wave);
    }
    glEnd();

    float mid_x = fx + 42;
    float mid_y = fy + 9 + (float)sin(flag_wave + 1.2f) * amp * 0.5f;
    glColor3f(0.85f, 0.05f, 0.10f);
    glPushMatrix(); glTranslatef(mid_x, mid_y, 0); circle(7); glPopMatrix();

    glColor3f(1.0f, 1.0f, 1.0f);
    displayRasterText(hx + 8, hy + 15, 0, "University of Frontier Technology, Bangladesh", GLUT_BITMAP_HELVETICA_10);
}

void drawHelicopter(float hx, float hy, float spinRate=1.0f)
{
    glColor3f(0.18f, 0.22f, 0.70f);
    glBegin(GL_POLYGON);
    glVertex2f(hx,      hy+8);
    glVertex2f(hx+50,   hy+12);
    glVertex2f(hx+55,   hy+5);
    glVertex2f(hx+30,   hy);
    glVertex2f(hx+5,    hy+2);
    glEnd();

    glColor3f(0.55f, 0.85f, 1.0f);
    glBegin(GL_POLYGON);
    glVertex2f(hx+38, hy+5);
    glVertex2f(hx+55, hy+10);
    glVertex2f(hx+55, hy+5);
    glEnd();

    glColor3f(0.15f, 0.18f, 0.58f);
    rect(hx, hy+7, hx+20, hy+10);

    glColor3f(0.80f, 0.80f, 0.80f);
    glPushMatrix();
    glTranslatef(hx+8, hy+9, 0);
    glRotatef(spin*3*spinRate, 0, 0, 1);
    glBegin(GL_LINES);
    glVertex2f(0, -6); glVertex2f(0, 6);
    glVertex2f(-6, 0); glVertex2f(6, 0);
    glEnd();
    glPopMatrix();

    glColor3f(0.70f, 0.70f, 0.72f);
    glPushMatrix();
    glTranslatef(hx+38, hy+13, 0);
    glRotatef(spin*5*spinRate, 0, 0, 1);
    glBegin(GL_LINES);
    glVertex2f(-32, 0); glVertex2f(32, 0);
    glVertex2f(0, -4); glVertex2f(0, 4);
    glEnd();
    glPopMatrix();

    glColor3f(0.40f, 0.40f, 0.45f);
    glBegin(GL_LINES);
    glVertex2f(hx+20, hy); glVertex2f(hx+20, hy-5);
    glVertex2f(hx+40, hy); glVertex2f(hx+40, hy-5);
    glVertex2f(hx+15, hy-5); glVertex2f(hx+45, hy-5);
    glEnd();
}

///=================================================///
///*** SCENE 0: Main University (existing scene) ***///
///=================================================///
void drawScene0()
{
    drawSky();
    Sun_Model();
    drawClouds();
    drawGround();
    drawTrees();
    drawUniversity();
    drawPeople();
    drawHelicopter(heli_x, 340);
    drawFlag(heli_x - 105, 340);

    // scene label
    glColor3f(0.1f, 0.1f, 0.3f);
    displayRasterText(10, 10, 0, "Scene 1: Smart University  |  Right Click = Next", GLUT_BITMAP_HELVETICA_12);
}

///=====================================================///
///*** SCENE 1: Entrance Close-Up + Sliding Door   ***///
///=====================================================///
void drawScene1()
{
    // Sky background
    glBegin(GL_POLYGON);
    glColor3f(0.45f, 0.73f, 1.0f); glVertex2f(0,   500);
    glColor3f(0.45f, 0.73f, 1.0f); glVertex2f(1000, 500);
    glColor3f(0.68f, 0.88f, 1.0f); glVertex2f(1000, 200);
    glColor3f(0.68f, 0.88f, 1.0f); glVertex2f(0,   200);
    glEnd();

    // Ground
    glColor3f(0.30f, 0.70f, 0.24f);
    rect(0, 0, 1000, 210);
    // Pathway to door
    glColor3f(0.50f, 0.50f, 0.52f);
    rect(360, 0, 640, 200);
    // Path markings
    glColor3f(0.75f, 0.75f, 0.78f);
    for (int py = 20; py < 200; py += 40)
        rect(485, py, 515, py+20);

    // Side bushes
    glColor3f(0.18f, 0.60f, 0.15f);
    glPushMatrix(); glTranslatef(340, 195, 0); circle(15); glPopMatrix();
    glPushMatrix(); glTranslatef(660, 195, 0); circle(15); glPopMatrix();
    glColor3f(0.25f, 0.72f, 0.22f);
    glPushMatrix(); glTranslatef(310, 200, 0); circle(12); glPopMatrix();
    glPushMatrix(); glTranslatef(690, 200, 0); circle(12); glPopMatrix();

    // ---- University Entrance Wall ----
    glColor3f(0.82f, 0.82f, 0.86f);
    rect(200, 200, 800, 430);

    // Wall texture lines
    glColor3f(0.75f, 0.75f, 0.79f);
    for (int wy = 210; wy < 430; wy += 20)
    {
        glBegin(GL_LINES);
        glVertex2f(200, wy); glVertex2f(800, wy);
        glEnd();
    }

    // Pillars on sides of entrance
    glColor3f(0.65f, 0.65f, 0.70f);
    rect(200, 200, 250, 440);
    rect(750, 200, 800, 440);
    // Pillar caps
    glColor3f(0.55f, 0.55f, 0.60f);
    rect(195, 430, 255, 445);
    rect(745, 430, 805, 445);

    // Gate frame
    glColor3f(0.45f, 0.45f, 0.50f);
    rect(380, 200, 400, 380);  // left frame
    rect(600, 200, 620, 380);  // right frame
    rect(380, 370, 620, 390);  // top beam

    // "UFTB" above gate
    glColor3f(0.90f, 0.50f, 0.10f);
    displayRasterText(440, 395, 0, "UFTB - SMART ENTRANCE", GLUT_BITMAP_TIMES_ROMAN_24);

    // Sliding doors (auto-open)
    float doorSlide = doorOpen * 80.0f;

    // Left door panel
    glColor3f(0.30f, 0.55f, 0.75f);
    rect(400, 200, 500 - doorSlide, 368);
    // Glass effect on left door
    glColor3f(0.55f, 0.80f, 0.95f);
    rect(408, 208, 480 - doorSlide, 360);
    // Door handle
    if (doorSlide < 70)
    {
        glColor3f(0.80f, 0.75f, 0.20f);
        rect(488 - doorSlide, 278, 495 - doorSlide, 290);
    }

    // Right door panel
    glColor3f(0.30f, 0.55f, 0.75f);
    rect(500 + doorSlide, 200, 600, 368);
    // Glass effect on right door
    glColor3f(0.55f, 0.80f, 0.95f);
    rect(520 + doorSlide, 208, 592, 360);
    // Door handle
    if (doorSlide < 70)
    {
        glColor3f(0.80f, 0.75f, 0.20f);
        rect(505 + doorSlide, 278, 512 + doorSlide, 290);
    }

    // Inside visible when door opens
    if (doorOpen > 0.3f)
    {
        float alpha = (doorOpen - 0.3f) / 0.7f;
        glColor3f(0.85f * alpha, 0.88f * alpha, 0.90f * alpha);
        rect(420, 205, 580, 367);
        // inside lobby
        glColor3f(0.60f * alpha, 0.62f * alpha, 0.65f * alpha);
        rect(450, 205, 480, 350);
        rect(520, 205, 550, 350);
        // reception desk visible inside
        glColor3f(0.55f * alpha, 0.40f * alpha, 0.25f * alpha);
        rect(460, 210, 540, 240);
        glColor3f(1.0f * alpha, 1.0f * alpha, 1.0f * alpha);
        if (alpha > 0.6f)
            displayRasterText(462, 220, 0, "RECEPTION", GLUT_BITMAP_HELVETICA_10);
    }

    // Sensor / motion detector (glows green when person near)
    float sensorGlow = (personX > 420 && personX < 550) ? 1.0f : 0.3f;
    glColor3f(0.0f, sensorGlow, 0.0f);
    glPushMatrix(); glTranslatef(395, 340, 0); circle(5); glPopMatrix();
    glPushMatrix(); glTranslatef(605, 340, 0); circle(5); glPopMatrix();
    // Sensor label
    glColor3f(0.0f, 0.8f, 0.0f);
    displayRasterText(370, 347, 0, "AUTO", GLUT_BITMAP_HELVETICA_10);
    displayRasterText(600, 347, 0, "AUTO", GLUT_BITMAP_HELVETICA_10);

    // CCTV camera on top
    glColor3f(0.30f, 0.30f, 0.35f);
    rect(488, 388, 512, 398);
    glColor3f(0.20f, 0.20f, 0.25f);
    glPushMatrix(); glTranslatef(500, 398, 0); circle(8); glPopMatrix();
    glColor3f(0.10f, 0.10f, 0.15f);
    glPushMatrix(); glTranslatef(500, 398, 0); circle(4); glPopMatrix();
    glColor3f(0.8f, 0.1f, 0.1f);
    glPushMatrix(); glTranslatef(508, 406, 0); circle(3); glPopMatrix(); // red light

    // University name board
    glColor3f(0.10f, 0.10f, 0.28f);
    rect(250, 420, 750, 440);
    glColor3f(1.0f, 0.95f, 0.20f);
    displayRasterText(258, 426, 0, "UNIVERSITY OF FRONTIER TECHNOLOGY, BANGLADESH - SMART ENTRANCE", GLUT_BITMAP_HELVETICA_10);

    // ---- Walking person approaching door ----
    // Only draw if not yet entered
    if (personX < 560)
    {
        drawPerson(personX, 150, 1.5f, 0.10f, 0.35f, 0.70f);
        // name tag above
        glColor3f(0.0f, 0.0f, 0.0f);
        displayRasterText(personX - 15, 178, 0, "Student", GLUT_BITMAP_HELVETICA_10);
    }

    // Trees on sides
    treeAt(150, 200, 30, 20);
    treeAt(840, 200, 30, 20);
    treeAt(100, 200, 25, 16);
    treeAt(890, 200, 25, 16);

    // Sun tiny
    glColor3f(1.0f, 0.95f, 0.2f);
    glPushMatrix(); glTranslatef(940, 470, 0); circle(22); glPopMatrix();

    // Clouds
    glColor3f(0.95f, 0.97f, 1.0f);
    cloud_model(cloud_ax + 50, 460, 0.7f);
    cloud_model(cloud_bx + 500, 475, 0.6f);

    // Label
    glColor3f(0.0f, 0.0f, 0.3f);
    displayRasterText(10, 10, 0, "Scene 2: Smart Entrance - Automatic Sliding Door  |  Right Click = Next | Left Click = Back", GLUT_BITMAP_HELVETICA_12);
}

///========================================================///
///*** SCENE 2: Attendance System – students scan & enter***///
///========================================================///
void drawScene2()
{
    // Indoor background (corridor)
    glColor3f(0.90f, 0.90f, 0.92f);
    rect(0, 0, 1000, 500);

    // Floor
    glColor3f(0.75f, 0.72f, 0.65f);
    rect(0, 0, 1000, 120);
    // Floor tiles
    glColor3f(0.68f, 0.65f, 0.58f);
    for (int tx = 0; tx < 1000; tx += 80)
        for (int ty = 0; ty < 120; ty += 40)
        {
            glBegin(GL_LINE_LOOP);
            glVertex2f(tx, ty); glVertex2f(tx+80, ty);
            glVertex2f(tx+80, ty+40); glVertex2f(tx, ty+40);
            glEnd();
        }

    // Ceiling
    glColor3f(0.96f, 0.96f, 0.98f);
    rect(0, 420, 1000, 500);
    // Ceiling lights
    for (int lx = 150; lx < 900; lx += 200)
    {
        glColor3f(1.0f, 1.0f, 0.85f);
        rect(lx, 440, lx+80, 455);
        // glow
        glColor3f(1.0f, 1.0f, 0.70f);
        rect(lx-10, 420, lx+90, 442);
    }

    // Left wall / corridor
    glColor3f(0.85f, 0.85f, 0.88f);
    rect(0, 120, 350, 420);
    // Right wall
    rect(650, 120, 1000, 420);

    // ------ Attendance Device (kiosk) ------
    // Stand
    glColor3f(0.35f, 0.35f, 0.40f);
    rect(470, 120, 530, 230);
    // Device body
    glColor3f(0.20f, 0.20f, 0.25f);
    rect(440, 230, 560, 380);
    // Screen
    glColor3f(0.05f, 0.12f, 0.25f);
    rect(450, 240, 550, 370);
    // Screen content - title
    glColor3f(0.20f, 0.90f, 0.30f);
    displayRasterText(455, 350, 0, "SMART ATTENDANCE", GLUT_BITMAP_HELVETICA_10);
    displayRasterText(463, 335, 0, "UFTB - IRE Dept.", GLUT_BITMAP_HELVETICA_10);

    // Divider line on screen
    glColor3f(0.20f, 0.70f, 0.20f);
    glBegin(GL_LINES);
    glVertex2f(452, 330); glVertex2f(548, 330);
    glEnd();

    // Display attendance on screen
    if (scanDone[0])
    {
        glColor3f(0.20f, 1.0f, 0.40f);
        displayRasterText(453, 315, 0, "Naimur Rashid", GLUT_BITMAP_HELVETICA_10);
        displayRasterText(453, 302, 0, "2101042  [OK]", GLUT_BITMAP_HELVETICA_10);
    }
    else
    {
        glColor3f(0.60f, 0.60f, 0.70f);
        displayRasterText(455, 308, 0, "Waiting...", GLUT_BITMAP_HELVETICA_10);
    }
    if (scanDone[1])
    {
        glColor3f(0.20f, 1.0f, 0.40f);
        displayRasterText(453, 288, 0, "Sourav Chakraborty", GLUT_BITMAP_HELVETICA_10);
        displayRasterText(453, 275, 0, "2101031  [OK]", GLUT_BITMAP_HELVETICA_10);
    }

    // Fingerprint sensor glow on device
    float fpGlow = (float)(0.5f + 0.5f * sin(scanAnim * 5.0f));
    glColor3f(0.0f, fpGlow * 0.8f, fpGlow);
    glPushMatrix(); glTranslatef(500, 248, 0); circle(8); glPopMatrix();
    glColor3f(0.5f, 1.0f, 0.5f);
    displayRasterText(487, 238, 0, "SCAN", GLUT_BITMAP_HELVETICA_10);

    // Camera icon on device
    glColor3f(0.50f, 0.50f, 0.55f);
    glPushMatrix(); glTranslatef(500, 362, 0); circle(5); glPopMatrix();

    // --- Student 1 (Naimur) approaching ---
    float s1x = studentX[0];
    if (!studentEntered[0])
    {
        drawPerson(s1x, 120, 1.8f, 0.10f, 0.28f, 0.65f);
        glColor3f(0.0f, 0.0f, 0.5f);
        displayRasterText(s1x - 30, 160, 0, "MD Naimur Rashid", GLUT_BITMAP_HELVETICA_10);
        // ID card hanging
        glColor3f(0.90f, 0.90f, 0.30f);
        rect(s1x + 3, 130, s1x + 13, 140);
        glColor3f(0.0f, 0.0f, 0.0f);
        displayRasterText(s1x + 3, 132, 0, "ID", GLUT_BITMAP_HELVETICA_10);
    }

    // --- Student 2 (Sourav) approaching ---
    float s2x = studentX[1];
    if (scanDone[0] && !studentEntered[1])
    {
        drawPerson(s2x, 120, 1.8f, 0.55f, 0.15f, 0.10f);
        glColor3f(0.5f, 0.0f, 0.0f);
        displayRasterText(s2x - 30, 160, 0, "Sourav Chakraborty", GLUT_BITMAP_HELVETICA_10);
        glColor3f(0.90f, 0.90f, 0.30f);
        rect(s2x + 3, 130, s2x + 13, 140);
        glColor3f(0.0f, 0.0f, 0.0f);
        displayRasterText(s2x + 3, 132, 0, "ID", GLUT_BITMAP_HELVETICA_10);
    }

    // "Entered" message
    if (studentEntered[0])
    {
        glColor3f(0.0f, 0.60f, 0.10f);
        displayRasterText(650, 300, 0, "MD Naimur Rashid", GLUT_BITMAP_HELVETICA_18);
        displayRasterText(650, 278, 0, "Successfully Entered!", GLUT_BITMAP_HELVETICA_18);
    }
    if (studentEntered[1])
    {
        glColor3f(0.60f, 0.0f, 0.0f);
        displayRasterText(650, 240, 0, "Sourav Chakraborty", GLUT_BITMAP_HELVETICA_18);
        displayRasterText(650, 218, 0, "Successfully Entered!", GLUT_BITMAP_HELVETICA_18);
    }

    // Big attendance log panel (right side wall)
    glColor3f(0.15f, 0.15f, 0.20f);
    rect(660, 130, 980, 400);
    glColor3f(0.0f, 0.50f, 0.10f);
    displayRasterText(700, 380, 0, "=== ATTENDANCE LOG ===", GLUT_BITMAP_HELVETICA_12);
    glColor3f(0.20f, 0.80f, 0.30f);
    displayRasterText(668, 360, 0, "Date: 2025-01-15  |  IRE 404", GLUT_BITMAP_HELVETICA_10);
    glBegin(GL_LINES);
    glColor3f(0.0f, 0.60f, 0.10f);
    glVertex2f(665, 355); glVertex2f(975, 355);
    glEnd();

    if (scanDone[0])
    {
        glColor3f(0.20f, 1.0f, 0.40f);
        displayRasterText(668, 338, 0, "[1] MD Naimur Rashid (2101042)", GLUT_BITMAP_HELVETICA_12);
        displayRasterText(668, 322, 0, "    Status: PRESENT  Time: 09:00", GLUT_BITMAP_HELVETICA_10);
    }
    if (scanDone[1])
    {
        glColor3f(0.20f, 1.0f, 0.40f);
        displayRasterText(668, 302, 0, "[2] Sourav Chakraborty (2101031)", GLUT_BITMAP_HELVETICA_12);
        displayRasterText(668, 286, 0, "    Status: PRESENT  Time: 09:02", GLUT_BITMAP_HELVETICA_10);
    }
    if (!scanDone[0])
    {
        glColor3f(0.50f, 0.50f, 0.55f);
        displayRasterText(668, 338, 0, "No attendance yet...", GLUT_BITMAP_HELVETICA_12);
    }

    // Label
    glColor3f(0.0f, 0.0f, 0.3f);
    displayRasterText(10, 10, 0, "Scene 3: Smart Attendance System  |  Right Click = Next | Left Click = Back", GLUT_BITMAP_HELVETICA_12);
}

///=========================================///
///*** SCENE 3: Classroom Smart Teaching  ***///
///=========================================///
void drawScene3()
{
    // Room background
    glColor3f(0.92f, 0.91f, 0.88f);
    rect(0, 0, 1000, 500);

    // Ceiling
    glColor3f(0.96f, 0.96f, 0.97f);
    rect(0, 440, 1000, 500);
    // Ceiling lights
    for (int lx = 100; lx < 950; lx += 180)
    {
        glColor3f(1.0f, 1.0f, 0.80f);
        rect(lx, 450, lx+100, 462);
        glColor3f(1.0f, 1.0f, 0.65f);
        rect(lx-5, 440, lx+105, 452);
    }

    // Back wall
    glColor3f(0.85f, 0.84f, 0.80f);
    rect(0, 200, 1000, 440);

    // Left wall
    glColor3f(0.80f, 0.79f, 0.76f);
    rect(0, 0, 60, 500);
    // Right wall
    rect(940, 0, 1000, 500);

    // Floor
    glColor3f(0.65f, 0.58f, 0.45f);
    rect(0, 0, 1000, 200);
    // Floor tiles
    glColor3f(0.58f, 0.52f, 0.40f);
    for (int tx = 0; tx < 1000; tx += 100)
    {
        glBegin(GL_LINES);
        glVertex2f(tx, 0); glVertex2f(tx, 200);
        glEnd();
    }
    for (int ty = 0; ty < 200; ty += 50)
    {
        glBegin(GL_LINES);
        glVertex2f(0, ty); glVertex2f(1000, ty);
        glEnd();
    }

    // ---- Smart Board ----
    // Board frame
    glColor3f(0.25f, 0.25f, 0.30f);
    rect(100, 290, 700, 430);
    // Board screen
    glColor3f(0.05f, 0.08f, 0.20f);
    rect(108, 298, 692, 422);

    // Board content
    glColor3f(0.20f, 0.80f, 1.0f);
    displayRasterText(260, 405, 0, "** SMART BOARD **", GLUT_BITMAP_TIMES_ROMAN_24);
    glColor3f(0.90f, 0.90f, 0.95f);
    displayRasterText(115, 385, 0, "Course: IRE 404 - Computer Graphics & Animation", GLUT_BITMAP_HELVETICA_12);
    glBegin(GL_LINES);
    glColor3f(0.40f, 0.40f, 0.80f);
    glVertex2f(112, 380); glVertex2f(688, 380);
    glEnd();
    // Content on board
    glColor3f(0.85f, 0.95f, 1.0f);
    displayRasterText(115, 365, 0, "Topic: OpenGL Animation & Scene Management", GLUT_BITMAP_HELVETICA_12);
    displayRasterText(115, 348, 0, "> Primitives: GL_POLYGON, GL_LINES, GL_TRIANGLE_STRIP", GLUT_BITMAP_HELVETICA_10);
    displayRasterText(115, 332, 0, "> Transformations: Translate, Rotate, Scale", GLUT_BITMAP_HELVETICA_10);
    displayRasterText(115, 316, 0, "> Animation: glutTimerFunc / glutPostRedisplay", GLUT_BITMAP_HELVETICA_10);
    displayRasterText(115, 300, 0, "> Scene Management: Right Click / Left Click Navigation", GLUT_BITMAP_HELVETICA_10);

    // Teacher's marker moving on board
    float mkx = 340 + markerX;
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_POLYGON);
    glVertex2f(mkx, 375); glVertex2f(mkx+4, 380); glVertex2f(mkx+8, 375); glVertex2f(mkx+4, 368);
    glEnd();

    // Board stand
    glColor3f(0.40f, 0.38f, 0.32f);
    rect(380, 270, 420, 292);
    rect(340, 267, 460, 275);

    // ---- Teacher ----
    // Teacher body (bigger)
    float tx_pos = 750.0f, ty_pos = 200.0f;
    // Head
    glColor3f(0.92f, 0.76f, 0.58f);
    glPushMatrix(); glTranslatef(tx_pos, ty_pos + 80, 0); circle(18); glPopMatrix();
    // Hair
    glColor3f(0.15f, 0.10f, 0.05f);
    glPushMatrix(); glTranslatef(tx_pos, ty_pos + 88, 0); circle(14); glPopMatrix();
    // Body
    glColor3f(0.15f, 0.25f, 0.55f);
    rect(tx_pos - 15, ty_pos + 30, tx_pos + 15, ty_pos + 62);
    // Arm pointing at board
    glColor3f(0.15f, 0.25f, 0.55f);
    glBegin(GL_LINES);
    glLineWidth(4.0f);
    glVertex2f(tx_pos - 15, ty_pos + 55);
    glVertex2f(tx_pos - 80, ty_pos + 65);
    glEnd();
    glLineWidth(1.0f);
    // Legs
    glColor3f(0.12f, 0.12f, 0.30f);
    rect(tx_pos - 12, ty_pos, tx_pos - 3, ty_pos + 32);
    rect(tx_pos + 3, ty_pos, tx_pos + 12, ty_pos + 32);
    // Pointer stick
    glColor3f(0.55f, 0.38f, 0.20f);
    glBegin(GL_LINES);
    glVertex2f(tx_pos - 80, ty_pos + 65);
    glVertex2f(tx_pos - 150, ty_pos + 80);
    glEnd();

    // Teacher label
    glColor3f(1.0f, 0.0f, 0.5f);
    displayRasterText(715, 295, 0, "Saurav Sir", GLUT_BITMAP_HELVETICA_10);

    // ---- Student Benches ----
    // 3 rows of benches, students sitting
    float bench_colors[4][3] = {
        {0.10f, 0.30f, 0.70f},
        {0.60f, 0.15f, 0.15f},
        {0.10f, 0.55f, 0.20f},
        {0.55f, 0.30f, 0.05f}
    };

    for (int row = 0; row < 3; row++)
    {
        float by = 135 - row * 45;
        // Bench
        glColor3f(0.50f, 0.38f, 0.22f);
        rect(60, by - 5, 860, by + 5);
        // Desk
        glColor3f(0.60f, 0.48f, 0.30f);
        rect(60, by + 12, 860, by + 18);

        // Students on bench
        for (int s = 0; s < 6; s++)
        {
            float sx = 90 + s * 130;
            int ci = (row + s) % 4;
            // Sitting person (head + upper body only)
            glColor3f(0.92f, 0.76f, 0.58f);
            glPushMatrix(); glTranslatef(sx, by + 22, 0); circle(9); glPopMatrix();
            // Shirt
            glColor3f(bench_colors[ci][0], bench_colors[ci][1], bench_colors[ci][2]);
            rect(sx - 7, by + 5, sx + 7, by + 14);
            // Laptop on desk
            glColor3f(0.25f, 0.25f, 0.30f);
            rect(sx - 12, by + 14, sx + 12, by + 22);
            glColor3f(0.05f, 0.25f, 0.55f);
            rect(sx - 10, by + 15, sx + 10, by + 21);
        }
    }

    // Windows on back wall
    for (int wx = 720; wx <= 900; wx += 90)
    {
        glColor3f(0.60f, 0.70f, 0.80f);
        rect(wx, 330, wx+60, 420);
        glColor3f(0.70f, 0.85f, 0.95f);
        rect(wx+3, 333, wx+57, 417);
        // Window cross
        glColor3f(0.55f, 0.60f, 0.65f);
        glBegin(GL_LINES);
        glVertex2f(wx+30, 333); glVertex2f(wx+30, 417);
        glVertex2f(wx+3, 375); glVertex2f(wx+57, 375);
        glEnd();
    }

    // Door on right side
    glColor3f(0.48f, 0.35f, 0.20f);
    rect(870, 200, 930, 340);
    glColor3f(0.75f, 0.65f, 0.45f);
    rect(875, 205, 925, 335);
    // Door knob
    glColor3f(0.80f, 0.70f, 0.15f);
    glPushMatrix(); glTranslatef(878, 270, 0); circle(5); glPopMatrix();
    // Door label
    glColor3f(1.0f, 1.0f, 1.0f);
    displayRasterText(876, 320, 0, "CLASS", GLUT_BITMAP_HELVETICA_10);
    displayRasterText(878, 308, 0, "ROOM", GLUT_BITMAP_HELVETICA_10);
    displayRasterText(880, 296, 0, "2002", GLUT_BITMAP_HELVETICA_10);

    // Label
    glColor3f(0.0f, 0.0f, 0.3f);
    displayRasterText(10, 10, 0, "Scene 4: Smart Classroom  |  Right Click = Next | Left Click = Back", GLUT_BITMAP_HELVETICA_12);
}

///=================================================///
///*** SCENE 4: Rooftop Helipad - Students Leave ***///
///=================================================///
void drawScene4()
{
    // Sky
    glBegin(GL_POLYGON);
    glColor3f(0.35f, 0.65f, 0.95f); glVertex2f(0, 500);
    glColor3f(0.35f, 0.65f, 0.95f); glVertex2f(1000, 500);
    glColor3f(0.60f, 0.82f, 1.0f);  glVertex2f(1000, 150);
    glColor3f(0.60f, 0.82f, 1.0f);  glVertex2f(0, 150);
    glEnd();

    // Sun
    glColor3f(1.0f, 0.95f, 0.2f);
    glPushMatrix(); glTranslatef(920, 460, 0); circle(30); glPopMatrix();

    // Clouds
    glColor3f(0.95f, 0.97f, 1.0f);
    cloud_model(cloud_ax + 20, 450, 0.8f);
    cloud_model(cloud_bx + 400, 470, 0.7f);

    // City skyline below (distant buildings)
    glColor3f(0.45f, 0.45f, 0.50f);
    rect(0, 0, 1000, 150);
    // Buildings in distance
    glColor3f(0.38f, 0.38f, 0.43f);
    rect(50, 80, 120, 150);  rect(180, 60, 250, 150);
    rect(300, 90, 370, 150); rect(450, 50, 530, 150);
    rect(600, 75, 680, 150); rect(750, 85, 820, 150);
    rect(870, 55, 950, 150);
    // Windows on distant buildings
    glColor3f(0.90f, 0.85f, 0.40f);
    for (int bx = 60; bx < 950; bx += 18)
        for (int bly = 90; bly < 148; bly += 15)
            rect(bx, bly, bx+8, bly+8);

    // Rooftop (main platform)
    glColor3f(0.55f, 0.55f, 0.58f);
    rect(0, 145, 1000, 200);
    // Rooftop texture
    glColor3f(0.50f, 0.50f, 0.53f);
    for (int rx = 0; rx < 1000; rx += 60)
    {
        glBegin(GL_LINES);
        glVertex2f(rx, 145); glVertex2f(rx, 200);
        glEnd();
    }

    // Rooftop edge railing
    glColor3f(0.35f, 0.35f, 0.40f);
    rect(0, 195, 1000, 205);
    for (int rpx = 10; rpx < 1000; rpx += 30)
        rect(rpx, 198, rpx+4, 210);

    // ---- HELIPAD ----
    // Helipad circle
    glColor3f(0.30f, 0.30f, 0.32f);
    glPushMatrix(); glTranslatef(500, 170, 0); circle(80); glPopMatrix();
    glColor3f(0.90f, 0.90f, 0.10f);
    glPushMatrix(); glTranslatef(500, 170, 0); circle(75); glPopMatrix();
    glColor3f(0.30f, 0.30f, 0.32f);
    glPushMatrix(); glTranslatef(500, 170, 0); circle(68); glPopMatrix();
    glColor3f(0.90f, 0.90f, 0.10f);
    // H letter on helipad
    glLineWidth(6.0f);
    glBegin(GL_LINES);
    glVertex2f(480, 155); glVertex2f(480, 185);
    glVertex2f(520, 155); glVertex2f(520, 185);
    glVertex2f(480, 170); glVertex2f(520, 170);
    glEnd();
    glLineWidth(1.0f);

    // Corner lights on helipad
    float hpad_lights[4][2] = {{440,148},{560,148},{440,192},{560,192}};
    for (int hl = 0; hl < 4; hl++)
    {
        glColor3f(1.0f, 0.20f, 0.20f);
        glPushMatrix(); glTranslatef(hpad_lights[hl][0], hpad_lights[hl][1], 0); circle(4); glPopMatrix();
    }

    // Waiting students on rooftop (3 students waiting)
    const char* studentNames[3] = {"Naimur", "Sourav", "Other Student/s"};
    float studentShirtR[3] = {0.10f, 0.60f, 0.10f};
    float studentShirtG[3] = {0.28f, 0.15f, 0.50f};
    float studentShirtB[3] = {0.65f, 0.15f, 0.20f};

    for (int st = 0; st < 3; st++)
    {
        if (!roofHeli[st].pickedUp)
        {
            float sy = roofHeli[st].y;
            drawPerson(waitX_anim[st], sy, 2.0f, studentShirtR[st], studentShirtG[st], studentShirtB[st]);
            glColor3f(0.0f, 0.0f, 0.4f);
            displayRasterText(waitX_anim[st] - 20, sy + 40, 0, studentNames[st], GLUT_BITMAP_HELVETICA_12);
            // Bag / luggage
            glColor3f(0.55f, 0.40f, 0.25f);
            rect(waitX_anim[st] + 10, sy + 5, waitX_anim[st] + 25, sy + 20);
            glColor3f(0.40f, 0.30f, 0.18f);
            rect(waitX_anim[st] + 13, sy + 20, waitX_anim[st] + 22, sy + 25);
        }

    }

    // ---- Flying Helicopter ----
    drawHelicopter(roofHeliX, roofHeliY, 1.5f);

    // Scene label
    glColor3f(0.0f, 0.0f, 0.3f);
    displayRasterText(10, 10, 0, "Scene 5: Rooftop Helipad - Students going home!  |  Right Click = Next | Left Click = Back", GLUT_BITMAP_HELVETICA_12);
}

///=======================================================///
///*** SCENE 5: Funny Text - Dream University Message  ***///
///=======================================================///
void drawScene5()
{
    // Dark cool background
    glBegin(GL_POLYGON);
    glColor3f(0.04f, 0.04f, 0.18f); glVertex2f(0, 0);
    glColor3f(0.04f, 0.04f, 0.18f); glVertex2f(1000, 0);
    glColor3f(0.08f, 0.08f, 0.28f); glVertex2f(1000, 500);
    glColor3f(0.08f, 0.08f, 0.28f); glVertex2f(0, 500);
    glEnd();

    // Stars
    srand(42);
    glColor3f(1.0f, 1.0f, 1.0f);
    for (int st = 0; st < 80; st++)
    {
        float sx = (float)(rand() % 1000);
        float sy = (float)(rand() % 500);
        float sb = 0.5f + 0.5f * (float)sin(blinkTimer * 2.0f + st);
        glColor3f(sb, sb, sb);
        glPushMatrix(); glTranslatef(sx, sy, 0); circle(1.2f); glPopMatrix();
    }

    // Moon
    glColor3f(0.95f, 0.95f, 0.80f);
    glPushMatrix(); glTranslatef(900, 440, 0); circle(30); glPopMatrix();
    // Moon crater details
    glColor3f(0.85f, 0.85f, 0.70f);
    glPushMatrix(); glTranslatef(890, 448, 0); circle(7); glPopMatrix();
    glPushMatrix(); glTranslatef(912, 432, 0); circle(5); glPopMatrix();

    // Mini university silhouette at bottom
    glColor3f(0.12f, 0.12f, 0.25f);
    rect(0, 0, 1000, 80);
    // Building silhouettes
    rect(50, 30, 120, 80);   rect(200, 20, 280, 80);
    rect(360, 10, 420, 80);  rect(450, 25, 550, 80);
    rect(580, 12, 640, 80);  rect(720, 22, 800, 80);
    rect(880, 30, 940, 80);
    // Tiny windows glowing
    glColor3f(1.0f, 0.95f, 0.40f);
    for (int bx = 55; bx < 950; bx += 18)
        for (int bly = 35; bly < 78; bly += 14)
            rect(bx, bly, bx+6, bly+7);

    // ---- Funny Text Lines ----
    struct FunnyLine { float y; float r, g, b; const char* text; void* font; };
    FunnyLine lines[] = {
        {460, 1.0f, 0.90f, 0.0f,  "Welcome to UFTB — The FUTURE is NOW!", GLUT_BITMAP_TIMES_ROMAN_24},
        {435, 0.4f, 1.0f, 0.9f,  "(and by future we mean the WiFi actually works here)", GLUT_BITMAP_HELVETICA_18},
        {408, 1.0f, 0.60f, 0.2f, "Our Helicopters don't need fuel... they run on CGPA!", GLUT_BITMAP_TIMES_ROMAN_24},
        {383, 0.8f, 0.8f, 1.0f,  "(Students with 4.0 GPA get priority boarding  )", GLUT_BITMAP_HELVETICA_18},
        {358, 0.4f, 1.0f, 0.4f,  "Smart Attendance: Even your clone can't fool it!", GLUT_BITMAP_TIMES_ROMAN_24},
        {333, 1.0f, 0.7f, 0.8f,  "(We've seen the movies. We know about twins.  )", GLUT_BITMAP_HELVETICA_18},
        {308, 0.9f, 1.0f, 0.3f,  "Our Smart Board learns FASTER than students!", GLUT_BITMAP_TIMES_ROMAN_24},
        {283, 0.7f, 0.9f, 1.0f,  "(Students are still googling 'what is OpenGL')", GLUT_BITMAP_HELVETICA_18},
        {258, 1.0f, 0.5f, 0.5f,  "Sliding Doors auto-open — No more excuses for late!", GLUT_BITMAP_TIMES_ROMAN_24},
        {233, 0.8f, 1.0f, 0.8f,  "(The door sees you coming. The teacher also.)", GLUT_BITMAP_HELVETICA_18},
        {200, 1.0f, 1.0f, 0.0f,  "University of Frontier Technology, Bangladesh", GLUT_BITMAP_TIMES_ROMAN_24},
        {175, 0.5f, 0.9f, 1.0f,  "Where Engineering meets Imagination...and Helicopters!", GLUT_BITMAP_TIMES_ROMAN_24},
        {145, 1.0f, 0.85f, 0.0f, "Our DREAM — Your FUTURE — UFTB FOREVER!", GLUT_BITMAP_TIMES_ROMAN_24},
        {115, 0.6f, 1.0f, 0.6f,  "MD Naimur Rashid (2101042) & Sourav Chakraborty (2101031)", GLUT_BITMAP_HELVETICA_18},
        {90,  0.9f, 0.9f, 0.9f,  "IRE 404 - Computer Graphics & Animation  |  2025-2026", GLUT_BITMAP_HELVETICA_18},
    };

    int numLines = sizeof(lines) / sizeof(lines[0]);
    for (int li = 0; li < numLines; li++)
    {
        // Scrolling effect: offset by textScrollY
        float drawY = lines[li].y + textScrollY;
        if (drawY > 5 && drawY < 495)
        {
            glColor3f(lines[li].r, lines[li].g, lines[li].b);
            displayRasterText(30, drawY, 0, lines[li].text, lines[li].font);
        }
    }

    // Blinking bottom message
    float bl = (float)(0.5f + 0.5f * sin(blinkTimer * 3.0f));
    glColor3f(bl, 1.0f * bl, 0.0f);
    displayRasterText(280, 25, 0, ">> Left Click to go back  |  Press ESC to Exit <<", GLUT_BITMAP_HELVETICA_18);

    // Floating UFTB logo top
    glColor3f(0.20f, 0.80f, 1.0f);
    displayRasterText(390, 480, 0, "[ UFTB SMART UNIVERSITY ]", GLUT_BITMAP_HELVETICA_18);
}

///==================///
///***   Display   ***///
///==================///
void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT);

    if (!startProject)
    {
        introScreen();
        return;
    }

    switch (currentScene)
    {
        case 0: drawScene0(); break;
        case 1: drawScene1(); break;
        case 2: drawScene2(); break;
        case 3: drawScene3(); break;
        case 4: drawScene4(); break;
        case 5: drawScene5(); break;
    }
    // Speed / Pause indicator
    char speedInfo[50];
    sprintf(speedInfo, "Speed: %.2fx  %s  [SPACE=Pause | +/-=Speed]",
            animSpeed, animPaused ? "|| PAUSED" : "> RUNNING");
    glColor3f(0.0f, 0.0f, 0.0f);
    displayRasterText(10, 480, 0, speedInfo, GLUT_BITMAP_HELVETICA_12);
    glutSwapBuffers();
}

///===========================///
///***  Animation / Timer  ***///
///===========================///
void animate(int value)
{
    if (!animPaused)  // এই if দিয়ে পুরো block wrap করো
    {
        // Global
    sun_spin  += 0.015f;
    cloud_ax  += 0.04f * animSpeed; if (cloud_ax > 1050) cloud_ax = -200;
    cloud_bx  += 0.06f * animSpeed; if (cloud_bx > 750)  cloud_bx = -350;
    cloud_cx  += 0.03f * animSpeed; if (cloud_cx > 450)  cloud_cx = -600;
    cloud_dx  += 0.08f * animSpeed; if (cloud_dx > 900)  cloud_dx = -200;
    spin      += 4.0f * animSpeed;
    flag_wave += 0.08f;
    blinkTimer += 0.04f;

    // Scene 0
    if (currentScene == 0)
    {
        heli_x += 0.6f * animSpeed;
        if (heli_x > 1150) heli_x = -250;
    }

    // Scene 1 – person walks, door opens
    if (currentScene == 1)
    {
        if (personWalking && personX < 490)
        {
            personX += 0.5f * animSpeed;
        }
        // Open door when person near
        if (personX > 430 && personX < 510)
        {
            doorOpening = true;
        }
        else
        {
            doorOpening = false;
        }
        if (doorOpening && doorOpen < 1.0f)
            doorOpen += 0.015f * animSpeed;
        else if (!doorOpening && doorOpen > 0.0f && personX > 510)
            doorOpen -= 0.02f  * animSpeed;
        if (doorOpen < 0.0f) doorOpen = 0.0f;
        if (doorOpen > 1.0f) doorOpen = 1.0f;

        // Person entered the building
        if (personX >= 490)
        {
            personWalking = false;
            // Reset after a while
            if (personX < 600) personX += 0.5f * animSpeed;
            else { personX = 200.0f; personWalking = true; }
        }
    }

    // Scene 2 – attendance
    if (currentScene == 2)
    {
        scanAnim += 0.05f * animSpeed;
        // Student 1 walks to device
        if (!scanDone[0] && studentX[0] < 460)
            studentX[0] += 0.5f * animSpeed;
        else if (!scanDone[0] && studentX[0] >= 460)
        {
            scanDone[0] = true;
            studentX[0] = 460;
        }
        // After scan[0] done, student enters
        if (scanDone[0] && !studentEntered[0])
        {
            if (studentX[0] < 560) studentX[0] += 0.4f * animSpeed;
            else studentEntered[0] = true;
        }
        // Student 2 walks after student 1 enters
        if (studentEntered[0] && !scanDone[1] && studentX[1] < 460)
            studentX[1] += 0.5f * animSpeed;
        else if (studentEntered[0] && !scanDone[1] && studentX[1] >= 460)
        {
            scanDone[1] = true;
            studentX[1] = 460;
        }
        if (scanDone[1] && !studentEntered[1])
        {
            if (studentX[1] < 560) studentX[1] += 0.4f * animSpeed;
            else studentEntered[1] = true;
        }
        // Reset after both entered
        if (studentEntered[0] && studentEntered[1])
        {
            static int resetTimer = 0;
            resetTimer++;
            if (resetTimer > 200)
            {
                resetTimer = 0;
                scanDone[0] = false; scanDone[1] = false;
                studentEntered[0] = false; studentEntered[1] = false;
                studentX[0] = 100.0f; studentX[1] = 200.0f;
            }
        }
    }

    // Scene 3 – marker on board
    if (currentScene == 3)
    {
        if (markerDir) markerX += 0.5f * animSpeed;
        else           markerX -= 0.5f * animSpeed;
        if (markerX > 200) markerDir = false;
        if (markerX < 0)   markerDir = true;
    }

    // Scene 4 – roof helicopter / pickup
    if (currentScene == 4)
    {
        roofSpin += 4.0f * animSpeed;

        float helipadY = 255.0f;  // helipad surface
        float approachY = 330.0f; // approach height

        if (roofPhase == 0)  // Helicopter আসছে
        {
            roofHeliX += 1.5f * animSpeed;
            // helipad এর উপরে পৌঁছালে
            if (roofHeliX >= 460)
            {
                roofHeliX = 460;
                roofPhase = 1;
            }
        }
        else if (roofPhase == 1)  // Landing
        {
            if (roofHeliY > helipadY)
                roofHeliY -= 0.8f * animSpeed;
            else
            {
                roofHeliY = helipadY;
                roofPhase = 2;
                boardingTimer = 0;
            }
        }
        else if (roofPhase == 2)  // Student boarding
        {
            boardingTimer += animSpeed;
            float targetX = 480.0f;
            if (studentsPicked < 3)
            {
                float& sx = waitX_anim[studentsPicked];
                if (sx < targetX)       sx += 0.8f * animSpeed;  // বাম থেকে আসলে
                else if (sx > targetX)  sx -= 0.8f * animSpeed;  // ডান থেকে আসলে
            }
            if (boardingTimer > 120)
            {
                if (studentsPicked < 3)
                    roofHeli[studentsPicked].pickedUp = true;
                roofPhase = 3;
            }
        }
        else if (roofPhase == 3)  // Takeoff
        {
            roofHeliY += 1.0f * animSpeed;
            if (roofHeliY > 400)  // উপরে উঠে গেলে
            {
                roofHeliX += 2.0f * animSpeed;
            }
            if (roofHeliX > 1200)  // চলে গেলে
            {
                studentsPicked++;
                roofHeliX = -200.0f;
                roofHeliY = approachY;
                roofPhase = (studentsPicked < 3) ? 0 : 0;
                if (studentsPicked >= 3) studentsPicked = 0; // reset
            }
        }
    }

    // Scene 5 – scroll text
    if (currentScene == 5)
    {
        textScrollY -= 0.2f;
        if (textScrollY < -520) textScrollY = 20.0f;
    }
    }
    glutPostRedisplay();
    glutTimerFunc(16, animate, 0);
}

///======================///
///*** Reset per scene ***///
///======================///
void resetScene(int scene)
{
    if (scene == 1)
    {
        personX = 200.0f; personWalking = true; doorOpen = 0.0f; doorOpening = false;
    }
    if (scene == 2)
    {
        scanDone[0] = false; scanDone[1] = false;
        studentEntered[0] = false; studentEntered[1] = false;
        studentX[0] = 100.0f; studentX[1] = 200.0f;
        scanAnim = 0.0f;
    }
    if (scene == 4)
    {
        roofPhase = 0;
        boardingTimer = 0;
        waitX_anim[0]=300; waitX_anim[1]=680; waitX_anim[2]=150;
        roofHeliX = -200.0f; roofHeliY = 300.0f; studentsPicked = 0;
        for (int s = 0; s < 3; s++)
        {
            roofHeli[s].x = 0; roofHeli[s].y = 155;
            roofHeli[s].pickedUp = false; roofHeli[s].flyY = 155;
        }
    }
    if (scene == 5)
    {
        textScrollY = 20.0f;
    }
}

///==================///
///***  Mouse     ***///
///==================///
void mouse(int button, int state, int x, int y)
{
    if (state != GLUT_DOWN) return;
    if (!startProject) return;

    if (button == GLUT_RIGHT_BUTTON)
    {
        if (currentScene < 5)
        {
            currentScene++;
            resetScene(currentScene);
            glutPostRedisplay();
        }
    }
    else if (button == GLUT_LEFT_BUTTON)
    {
        if (currentScene > 0)
        {
            currentScene--;
            resetScene(currentScene);
            glutPostRedisplay();
        }
    }
}

///==================///
///***  Keyboard  ***///
///==================///
void keyboard(unsigned char key, int x, int y)
{
    if (key == 13) // ENTER
    {
        startProject = true;
        resetScene(0);
        glutTimerFunc(16, animate, 0);
        glutPostRedisplay();
    }
    if (key == 27) exit(0);      // ESC
    if (key == ' ') animPaused = !animPaused;
    if (key == '+') { if (animSpeed < 4.0f)  animSpeed += 0.5f; }
    if (key == '-') { if (animSpeed > 0.25f) animSpeed -= 0.25f; }
    if (key == 'r' || key == 'R') animSpeed = 1.0f;
}

///==================///
///***    Main    ***///
///==================///
int main(int argc, char** argv)
{
    // Init rooftop helipad students
    for (int s = 0; s < 3; s++)
    {
        roofHeli[s].x = 0; roofHeli[s].y = 155;
        roofHeli[s].pickedUp = false; roofHeli[s].flyY = 155;
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowPosition(50, 30);
    glutInitWindowSize(1280, 640);
    glutCreateWindow("Smart University - UFTB");
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMainLoop();
    return 0;
}
