#include <ft2build.h>
#include FT_FREETYPE_H
#include <GL/glut.h>
#include <math.h>
#include <algorithm>
#include <iostream>
#include <chrono>

void *opengl(void *arg);

using clk = std::chrono::system_clock;
using sec = std::chrono::duration<int>;
using namespace std;

const float PI = 3.14159265f;
int X_BORDER = 2;
int Y_BORDER = 1;
int WIN_HEIGHT;
int WIN_WIDTH;
int TIME_BAR;
int RUN_TIME;
int fontSize = 24;
char TIME[20];
std::chrono::_V2::system_clock::time_point start;

// FreeType library context
FT_Library ftLibrary;

// Font face
FT_Face fontFace;

// Function to initialize FreeType
void initFreeType()
{
    // Initialize FreeType library
    FT_Init_FreeType(&ftLibrary);

    // Load the font face

    // /usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf
    FT_New_Face(ftLibrary, "./res/UbuntuMono-R.ttf", 0, &fontFace);

    // Set the font size (in pixels)
    FT_Set_Pixel_Sizes(fontFace, 0, fontSize);
}

// Function to render text
void renderText(const char *text, float x, float y, int size)
{
    x = (x + 1) * WIN_WIDTH / 2;
    y = (y + 1) * (WIN_HEIGHT - TIME_BAR) / 2;
    float sizeF = (float)size * min(WIN_WIDTH, WIN_HEIGHT) / 600;
    // fontSize = int(sizeF);
    FT_Set_Pixel_Sizes(fontFace, 0, int(sizeF));

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, WIN_WIDTH, 0, WIN_HEIGHT);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    int textWidth = 0;
    const char *c;
    for (c = text; *c; ++c)
    {
        FT_Load_Char(fontFace, *c, FT_LOAD_RENDER);
        textWidth += (fontFace->glyph->advance.x >> 6);
    }

    // Calculate the starting position to center the text
    float startX = x - (textWidth / 2.0f);

    // Set the position of the text
    glTranslatef(startX, y, 0);

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set font rendering parameters
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Iterate over the characters in the text
    for (c = text; *c; ++c)
    {
        // Load the glyph for the current character
        if (FT_Load_Char(fontFace, *c, FT_LOAD_RENDER))
            continue;

        // Access the glyph's bitmap
        FT_Bitmap *bitmap = &(fontFace->glyph->bitmap);

        for (unsigned row = 0; row < bitmap->rows / 2; ++row)
        {
            unsigned char *topRow = bitmap->buffer + row * bitmap->width;
            unsigned char *bottomRow = bitmap->buffer + (bitmap->rows - row - 1) * bitmap->width;
            for (unsigned col = 0; col < bitmap->width; ++col)
            {
                unsigned char temp = topRow[col];
                topRow[col] = bottomRow[col];
                bottomRow[col] = temp;
            }
        }
        fontFace->glyph->bitmap_top = bitmap->rows - fontFace->glyph->bitmap_top;

        // Render the glyph using OpenGL
        glRasterPos2f(fontFace->glyph->bitmap_left, -fontFace->glyph->bitmap_top);
        glDrawPixels(bitmap->width, bitmap->rows, GL_ALPHA, GL_UNSIGNED_BYTE, bitmap->buffer);

        // Move the pen position
        glTranslatef((fontFace->glyph->advance.x >> 6) + 0 * fontSize, 0, 0);
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}

void drawCircle(float r, float x, float y)
{

    float i = 0.0f;

    glBegin(GL_TRIANGLE_FAN);

    glVertex2f(x, y); // Center
    for (i = 0.0f; i <= 360; i++)
        glVertex2f(r * cos(M_PI * i / 180.0) + x, r * sin(M_PI * i / 180.0) + y);

    glEnd();
}

/* This function draws a rectangle with center point, length, and options passed */
void drawRectangle(float x, float y, float xLength, float yLength, bool drawBorder, bool clearColor = false)
{
    if (clearColor)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glColor4f(0.0f, 0.0f, 0.0f, 0.0f); // Set clear color (transparent)
    }
    glBegin(GL_QUADS); // draw a quad
    float x1 = x - (0.5 * xLength);
    float x2 = x + (0.5 * xLength);
    float y1 = y - (0.5 * yLength);
    float y2 = y + (0.5 * yLength);
    glVertex2f(x1, y1); // bottom left corner
    glVertex2f(x2, y1); // bottom right corner
    glVertex2f(x2, y2); // top right corner
    glVertex2f(x1, y2); // top left corner
    glEnd();
    if (clearColor)
    {
        glDisable(GL_BLEND);
    }

    if (drawBorder)
    {
        glBegin(GL_LINE_LOOP);
        glColor3f(0.0f, 0.0f, 0.0f);
        glVertex2f(x1, y1); // bottom left corner
        glVertex2f(x2, y1); // bottom right corner
        glVertex2f(x2, y2); // top right corner
        glVertex2f(x1, y2); // top left corner
        glEnd();
    }
}

void drawOval(float xRadius, float yRadius, int numSegments)
{
    glBegin(GL_POLYGON);
    for (int i = 0; i < numSegments; i++)
    {
        float angle = 2.0f * PI * static_cast<float>(i) / numSegments;
        float x = xRadius * cos(angle);
        float y = yRadius * sin(angle);
        glVertex2f(x, y);
    }
    glEnd();
}

void drawAnt(double x, double y, double direction)
{
    double scaleDown = 17;
    float legHeight = 0.45f / scaleDown;
    float legWidth = 0.03f / scaleDown;

    glColor3f(0.0f, 0.0f, 0.0f);
    glPushMatrix();
    /* Draw ant body */
    glTranslatef(x, y, 0.0f);
    glRotatef(direction, 0.0f, 0.0f, 1.0f);
    drawOval(0.17f / scaleDown, 0.25f / scaleDown, 100);

    glTranslatef(0.0f / scaleDown, 0.38f / scaleDown, 0.0f);
    drawOval(0.13f / scaleDown, 0.15f / scaleDown, 100);

    glTranslatef(0.0f / scaleDown, 0.3f / scaleDown, 0.0f);
    drawOval(0.13f / scaleDown, 0.17f / scaleDown, 100);

    /* Draw ant legs */

    glPushMatrix();
    glTranslatef(0, -0.2f / scaleDown, 0.0f);    // Translate to the left leg position
    glRotatef(35.0f, 0.0f, 0.0f, 1.0f);          // Rotate the leg
    glTranslatef(0.0f, -legHeight / 2.0f, 0.0f); // Translate to the center of the leg
    glScalef(legWidth, legHeight, legWidth);     // Scale the leg dimensions
    glutSolidCube(1.0f);                         // Cube-shaped leg
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.25 / scaleDown, -0.68 / scaleDown, 1.0f);   // Translate to the end of the leg
    glScalef(legWidth / 1.1, legHeight / 2.0, legWidth / 1.1); // Scale the dimensions for the vertical line
    glutSolidCube(1.0f);                                       // Cube-shaped vertical line
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0, -0.2f / scaleDown, 0.0f);      // Translate to the left leg position
    glRotatef(60.0f, 0.0f, 0.0f, 1.0f);            // Rotate the leg
    glTranslatef(0.0f, -legHeight / 2.0f, 0.0f);   // Translate to the center of the leg
    glScalef(legWidth, 0.3 / scaleDown, legWidth); // Scale the leg dimensions
    glutSolidCube(1.0f);                           // Cube-shaped leg
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.34 / scaleDown, -0.48 / scaleDown, 1.0f); // Translate to the end of the leg
    glRotatef(10.0f, 0.0f, 0.0f, 1.0f);
    glScalef(legWidth / 1.1, legHeight / 2.0, legWidth / 1.1); // Scale the dimensions for the vertical line
    glutSolidCube(1.0f);                                       // Cube-shaped vertical line
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0, -0.2f / scaleDown, 0.0f);      // Translate to the left leg position
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);            // Rotate the leg
    glTranslatef(0.0f, -legHeight / 2.8f, 0.0f);   // Translate to the center of the leg
    glScalef(legWidth, 0.2 / scaleDown, legWidth); // Scale the leg dimensions
    glutSolidCube(1.0f);                           // Cube-shaped leg
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.29 / scaleDown, -0.12 / scaleDown, 1.0f); // Translate to the end of the leg
    glRotatef(150.0f, 0.0f, 0.0f, 1.0f);
    glScalef(legWidth / 1.1, legHeight / 2.0, legWidth / 1.1); // Scale the dimensions for the vertical line
    glutSolidCube(1.0f);                                       // Cube-shaped vertical line
    glPopMatrix();

    //////////////////////

    glPushMatrix();
    glTranslatef(0, -0.2f / scaleDown, 0.0f);    // Translate to the left leg position
    glRotatef(-35.0f, 0.0f, 0.0f, 1.0f);         // Rotate the leg
    glTranslatef(0.0f, -legHeight / 2.0f, 0.0f); // Translate to the center of the leg
    glScalef(legWidth, legHeight, legWidth);     // Scale the leg dimensions
    glutSolidCube(1.0f);                         // Cube-shaped leg
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.25 / scaleDown, -0.68 / scaleDown, 1.0f);  // Translate to the end of the leg
    glScalef(legWidth / 1.1, legHeight / 2.0, legWidth / 1.1); // Scale the dimensions for the vertical line
    glutSolidCube(1.0f);                                       // Cube-shaped vertical line
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0, -0.2f / scaleDown, 0.0f);      // Translate to the left leg position
    glRotatef(-60.0f, 0.0f, 0.0f, 1.0f);           // Rotate the leg
    glTranslatef(0.0f, -legHeight / 2.0f, 0.0f);   // Translate to the center of the leg
    glScalef(legWidth, 0.3 / scaleDown, legWidth); // Scale the leg dimensions
    glutSolidCube(1.0f);                           // Cube-shaped leg
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.34 / scaleDown, -0.48 / scaleDown, 1.0f); // Translate to the end of the leg
    glRotatef(-10.0f, 0.0f, 0.0f, 1.0f);
    glScalef(legWidth / 1.1, legHeight / 2.0, legWidth / 1.1); // Scale the dimensions for the vertical line
    glutSolidCube(1.0f);                                       // Cube-shaped vertical line
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0, -0.2f / scaleDown, 0.0f);      // Translate to the left leg position
    glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);           // Rotate the leg
    glTranslatef(0.0f, -legHeight / 2.8f, 0.0f);   // Translate to the center of the leg
    glScalef(legWidth, 0.2 / scaleDown, legWidth); // Scale the leg dimensions
    glutSolidCube(1.0f);                           // Cube-shaped leg
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.29 / scaleDown, -0.12 / scaleDown, 1.0f); // Translate to the end of the leg
    glRotatef(-150.0f, 0.0f, 0.0f, 1.0f);
    glScalef(legWidth / 1.1, legHeight / 2.0, legWidth / 1.1); // Scale the dimensions for the vertical line
    glutSolidCube(1.0f);                                       // Cube-shaped vertical line
    glPopMatrix();

    /////////////////////////////////

    glPushMatrix();
    glTranslatef(0.13 / scaleDown, 0.2 / scaleDown, 1.0f); // Translate to the end of the leg
    glRotatef(150.0f, 0.0f, 0.0f, 1.0f);
    glScalef(legWidth / 1.1, legHeight / 2.5, legWidth / 1.1); // Scale the dimensions for the vertical line
    glutSolidCube(1.0f);                                       // Cube-shaped vertical line
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.13 / scaleDown, 0.2 / scaleDown, 1.0f); // Translate to the end of the leg
    glRotatef(-150.0f, 0.0f, 0.0f, 1.0f);
    glScalef(legWidth / 1.1, legHeight / 2.5, legWidth / 1.1); // Scale the dimensions for the vertical line
    glutSolidCube(1.0f);                                       // Cube-shaped vertical line
    glPopMatrix();

    glPopMatrix();

    glutPostRedisplay();
}

void modifyMatrix()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-2.0, 2.0, -1.0, 1.1, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
}

// GLUT reshape function
void reshape(int width, int height)
{
    // make the view port always 2:1 with margin
    int margin;
    if (width < 2 * height)
    {
        margin = 0.05 * width;
        WIN_HEIGHT = (width - 2 * margin) / 2;
        WIN_WIDTH = width - 2 * margin;
        TIME_BAR = 0.1 * WIN_HEIGHT;
        WIN_HEIGHT += TIME_BAR;
        glViewport(margin, (height - width / 2) / 2 + margin / 2 - TIME_BAR / 2, WIN_WIDTH, WIN_HEIGHT);
    }
    else
    {
        margin = 0.1 * height;
        WIN_HEIGHT = height - 2 * margin;
        WIN_WIDTH = (height - 2 * margin) * 2;
        TIME_BAR = 0.1 * WIN_HEIGHT;
        WIN_HEIGHT += TIME_BAR;
        glViewport((width - height * 2) / 2 + 2 * margin, margin - TIME_BAR / 2, WIN_WIDTH, WIN_HEIGHT);
    }
}

void updateTimer(int value)
{
    const auto time = std::chrono::duration_cast<std::chrono::seconds>(clk::now() - start);
    int minutes = time.count() / 60;
    int seconds = time.count() % 60;
    sprintf(TIME, "%02d:%02d", minutes, seconds);

    glutPostRedisplay();
    if (minutes >= RUN_TIME)
    {
        return;
    }
    glutTimerFunc(20, updateTimer, 0); // 20 milliseconds between updates (approximately 60 FPS)
}