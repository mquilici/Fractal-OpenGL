#define GL_SILENCE_DEPRECATION

#include "GL/glew.h"     // Include glew header
#include <GL/freeglut.h> // Includes freeglut header
#include <cmath>         // Include cmath for trig functions
#include <vector>
#include <string>
#include <iostream>
#include <random>

GLint rotateObject = GL_FALSE;
GLint moveObject = GL_FALSE;
GLint zoomObject = GL_FALSE;
GLfloat rotationH = 0;
GLfloat rotationV = 75;
GLint mousex, mousey;
GLboolean perspective = true;
GLboolean jitter = true;
GLfloat zinitial = -7.0f;
GLfloat xinitial = 0.0f;
GLfloat yinitial = 0.0f;
GLint windowWidth = 1024;
GLint windowHeight = 1024;
GLint xResolution = 201;
GLint yResolution = 201;
GLint zResolution = 16;
GLfloat fracColor[4] = {0.1f,0.1f,1.0f,0.5f};
GLfloat midColor[4] = {1.0f,0.0f,0.0f,1.0f};
GLfloat transparency = 0.5;
GLfloat pointSize = 1.0;
GLfloat xmin = -1.5;
GLfloat xmax = 1.5;
GLfloat ymin = -1.5;
GLfloat ymax = 1.5;
GLfloat xshift = -0.5;

// Fractal color struct
struct vec4 {
	GLfloat r;
	GLfloat g;
	GLfloat b;
	GLfloat a;
	vec4() {
		r = 0;
		g = 0;
		b = 0;
		a = 0;
	}
	vec4(GLfloat ir, GLfloat ig, GLfloat ib, GLfloat ia) {
		r = ir;
		g = ig;
		b = ib;
		a = ia;
	}
};

// Data point struct
struct vec3 {
	GLfloat x;
	GLfloat y;
	GLfloat z;
	vec3() {
		x = 0;
		y = 0;
		z = 0;
	}
	vec3(GLfloat ix, GLfloat iy, GLfloat iz) {
		x = ix;
		y = iy;
		z = iz;
	}
};

std::vector<vec3> imageVertices;
std::vector<vec4> imageColors;
std::vector<vec3> fracVertices;

// Compute 2D fractal image colors
vec4 getFractalColor(float x, float y)
{
	float cr = x;
	float ci = y;
	float zr = 0;
	float zi = 0;
	float z2 = 0;
	float tmp = 0;
	float c = 0;
	int max_itr = 255;

	cr = (abs(cr)<1e-6) ? 0 : cr;
	ci = (abs(ci)<1e-6) ? 0 : ci;

	for (int i=0; i<max_itr; ++i) {
		tmp = zr * zr - zi * zi + cr;
		zi = 2*zr*zi + ci;
		zr = tmp;
		z2 = zr*zr + zi*zi;
		if (z2 > 4) {
			c = 10.0*i/255.0;
			return vec4(c, c, 1.0f-c, 1.0f);
		}
	}

	return vec4(0.0f, 0.0f, 0.0f, 1.0f);
}

// Load 2D fractal image pixel coordinates and colors
void loadFractalImage(void)
{
	imageVertices.clear();
	imageColors.clear();
	int nx = xResolution;
	int ny = yResolution;
	float dx = (xmax-xmin)*1.0/nx;
	float dy = (ymax-ymin)*1.0/ny;
	vec3 v1, v2, v3, v4, v5, v6;
	vec4 vc;

	for (int i=0; i<nx; ++i) {
		for (int j=0; j<ny; ++j) {

			v1 = vec3(xmin+(i-0)*dx, -1.5, ymin+(j-0)*dy);
			v2 = vec3(xmin+(i+1)*dx, -1.5, ymin+(j-0)*dy);
			v3 = vec3(xmin+(i+1)*dx, -1.5, ymin+(j+1)*dy);
			v4 = vec3(xmin+(i-0)*dx, -1.5, ymin+(j+1)*dy);

			imageVertices.push_back(v1);
			imageVertices.push_back(v2);
			imageVertices.push_back(v3);
			imageVertices.push_back(v4);

			vc = getFractalColor(xmin+(i+0.5)*dx+xshift, ymin+(j+1.5)*dy);
			imageColors.push_back(vc);
		}
	}
	return;
}

// Get the 3D fractal vertices
void get3DVertices(float x, float y)
{
	float cr = x;
	float ci = y;
	float zr = 0;
	float zi = 0;
	float z2 = 0;
	float zrprev = 1.0f;
	float tmp = 0;
	int max_itr = 255;

	cr = (abs(cr)<1e-6) ? 0 : cr;
	ci = (abs(ci)<1e-6) ? 0 : ci;

	for (int i=max_itr; i>0; --i) {
		tmp = zr * zr - zi * zi + cr;
		zi = 2*zr*zi + ci;
		zr = tmp;
		z2 = zr*zr + zi*zi;
		if (i < zResolution) {
			fracVertices.push_back(vec3(x-xshift, -zr/1.5, y));
			if (z2 > 4) {
				return;
			}
		}
		zrprev = z2;
	}

	return;
}

// Load the 3D fractal points
void loadFractal3D(void)
{
	fracVertices.clear();
	int nx = xResolution;
	int ny = yResolution;
	float dx = (xmax-xmin)*1.0/nx;
	float dy = (ymax-ymin)*1.0/ny;
	float offsetx = 0;
	float offsety = 0;

	for (int i=0; i<nx; ++i) {
		for (int j=0; j<ny; ++j) {

			float xpos = xmin+(i+0.5)*dx+xshift;
			float ypos = ymin+(j+1.5)*dy;

			if (jitter) {
				if (abs(ypos) < 1e-6 ) {
					offsetx = 0;
					offsety = 0;
				} else {
					offsetx = (rand() % 100)/200.0*dx;
					offsety = (rand() % 100)/200.0*dy;
				}
			}

			get3DVertices(xpos+offsetx, ypos+offsety);
		}
	}

	return;
}

// Fractal rotation and translation
void motion(int x, int y)
{
	if (rotateObject) {
		GLfloat dy = (y - mousey) * 360.0 / windowHeight;
		GLfloat dx = (x - mousex) * 360.0 / windowWidth;

		rotationH += dx;
		if (rotationH > 360)
			rotationH -= 360;
		if (rotationH < 0)
			rotationH += 360;
		rotationV -= dy;
		if (rotationV > 180)
			rotationV = 180;
		if (rotationV < 0)
			rotationV = 0;

	} else if (moveObject) {
		GLfloat dy = (y - mousey) * 5.0 / windowHeight;
		GLfloat dx = (x - mousex) * 5.0 / windowWidth;

		xinitial += dx;
		yinitial -= dy;

	} else if (zoomObject) {
		GLfloat dy = (y - mousey) * 10.0 / windowHeight;

		zinitial += dy;
		zinitial = zinitial > 0 ? 0 : zinitial;

	}

	mousex = x;
	mousey = y;

	glutPostRedisplay();
}

// Mouse motion controls
void mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN) {
		switch (button) {
		case GLUT_LEFT_BUTTON:
			rotateObject = GL_TRUE;
			motion(mousex = x, mousey = y);
			break;
		case GLUT_MIDDLE_BUTTON:
			moveObject = GL_TRUE;
			motion(mousex = x, mousey = y);
			break;
		case GLUT_RIGHT_BUTTON:
			zoomObject = GL_TRUE;
			motion(mousex = x, mousey = y);
			break;
		}
	} else if (state == GLUT_UP) {
		switch (button) {
		case GLUT_LEFT_BUTTON:
			rotateObject = GL_FALSE;
			break;
		case GLUT_MIDDLE_BUTTON:
			moveObject = GL_FALSE;
			break;
		case GLUT_RIGHT_BUTTON:
			zoomObject = GL_FALSE;
			break;
		}
	}
}

// Keyboard motion controls
void key(unsigned char key, int x, int y)
{
	switch (key) {
	case '\033':
		exit(0);
	case 'q':
		exit(0);
	case 'p':
		perspective = !perspective;
		break;
	case '1':
		xResolution -= 50;
		yResolution -= 50;
		xResolution = xResolution < 50 ? 51 : xResolution;
		yResolution = yResolution < 50 ? 51 : yResolution;
		loadFractalImage();
		loadFractal3D();
		break;
	case '2':
		xResolution += 50;
		yResolution += 50;
		xResolution = xResolution > 400 ? 401 : xResolution;
		yResolution = yResolution > 400 ? 401 : yResolution;
		loadFractalImage();
		loadFractal3D();
		break;
	case 'd':
		rotationH += 10;
		break;
	case 'a':
		rotationH -= 10;
		break;
	case 'w':
		rotationV += 10;
		break;
	case 's':
		rotationV -= 10;
		break;
	case '=':
		pointSize += 0.5;
		pointSize = pointSize > 3 ? 3 : pointSize;
		break;
	case '-':
		pointSize -= 0.5;
		pointSize = pointSize < 0 ? 0.5 : pointSize;
		break;
	}

	if (rotationH > 360)
		rotationH -= 360;
	if (rotationH < 0)
		rotationH += 360;

	if (rotationV > 180)
		rotationV = 180;
	if (rotationV < 0)
		rotationV = 0;

	glutPostRedisplay();
}

// Display
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glShadeModel(GL_FLAT);
	//glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	GLfloat aspect = windowWidth*1.0/windowHeight;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (perspective) {
		gluPerspective(45.0, windowWidth*1.0/windowHeight, 0.01, 100.0);
	} else {
		glOrtho(zinitial/2*aspect, -zinitial/2*aspect, zinitial/2, -zinitial/2, 0.01, 100);
	}

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(xinitial, yinitial, zinitial);
	glRotatef(90-rotationV, 1, 0, 0);
	glRotatef(rotationH, 0, 1, 0);

	for (int i = 0; i < imageVertices.size(); i += 4) {
		glBegin(GL_QUADS);

		glVertex3f(imageVertices.at(i + 0).x, imageVertices.at(i + 0).y, imageVertices.at(i + 0).z);
		glVertex3f(imageVertices.at(i + 1).x, imageVertices.at(i + 1).y, imageVertices.at(i + 1).z);
		glVertex3f(imageVertices.at(i + 2).x, imageVertices.at(i + 2).y, imageVertices.at(i + 2).z);
		glVertex3f(imageVertices.at(i + 3).x, imageVertices.at(i + 3).y, imageVertices.at(i + 3).z);

		glColor4f(imageColors.at(i/4).r, imageColors.at(i/4).g, imageColors.at(i/4).b, imageColors.at(i/4).a);

		glEnd();
	}

	glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	glPointSize(-10.0f/(zinitial-0.25)*pointSize);

	glBegin(GL_POINTS);
	for (int i = 0; i < fracVertices.size(); i++) {
		if (abs(fracVertices.at(i).z) < 1e-6 ) {
			glColor4f(midColor[0],midColor[1],midColor[2],midColor[3]); // Sets the RGB values for the point
		} else {
			glColor4f(fracColor[0],fracColor[1],fracColor[2],fracColor[3]); // Sets the RGB values for the point
		}
		glVertex3f(fracVertices.at(i).x, fracVertices.at(i).y, fracVertices.at(i).z);
	}
	glEnd();

	glColor4f(1.0f,1.0f,1.0f,0.25f); // Sets the RGB values for the cube
	glutWireCube(3.0f);

	glPopMatrix();
	glFlush();
	glutSwapBuffers();
}

// Viewport scaling
void reshape(int w, int h)
{
	glViewport(0, 0, windowWidth = w, windowHeight = h);
}

// Load fractal
void init(void)
{

	glClearColor(0.0, 0.0, 0.0, 1.0);
	loadFractalImage();
	loadFractal3D();
}

// Main method
int main(int argc, char **argv)
{
	glutInit(&argc, argv);

	windowWidth = glutGet(GLUT_SCREEN_WIDTH);
	windowHeight = glutGet(GLUT_SCREEN_HEIGHT);
	glutInitWindowSize(windowWidth, windowHeight);

	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);

	// Center window using screen width and current width
	glutInitWindowPosition((glutGet(GLUT_SCREEN_WIDTH) - windowWidth) / 2,
			(glutGet(GLUT_SCREEN_HEIGHT) - windowHeight) / 2);

	glutCreateWindow("Fractal"); // Create a window and title

	// Callbacks
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(key);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);

	init();
	glutMainLoop();
	return 0; /* ANSI C requires main to return int. */
}
