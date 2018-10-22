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

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <fstream>

using namespace std;


typedef struct Point {
    int x;
    int y;
    int z;
    uint intensity;
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
double slider4_offset = 0.0;

GLdouble x_rotation_angle = 0.0;
GLdouble y_rotation_angle = 0.0;
GLdouble z_rotation_angle = 0.0;
GLdouble overall_scale_factor = 1.0;

int dragging = 0;
int dragging_x = 0;
int dragging_y = 0;
int dragging_z = 0;
int dragging_scale = 0;

int volume_width;
int volume_height;
int volume_depth;

int number_points;

Point* volumetric_points;


/**
 * Draw the rotation sliders into the bottom left viewport.
 */
void draw_slider(double slider_offset_x) {
    glColor3d(0.0, 0.0, 0.0);

#define TOP_LEFT (glVertex3d(15, 50, 0))
#define TOP_RIGHT (glVertex3d((width / 4) - 15, 50, 0))
#define BOTTOM_LEFT (glVertex3d(15, 0, 0))
#define BOTTOM_RIGHT (glVertex3d((width / 4) - 15, 0, 0))

    glBegin(GL_LINES);
    TOP_LEFT;
    TOP_RIGHT;
    glEnd();

    glBegin(GL_LINES);
    BOTTOM_LEFT;
    BOTTOM_RIGHT;
    glEnd();

    glBegin(GL_LINES);
    TOP_LEFT;
    BOTTOM_LEFT;
    glEnd();

    glBegin(GL_LINES);
    TOP_RIGHT;
    BOTTOM_RIGHT;
    glEnd();

    glColor3d(1.0, 0.0, 0.0);

    glPushMatrix();
    glTranslated(slider_offset_x, 0, 0);

    glBegin(GL_QUADS);
    glVertex3d(15, 50, 0);
    glVertex3d(35, 50, 0);
    glVertex3d(35, 0, 0);
    glVertex3d(15, 0, 0);
    glEnd();

    glPopMatrix();
}


void print_bitmap_string(void* font, char* s) {
    if (s && strlen(s)) {
        while (*s) {
            glutBitmapCharacter(font, *s);
            s++;
        }
    }
}


void draw_quit_button() {
#define QUIT_TOP_LEFT (glVertex3d(15, 50, 0))
#define QUIT_TOP_RIGHT (glVertex3d((width / 4) - 15, 50, 0))
#define QUIT_BOTTOM_LEFT (glVertex3d(15, 0, 0))
#define QUIT_BOTTOM_RIGHT (glVertex3d((width / 4) - 15, 0, 0))

    glBegin(GL_LINES);
    QUIT_TOP_LEFT;
    QUIT_TOP_RIGHT;
    glEnd();

    glBegin(GL_LINES);
    QUIT_BOTTOM_LEFT;
    QUIT_BOTTOM_RIGHT;
    glEnd();

    glBegin(GL_LINES);
    QUIT_TOP_LEFT;
    QUIT_BOTTOM_LEFT;
    glEnd();

    glBegin(GL_LINES);
    QUIT_TOP_RIGHT;
    QUIT_BOTTOM_RIGHT;
    glEnd();
}

/**
 * Draw x, y, and z axes centered around a vertex to the current viewport.
 */
void draw_axes() {
    // Z axis is blue.
    glColor3d(0.0, 0.0, 1.0);

    glBegin(GL_LINES);

    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, 0 -100);

    glEnd();

    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, 0 + 100);

    glEnd();

    // Y axis is green.
    glColor3d(0.0, 1.0, 0.0);

    glBegin(GL_LINES);

    glVertex3f(0, 0, 0);
    glVertex3f(0, 0 + 100, 0);

    glEnd();

    glBegin(GL_LINES);

    glVertex3f(0, 0, 0);
    glVertex3f(0, 0 -100, 0);

    glEnd();

    // X axis is red.
    glColor3d(1.0, 0.0, 0.0);

    glBegin(GL_LINES);

    glVertex3f(0, 0, 0);
    glVertex3f(0 + 100, 0, 0);

    glEnd();

    glBegin(GL_LINES);

    glVertex3f(0, 0, 0);
    glVertex3f(0 - 100, 0, 0);

    glEnd();
}


void render_volume() {
    glColor3d(1.0, 0.0, 0.0);
    glPointSize(1.0);

    glBegin(GL_POINTS);

    for (int i = 0; i < number_points; i++ ) {
        Point current_point = volumetric_points[i];
        glColor3d(current_point.intensity, current_point.intensity, current_point.intensity);
        glVertex3d(current_point.x, current_point.y, current_point.z);
    }

    glEnd();
}


/**
 * Given the dimensions for a viewport, setup the necessary perspective projections,
 * camera and render the axes and ply file.
 * @param x The x origin for the viewport.
 * @param y The y origin for the viewport.
 * @param width The width of the viewport.
 * @param height The height of the viewport.
 */
void draw_viewport(int x, int y, int width, int height) {
    // The top-right viewport, top view of model.
    glViewport(x, y, width, height);

    // Setup the perspective and look down the z-axis.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(120, width / height, 0.1, 250.0);
    gluLookAt(0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    glMatrixMode(GL_MODELVIEW);

    glPushMatrix();

    glRotated(x_rotation_angle, 1.0, 0.0, 0.0);
    glRotated(y_rotation_angle, 0.0, 1.0, 0.0);
    glRotated(z_rotation_angle, 0.0, 0.0, 1.0);
    glScaled(overall_scale_factor, overall_scale_factor, overall_scale_factor);

    draw_axes();

    glScaled((2.0 / volume_width), (2.0 / volume_height), (2.0 / volume_depth));
    glTranslatef(-(volume_width / 2), -(volume_height / 2), -(volume_depth / 2));

    // Render the axes and ply file.
    render_volume();

    glPopMatrix();
}


/**
 * Define the slider viewport in the bottom right corner and draw all the sliders in their current positions.
 */
void draw_slider_viewport() {
    glViewport(width * 3 / 4, // origin x
               height / 2, // origin y
               width / 4, // width
               height / 2); // height

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(width * 3 / 4,
            width,
            0,
            height / 2,
            -1,
            1);

    glMatrixMode(GL_MODELVIEW);

    glRasterPos2f((width * 7 / 8) - 25, (height / 2) - 25);
    print_bitmap_string(GLUT_BITMAP_TIMES_ROMAN_24, "Scale");

    glPushMatrix();

    glTranslated(width * 3 / 4, (height / 2) - 85, 0);
    draw_slider(slider4_offset);

    glPopMatrix();

    glColor3d(0.0, 0.0, 0.0);

    glRasterPos2f((width * 7 / 8) - 25, (height / 2) - 110);
    print_bitmap_string(GLUT_BITMAP_TIMES_ROMAN_24, "Rotate");

    glPushMatrix();

    glTranslated(width * 3 / 4, (height / 2) - 170, 0);
    draw_slider(slider1_offset);

    glPopMatrix();
    glPushMatrix();

    glTranslated(width * 3 / 4, (height / 2) - 230, 0);
    draw_slider(slider2_offset);

    glPopMatrix();
    glPushMatrix();

    glTranslated(width * 3 / 4, (height / 2) - 290, 0);
    draw_slider(slider3_offset);

    glPopMatrix();

    glColor3d(0.0, 0.0, 0.0);

    glPushMatrix();

    glTranslated(width * 3 / 4, (height / 2) - 350, 0);
    draw_quit_button();

    glPopMatrix();

    glRasterPos2f((width * 7 / 8) - 25, (height / 2) - 330);
    print_bitmap_string(GLUT_BITMAP_TIMES_ROMAN_24, "Quit");
}


/**
 * OpenGL draw function, draw a top, side and front viewport of the ply file to the screen with axes.
 */
void draw() {
    glShadeModel(GL_SMOOTH);

    glClearColor(1.0, 1.0, 1.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Rotate the model 90 degrees around the x-axis to view the top.
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Top-left viewport, top view.
    draw_viewport(0, 0, width * 3 / 4, height);
    draw_slider_viewport();

    // Done rendering, swap the buffers.
    glutSwapBuffers();
}


/**
 * OpenGL resize function, update our width and height and redraw ply file.
 * @param new_width The new window width.
 * @param new_height The new window height.
 */
void resize(int new_width, int new_height) {
    // Update our width and height values, and redraw models.
    width = new_width;
    height = new_height;
    glutPostRedisplay();
}


/**
 * Helper function to calculate the slider offset and rotation value given the world x coordinate of the slider.
 * @param world_x The x coordinate of the slider/mouse pointer.
 * @param slider_offset Pointer where the slider offset value should be stored.
 * @param rotation_angle Pointer where the slider rotation angle should be stored.
 * @post slider_offset will contain the slider offset relative to the world x coordinate.
 *       rotation_angle will contain the rotation angle relative to the world x coordinate.
 */
void calculate_rotation_offsets(double x, double* slider_offset, GLdouble* rotation_angle) {
    // All we need to do for th slider offset based on the world x coordinate is remove the padding.
    *slider_offset = x - (width * 3 / 4) - 15;

    if (*slider_offset >= (width / 4) - 50) {
        *slider_offset = (width / 4) - 50;
        *rotation_angle = 360;
    } else if (*slider_offset <= 0) {
        *slider_offset = 0;
        *rotation_angle = 0;
    } else {
        // The rotation angle is defined as 360 = 4.5 x, so divide evenly based on that.
        *rotation_angle = (*slider_offset * 360 * 4) / width;
    }
}


void calculate_scalar_offset(double x, double* slider_offset, GLdouble* scale_factor) {
    *slider_offset = x - (width * 3 / 4) - 15;

    if (*slider_offset >= (width / 4) - 50) {
        *slider_offset = (width / 4) - 50;
        *scale_factor = 5;
    } else if (*slider_offset <= 0) {
        *slider_offset = 0;
        *scale_factor = 1;
    } else {
        *scale_factor = ((*slider_offset * 5 * 4) / width) + 1;
    }
}


/**
 * Callback function registered with GLUT to be called whenever a mouse button is pressed or released.
 * @param button The mouse button that pressed, one of GLUT_LEFT_BUTTON, GLUT_RIGHT_BUTTON, OR GLUT_MIDDLE_BUTTON.
 * @param state The state of the button, either pressed or released (GLUT_DOWN, GLUT_UP respectively).
 * @param x The screen x coordinate of the pointer when the button was pressed.
 * @param y The screen y coordinate of the pointer when the button was pressed.
 * @post If button is not GLUT_LEFT_BUTTON, no change.
 *       If the pointer was not in the slider viewport and state is GLUT_UP, dragging will be 0.
 *       dragging will be 1 if the button was pressed, 0 otherwise.
 *       One of dragging_x, dragging_y, or dragging_z will be set to 1, based on matching the screen coordinate to
 *          one of the sliders in the viewport, the others will be 0.
 *       Based on the location of the pointer:
 *          slider1_offset, slider2_offset or slider3_offset will be set to the offset of the pointer within the slider.
 *          x_rotation_angle, y_rotation_angle or z_rotation_angle will be set to the rotation angle based on
 *              the relevant slider offset.
 *       The draw function will be called at the end of the glut event loop.
 */
void mouse_input(int button, int state, int x, int y) {
    if (button != GLUT_LEFT_BUTTON) {
        // We're only interested in left mouse button presses.
        return;
    } else if (x < (width * 3 / 4) || y > (height / 2)) {
        // We're not in the bottom-right viewport,
        // so the only thing that could be necessary is updating the dragging state.
        if (state != GLUT_UP) {
            // The button was pressed, but we're not within the viewport so nothing to do.
            // However, if we are already dragging, and the button was released,
            // even though the pointer isn't within the viewport, we still want to stop dragging.
            return;
        } else if (dragging == 0) {
            return;
        }
    }

    if (state == GLUT_UP) {
        // The button was released so stop dragging now.
        // We don't return here, because we still want to ensure
        // the offsets and angles get updated properly one last time.
        dragging = 0;
        return;
    } else {
        dragging = 1;
    }

    dragging_x = 0;
    dragging_y = 0;
    dragging_z = 0;
    dragging_scale = 0;

    double viewport_y = height - y - (height / 2);

    if (viewport_y >= (height / 2) - 85 && viewport_y <= (height / 2) - 25) {
        dragging_scale = 1;
        calculate_scalar_offset(x, &slider4_offset, &overall_scale_factor);
    } else if (viewport_y >= (height / 2) - 170 && viewport_y <= (height / 2) - 110) {
        dragging_x = 1;
        calculate_rotation_offsets(x, &slider1_offset, &x_rotation_angle);
    } else if (viewport_y >= (height / 2) - 230) {
        dragging_y = 1;
        calculate_rotation_offsets(x, &slider2_offset, &y_rotation_angle);
    } else if (viewport_y >= (height / 2) - 290) {
        dragging_z = 1;
        calculate_rotation_offsets(x, &slider3_offset, &z_rotation_angle);
    } else if (viewport_y >= (height / 2) - 350) {
        exit(0);
    }

    glutPostRedisplay();
}


void mouse_motion(int x, int y) {
    if (dragging == 0) {
        return;
    }

    if (dragging_x) {
        calculate_rotation_offsets(x, &slider1_offset, &x_rotation_angle);
    } else if (dragging_y) {
        calculate_rotation_offsets(x, &slider2_offset, &y_rotation_angle);
    } else if (dragging_z) {
        calculate_rotation_offsets(x, &slider3_offset, &z_rotation_angle);
    } else if (dragging_scale) {
        calculate_scalar_offset(x, &slider4_offset, &overall_scale_factor);
    }

    glutPostRedisplay();
}


void read_volume_file(char* filename) {
    ifstream volume_file(filename);
    char current_character;
    int character_number = 1;
    int slice_length = volume_width * volume_height;
    volumetric_points = new Point[slice_length * volume_depth];

    while (volume_file.get(current_character)) {
        Point point{};
        point.x = (character_number % slice_length) % volume_width;
        point.y = (character_number % slice_length) / volume_width;
        point.z = character_number / slice_length;
        point.intensity = (uint) current_character;
        volumetric_points[character_number - 1] = point;
        character_number++;
    }

    number_points = character_number - 1;
}


/**
 * Application entry point, parse command-line, load the ply file, setup the glut window, and enter event loop.
 * @param argc The number of command-line arguments.
 * @param argv The command-line arguments.
 * @return Return code.
 */
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
    glutMotionFunc(mouse_motion);

    glEnable(GL_DEPTH_TEST);

    glutMainLoop();

    return 0;
}
