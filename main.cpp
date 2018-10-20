#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <windows.h>
#include <GL/GL.h>
#include <GL/GLU.h>
#include <GL/glut.h>
#elif defined(__APPLE__)
#include <OpenGL/gl.h>
#include <glut.h>
#else
#include <GL/GL.h>
#include <GL/GLU.h>
#include <GL/glut.h>
#endif

#include <iostream>
#include <fstream>

using namespace std;


typedef struct Point {
    float x;
    float y;
    float z;
    float intensity;
};


int width = 1024;
int height = 768;

float min_x;
float max_x;

float min_y;
float max_y;

float min_z;
float max_z;

// The scale factors are calculated using the min and max x, y and z values
// to scale the model to fit in the viewport.
GLdouble x_scale_factor;
GLdouble y_scale_factor;
GLdouble z_scale_factor;

double slider1_offset = 0.0;
double slider2_offset = 0.0;
double slider3_offset = 0.0;

GLdouble x_rotation_angle = 0.0;
GLdouble y_rotation_angle = 0.0;
GLdouble z_rotation_angle = 0.0;

int volume_width;
int volume_height;
int volume_depth;

Point* volumetric_points;


void render_point_cloud() {
    glViewport(0, 0, (2 * width) / height, height);

}


void render_rotation_sliders() {
    glViewport(width / 3, (height * 3) / 4,
               width / 3, height / 4);

    glPushMatrix();
    glColor3f(1.0, 1.0, 1.0);
    glRasterPos2f(0, 1);
    glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_18, "rotate");
    glPopMatrix();
}


void render_scale_slider(double slider_offset_x) {
    glColor3d(0.0, 0.0, 0.0);

    glBegin(GL_LINES);

    glVertex3d(-0.5, 0.25, 0);
    glVertex3d(0.5, 0.25, 0);

    glEnd();

    glBegin(GL_LINES);

    glVertex3d(0.5, 0.25, 0);
    glVertex3d(0.5, 0, 0);

    glEnd();

    glBegin(GL_LINES);

    glVertex3d(0.5, 0, 0);
    glVertex3d(-0.5, 0, 0);

    glEnd();

    glBegin(GL_LINES);

    glVertex3d(-0.5, 0, 0);
    glVertex3d(-0.5, 0.25, 0);

    glEnd();

    glColor3d(1.0, 0.0, 0.0);

    glPushMatrix();
    glTranslated(slider_offset_x, 0, 0);

    glBegin(GL_QUADS);

    glVertex3d(-.5, 0.25, 0);
    glVertex3d(-.5, 0.25, 0);
    glVertex3d(-.25, 0, 0);
    glVertex3d(-.5, 0, 0);

    glEnd();

    glPopMatrix();
}


void render_quit_button() {

}


void render_control_window() {
    glViewport(width / 3, 0, width / 3, height);
    render_rotation_sliders();
    render_scale_slider();
    render_quit_button();
}


void draw() {
    render_rotation_sliders();
}


void resize(int width, int height) {

}


void mouse_input(int button, int state, int x, int y) {

}


void read_volume_file(char* filename) {
    ifstream volume_file(filename);
    char current_character;
    int character_number = 1;
    int slice_length = volume_width * volume_height;
    volumetric_points = new Point[slice_length * volume_depth];

    while (volume_file.get(current_character)) {
        Point point;
        point.z = character_number / volume_depth;
        point.y = (character_number % slice_length) / volume_width;
        point.x = (character_number % slice_length) / volume_height;
    }
}


int main(int argc, char** argv) {
    if (argc < 5) {
        printf("You must provide the path to the volumetric image file to render and the volume width, height and depth!");
        return 100;
    }

    char* filename = argv[1];
    char* given_width = argv[2];
    char* given_height = argv[3];
    char* given_depth = argv[4];

    volume_width = stoi(given_width);
    volume_height = stoi(given_height);
    volume_depth = stoi(given_depth);

    read_volume_file(filename);

    max_x = volume_width;
    min_x = 0;

    max_y = volume_height;
    min_y = 0;

    max_z = volume_depth;
    min_z = 0;

    float x_range = abs(max_x - min_x);
    float y_range = abs(max_y - min_y);
    float z_range = abs(max_z - min_z);

    x_scale_factor = 2.0 / x_range;
    y_scale_factor = 2.0 / y_range;
    z_scale_factor = 2.0 / z_range;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(width, height);
    glutCreateWindow("3D PLY Model Viewer");
    glutDisplayFunc(draw);
    glutReshapeFunc(resize);
    glutMouseFunc(mouse_input);

    glEnable(GL_DEPTH_TEST);

    glutMainLoop();

    return 0;
}
