/***********************************************************
 CSC418/2504, Fall 2011
 
 robot.cpp
 
 Simple demo program using OpenGL and the glut/glui
 libraries
 
 
 Instructions:
 Please read the assignment page to determine
 exactly what needs to be implemented.  Then read
 over this file and become acquainted with its
 design.
 
 Add source code where it appears appropriate. In
 particular, see lines marked 'TODO'.
 
 You should not need to change the overall structure
 of the program. However it should be clear what
 your changes do, and you should use sufficient comments
 to explain your code.  While the point of the assignment
 is to draw and animate the character, you will
 also be marked based on your design.
 
 Some windows-specific code remains in the includes;
 we are not maintaining windows build files this term,
 but we're leaving that here in case you want to try building
 on windows on your own.
 
 ***********************************************************/

#ifdef _WIN32
#include <windows.h>
#endif


//#include <GL/gl.h>
//#include <GL/glu.h>
//#include <GL/glut.h>
//#include <glui.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <GLUI/glui.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <iostream>


//#include "penguin.h"

#ifndef _WIN32
#include <unistd.h>
#else
void usleep(unsigned int nanosec)
{
    Sleep(nanosec / 1000);
}
#endif


typedef struct{
    float x;
    float y;
} Point;

// *************** GLOBAL VARIABLES *************************
// Joint parameters
// These constraints defined the min and max value of the rotating/scaling values.
const float JOINT_MIN = -45.0f;
const float JOINT_MAX =  45.0f;
const float LEG_ROTATE_MIN = -30.0f;
const float LEG_ROTATE_MAX =  30.0f;
const float FOOT_ROTATE_MIN = -17.0f;
const float FOOT_ROTATE_MAX = 13.0f;
const float ARM_ROTATE_MIN = -20.0f;
const float ARM_ROTATE_MAX = 20.0f;
const float HEAD_ROTATE_MIN = -15.0f;
const float HEAD_ROTATE_MAX = 15.0f;
const float HORIZONTAL_SPAN = 15.0f;
const float VERTICAL_SPAN = 15.0f;

float joint_rot = 0.0f;


const float PI = 3.14159;
const float DEG2RAD = 3.14159 / 180;

//These values are used to define the body of the penguin.
//Every parameter can be easily adjusted, without affecting the DOFs functionality.
//All parts of bodies are based on portion of these const values, so if want to make the penguin taller
// Just change the BODY_LENGTH value and no need to worry about the drawing part.
const float BODY_LENGTH = 11.0f;
const float GENERAL_SCALE = 15.0f;
const float NECK_WIDTH = 4.0f;
const float MIDDLEBODY_WIDTH = 8.0f;
const float BOTTOMBODY_WIDTH = 3.0f;

const float LEG_FACTOR = 20.0f;
const float LEG_WIDTH = 1.2f;
const float LEG_LENGTH = 4.0f;
const float FOOT_LENGTH = 3.0f;
const float FOOT_WIDTH = 1.0f;
const float ARMTOP_WIDTH = 2.0f;
const float ARMBOT_WIDTH = 1.5f;
const float ARM_LENGTH = 4.5f;
const float HEADTOP_WIDTH = 3.5f;
const float HEADBOT_WIDTH = 5.0f;
const float HEAD_LENGTH = 4.0f;
const float BEAK_WIDTH = 2.4f;
const float TOPBEAK_HEIGHT = 0.7f;
const float LOWBEAK_HEIGHT = 0.3f;

// These are the rotation values for each joints. These value changes once the animation start, based on sinusoidal
// function mostly. With some adjustments.
float ARM_SCALE_FACTOR = 1.0f;
float leftlegRotation = 0.0f;
float rightlegRotation = LEG_ROTATE_MIN;
float leftfootRotation = 0.0f;
float rightfootRotation = 0.0f;
float armRotation = 0.0f;
float headRotation = 0.0f;
float BeakPositionFactor = 0.0f;
float BodyPositionX = 0.0f;
float BodyPositionY = 0.0f;
float ReflectionX = 1.0f;
float ReflectionY=1.0f;

// --------------- USER INTERFACE VARIABLES -----------------

// Window settings
int windowID;               // Glut window ID (for display)
GLUI *glui;                 // Glui window (for controls)
int Win[2];                 // window (x,y) size


// ---------------- ANIMATION VARIABLES ---------------------

// Animation settings
//These are the animation flag values to take control of the button of GUI.

int animate_mode = 0;       // 0 = no anim, 1 = animate
int animation_frame = 0;      // Specify current frame of animation
int animate_LEFTLEG = 0;
int animate_RIGHTLEG = 0;
int animate_LEFTFOOT = 0;
int animate_RIGHTFOOT = 0;
int animate_ARM = 0;
int animate_ARMSCALE = 0;
int animate_HEAD = 0;
int animate_BEAK = 0;
int animate_HORIZONTAL = 0;
int animate_VERTICAL = 0;

int AUTO_ANIMATE = 0;



//////////////////////////////////////////////////////
// TODO: Add additional joint parameters here
//////////////////////////////////////////////////////



// ***********  FUNCTION HEADER DECLARATIONS ****************


// Initialization functions
void initGlut(char* winName);
void initGlui();
void initGl();


// Callbacks for handling events in glut
void myReshape(int w, int h);
void animate();
void AutoanimateButton(int);
void refresh();

void display(void);

// Callback for handling events in glui
void GLUI_Control(int id);


// Functions to help draw the object, part by part.
//drawPolygon and drawCicle are general functions that is used to draw any shape based on the given points.
void drawSquare(float size);

void drawPolygon(int n, Point points[]);
void drawPolygonFILL(int n, Point points[]);
void drawCircle(float radius);
void drawCircleFill(float radius);

void drawBeak();
void drawHead();
void drawArm();
void drawLeg();
void drawBody();
void drawFoot(float rotation);
void drawJointcircle();

// Return the current system clock (in seconds)
double getTime();


// ******************** FUNCTIONS ************************



// main() function
// Initializes the user interface (and any user variables)
// then hands over control to the event handler, which calls
// display() whenever the GL window needs to be redrawn.


int main(int argc, char** argv)
{
    
    // Process program arguments
    if(argc != 3) {
        printf("Usage: demo [width] [height]\n");
        printf("Using 300x200 window by default...\n");
        Win[0] = 600;
        Win[1] = 600;
    } else {
        Win[0] = atoi(argv[1]);
        Win[1] = atoi(argv[2]);
    }
    
    
    // Initialize glut, glui, and opengl
    glutInit(&argc, argv);
    initGlut(argv[0]);
    initGlui();
    initGl();
    
    // Invoke the standard GLUT main event loop
    glutMainLoop();
    
    return 0;         // never reached
}


// Initialize glut and create a window with the specified caption
void initGlut(char* winName)
{
    // Set video mode: double-buffered, color, depth-buffered
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    
    // Create window
    glutInitWindowPosition (0, 0);
    glutInitWindowSize(Win[0],Win[1]);
    windowID = glutCreateWindow(winName);
    
    // Setup callback functions to handle events
    glutReshapeFunc(myReshape); // Call myReshape whenever window resized
    glutDisplayFunc(display);   // Call display whenever new frame needed
}


// Quit button handler.  Called when the "quit" button is pressed.
void quitButton(int)
{
    exit(0);
}

// Animate button handler.  Called when the "animate" checkbox is pressed.
void animateButton(int)
{
    // synchronize variables that GLUT uses
    glui->sync_live();
    
    
    //Set the logic for the GUI button,
    //so that there won't be any conflict or asynchronization.
    
    animation_frame = 0;
    if((animate_mode == 1)
       ||(animate_LEFTLEG == 1)
       ||(animate_LEFTFOOT == 1)
       ||(animate_RIGHTLEG == 1)
       ||(animate_RIGHTFOOT == 1)
       ||(animate_ARM == 1)
       ||(animate_ARMSCALE == 1)
       ||(animate_HEAD == 1)
       ||(animate_BEAK == 1)
       ||(animate_HORIZONTAL == 1)
       ||(animate_VERTICAL == 1)) {
        // start animation
        if(((animate_mode == 1)
            &&(animate_LEFTLEG == 1)
            &&(animate_LEFTFOOT == 1)
            &&(animate_RIGHTLEG == 1)
            &&(animate_RIGHTFOOT == 1)
            &&(animate_ARM == 1)
            &&(animate_ARMSCALE == 1)
            &&(animate_HEAD == 1)
            &&(animate_BEAK == 1)
            &&(animate_HORIZONTAL == 1)
            &&(animate_VERTICAL == 1)) != 1)
            
            AUTO_ANIMATE = 0;
    
        ReflectionX = 1.0f;
        GLUI_Master.set_glutIdleFunc(animate);
    } else {
        // stop animation
        GLUI_Master.set_glutIdleFunc(NULL);
    }
}

void AutoanimateButton(int)
{
    // synchronize variables that GLUT uses
    glui->sync_live();
    
    
    //Set the logic for the GUI button,
    //so that there won't be any conflict or asynchronization.
    // This button is used to select all DOFs and move them at the same time.
    animation_frame = 0;
    if(AUTO_ANIMATE == 1) {
        animate_LEFTLEG = 1;
        animate_RIGHTLEG = 1;
        animate_LEFTFOOT = 1;
        animate_RIGHTFOOT = 1;
        animate_ARM = 1;
        animate_ARMSCALE = 1;
        animate_HEAD = 1;
        animate_BEAK = 1;
        animate_HORIZONTAL = 1;
        animate_VERTICAL = 1;
        
        ReflectionX = 1.0f;
        
        GLUI_Master.set_glutIdleFunc(animate);
    } else {
        animate_LEFTLEG = 0;
        animate_RIGHTLEG = 0;
        animate_LEFTFOOT = 0;
        animate_RIGHTFOOT = 0;
        animate_ARM = 0;
        animate_ARMSCALE = 0;
        animate_HEAD = 0;
        animate_BEAK = 0;
        animate_HORIZONTAL = 0;
        animate_VERTICAL = 0;
        GLUI_Master.set_glutIdleFunc(animate);
    }
}


// Initialize GLUI and the user interface
void initGlui()
{
    GLUI_Master.set_glutIdleFunc(NULL);
    
    // Create GLUI window
    glui = GLUI_Master.create_glui("Glui Window", 0, Win[0]+10, 0);
    
    ///////////////////////////////////////////////////////////
    // TODO:
    //   Add controls for additional joints here
    ///////////////////////////////////////////////////////////
    
    // Add button to specify animation mode.
    // Added 10 DOFs checkbox and 10 spinners to track the value of the joint/scale values.
    glui->add_separator();
    glui->add_checkbox("SelectAll (Auto Animation)", &AUTO_ANIMATE, 0, AutoanimateButton);
    
    glui->add_separator();
    glui->add_checkbox("Left Leg", &animate_LEFTLEG, 0, animateButton);
    GLUI_Spinner *leftlegspinner = glui->add_spinner("LeftLegToBodyJoint", GLUI_SPINNER_FLOAT, &leftlegRotation);
    leftlegspinner->set_speed(0.1);
    leftlegspinner->set_float_limits(LEG_ROTATE_MIN, LEG_ROTATE_MAX, GLUI_LIMIT_CLAMP);
    
    glui->add_separator();
    glui->add_checkbox("Left Foot", &animate_LEFTFOOT, 0, animateButton);
    GLUI_Spinner *leftfootspinner = glui->add_spinner("LeftFootToBodyJoint", GLUI_SPINNER_FLOAT, &leftfootRotation);
    leftfootspinner->set_speed(0.1);
    leftfootspinner->set_float_limits(FOOT_ROTATE_MIN, FOOT_ROTATE_MAX, GLUI_LIMIT_CLAMP);
    
    glui->add_separator();
    glui->add_checkbox("Right Leg", &animate_RIGHTLEG, 0, animateButton);
    GLUI_Spinner *rightlegspinner = glui->add_spinner("RightLegToBodyJoint", GLUI_SPINNER_FLOAT, &rightlegRotation);
    rightlegspinner->set_speed(0.1);
    rightlegspinner->set_float_limits(LEG_ROTATE_MIN, LEG_ROTATE_MAX, GLUI_LIMIT_CLAMP);
    
    glui->add_separator();
    glui->add_checkbox("Right Foot", &animate_RIGHTFOOT, 0, animateButton);
    GLUI_Spinner *rightfootspinner = glui->add_spinner("RightFootToBodyJoint", GLUI_SPINNER_FLOAT, &rightfootRotation);
    rightfootspinner->set_speed(0.1);
    rightfootspinner->set_float_limits(FOOT_ROTATE_MIN, FOOT_ROTATE_MAX, GLUI_LIMIT_CLAMP);
    
    glui->add_separator();
    glui->add_checkbox("Arm", &animate_ARM, 0, animateButton);
    GLUI_Spinner *armrotatespinner = glui->add_spinner("ArmToBodyJoint", GLUI_SPINNER_FLOAT, &armRotation);
    armrotatespinner->set_speed(0.1);
    armrotatespinner->set_float_limits(ARM_ROTATE_MIN, ARM_ROTATE_MAX, GLUI_LIMIT_CLAMP);
    
    glui->add_separator();
    glui->add_checkbox("Arm Scale", &animate_ARMSCALE, 0, animateButton);
    GLUI_Spinner *armscalespinner = glui->add_spinner("ArmScale", GLUI_SPINNER_FLOAT, &ARM_SCALE_FACTOR);
    armscalespinner->set_speed(0.1);
    armscalespinner->set_float_limits(0.8, 1.2, GLUI_LIMIT_CLAMP);
    
    glui->add_separator();
    glui->add_checkbox("Head", &animate_HEAD, 0, animateButton);
    GLUI_Spinner *headspinner = glui->add_spinner("HeadToBodyJoint", GLUI_SPINNER_FLOAT, &headRotation);
    headspinner->set_speed(0.1);
    headspinner->set_float_limits(HEAD_ROTATE_MIN, HEAD_ROTATE_MAX, GLUI_LIMIT_CLAMP);
    
    glui->add_separator();
    glui->add_checkbox("Beak", &animate_BEAK, 0, animateButton);
    GLUI_Spinner *beakspinner = glui->add_spinner("BeakTranslation", GLUI_SPINNER_FLOAT, &BeakPositionFactor);
    beakspinner->set_speed(0.1);
    beakspinner->set_float_limits(-3.5*LOWBEAK_HEIGHT, 0, GLUI_LIMIT_CLAMP);
    
    glui->add_separator();
    glui->add_checkbox("Move Horizontally", &animate_HORIZONTAL, 0, animateButton);
    GLUI_Spinner *horizontalsipnner = glui->add_spinner("MoveHorizontally", GLUI_SPINNER_FLOAT, &BodyPositionX);
    horizontalsipnner->set_speed(0.1);
    horizontalsipnner->set_float_limits(-HORIZONTAL_SPAN/2, HORIZONTAL_SPAN/2, GLUI_LIMIT_CLAMP);
    
    glui->add_separator();
    glui->add_checkbox("Move Vertically", &animate_VERTICAL, 0, animateButton);
    GLUI_Spinner *verticalspinner = glui->add_spinner("MoveVertically", GLUI_SPINNER_FLOAT, &BodyPositionY);
    verticalspinner->set_speed(0.1);
    verticalspinner->set_float_limits(-VERTICAL_SPAN/2, VERTICAL_SPAN/2, GLUI_LIMIT_CLAMP);
    
    
    // Add "Quit" button
    glui->add_separator();
    glui->add_button("Quit", 0, quitButton);
    
    // Set the main window to be the "active" window
    glui->set_main_gfx_window(windowID);
}


// Performs most of the OpenGL intialization
void initGl(void)
{
    // glClearColor (red, green, blue, alpha)
    // Ignore the meaning of the 'alpha' value for now
    glClearColor(0.7f,0.7f,0.9f,1.0f);
}


// Callback idle function for animating the scene
void animate()
{
    // Define the rotation speed of each joint.
    // Change these values can result the penguin to move faster/slower.
    const double leg_rotate_speed = 0.2;
    const double foot_rotate_speed = 0.2;
    const double arm_rotate_speed = 0.3;
    const double arm_scale_speed = 0.3;
    const double head_rotate_speed = 0.5;
    const double beak_translate_speed = 0.3;
    const double horizontal_speed = 0.03;
    const double vertical_speed = 0.03;
    
    //Factor that is used to help determine the rotation/scaling values.
    //Sinusoidal functions are used here mostly.
    
    double leftlegRotatefactor = (sin(animation_frame*leg_rotate_speed) + 1.0) / 2.0;
    
    double rightlegRotatefactor = (sin(animation_frame*leg_rotate_speed + 2.7) + 1.0) / 2.0;
    
    double leftfootRotatefactor = (cos(animation_frame*foot_rotate_speed) + 1.0) / 2.0;
    
    double rightfootRotatefactor = (cos(animation_frame*foot_rotate_speed + 2.7) + 1.0) / 2.0;
    
    double armRotateFactor = (sin(animation_frame*arm_rotate_speed) + 1.0) / 2.0;
    
    double armScale = (sin(animation_frame*arm_scale_speed) + 1.0) / 2.0;
    
    double headRotateFactor = (sin(animation_frame*head_rotate_speed) + 1.0) / 2.0;
    
    double beakTranslation = (-cos(animation_frame*beak_translate_speed) + 1.0) / 2.0;
    
    double bodyTranslationX = (-sin(animation_frame*horizontal_speed) + 1.0) / 2.0;
    
    double bodyTranslationY = (-sin(animation_frame*vertical_speed) + 1.0) / 2.0;
    
    //std::cout<<joint_rot_t<<std::endl;
    ///////////////////////////////////////////////////////////
    // TODO:
    //   Modify this function animate the character's joints
    //   Note: Nothing should be drawn in this function!  OpenGL drawing
    //   should only happen in the display() callback.
    ///////////////////////////////////////////////////////////
    
    
    
    // Each joint values change/not change depends on the current GUI button checkbox situation.
    // Basically the flag value reprensent the state of the checkbox,
    // whenever there is a checkbox checked, the value of that joint will be updated due to these IF statements.
    if(animate_LEFTLEG == 1)
        leftlegRotation = leftlegRotatefactor * LEG_ROTATE_MIN + (1 - leftlegRotatefactor) * LEG_ROTATE_MAX;
    
    if(animate_RIGHTLEG == 1)
        rightlegRotation = rightlegRotatefactor * LEG_ROTATE_MIN + (1 - rightlegRotatefactor) * LEG_ROTATE_MAX;
    
    if(animate_LEFTFOOT == 1)
        leftfootRotation = leftfootRotatefactor * FOOT_ROTATE_MIN + (1 - leftfootRotatefactor) * FOOT_ROTATE_MAX;
    
    if(animate_RIGHTFOOT == 1)
        rightfootRotation = rightfootRotatefactor * FOOT_ROTATE_MIN + (1 - rightfootRotatefactor) * FOOT_ROTATE_MAX;
    
    if(animate_ARM == 1)
        armRotation = armRotateFactor * ARM_ROTATE_MIN + (1 - armRotateFactor) * ARM_ROTATE_MAX;
    
    if(animate_HEAD == 1)
        headRotation = headRotateFactor * HEAD_ROTATE_MIN + (1 - headRotateFactor) * HEAD_ROTATE_MAX;
    
    if(animate_BEAK == 1)
        BeakPositionFactor = -(beakTranslation * 3.5 * LOWBEAK_HEIGHT);
    
    if(animate_ARMSCALE == 1)
        ARM_SCALE_FACTOR = armScale/2.5 + 0.8;
    
    if(animate_HORIZONTAL == 1)
        BodyPositionX = bodyTranslationX * 15 - 7.5;
    
    if(animate_VERTICAL == 1)
        BodyPositionY = bodyTranslationY*15 - 7.5;
    
    //Reflection factor so that the penguin will turn-around when it reaches the left end or the right end.
    if (BodyPositionX <= -7.45f )
        ReflectionX = -1.0;
    if(BodyPositionX >= 7.45f )
        ReflectionX = 1.0;
    
    // Update user interface
    glui->sync_live();
    
    // Tell glut window to update itself.  This will cause the display()
    // callback to be called, which renders the object (once you've written
    // the callback).
    glutSetWindow(windowID);
    glutPostRedisplay();
    
    // increment the frame number.
    animation_frame++;
    
    // Wait 50 ms between frames (20 frames per second)
    usleep(50000);
}

//Refresh function that is not used here.
// Can be helpful if have multiple animate function, since these save lots of time/lines of code.
void refresh(){
    // Update user interface
    glui->sync_live();
    
    // Tell glut window to update itself.  This will cause the display()
    // callback to be called, which renders the object (once you've written
    // the callback).
    glutSetWindow(windowID);
    glutPostRedisplay();
    
    // increment the frame number.
    animation_frame++;
    
    // Wait 50 ms between frames (20 frames per second)
    usleep(50000);
}

// Handles the window being resized by updating the viewport
// and projection matrices
void myReshape(int w, int h)
{
    // Setup projection matrix for new window
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-w/2, w/2, -h/2, h/2);
    
    // Update OpenGL viewport and internal variables
    glViewport(0,0, w,h);
    Win[0] = w;
    Win[1] = h;
}



// display callback
//
// This gets called by the event handler to draw
// the scene, so this is where you need to build
// your scene -- make your changes and additions here.
// All rendering happens in this function.  For Assignment 1,
// updates to geometry should happen in the "animate" function.
void display(void)
{
    // glClearColor (red, green, blue, alpha)
    // Ignore the meaning of the 'alpha' value for now
    glClearColor(0.7f,0.7f,0.9f,1.0f);
    
    // OK, now clear the screen with the background colour
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Setup the model-view transformation matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    ///////////////////////////////////////////////////////////
    // TODO:
    //   Modify this function draw the scene
    //   This should include function calls to pieces that
    //   apply the appropriate transformation matrice and
    //   render the individual body parts.
    ///////////////////////////////////////////////////////////
    
    // Draw the entire whole Body, and each of the sub-parts will be called in
    // the drawBody function to make the stucture clearly and set-up the right
    // system of frames.
    
    
    drawBody();
    
    
    // Execute any GL functions that are in the queue just to be safe
    glFlush();
    
    // Now, show the frame buffer that we just drew into.
    // (this prevents flickering).
    glutSwapBuffers();
    
}


// Draw the Body of the penguin.
// Inside this function, the other parts drawing function will be called because other parts
// are needed to move with respect to the body.
void drawBody(){
    glPushMatrix();
    
        // Scale the size of the penguin.
        glScalef(GENERAL_SCALE, GENERAL_SCALE, 1.0);
    
        //Move the penguin in X, Y direction. These two position values are
        //in a rance of VERTICAL_SPAN and HORIZONTAL_SPAN as defined above.
        glTranslatef(BodyPositionX, BodyPositionY, 0.0);
    
        //Reflect the penguin when it reached the left/right end. Based on the
        // current location.
        glScalef(ReflectionX, ReflectionY, 1.0);

        
        // Set the colour to green
        glColor3f(0.0, 1.0, 0.0);
        
        // Draw the body
        Point bodyPoints[] = {
            // top left, right points
            {-NECK_WIDTH/2, BODY_LENGTH/2}, {NECK_WIDTH/2, BODY_LENGTH/2},
            // middle right point
            {MIDDLEBODY_WIDTH/2, -BODY_LENGTH/4},
            // bottom right point
            {BOTTOMBODY_WIDTH/2, -BODY_LENGTH/2},
            // bottom left point
            {-BOTTOMBODY_WIDTH/2, -BODY_LENGTH/2},
            // middle left point
            {-MIDDLEBODY_WIDTH/2, -BODY_LENGTH/4}};
        drawPolygon(6, bodyPoints);
    
    
        // Draw other parts of the body in order.
        // These parts are also affected by the matrix defined in this function, as they should
        // move with the body together.
        drawLeg();
        drawArm();
        drawHead();
    
    glPopMatrix();
    
}

void drawHead() {
    glPushMatrix();
        glColor3f(1, 0.5, 0.5);
        Point HEADPoints[] = {
            // top point
            {-HEADTOP_WIDTH/3, HEAD_LENGTH*11/20},
            // middle right point
            {HEADTOP_WIDTH*2/5, HEAD_LENGTH*9/20},
            // bottom right point
            {HEADBOT_WIDTH/2, -HEAD_LENGTH*9/20},
            // bottom left point
            {-HEADBOT_WIDTH/2, -HEAD_LENGTH*9/20},
            // middle left point
            {-HEADTOP_WIDTH*3/5, HEAD_LENGTH*9/20}};
    
        //Set the rotation hinge point.
        glTranslatef(0.0, 0.48*BODY_LENGTH , 0.0);
        glRotatef(headRotation, 0.0, 0.0, 1.0);
        //  glRotatef(0.0, 0.0, 0.0, 1.0);
        
        //Set the location of where the head should be drawn.
        glTranslatef(0.0, 0.02*BODY_LENGTH + 0.3* HEAD_LENGTH, 0.0);
        drawPolygon(5, HEADPoints);
    
        //Call draw beak function as Beak should move with respect to the head-move.
        drawBeak();
        
        // Draw the eye of the penguin
        glPushMatrix();
            glTranslatef(-HEADTOP_WIDTH/3, 0.25*HEAD_LENGTH, 0.0);
            glColor3f(1, 1, 1);
            drawCircleFill(0.4);
            glTranslatef(-0.09, 0.0 , 0.0);
            glColor3f(0, 0, 0);
            drawCircleFill(0.3);
        glPopMatrix();
        
    glPopMatrix();
    
        // Draw Head to body joint circle.
    glPushMatrix();
        glColor3f(1, 1, 1);
        glTranslatef(0.0, 0.48*BODY_LENGTH, 0.0);
        drawCircle(0.2);
    glPopMatrix();
    
}

void drawBeak(){
    glPushMatrix();
        glTranslatef(-HEADTOP_WIDTH*3/5, 0.0, 0.0);
        glColor3f(1, 1, 0);
        Point TOPBEAKPoints[] = {
            // top left point
            {-BEAK_WIDTH/2, -TOPBEAK_HEIGHT/6},
            // top right point
            {BEAK_WIDTH/2, TOPBEAK_HEIGHT/2},
            // bottom right point
            {BEAK_WIDTH/2, -TOPBEAK_HEIGHT/2},
            // bottom left point
            {-BEAK_WIDTH/2, -TOPBEAK_HEIGHT/2}};
    
        //Draw top-beak, which has no animation on itself.
        glTranslatef(-BEAK_WIDTH*11/20, 0.0, 0.0);
        drawPolygon(4, TOPBEAKPoints);
        
        glColor3f(1, 0.5, 0);
        Point LOWBEAKPoints[] = {
            // top left point
            {-BEAK_WIDTH/2, LOWBEAK_HEIGHT/2},
            // top right point
            {BEAK_WIDTH/2, LOWBEAK_HEIGHT/2},
            // bottom right point
            {BEAK_WIDTH/2, -LOWBEAK_HEIGHT/2},
            // bottom left point
            {-BEAK_WIDTH/2, -LOWBEAK_HEIGHT/2}};
        
        // Translation on the lower beak to move up and down. Beak can move up/down depends
        // on the BeakPositionFactor value as changed in the animation automatically.
        glTranslatef(0.0, BeakPositionFactor, 0.0);
    
        //Draw the low beak.
        glTranslatef(0.0, -(TOPBEAK_HEIGHT + LOWBEAK_HEIGHT)/2, 0.0);
        drawPolygon(4, LOWBEAKPoints);
    
    
    glPopMatrix();
}

void drawArm() {
    glPushMatrix();
        glColor3f(0, 0, 1);
        Point ARMPoints[] = {
            // top left point
            {-ARMTOP_WIDTH/2, ARM_LENGTH/2},
            // top right point
            {ARMTOP_WIDTH/2, ARM_LENGTH/2},
            // bottom right point
            {ARMBOT_WIDTH/2, -ARM_LENGTH/2},
            // bottom left point
            {-ARMBOT_WIDTH/2, -ARM_LENGTH/2}};
    
        //Set the hinge point of the arm to rotate.
        glTranslatef(0.1*MIDDLEBODY_WIDTH, 0.3*BODY_LENGTH, 0.0);
        glRotatef(armRotation, 0.0, 0.0, 1.0);
    
        //Set the point to scale the arm.
        //This point should be set at the top edge of the arm.
        //Since we only want the arm to be scaled in the -Y direction.
        glTranslatef(0.0, 0.5*ARM_LENGTH-0.15*BODY_LENGTH, 0.0);
        glScalef(1.0, ARM_SCALE_FACTOR, 1.0);
    
        //Translate to where the arm should be drawn.
        glTranslatef(0.0, -0.5*ARM_LENGTH, 0.0);
        
        drawPolygon(4, ARMPoints);
    
    glPopMatrix();
    
    // Draw arm to body joint circle.
    
    glPushMatrix();
        glColor3f(1, 1, 1);
        glTranslatef(0.1*MIDDLEBODY_WIDTH, 0.3*BODY_LENGTH, 0.0);
        drawCircle(0.3);
    glPopMatrix();
}

void drawLeg() {
    glPushMatrix();

        glColor3f(1.0, 0.5, 0.5);
        Point LEGPoints[] = {
            // top left point
            {-LEG_WIDTH/2, LEG_LENGTH/2},
            // top right point
            {LEG_WIDTH/2, LEG_LENGTH/2},
            // bottom right point
            {LEG_WIDTH/2, -LEG_LENGTH/2},
            // bottom left point
            {-LEG_WIDTH/2, -LEG_LENGTH/2}};
        
        // Draw the right leg, inside the matrix call the drawFoot function to draw right foot.
        glPushMatrix();
            glTranslatef(MIDDLEBODY_WIDTH/4, -0.41*BODY_LENGTH, 0.0);
            glRotatef(rightlegRotation, 0.0, 0.0, 1.0);
            glTranslatef(0, -0.09*BODY_LENGTH, 0.0);
            drawPolygon(4, LEGPoints);
            
            drawFoot(rightfootRotation);
        
        glPopMatrix();
        // Draw the left leg, inside the matrix call the drawFoot function to draw left foot.
        glPushMatrix();
            glTranslatef(-MIDDLEBODY_WIDTH/4, -0.41*BODY_LENGTH, 0.0);
            glRotatef(leftlegRotation, 0.0, 0.0, 1.0);
            glTranslatef(0.0, -0.09*BODY_LENGTH, 0.0);
            glColor3f(1.0, 0.5, 0.5);
            drawPolygon(4, LEGPoints);
            drawFoot(leftfootRotation);
        glPopMatrix();
    glPopMatrix();
    
    //Draw leg to body joint circle
    glPushMatrix();
        glColor3f(1, 1, 1);
        glTranslatef(MIDDLEBODY_WIDTH/4, -0.41*BODY_LENGTH, 0.0);
        drawCircle(0.3);
        glTranslatef(-MIDDLEBODY_WIDTH/2, 0.0, 0.0);
        drawCircle(0.3);
    glPopMatrix();
}

//Function to draw the joint circle.
void drawJointcircle() {
    glPushMatrix();
    glColor3f(1, 1, 1);
    drawCircle(0.3);
    glPopMatrix();
}

//Function to draw the foot. Both left/right foot can be drawn using this function.
void drawFoot(float rotation) {
    glPushMatrix();
        Point FOOTPoints[] = {
            // top left point
            {-FOOT_LENGTH/2, FOOT_WIDTH/2},
            // top right point
            {FOOT_LENGTH/2, FOOT_WIDTH/2},
            // bottom right point
            {FOOT_LENGTH/2, -FOOT_WIDTH/2},
            // bottom left point
            {-FOOT_LENGTH/2, -FOOT_WIDTH/2}};
        glTranslatef(0.0, -0.4*LEG_LENGTH, 0.0);
        glRotatef(rotation, 0.0, 0.0, 1.0);
        glTranslatef(-0.3 * FOOT_LENGTH, 0.0, 0.0);
        glColor3f(1, 1, 0);
        drawPolygon(4, FOOTPoints);
    glPopMatrix();
    
    // Draw foot to leg joint circle.
    glPushMatrix();
        glColor3f(1, 1, 1);
        glTranslatef(0.0, -0.4*LEG_LENGTH, 0.0);
        drawCircle(0.3);
    glPopMatrix();
}


// Draw a square of the specified size, centered at the current location
void drawSquare(float width)
{
    // Draw the square
    glBegin(GL_POLYGON);
    glVertex2d(-width/2, -width/2);
    glVertex2d(width/2, -width/2);
    glVertex2d(width/2, width/2);
    glVertex2d(-width/2, width/2);
    glEnd();
}

// Draw a polygon with given points

void drawPolygonFILL(int n, Point points[]){
    glBegin(GL_POLYGON);
    for (int i = 0; i < n; i++){
        glVertex2f(points[i].x, points[i].y);
    }
    glEnd();
    
}

void drawPolygon(int n, Point points[]){
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < n; i++){
        glVertex2f(points[i].x, points[i].y);
    }
    glEnd();
    
}


// Draw a circle with given radius
void drawCircle(float radius){
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 360; i++){
        float degInRad = i * DEG2RAD;
        glVertex2f(cos(degInRad) * radius, sin(degInRad) * radius);
    }
    glEnd();
}

// Draw a circle filled with colors.
void drawCircleFill(float radius){
    glBegin(GL_POLYGON);
    for (int i = 0; i < 360; i++){
        float degInRad = i * DEG2RAD;
        glVertex2f(cos(degInRad) * radius, sin(degInRad) * radius);
    }
    glEnd();
}


































