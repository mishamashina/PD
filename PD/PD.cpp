#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include "Bmp.h"
#include "Sphere.h"
#include <windows.h>
using namespace std;

// GLUT CALLBACK functions
void displayCB();
void reshapeCB(int w, int h);
void timerCB(int millisec);
//void mouseCB(int button, int stat, int x, int y);
//void mouseMotionCB(int x, int y);
void keyboardCB(unsigned char key, int x, int y);
void SkeyboardCB(int key, int x, int y);
//void SkeyboardCB1(int key, int x, int y);
//void keyboardCB1(const char* fName, int i);

void initGL();
int  initGLUT(int argc, char **argv);
bool initSharedMem();
void clearSharedMem();
void initLights();
void setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ);
void drawString(const char *str, int x, int y, float color[4], void *font);
void drawString3D(const char *str, float pos[3], float color[4], void *font);
//void drawButton(const Button& button, float x, float y, float width, float height);
void toOrtho();
void toPerspective();
GLuint loadTexture(const char* fileName, bool wrap=true);
void showInfo();

void loadFiles();

// constants
const int   SCREEN_WIDTH    = 500;
const int   SCREEN_HEIGHT   = 500;
const float CAMERA_DISTANCE = 4.0f;
const int   TEXT_WIDTH      = 8;
const int   TEXT_HEIGHT     = 13;
int i = -1;
int flag = 0;

// global variables
void *font = GLUT_BITMAP_8_BY_13;
int screenWidth;
int screenHeight;
bool mouseLeftDown;
bool mouseRightDown;
bool mouseMiddleDown;
float mouseX, mouseY;
float cameraAngleX;
float cameraAngleY;
float cameraDistance;
int drawMode;
GLuint texId;
int imageWidth;
int imageHeight;
string masloadFiles[10] = {};



// sphere: min sector = 3, min stack = 2
Sphere sphere(1.0f, 36, 18, true,  2);    // radius, sectors, stacks, smooth(default), Y-up

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{

    // init global vars
    initSharedMem();

    // init GLUT and GL
    initGLUT(argc, argv);
    initGL();
    //loadFiles();
    //texId = loadTexture("earth_daymap.bmp", true);
 
    // the last GLUT call (LOOP)
    // window will be shown and display callback is triggered by events
    // NOTE: this call never return main().
    glutMainLoop(); /* Start GLUT event-processing loop */


    return 0;
}

void loadFiles()
{
    //string masloadFiles[2] = {};
    //cout << "Enter file name:" << endl;
    //cin >> masloadFiles[0] >> masloadFiles[1];
    //cout << masloadFiles[0] << " " << masloadFiles[1];
    //for (int i = 0; i <= 1; i++)
    //{
    //    texId = loadTexture(masloadFiles[i].c_str(), true);
    //}
    //texId = loadTexture(masloadFiles[0].c_str(), true);

    //for (int i = 0; i <= 1; i++)
    //{
    //    cout << "Enter file name:" << endl;
    //    cin >> masloadFiles[i];
    //    cout << masloadFiles[i] << endl;
    //    loadTexture(masloadFiles[i].c_str(), true);
    //}
}

///////////////////////////////////////////////////////////////////////////////
// initialize GLUT for windowing
///////////////////////////////////////////////////////////////////////////////
int initGLUT(int argc, char **argv)
{
    // GLUT stuff for windowing
    // initialization openGL window.
    // it is called before any other GLUT routine
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);   // display mode

    glutInitWindowSize(screenWidth, screenHeight);  // window size

    glutInitWindowPosition(100, 100);               // window location

    // finally, create a window with openGL context
    // Window will not displayed until glutMainLoop() is called
    // it returns a unique ID
    int handle = glutCreateWindow(argv[0]);     // param is the title of window


    // register GLUT callback functions
    glutDisplayFunc(displayCB);
    glutTimerFunc(33, timerCB, 33);             // redraw only every given millisec
    glutKeyboardFunc(keyboardCB);
    glutReshapeFunc(reshapeCB);
    //glutMouseFunc(mouseCB);
    //glutMotionFunc(mouseMotionCB);
    glutSpecialFunc(SkeyboardCB);
    //glutSpecialFunc(SkeyboardCB1);
    //glutKeyboardFunc(keyboardCB1);

    return handle;
}



///////////////////////////////////////////////////////////////////////////////
// initialize OpenGL
// disable unused features
///////////////////////////////////////////////////////////////////////////////
void initGL()
{
    glShadeModel(GL_SMOOTH);                    // shading mathod: GL_SMOOTH or GL_FLAT
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);      // 4-byte pixel alignment

    // enable /disable features
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);

    glClearColor(0, 0, 0, 0);                   // background color
    glClearStencil(0);                          // clear stencil buffer
    glClearDepth(1.0f);                         // 0 is near, 1 is far
    glDepthFunc(GL_LEQUAL);

    initLights();
}



///////////////////////////////////////////////////////////////////////////////
// write 2d text using GLUT
// The projection matrix must be set to orthogonal before call this function.
///////////////////////////////////////////////////////////////////////////////
void drawString(const char *str, int x, int y, float color[4], void *font)
{
    glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT); // lighting and color mask
    glDisable(GL_LIGHTING);     // need to disable lighting for proper text color
    glDisable(GL_TEXTURE_2D);

    glColor4fv(color);          // set text color
    glRasterPos2i(x, y);        // place text position

    // loop all characters in the string
    while(*str)
    {
        glutBitmapCharacter(font, *str);
        ++str;
    }

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glPopAttrib();
}



///////////////////////////////////////////////////////////////////////////////
// draw a string in 3D space
///////////////////////////////////////////////////////////////////////////////
void drawString3D(const char *str, float pos[3], float color[4], void *font)
{
    glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT); // lighting and color mask
    glDisable(GL_LIGHTING);     // need to disable lighting for proper text color
    glDisable(GL_TEXTURE_2D);

    glColor4fv(color);          // set text color
    glRasterPos3fv(pos);        // place text position

    // loop all characters in the string
    while(*str)
    {
        glutBitmapCharacter(font, *str);
        ++str;
    }

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glPopAttrib();
}



///////////////////////////////////////////////////////////////////////////////
// initialize global variables
///////////////////////////////////////////////////////////////////////////////
bool initSharedMem()
{
    screenWidth = SCREEN_WIDTH;
    screenHeight = SCREEN_HEIGHT;

    mouseLeftDown = mouseRightDown = mouseMiddleDown = false;
    mouseX = mouseY = 0;

    cameraAngleX = cameraAngleY = 0.0f;
    cameraDistance = CAMERA_DISTANCE;

    drawMode = 0; // 0:fill, 1: wireframe, 2:points

    sphere.printSelf();

    return true;
}



///////////////////////////////////////////////////////////////////////////////
// clean up global vars
///////////////////////////////////////////////////////////////////////////////
void clearSharedMem()
{
}



///////////////////////////////////////////////////////////////////////////////
// initialize lights
///////////////////////////////////////////////////////////////////////////////
void initLights()
{
    // set up light colors (ambient, diffuse, specular)
    GLfloat lightKa[] = {.3f, .3f, .3f, 1.0f};  // ambient light
    GLfloat lightKd[] = {.7f, .7f, .7f, 1.0f};  // diffuse light
    GLfloat lightKs[] = {1, 1, 1, 1};           // specular light
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightKa);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightKd);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightKs);

    // position the light
    float lightPos[4] = {0, 0, 1, 0}; // directional light
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    glEnable(GL_LIGHT0);                        // MUST enable each light source after configuration
}



///////////////////////////////////////////////////////////////////////////////
// set camera position and lookat direction
///////////////////////////////////////////////////////////////////////////////
void setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(posX, posY, posZ, targetX, targetY, targetZ, 0, 1, 0); // eye(x,y,z), focal(x,y,z), up(x,y,z)
}



///////////////////////////////////////////////////////////////////////////////
// load raw image as a texture
///////////////////////////////////////////////////////////////////////////////
GLuint loadTexture(const char* fileName, bool wrap)
{
    Image::Bmp bmp;
    if(!bmp.read(fileName))
        return 0;     // exit if failed load image

    // get bmp info
    int width = bmp.getWidth();
    int height = bmp.getHeight();
    const unsigned char* data = bmp.getDataRGB();
    GLenum type = GL_UNSIGNED_BYTE;    // only allow BMP with 8-bit per channel

    // We assume the image is 8-bit, 24-bit or 32-bit BMP
    GLenum format;
    int bpp = bmp.getBitCount();
    if(bpp == 8)
        format = GL_LUMINANCE;
    else if(bpp == 24)
        format = GL_RGB;
    else if(bpp == 32)
        format = GL_RGBA;
    else
        return 0;               // NOT supported, exit

    // gen texture ID
    GLuint texture;
    glGenTextures(1, &texture);

    // set active texture and configure it
    glBindTexture(GL_TEXTURE_2D, texture);

    // select modulate to mix texture with color for shading
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

    // if wrap is true, the texture wraps over at the edges (repeat)
    //       ... false, the texture ends at the edges (clamp)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap ? GL_REPEAT : GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap ? GL_REPEAT : GL_CLAMP);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // copy texture data
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, type, data);
    //glGenerateMipmap(GL_TEXTURE_2D);

    // build our texture mipmaps
    switch(bpp)
    {
    case 8:
        gluBuild2DMipmaps(GL_TEXTURE_2D, 1, width, height, GL_LUMINANCE, type, data);
        break;
    case 24:
        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_RGB, type, data);
        break;
    case 32:
        gluBuild2DMipmaps(GL_TEXTURE_2D, 4, width, height, GL_RGBA, type, data);
        break;
    }

    bmp.printSelf();
    return texture;
}



///////////////////////////////////////////////////////////////////////////////
// display info messages
///////////////////////////////////////////////////////////////////////////////
void showInfo()
{
    // backup current model-view matrix
    glPushMatrix();                     // save current modelview matrix
    glLoadIdentity();                   // reset modelview matrix

    // set to 2D orthogonal projection
    glMatrixMode(GL_PROJECTION);        // switch to projection matrix
    glPushMatrix();                     // save current projection matrix
    glLoadIdentity();                   // reset projection matrix
    //gluOrtho2D(0, screenWidth, 0, screenHeight); // set to orthogonal projection
    glOrtho(0, screenWidth, 0, screenHeight, -1, 1); // set to orthogonal projection

    float color[4] = {1, 1, 1, 1};

    std::stringstream ss;
    ss << std::fixed << std::setprecision(3);

    //ss << "Picture: " << fName << std::ends;
    //drawString(ss.str().c_str(), 1, screenHeight - (1 * TEXT_HEIGHT), color, font);
    //ss.str("");
    /*if (i == 0)
    {
        ss << "Picture: " << "Earth_Daymap" << std::ends;
        drawString(ss.str().c_str(), 1, screenHeight - (1 * TEXT_HEIGHT), color, font);
        ss.str("");
    }
    if (i == 1)
    {
        ss << "Picture: " << "Earth_Nightmap" << std::ends;
        drawString(ss.str().c_str(), 1, screenHeight - (1 * TEXT_HEIGHT), color, font);
        ss.str("");
    }
    if (i == 2)
    {
        ss << "Picture: " << "Earth_Clouds" << std::ends;
        drawString(ss.str().c_str(), 1, screenHeight - (1 * TEXT_HEIGHT), color, font);
        ss.str("");
    }
    if (i == 3)
    {
        ss << "Picture: " << "Jupiter" << std::ends;
        drawString(ss.str().c_str(), 1, screenHeight - (1 * TEXT_HEIGHT), color, font);
        ss.str("");
    }
    if (i == 4)
    {
        ss << "Picture: " << "Mars" << std::ends;
        drawString(ss.str().c_str(), 1, screenHeight - (1 * TEXT_HEIGHT), color, font);
        ss.str("");
    }
    if (i == 5)
    {
        ss << "Picture: " << "Saturn" << std::ends;
        drawString(ss.str().c_str(), 1, screenHeight - (1 * TEXT_HEIGHT), color, font);
        ss.str("");
    }
    if (i == 6)
    {
        ss << "Picture: " << "Venus_Atmosphere" << std::ends;
        drawString(ss.str().c_str(), 1, screenHeight - (1 * TEXT_HEIGHT), color, font);
        ss.str("");
    }
    if (i == 7)
    {
        ss << "Picture: " <<"Venus_Surface" << std::ends;
        drawString(ss.str().c_str(), 1, screenHeight - (1 * TEXT_HEIGHT), color, font);
        ss.str("");
    }
    if (i == 8)
    {
        ss << "Picture: " << "Mercury" << std::ends;
        drawString(ss.str().c_str(), 1, screenHeight - (1 * TEXT_HEIGHT), color, font);
        ss.str("");
    }
    if (i == 9)
    {
        ss << "Picture: " << "Neptune" << std::ends;
        drawString(ss.str().c_str(), 1, screenHeight - (1 * TEXT_HEIGHT), color, font);
        ss.str("");
    }
    if (i == 10)
    {
        ss << "Picture: " << "Uranus" << std::ends;
        drawString(ss.str().c_str(), 1, screenHeight - (1 * TEXT_HEIGHT), color, font);
        ss.str("");
    }
    if (i == 11)
    {
        ss << "Picture: " << "Moon" << std::ends;
        drawString(ss.str().c_str(), 1, screenHeight - (1 * TEXT_HEIGHT), color, font);
        ss.str("");
    }*/


    // unset floating format
    ss << std::resetiosflags(std::ios_base::fixed | std::ios_base::floatfield);

    // restore projection matrix
    glPopMatrix();                   // restore to previous projection matrix

    // restore modelview matrix
    glMatrixMode(GL_MODELVIEW);      // switch to modelview matrix
    glPopMatrix();                   // restore to previous modelview matrix
}



///////////////////////////////////////////////////////////////////////////////
// set projection matrix as orthogonal
///////////////////////////////////////////////////////////////////////////////
void toOrtho()
{
    // set viewport to be the entire window
    glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight);

    // set orthographic viewing frustum
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screenWidth, 0, screenHeight, -1, 1);

    // switch to modelview matrix in order to set scene
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}



///////////////////////////////////////////////////////////////////////////////
// set the projection matrix as perspective
///////////////////////////////////////////////////////////////////////////////
void toPerspective()
{
    // set viewport to be the entire window
    glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight);

    // set perspective viewing frustum
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(40.0f, (float)(screenWidth)/screenHeight, 1.0f, 1000.0f); // FOV, AspectRatio, NearClip, FarClip

    // switch to modelview matrix in order to set scene
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}









//=============================================================================
// CALLBACKS
//=============================================================================

void displayCB()
{
    // clear buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // save the initial ModelView matrix before modifying ModelView matrix
    glPushMatrix();

    // tramsform modelview matrix
    glTranslatef(0, 0, -cameraDistance);

    // set material
    float ambient[]  = {0.5f, 0.5f, 0.5f, 1};
    float diffuse[]  = {0.7f, 0.7f, 0.7f, 1};
    float specular[] = {1.0f, 1.0f, 1.0f, 1};
    float shininess  = 128;
    glMaterialfv(GL_FRONT, GL_AMBIENT,   ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  specular);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);

    // line color
    float lineColor[] = {0.2f, 0.2f, 0.2f, 1};

    // draw sphere with texture
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse); // reset diffuse
    glPushMatrix();
    glTranslatef(0, 0, 0);
    glRotatef(cameraAngleX, 1, 0, 0);
    glRotatef(cameraAngleY, 0, 1, 0);
    glBindTexture(GL_TEXTURE_2D, texId);
    sphere.draw();
    glPopMatrix();

    glBindTexture(GL_TEXTURE_2D, 0);

    showInfo();    // print max range of glDrawRangeElements

    glPopMatrix();
    glutSwapBuffers();
}


void reshapeCB(int w, int h)
{
    screenWidth = w;
    screenHeight = h;
    toPerspective();
    //std::cout << "window resized: " << w << " x " << h << std::endl;

#ifdef _WIN32
    HWND handle = ::GetActiveWindow();
    RECT rect;
    //::GetWindowRect(handle, &rect); // with non-client area; border, titlebar etc.
    //std::cout << "=========================" << std::endl;
    //std::cout << "full window size with border: " << (rect.right - rect.left) << "x" << (rect.bottom - rect.top) << std::endl;
    //::GetClientRect(handle, &rect); // only client dimension
    //std::cout << "client window size: " << (rect.right - rect.left) << "x" << (rect.bottom - rect.top) << std::endl;
    //std::cout << "=========================" << std::endl;
#endif
}


void timerCB(int millisec)
{
    glutTimerFunc(millisec, timerCB, millisec);
    glutPostRedisplay();
}

void keyboardCB(unsigned char key, int x, int y)
{
    if (flag == 0)
    {
        //int k;
        //cout << "Enter the number of files:" << endl;
        //cin >> k;
        //for (int i = 0; i < k; i++)
        //{
        //    cout << "Enter the name of the file:" << endl;
        //    cin >> masloadFiles[i];
        //}
        //------------------------//




        //cout << "Enter the name of the files:" << endl;
        //cin >> masloadFiles[0] >> masloadFiles[1] >> masloadFiles[2] >> masloadFiles[3] >> masloadFiles[4] >> masloadFiles[5] >> masloadFiles[6] >> masloadFiles[7] >> masloadFiles[8] >> masloadFiles[9];
        //cout << masloadFiles[0] << " " << masloadFiles[1] << " " << masloadFiles[2];
        flag = 1;
    }   
    float color[4] = { 1, 1, 1, 1 };
    switch (key)
    {
    
    case '+':
        cameraDistance -= (y + mouseY) * 0.001f;
        mouseY = y;
        std::cout << "+" << std::endl;
        break;

    case '-':
        cameraDistance += (y + mouseY) * 0.001f;
        mouseY = y;
        break;

    case '1':
        if (masloadFiles[0].c_str() != 0){ texId = loadTexture(masloadFiles[0].c_str(), true);}
        texId = loadTexture("neptune.bmp", true);
        //showInfo(masloadFiles[0].c_str());
        break;
    case '2':
        if (masloadFiles[1].c_str() != 0) { texId = loadTexture(masloadFiles[1].c_str(), true); }
        //showInfo(masloadFiles[1].c_str());
        break;
    case '3':
        if (masloadFiles[2].c_str() != 0) { texId = loadTexture(masloadFiles[2].c_str(), true); }
        break;
    case '4':
        if (masloadFiles[3].c_str() != 0) { texId = loadTexture(masloadFiles[3].c_str(), true); }
        break;
    case '5':
        if (masloadFiles[4].c_str() != 0) { texId = loadTexture(masloadFiles[4].c_str(), true); }
        break;
    case '6':
        if (masloadFiles[5].c_str() != 0) { texId = loadTexture(masloadFiles[5].c_str(), true); }
        break;
    case '7':
        if (masloadFiles[6].c_str() != 0) { texId = loadTexture(masloadFiles[6].c_str(), true); }
        break;
    case '8':
        if (masloadFiles[7].c_str() != 0){ texId = loadTexture(masloadFiles[7].c_str(), true);}
        break;
    case '9':
        if (masloadFiles[8].c_str() != 0) { texId = loadTexture(masloadFiles[8].c_str(), true); }
        break;
    case '0':
        if (masloadFiles[9].c_str() != 0) { texId = loadTexture(masloadFiles[9].c_str(), true); }
        break;
    }
}

//void keyboardCB1(const char* fName, int i)
//{
//    switch (i)
//    {
//       case 1:
//           texId = loadTexture(fName, true);
//           key = 
//           break;
//       case 2:
//           texId = loadTexture(fName, true);
//           break;
//       /*case '3':
//           texId = loadTexture("earth_clouds.bmp", true);
//           i = 2;
//           break;
//       case '4':
//           texId = loadTexture("jupiter.bmp", true);
//           i = 3;
//           break;
//       case '5':
//           texId = loadTexture("mars.bmp", true);
//           i = 4;
//           break;
//       case '6':
//           texId = loadTexture("saturn.bmp", true);
//           i = 5;
//           break;
//       case '7':
//           texId = loadTexture("venus_atmosphere.bmp", true);
//           i = 6;
//           break;
//       case '8':
//           texId = loadTexture("venus_surface.bmp", true);
//           i = 7;
//           break;
//       case '9':
//           texId = loadTexture("mercury.bmp", true);
//           i = 8;
//           break;
//       case '0':
//           texId = loadTexture("neptune.bmp", true);
//           i = 9;
//           break;*/
//    }
//}


void SkeyboardCB(int key, int x, int y)
{
    switch (key)
    {
    
        case GLUT_KEY_UP:
            cameraAngleX += (y - mouseY) * 0.01f;
            std::cout << "UP" << std::endl;
            mouseX = 0;
            break;

        case GLUT_KEY_DOWN:
            cameraAngleX -= (y - mouseY) * 0.01f;
            std::cout << "DOWN" << std::endl;
            mouseX = 0;
            break;

        case GLUT_KEY_LEFT:
            cameraAngleY += (x - mouseX) * 0.005f;
            std::cout << "LEFT" << std::endl;
            mouseY = 0;
            break;
        case GLUT_KEY_RIGHT:
            cameraAngleY -= (x - mouseX) * 0.005f;
            std::cout << "RIGHT" << std::endl;
            mouseY = 0;
            break;
    }
}

//void mouseCB(int button, int state, int x, int y)
//{
//    mouseX = x;
//    mouseY = y;
//
//    if(button == GLUT_LEFT_BUTTON)
//    {
//        if(state == GLUT_DOWN)
//        {
//            mouseLeftDown = true;
//        }
//        else if(state == GLUT_UP)
//            mouseLeftDown = false;
//    }
//
//    else if(button == GLUT_RIGHT_BUTTON)
//    {
//        if(state == GLUT_DOWN)
//        {
//            mouseRightDown = true;
//        }
//        else if(state == GLUT_UP)
//            mouseRightDown = false;
//    }
//
//    else if(button == GLUT_MIDDLE_BUTTON)
//    {
//        if(state == GLUT_DOWN)
//        {
//            mouseMiddleDown = true;
//        }
//        else if(state == GLUT_UP)
//            mouseMiddleDown = false;
//    }
//}


//void mouseMotionCB(int x, int y)
//{
//    if(mouseLeftDown)
//    {
//        cameraAngleY += (x - mouseX);
//        cameraAngleX += (y - mouseY);
//        mouseX = x;
//        mouseY = y;
//    }
//    if(mouseRightDown)
//    {
//        //cameraDistance -= (y - mouseY) * 0.2f;
//        //mouseY = y;
//    }
//}
