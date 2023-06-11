#include <ft2build.h>
#include FT_FREETYPE_H
#include <GL/glut.h>
#include <math.h>
#include <algorithm>
#include <iostream>
#include <chrono>

using clk = std::chrono::system_clock;
using sec = std::chrono::duration<int>;
using namespace std;

int WIN_HEIGHT;
int WIN_WIDTH;
int TIME_BAR;
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
    FT_New_Face(ftLibrary, "./UbuntuMono-R.ttf", 0, &fontFace);

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

        for (int row = 0; row < bitmap->rows / 2; ++row)
        {
            unsigned char *topRow = bitmap->buffer + row * bitmap->width;
            unsigned char *bottomRow = bitmap->buffer + (bitmap->rows - row - 1) * bitmap->width;
            for (int col = 0; col < bitmap->width; ++col)
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

void modifyMatrix()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-2.0, 2.0, -1.0, 1.1, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
}

// GLUT display function
void display()
{
    modifyMatrix();

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(0.0f, 0.0f, 0.0f);

    glColor3f(1.0f, 0.0f, 0.0f);
    drawRectangle(0, 0, 10, 10, true);
    glColor3f(0.0f, 1.0f, 0.0f);
    drawRectangle(0, 0, 10, 2, true);

    renderText(TIME, 0, 1.12, 24);

    // Render the text at position (0, 0)
    renderText("Hello, World!", 0, 0, 24);

    drawCircle(0.05, 0.5, 0.5);
    glutSwapBuffers();
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
    // strcpy(TIME, timeStr.c_str());

    glutPostRedisplay();
    glutTimerFunc(20, updateTimer, 0); // 20 milliseconds between updates (approximately 60 FPS)
}

// GLUT main function
int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowSize(800, 600);
    glutCreateWindow("FreeType Text Rendering");

    initFreeType();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(0, updateTimer, 0);

    start = clk::now();
    glutMainLoop();

    // Cleanup FreeType resources
    FT_Done_Face(fontFace);
    FT_Done_FreeType(ftLibrary);

    return 0;
}
