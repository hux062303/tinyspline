#include "tinyspline.h"

#if defined(__APPLE__)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h> 
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <tinyspline.h>

tsBSpline spline;
GLUnurbsObj *theNurb;
size_t i; // loop counter
float t = 0.f;
// A, C, D, and E are convenience variables and simple wrap the access
// to the control points of `spline`. B is required to store the original
// second control point of `spline` which gets replaced with D and E.
// v and w are the vectors AB and CB, respectively.
float *A, *C, *D, *E;
float B[3], v[3], w[3];

/********************************************************
*                                                       *
* Modify these lines for experimenting.                 *
*                                                       *
********************************************************/
void setup()
{
    ts_bspline_new(
        2,      // degree of spline
        3,      // dimension of each point
        3,      // number of control points
        TS_CLAMPED,// used to hit first and last control point
        &spline // the spline to setup
    );
    
    // Setup control points.
    spline.ctrlp[0] = -1.0f;
    spline.ctrlp[1] = 1.0f;
    spline.ctrlp[2] = 0.0f;
    spline.ctrlp[3] = 1.0f;
    spline.ctrlp[4] = 1.0f;
    spline.ctrlp[5] = 0.0f;
    spline.ctrlp[6] = 1.0f;
    spline.ctrlp[7] = -1.0f;
    spline.ctrlp[8] = 0.0f;

    for (i = 0; i < 3; i++)
        B[i] = spline.ctrlp[i+3];

    float mid = (spline.knots[spline.n_knots - 1] - spline.knots[0]) /2;
    size_t k; // not required here
    ts_bspline_insert_knot(&spline, mid, 1, &spline, &k);

    A = spline.ctrlp;
    D = spline.ctrlp + 3;
    E = spline.ctrlp + 6;
    C = spline.ctrlp + 9;

    for (i = 0; i < 3; i++) {
        v[i] = B[i] - A[i];
        w[i] = B[i] - C[i];
    }
}

void tear_down()
{
    ts_bspline_free(&spline);
}

void displayText(float x, float y, float r, float g, float b, const char *string)
{
    size_t str_sz = strlen(string);
    glColor3f(r, g, b);
    glRasterPos2f(x, y);
    for(i = 0; i < str_sz; i++) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string[i]);
    }
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // keep in mind that `D` and `E` are simply pointers to the
    // corresponding control points of `spline`.
    for (i = 0; i < 3; i++) {
        D[i] = A[i] + t*v[i];
        E[i] = C[i] + t*w[i];
    }
    
    // draw spline
    glColor3f(1.0, 1.0, 1.0);
    glLineWidth(3);
    gluBeginCurve(theNurb);
        gluNurbsCurve(
            theNurb, 
            (GLint)spline.n_knots,
            spline.knots,
            (GLint)spline.dim,
            spline.ctrlp,
            (GLint)spline.order,
            GL_MAP1_VERTEX_3
        );
    gluEndCurve(theNurb);

    // draw control points
    glColor3f(1.0, 0.0, 0.0);
    glPointSize(5.0);
    glBegin(GL_POINTS);
      for (i = 0; i < spline.n_ctrlp; i++) 
         glVertex3fv(&spline.ctrlp[i * spline.dim]);
    glEnd();

    // draw B
    glColor3f(0.0, 0.0, 1.0);
    glBegin(GL_POINTS);
        glVertex3fv(B);
    glEnd();

    // display t
    char buffer[256];
    sprintf(buffer, "t: %.2f", t);
    displayText(-.2f, 1.2f, 0.0, 1.0, 0.0, buffer);
    
    glutSwapBuffers();
    glutPostRedisplay();

    t += 0.001f;
    if (t > 1.f) {
        t = 0.f;
    }
}




/********************************************************
*                                                       *
* Framework                                             *
*                                                       *
********************************************************/
void nurbsError(GLenum errorCode)
{
   const GLubyte *estring;

   estring = gluErrorString(errorCode);
   fprintf (stderr, "Nurbs Error: %s\n", estring);
   exit (0);
}
   
void init(void)
{
    glClearColor (0.0, 0.0, 0.0, 0.0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    theNurb = gluNewNurbsRenderer();
    gluNurbsProperty (theNurb, GLU_SAMPLING_TOLERANCE, 10.0);
    gluNurbsCallback(theNurb, GLU_ERROR, (GLvoid (*)()) nurbsError);
    setup();
}

void reshape(int w, int h)
{
   glViewport(0, 0, (GLsizei) w, (GLsizei) h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective (45.0, (GLdouble)w/(GLdouble)h, 3.0, 8.0);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslatef (0.0, 0.0, -5.0);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize (500, 500);
    glutInitWindowPosition (100, 100);
    glutCreateWindow(argv[0]);
    init();
    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutMainLoop();
    tear_down();
    return 0; 
}