#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifndef F_PI
#define F_PI		((float)(M_PI))
#define F_2_PI		((float)(2.f*F_PI))
#define F_PI_2		((float)(F_PI/2.f))
#endif



#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include "glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "glut.h"

// Maximum number of colors
#define MAX_COLORS 6
#define NUM_LEVELS 2


typedef struct Node {
    int id;
    float position[3]; // x, y, z coordinates
    int color;          // -1 for uncolored, 0 to MAX_COLORS-1 for colors
} Node;

typedef struct Edge {
    int from;
    int to;
} Edge;

typedef struct Graph {
    Node *nodes;
    int numNodes;
    Edge *edges;
    int numEdges;
} Graph;




//	This is a sample OpenGL / GLUT program
//
//	The objective is to draw a 3d object and change the color of the axes
//		with a glut menu
//
//	The left mouse button does rotation
//	The middle mouse button does scaling
//	The user interface allows:
//		1. The axes to be turned on and off
//		2. The color of the axes to be changed
//		3. Debugging to be turned on and off
//		4. Depth cueing to be turned on and off
//		5. The projection to be changed
//		6. The transformations to be reset
//		7. The program to quit
//
//	Author:			Joe Graphics

// title of these windows:

const char *WINDOWTITLE = "OpenGL / GLUT Sample -- Joe Graphics";
const char *GLUITITLE   = "User Interface Window";

// what the glui package defines as true and false:

const int GLUITRUE  = true;
const int GLUIFALSE = false;

// the escape key:

const int ESCAPE = 0x1b;

// initial window size:

const int INIT_WINDOW_SIZE = 600;

// size of the 3d box to be drawn:

const float BOXSIZE = 2.f;

// multiplication factors for input interaction:
//  (these are known from previous experience)

const float ANGFACT = 1.f;
const float SCLFACT = 0.005f;

// minimum allowable scale factor:

const float MINSCALE = 0.05f;

// scroll wheel button values:

const int SCROLL_WHEEL_UP   = 3;
const int SCROLL_WHEEL_DOWN = 4;

// equivalent mouse movement when we click the scroll wheel:

const float SCROLL_WHEEL_CLICK_FACTOR = 5.f;

// active mouse buttons (or them together):

const int LEFT   = 4;
const int MIDDLE = 2;
const int RIGHT  = 1;

// which projection:

enum Projections
{
	ORTHO,
	PERSP
};

// which button:

enum ButtonVals
{
	RESET,
	QUIT
};

// window background color (rgba):

const GLfloat BACKCOLOR[ ] = { 0., 0., 0., 1. };

// line width for the axes:

const GLfloat AXES_WIDTH   = 3.;

// the color numbers:
// this order must match the radio button order, which must match the order of the color names,
// 	which must match the order of the color RGB values

enum ColorEnum
{
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	MAGENTA
};

char * ColorNames[ ] =
{
	(char *)"Red",
	(char*)"Yellow",
	(char*)"Green",
	(char*)"Cyan",
	(char*)"Blue",
	(char*)"Magenta"
};

// the color definitions:
// this order must match the menu order

const GLfloat Colors[ ][3] = 
{
	{ 1., 0., 0. },		// red
	{ 1., 1., 0. },		// yellow
	{ 0., 1., 0. },		// green
	{ 0., 1., 1. },		// cyan
	{ 0., 0., 1. },		// blue
	{ 1., 0., 1. },		// magenta
};

// fog parameters:

const GLfloat FOGCOLOR[4] = { .0f, .0f, .0f, 1.f };
const GLenum  FOGMODE     = GL_LINEAR;
const GLfloat FOGDENSITY  = 0.30f;
const GLfloat FOGSTART    = 1.5f;
const GLfloat FOGEND      = 4.f;

// for lighting:

const float	WHITE[ ] = { 1.,1.,1.,1. };

// for animation:

const int MS_PER_CYCLE = 10000;		// 10000 milliseconds = 10 seconds


// what options should we compile-in?
// in general, you don't need to worry about these
// i compile these in to show class examples of things going wrong
//#define DEMO_Z_FIGHTING
//#define DEMO_DEPTH_BUFFER


// non-constant global variables:

int		ActiveButton;			// current button that is down
GLuint	AxesList;				// list to hold the axes
int		AxesOn;					// != 0 means to draw the axes
GLuint	BoxList;				// object display list
int		DebugOn;				// != 0 means to print debugging info
int		DepthCueOn;				// != 0 means to use intensity depth cueing
int		DepthBufferOn;			// != 0 means to use the z-buffer
int		DepthFightingOn;		// != 0 means to force the creation of z-fighting
int		MainWindow;				// window id for main graphics window
int		NowColor;				// index into Colors[ ]
int		NowProjection;		// ORTHO or PERSP
float	Scale;					// scaling factor
int		ShadowsOn;				// != 0 means to turn shadows on
float	Time;					// used for animation, this has a value between 0. and 1.
int		Xmouse, Ymouse;			// mouse values
float	Xrot, Yrot;				// rotation angles in degrees
GLuint  sphereList;
int 	selectedNode = -1; // -1 indicates no selection
float 	Tx = 0.0f, Ty = 0.0f;
// Graph Levels
Graph   levels[NUM_LEVELS];
int     currentLevel = 0;
// Scoring Variables
int 	score = 0;
int 	moves = 0;
int     CurrentColor = -1;  
// Transition variables
float transitionTime = 0.0f;       // Time counter for transition
bool inTransition = false;         // Whether we're in transition
bool edgesVisible = true;          // Whether to draw edges
Node startPositions[5];           // Store starting positions
Node endPositions[5];             // Store end positions
const float TRANSITION_DURATION = 2.0f;  // Seconds for transition
float CameraY = 0.0f;
float StartCameraY = 0.0f;
float EndCameraY = -1.5f;
bool gameCompleted = false;
const char* VICTORY_TEXT = "3D Color Graph Game";
GLuint textDisplayList;
const char* CREDITS_TEXT = "By Guillermo Morales";
// function prototypes:

void	Animate( );
void	Display( );
void	DoAxesMenu( int );
void	DoColorMenu( int );
void	DoDepthBufferMenu( int );
void	DoDepthFightingMenu( int );
void	DoDepthMenu( int );
void	DoDebugMenu( int );
void	DoMainMenu( int );
void	DoProjectMenu( int );
void	DoRasterString( float, float, float, char * );
void	DoStrokeString( float, float, float, float, char * );
float	ElapsedSeconds( );
void	InitGraphics( );
void	InitLists( );
void	InitMenus( );
void	Keyboard( unsigned char, int, int );
void	MouseButton( int, int, int, int );
void	MouseMotion( int, int );
void	Reset( );
void	Resize( int, int );
void	Visibility( int );
void 	provideFeedback();
void			Axes( float );
void			HsvRgb( float[3], float [3] );
void			Cross(float[3], float[3], float[3]);
float			Dot(float [3], float [3]);
float			Unit(float [3], float [3]);
float			Unit(float [3]);


// utility to create an array from 3 separate values:

float *
Array3( float a, float b, float c )
{
	static float array[4];

	array[0] = a;
	array[1] = b;
	array[2] = c;
	array[3] = 1.;
	return array;
}

// utility to create an array from a multiplier and an array:

float *
MulArray3( float factor, float array0[ ] )
{
	static float array[4];

	array[0] = factor * array0[0];
	array[1] = factor * array0[1];
	array[2] = factor * array0[2];
	array[3] = 1.;
	return array;
}


float *
MulArray3(float factor, float a, float b, float c )
{
	static float array[4];

	float* abc = Array3(a, b, c);
	array[0] = factor * abc[0];
	array[1] = factor * abc[1];
	array[2] = factor * abc[2];
	array[3] = 1.;
	return array;
}

// these are here for when you need them -- just uncomment the ones you need:

//#include "setmaterial.cpp"
//#include "setlight.cpp"
//#include "osusphere.cpp"
//#include "osucone.cpp"
//#include "osutorus.cpp"
//#include "bmptotexture.cpp"
//#include "loadobjfile.cpp"
#include "keytime.cpp"
//#include "glslprogram.cpp"

Keytimes xPos[5];  // Array for each node's x position
Keytimes yPos[5];  // Array for each node's y position
Keytimes zPos[5];  // Array for each node's z position

// Function to initialize Level 1 (Square)
Graph createLevel1() {
    Graph g;
    g.numNodes = 4;
    g.nodes = (Node *)malloc(g.numNodes * sizeof(Node));
    if (!g.nodes) {
        fprintf(stderr, "Memory allocation failed for nodes\n");
        exit(EXIT_FAILURE);
    }

    // Define node positions
    g.nodes[0] = (Node){0, {-1.0f, -1.0f, 0.0f}, -1};
    g.nodes[1] = (Node){1, {1.0f, -1.0f, 0.0f}, -1};
    g.nodes[2] = (Node){2, {1.0f, 1.0f, 0.0f}, -1};
    g.nodes[3] = (Node){3, {-1.0f, 1.0f, 0.0f}, -1};

    g.numEdges = 4;
    g.edges = (Edge *)malloc(g.numEdges * sizeof(Edge));
    if (!g.edges) {
        fprintf(stderr, "Memory allocation failed for edges\n");
        exit(EXIT_FAILURE);
    }

    // Define edges (forming a square)
    g.edges[0] = (Edge){0, 1};
    g.edges[1] = (Edge){1, 2};
    g.edges[2] = (Edge){2, 3};
    g.edges[3] = (Edge){3, 0};

    return g;
}

// Function to initialize Level 2 with distinct 3D positions
   Graph createLevel2() {
    // Initialize Level 1 structure first
    Graph g = createLevel1();
	

    // Add an additional node (Center)
    g.numNodes = 5;
    g.nodes = (Node *)realloc(g.nodes, g.numNodes * sizeof(Node));
    if (!g.nodes) {
        fprintf(stderr, "Memory reallocation failed for nodes\n");
        exit(EXIT_FAILURE);
    }

    // Assign new positions with distinct z-values
    g.nodes[0] = (Node){0, {-0.8f, -0.8f, 0.2f}, -1}; // Slightly inward and elevated
    g.nodes[1] = (Node){1, {0.8f, -0.6f, 0.4f}, -1};  // Slightly shifted and elevated
    g.nodes[2] = (Node){2, {0.6f, 0.8f, 0.6f}, -1};   // Slightly shifted and elevated
    g.nodes[3] = (Node){3, {-0.6f, 0.6f, 0.8f}, -1};  // Slightly shifted and elevated

    // Add the center node with highest elevation
    g.nodes[4] = (Node){4, {0.0f, 0.0f, 1.0f}, -1};

    // Update edges
    g.numEdges = 8; // Increased number of edges for enhanced connectivity
    g.edges = (Edge *)realloc(g.edges, g.numEdges * sizeof(Edge));
    if (!g.edges) {
        fprintf(stderr, "Memory reallocation failed for edges\n");
        exit(EXIT_FAILURE);
    }

    // Retain existing edges from Level 1
    g.edges[0] = (Edge){0, 1};
    g.edges[1] = (Edge){1, 2};
    g.edges[2] = (Edge){2, 3};
    g.edges[3] = (Edge){3, 0};

    // Add new edges connecting the center node to all other nodes
    g.edges[4] = (Edge){0, 4};
    g.edges[5] = (Edge){1, 4};
    g.edges[6] = (Edge){2, 4};
    g.edges[7] = (Edge){3, 4};

    return g;
}




void calculateScore() {
    Graph currentGraph = levels[currentLevel];
    
    // Count how many different colors were used
    int usedColors[MAX_COLORS] = {0};  // Track which colors are used
    int numColorsUsed = 0;
    
    for(int i = 0; i < currentGraph.numNodes; i++) {
        if(currentGraph.nodes[i].color >= 0 && usedColors[currentGraph.nodes[i].color] == 0) {
            usedColors[currentGraph.nodes[i].color] = 1;
            numColorsUsed++;
        }
    }
    
    // Base score: 100 points per level
    int baseScore = 100;
    
    // Penalty for moves
    int penalties = (moves * 2);
    
    // Bonus for using fewer colors
    // For level 1 (square): optimal is 2 colors
    // For level 2 (square with center): optimal is 3 colors
    int optimalColors;
    if(currentLevel == 0) {
        optimalColors = 2;  // Square can be colored with 2 colors
    } else if(currentLevel == 1) {
        optimalColors = 3;  // Square with center needs 3 colors
    } else {
        optimalColors = 3;  // Default for other levels
    }
    
    // Bonus points for being close to optimal coloring
    int colorBonus = 50 * (MAX_COLORS - numColorsUsed);
    if(numColorsUsed == optimalColors) {
        colorBonus += 100;  // Extra bonus for achieving optimal coloring
    }
    
    // Calculate final score
    score += baseScore + colorBonus - penalties;
    if(score < 0) score = 0;
    
    printf("Level %d scoring:\n", currentLevel + 1);
    printf("Base score: %d\n", baseScore);
    printf("Colors used: %d (optimal: %d)\n", numColorsUsed, optimalColors);
    printf("Color bonus: %d\n", colorBonus);
    printf("Penalties: %d\n", penalties);
    printf("Final score for level: %d\n", baseScore + colorBonus - penalties);
}
int isValidColoring(Graph graph) {
    // First check if all nodes are colored
    for(int i = 0; i < graph.numNodes; i++) {
        if(graph.nodes[i].color == -1) {
            return 0; // Not all nodes are colored yet
        }
    }
    
    // Then check if any adjacent nodes share colors
    for(int i = 0; i < graph.numEdges; i++) {
        int from = graph.edges[i].from;
        int to = graph.edges[i].to;
        if(graph.nodes[from].color == graph.nodes[to].color) {
            return 0; // Invalid coloring
        }
    }
    return 1; // Valid coloring - all nodes colored and no conflicts
}
void provideFeedback() {
    Graph currentGraph = levels[currentLevel];
    int allColored = 1;
    int validColoring = 1;
    
    // Check if all nodes are colored
    for(int i = 0; i < currentGraph.numNodes; i++) {
        printf("Node %d color: %d\n", i, currentGraph.nodes[i].color);
        if(currentGraph.nodes[i].color == -1) {
            allColored = 0;
            printf("Not all nodes are colored yet\n");
            break;
        }
    }
    
    // Check for adjacent nodes with same color
    if(allColored) {
        for(int i = 0; i < currentGraph.numEdges; i++) {
            int from = currentGraph.edges[i].from;
            int to = currentGraph.edges[i].to;
            printf("Checking edge %d-%d: colors %d-%d\n", 
                   from, to, 
                   currentGraph.nodes[from].color, 
                   currentGraph.nodes[to].color);
            if(currentGraph.nodes[from].color == currentGraph.nodes[to].color) {
                validColoring = 0;
                printf("Invalid coloring: nodes %d and %d share color %d\n", 
                       from, to, currentGraph.nodes[from].color);
                break;
            }
        }
    }

    if(allColored && validColoring) {
        calculateScore();
        printf("Level %d Completed! Score: %d\n", currentLevel + 1, score);
        
        if(currentLevel < NUM_LEVELS - 1) {
            // Initialize keyframes for each node
            Graph nextGraph = levels[currentLevel + 1];
            
            for(int i = 0; i < nextGraph.numNodes; i++) {
                xPos[i].Init();
                yPos[i].Init();
                zPos[i].Init();
                
                // Current position at time 0
                xPos[i].AddTimeValue(0.0, currentGraph.nodes[i].position[0]);
                yPos[i].AddTimeValue(0.0, currentGraph.nodes[i].position[1]);
                zPos[i].AddTimeValue(0.0, currentGraph.nodes[i].position[2]);
                
                // Target position at time 1
                xPos[i].AddTimeValue(1.0, nextGraph.nodes[i].position[0]);
                yPos[i].AddTimeValue(1.0, nextGraph.nodes[i].position[1]);
                zPos[i].AddTimeValue(1.0, nextGraph.nodes[i].position[2]);
            }
            
            // Start transition
            inTransition = true;
            transitionTime = 0.0f;
            edgesVisible = false;
            CameraY = StartCameraY;  // Reset camera Y to starting position
            
            printf("Starting transition to next level\n");
        } else {
            // Game completion
            printf("Congratulations! Final Score: %d\n", score);
            gameCompleted = true;  // Set game completed flag
            glutPostRedisplay();
        }
        glutPostRedisplay();
    }
}

// Function to initialize all levels
void initializeLevels() {
    levels[0] = createLevel1();
    levels[1] = createLevel2();
}

void drawNode(Node node) {
    glPushMatrix();
    glTranslatef(node.position[0], node.position[1], node.position[2]);
    
    // Disable lighting while drawing colored nodes
    glDisable(GL_LIGHTING);
    
    // Add material properties to make colors more visible
    float mat_ambient[] = {0.2f, 0.2f, 0.2f, 1.0f};
    float mat_diffuse[4];
    float mat_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    float mat_shininess[] = {50.0f};
    
    if(node.color >= 0 && node.color < MAX_COLORS) {
        // Use the color from the Colors array
        mat_diffuse[0] = Colors[node.color][0];
        mat_diffuse[1] = Colors[node.color][1];
        mat_diffuse[2] = Colors[node.color][2];
        mat_diffuse[3] = 1.0f;
        glColor3fv(Colors[node.color]);
    } else if(node.id == selectedNode) {
        // gray for selected
        glColor3f(1.0f, 1.0f, 0.0f);
    } else {
        // White for uncolored
        glColor3f(1.0f, 1.0f, 1.0f);
    }
    
    // Draw the sphere
    glutSolidSphere(0.1, 20, 20);
    
    // Re-enable lighting
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

void drawEdge(Edge edge, Graph graph) {
    Node from = graph.nodes[edge.from];
    Node to = graph.nodes[edge.to];
    
    glDisable(GL_LIGHTING);  // Disable lighting for lines
    glLineWidth(3.0);        // Make lines thicker
    
    // Check if connected nodes have the same color
    if(from.color != -1 && to.color != -1 && from.color == to.color) {
        glColor3f(1.0f, 0.0f, 0.0f);  // Bright red for conflicts
    } else {
        glColor3f(1.0f, 1.0f, 1.0f);  // Bright white for normal edges
    }

    glBegin(GL_LINES);
        glVertex3f(from.position[0], from.position[1], from.position[2]);
        glVertex3f(to.position[0], to.position[1], to.position[2]);
    glEnd();
    
    glEnable(GL_LIGHTING);  // Re-enable lighting for other rendering
}


void drawText(const char *text, float x, float y, float z) {
    glRasterPos3f(x, y, z);
    for(const char *c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
}

// Function to perform picking
void pickNode(int x, int y) {
    GLuint selectBuf[512];
    GLint hits;
    GLint viewport[4];

    glGetIntegerv(GL_VIEWPORT, viewport);

    glSelectBuffer(512, selectBuf);
    glRenderMode(GL_SELECT);

    glInitNames();
    glPushName(0);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
        glLoadIdentity();
        gluPickMatrix((GLdouble)x, (GLdouble)(viewport[3] - y), 5.0, 5.0, viewport);
        gluPerspective(70.f, 1.f, 0.1f, 1000.f);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // Use the same camera setup as in Display()
        float currentCameraY;
        if(inTransition) {
            float t = transitionTime / TRANSITION_DURATION;
            if(t > 1.0f) t = 1.0f;
            currentCameraY = StartCameraY + (EndCameraY - StartCameraY) * t;
        } else {
            currentCameraY = CameraY;
        }

        // Match the Display function's camera setup exactly
        gluLookAt(0.0f, currentCameraY, 3.0f,     // eye position
                  0.f, 0.f, 0.f,                   // look-at point
                  0.f, 1.f, 0.f);                  // up vector

        // Apply the same transformations as in Display
        glRotatef((GLfloat)Yrot, 0.f, 1.f, 0.f);
        glRotatef((GLfloat)Xrot, 1.f, 0.f, 0.f);
        glScalef((GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale);

        // Draw nodes with unique names
        Graph currentGraph = levels[currentLevel];
        for(int i = 0; i < currentGraph.numNodes; i++) {
            glLoadName(currentGraph.nodes[i].id);
            glPushMatrix();
                if(inTransition) {
                    float t = transitionTime / TRANSITION_DURATION;
                    if(t > 1.0f) t = 1.0f;
                    float x = xPos[i].GetValue(t);
                    float y = yPos[i].GetValue(t);
                    float z = zPos[i].GetValue(t);
                    glTranslatef(x, y, z);
                } else {
                    glTranslatef(currentGraph.nodes[i].position[0], 
                               currentGraph.nodes[i].position[1], 
                               currentGraph.nodes[i].position[2]);
                }
                glutSolidSphere(0.1, 20, 20);
            glPopMatrix();
        }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    hits = glRenderMode(GL_RENDER);
    if(hits > 0) {
        GLuint *ptr = selectBuf;
        GLuint minZ = 0xFFFFFFFF;
        int selected = -1;
        for(int i = 0; i < hits; i++) {
            int numNames = ptr[0];
            GLuint z1 = ptr[1];
            for(int j = 0; j < numNames; j++) {
                if(z1 < minZ) {
                    minZ = z1;
                    selected = ptr[3 + j];
                }
            }
            ptr += 3 + ptr[0];
        }
        selectedNode = selected;

    } else {
        selectedNode = -1;
    }

    glutPostRedisplay();
}

void cleanup() {
    for(int i = 0; i < NUM_LEVELS; i++) {
        free(levels[i].nodes);
        free(levels[i].edges);
    }
}




// main program:

int
main( int argc, char *argv[ ] )
{
	// turn on the glut package:
	// (do this before checking argc and argv since glutInit might
	// pull some command line arguments out)

	glutInit( &argc, argv );

	// setup all the graphics stuff:

	InitGraphics( );

	// create the display lists that **will not change**:

	InitLists( );

	// init all the global variables used by Display( ):
	// this will also post a redisplay

	Reset( );

	// setup all the user interface stuff:

	InitMenus( );

	atexit(cleanup); // Register cleanup function


	// draw the scene once and wait for some interaction:
	// (this will never return)

	glutSetWindow( MainWindow );
	glutMainLoop( );

	// glutMainLoop( ) never actually returns
	// the following line is here to make the compiler happy:

	return 0;
}


// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutPostRedisplay( ) do it

void
Animate( )
{
	// put animation stuff in here -- change some global variables for Display( ) to find:

	int ms = glutGet(GLUT_ELAPSED_TIME);
	ms %= MS_PER_CYCLE;							// makes the value of ms between 0 and MS_PER_CYCLE-1
	Time = (float)ms / (float)MS_PER_CYCLE;		// makes the value of Time between 0. and slightly less than 1.

	if(inTransition) {
        transitionTime += 1.0f/60.0f;  
        
        if(transitionTime >= TRANSITION_DURATION) {
            // Transition complete
            inTransition = false;
            edgesVisible = true;
            currentLevel++;  // Now advance to next level
			CameraY = EndCameraY;  // Keep camera at end position

            printf("Transition complete, moving to level %d\n", currentLevel);
            
            // Reset node colors for new level
            for(int i = 0; i < levels[currentLevel].numNodes; i++) {
                levels[currentLevel].nodes[i].color = -1;
            }
        }
    }

	// for example, if you wanted to spin an object in Display( ), you might call: glRotatef( 360.f*Time,   0., 1., 0. );

	// force a call to Display( ) next time it is convenient:

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// draw the complete scene:

void
Display( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting Display.\n");

	// set which window we want to do the graphics into:
	glutSetWindow( MainWindow );

	// erase the background:
	glDrawBuffer( GL_BACK );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glEnable( GL_DEPTH_TEST );
#ifdef DEMO_DEPTH_BUFFER
	if( DepthBufferOn == 0 )
		glDisable( GL_DEPTH_TEST );
#endif


	// specify shading to be flat:

	glShadeModel( GL_FLAT );

	// set the viewport to be a square centered in the window:

	GLsizei vx = glutGet( GLUT_WINDOW_WIDTH );
	GLsizei vy = glutGet( GLUT_WINDOW_HEIGHT );
	GLsizei v = vx < vy ? vx : vy;			// minimum dimension
	GLint xl = ( vx - v ) / 2;
	GLint yb = ( vy - v ) / 2;
	glViewport( xl, yb,  v, v );


	// set the viewing volume:
	// remember that the Z clipping  values are given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	if( NowProjection == ORTHO )
		glOrtho( -2.f, 2.f,     -2.f, 2.f,     0.1f, 1000.f );
	else
		gluPerspective( 70.f, 1.f,	0.1f, 1000.f );

	// place the objects into the scene:

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );

	// Check for game completion before regular rendering
    if(gameCompleted) {
        // Position the camera for the victory text
        gluLookAt(0.0f, 0.0f, 2.5f,     // eye position
                  0.f, 0.f, 0.f,         // look-at point
                  0.f, 1.f, 0.f);        // up vector

        glDisable(GL_LIGHTING);
        
   
        
        glPushMatrix();
            // Position and rotate the text
            glTranslatef(-1.3f, 0.0f, 0.0f);  // Center text

            
            // Set text color (gold)
            glColor3f(1.0f, 0.843f, 0.0f);
            
            // Draw the text
            glScalef(0.0013f, 0.0013f, 0.0013f);  // Adjust size
            for(const char* c = VICTORY_TEXT; *c != '\0'; c++) {
                glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, *c);
            }
        glPopMatrix();
		
		glPushMatrix();
        // Position and draw the credits text
        glTranslatef(-0.7f, -0.2f, 0.0f);  // Positioned below victory text
        
        // Set text color (lighter gold)
        glColor3f(1.0f, 0.933f, 0.5f);  // Lighter shade of gold
        
        // Draw credits with smaller scale
        glScalef(0.001f, 0.001f, 0.001f);  
        for(const char* c = CREDITS_TEXT; *c != '\0'; c++) {
            glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
        }
    glPopMatrix();
    } else {
  
	float currentCameraY;
    if(inTransition) {
        // Calculate normalized time (0 to 1)
        float t = transitionTime / TRANSITION_DURATION;
        if(t > 1.0f) t = 1.0f;
        
        // Interpolate camera Y position
        currentCameraY = StartCameraY + (EndCameraY - StartCameraY) * t;
    } else {
        currentCameraY = CameraY;  // Use the actual CameraY when not transitioning
    }

    gluLookAt(0.0f, currentCameraY, 3.0f,     // eye position (y changes)
              0.f, 0.f, 0.f,                   // look-at point
              0.f, 1.f, 0.f);                  // up vector


	// rotate the scene:

	glRotatef( (GLfloat)Yrot, 0.f, 1.f, 0.f );
	glRotatef( (GLfloat)Xrot, 1.f, 0.f, 0.f );

	// uniformly scale the scene:

	if( Scale < MINSCALE )
		Scale = MINSCALE;
	glScalef( (GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale );

	// set the fog parameters:

	if( DepthCueOn != 0 )
	{
		glFogi( GL_FOG_MODE, FOGMODE );
		glFogfv( GL_FOG_COLOR, FOGCOLOR );
		glFogf( GL_FOG_DENSITY, FOGDENSITY );
		glFogf( GL_FOG_START, FOGSTART );
		glFogf( GL_FOG_END, FOGEND );
		glEnable( GL_FOG );
	}
	else
	{
		glDisable( GL_FOG );
	}

	

	// Inside Display() function, before drawing nodes and edges
glEnable(GL_LIGHTING);
glEnable(GL_LIGHT0);

// Set up light position
GLfloat light_position[] = {1.0f, 1.0f, 1.0f, 0.0f};
glLightfv(GL_LIGHT0, GL_POSITION, light_position);

// Set up light properties
GLfloat white_light[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat ambient_light[] = {0.2f, 0.2f, 0.2f, 1.0f};
glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_light);
	// possibly draw the axes:

	// if( AxesOn != 0 )
	// {
	// 	glColor3fv( &Colors[NowColor][0] );
	// 	glCallList( AxesList );
	// }

	// since we are using glScalef( ), be sure the normals get unitized:

	glEnable( GL_NORMALIZE );


	// draw the box object by calling up its display list:

	//glCallList( BoxList );

	//glCallList( sphereList);



    // Retrieve the current graph
    Graph currentGraph = levels[currentLevel];
	//printf("Drawing %d nodes in level %d\n", currentGraph.numNodes, currentLevel);
	 glDisable(GL_LIGHTING);

    // Draw edges first
    for(int i = 0; i < currentGraph.numEdges; i++) {
        drawEdge(currentGraph.edges[i], currentGraph);
    }

     // Draw nodes
    // Draw nodes
	for(int i = 0; i < currentGraph.numNodes; i++) {
    Node node = currentGraph.nodes[i];
    glPushMatrix();
        if(inTransition) {
            // Calculate normalized time (0 to 1)
            float t = transitionTime / TRANSITION_DURATION;
            if(t > 1.0f) t = 1.0f;
            
            // Get interpolated positions
            float x = xPos[i].GetValue(t);
            float y = yPos[i].GetValue(t);
            float z = zPos[i].GetValue(t);
            glTranslatef(x, y, z);
        } else {
            glTranslatef(node.position[0], node.position[1], node.position[2]);
        }
        
        // Set color based on node state
        if(node.color >= 0 && node.color < MAX_COLORS) {
            glColor3fv(Colors[node.color]);
        } else if(node.id == selectedNode) {
            glColor3f(0.3f, 0.3f, 0.3f);  
        } else {
            glColor3f(1.0f, 1.0f, 1.0f);  // White for uncolored
        }
        
        glCallList(sphereList);
    glPopMatrix();
}

	// Overlay text (Level and Score)
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, 100, 0, 100);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
        glLoadIdentity();

        // Disable lighting and depth testing for overlay
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);

        // Set text color
        glColor3f(1.0f, 1.0f, 1.0f);

        // Display level and score
        char info[50];
        sprintf(info, "Level: %d", currentLevel + 1);
        drawText(info, 5.0f, 95.0f, 0.0f);
        sprintf(info, "Score: %d", score);
        drawText(info, 5.0f, 90.0f, 0.0f);

        // Re-enable lighting and depth testing
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
    	glPopMatrix();
    	glMatrixMode(GL_PROJECTION);
   		glPopMatrix();
    	glMatrixMode(GL_MODELVIEW);

	}
#ifdef DEMO_Z_FIGHTING
	if( DepthFightingOn != 0 )
	{
		glPushMatrix( );
			glRotatef( 90.f,   0.f, 1.f, 0.f );
			glCallList( BoxList );
		glPopMatrix( );
	}
#endif


	// draw some gratuitous text that just rotates on top of the scene:
	// i commented out the actual text-drawing calls -- put them back in if you have a use for them
	// a good use for thefirst one might be to have your name on the screen
	// a good use for the second one might be to have vertex numbers on the screen alongside each vertex

	glDisable( GL_DEPTH_TEST );
	glColor3f( 0.f, 1.f, 1.f );
	//DoRasterString( 0.f, 1.f, 0.f, (char *)"Text That Moves" );


	// draw some gratuitous text that is fixed on the screen:
	//
	// the projection matrix is reset to define a scene whose
	// world coordinate system goes from 0-100 in each axis
	//
	// this is called "percent units", and is just a convenience
	//
	// the modelview matrix is reset to identity as we don't
	// want to transform these coordinates

	glDisable( GL_DEPTH_TEST );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	gluOrtho2D( 0.f, 100.f,     0.f, 100.f );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );
	glColor3f( 1.f, 1.f, 1.f );
	//DoRasterString( 5.f, 5.f, 0.f, (char *)"Text That Doesn't" );

	// swap the double-buffered framebuffers:

	glutSwapBuffers( );

	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !

	glFlush( );
}


void
DoAxesMenu( int id )
{
	AxesOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoColorMenu( int id )
{
	NowColor = id - RED;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDebugMenu( int id )
{
	DebugOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthBufferMenu( int id )
{
	DepthBufferOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthFightingMenu( int id )
{
	DepthFightingOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthMenu( int id )
{
	DepthCueOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// main menu callback:

void
DoMainMenu( int id )
{
	switch( id )
	{
		case RESET:
			Reset( );
			break;

		case QUIT:
			// gracefully close out the graphics:
			// gracefully close the graphics window:
			// gracefully exit the program:
			glutSetWindow( MainWindow );
			glFinish( );
			glutDestroyWindow( MainWindow );
			exit( 0 );
			break;

		default:
			fprintf( stderr, "Don't know what to do with Main Menu ID %d\n", id );
	}

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoProjectMenu( int id )
{
	NowProjection = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// use glut to display a string of characters using a raster font:

void
DoRasterString( float x, float y, float z, char *s )
{
	glRasterPos3f( (GLfloat)x, (GLfloat)y, (GLfloat)z );

	char c;			// one character to print
	for( ; ( c = *s ) != '\0'; s++ )
	{
		glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, c );
	}
}


// use glut to display a string of characters using a stroke font:

void
DoStrokeString( float x, float y, float z, float ht, char *s )
{
	glPushMatrix( );
		glTranslatef( (GLfloat)x, (GLfloat)y, (GLfloat)z );
		float sf = ht / ( 119.05f + 33.33f );
		glScalef( (GLfloat)sf, (GLfloat)sf, (GLfloat)sf );
		char c;			// one character to print
		for( ; ( c = *s ) != '\0'; s++ )
		{
			glutStrokeCharacter( GLUT_STROKE_ROMAN, c );
		}
	glPopMatrix( );
}


// return the number of seconds since the start of the program:

float
ElapsedSeconds( )
{
	// get # of milliseconds since the start of the program:

	int ms = glutGet( GLUT_ELAPSED_TIME );

	// convert it to seconds:

	return (float)ms / 1000.f;
}


// initialize the glui window:

void
InitMenus( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitMenus.\n");

	glutSetWindow( MainWindow );

	int axesmenu = glutCreateMenu( DoAxesMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthcuemenu = glutCreateMenu( DoDepthMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthbuffermenu = glutCreateMenu( DoDepthBufferMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthfightingmenu = glutCreateMenu( DoDepthFightingMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int debugmenu = glutCreateMenu( DoDebugMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int projmenu = glutCreateMenu( DoProjectMenu );
	glutAddMenuEntry( "Orthographic",  ORTHO );
	glutAddMenuEntry( "Perspective",   PERSP );

	int mainmenu = glutCreateMenu( DoMainMenu );
	glutAddSubMenu(   "Axes",          axesmenu);

#ifdef DEMO_DEPTH_BUFFER
	glutAddSubMenu(   "Depth Buffer",  depthbuffermenu);
#endif

#ifdef DEMO_Z_FIGHTING
	glutAddSubMenu(   "Depth Fighting",depthfightingmenu);
#endif

	glutAddSubMenu(   "Depth Cue",     depthcuemenu);
	glutAddSubMenu(   "Projection",    projmenu );
	glutAddMenuEntry( "Reset",         RESET );
	glutAddSubMenu(   "Debug",         debugmenu);
	glutAddMenuEntry( "Quit",          QUIT );

// attach the pop-up menu to the right mouse button:

	glutAttachMenu( GLUT_RIGHT_BUTTON );

}



// initialize the glut and OpenGL libraries:
//	also setup callback functions

void
InitGraphics( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitGraphics.\n");

	// request the display modes:
	// ask for red-green-blue-alpha color, double-buffering, and z-buffering:

	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );

	// set the initial window configuration:

	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize( INIT_WINDOW_SIZE, INIT_WINDOW_SIZE );

	// open the window and set its title:

	MainWindow = glutCreateWindow( WINDOWTITLE );
	glutSetWindowTitle( WINDOWTITLE );

	// set the framebuffer clear values:

	glClearColor( BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3] );

	glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	// setup the callback functions:
	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on

	glutSetWindow( MainWindow );
	glutDisplayFunc( Display );
	glutReshapeFunc( Resize );
	glutKeyboardFunc( Keyboard );
	glutMouseFunc( MouseButton );
	glutMotionFunc( MouseMotion );
	glutPassiveMotionFunc(MouseMotion);
	//glutPassiveMotionFunc( NULL );
	glutVisibilityFunc( Visibility );
	glutEntryFunc( NULL );
	glutSpecialFunc( NULL );
	glutSpaceballMotionFunc( NULL );
	glutSpaceballRotateFunc( NULL );
	glutSpaceballButtonFunc( NULL );
	glutButtonBoxFunc( NULL );
	glutDialsFunc( NULL );
	glutTabletMotionFunc( NULL );
	glutTabletButtonFunc( NULL );
	glutMenuStateFunc( NULL );
	glutTimerFunc( -1, NULL, 0 );

	// setup glut to call Animate( ) every time it has
	// 	nothing it needs to respond to (which is most of the time)
	// we don't need to do this for this program, and really should set the argument to NULL
	// but, this sets us up nicely for doing animation

	glutIdleFunc( Animate );

	// init the glew package (a window must be open to do this):

#ifdef WIN32
	GLenum err = glewInit( );
	if( err != GLEW_OK )
	{
		fprintf( stderr, "glewInit Error\n" );
	}
	else
		fprintf( stderr, "GLEW initialized OK\n" );
	fprintf( stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

	// all other setups go here, such as GLSLProgram and KeyTime setups:


}


// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void
InitLists( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitLists.\n");

	float dx = BOXSIZE / 2.f;
	float dy = BOXSIZE / 2.f;
	float dz = BOXSIZE / 2.f;
	glutSetWindow( MainWindow );

	// create the object:

	BoxList = glGenLists( 1 );
	glNewList( BoxList, GL_COMPILE );

		glBegin( GL_QUADS );

			glColor3f( 1., 0., 0. );

				glNormal3f( 1., 0., 0. );
					glVertex3f(  dx, -dy,  dz );
					glVertex3f(  dx, -dy, -dz );
					glVertex3f(  dx,  dy, -dz );
					glVertex3f(  dx,  dy,  dz );

				glNormal3f(-1., 0., 0.);
					glVertex3f( -dx, -dy,  dz);
					glVertex3f( -dx,  dy,  dz );
					glVertex3f( -dx,  dy, -dz );
					glVertex3f( -dx, -dy, -dz );

			glColor3f( 0., 1., 0. );

				glNormal3f(0., 1., 0.);
					glVertex3f( -dx,  dy,  dz );
					glVertex3f(  dx,  dy,  dz );
					glVertex3f(  dx,  dy, -dz );
					glVertex3f( -dx,  dy, -dz );

				glNormal3f(0., -1., 0.);
					glVertex3f( -dx, -dy,  dz);
					glVertex3f( -dx, -dy, -dz );
					glVertex3f(  dx, -dy, -dz );
					glVertex3f(  dx, -dy,  dz );

			glColor3f(0., 0., 1.);

				glNormal3f(0., 0., 1.);
					glVertex3f(-dx, -dy, dz);
					glVertex3f( dx, -dy, dz);
					glVertex3f( dx,  dy, dz);
					glVertex3f(-dx,  dy, dz);

				glNormal3f(0., 0., -1.);
					glVertex3f(-dx, -dy, -dz);
					glVertex3f(-dx,  dy, -dz);
					glVertex3f( dx,  dy, -dz);
					glVertex3f( dx, -dy, -dz);

		glEnd( );

	glEndList( );
	
	sphereList = glGenLists(1);
    glNewList(sphereList, GL_COMPILE);
		glutSolidSphere(0.1, 20, 20);
    glEndList( );

	textDisplayList = glGenLists(1);
    glNewList(textDisplayList, GL_COMPILE);
        glPushMatrix();
            // Scale and position the text
            glScalef(0.002f, 0.002f, 0.002f);  // Scale to reasonable size
            glTranslatef(-1500.0f, 0.0f, 0.0f);  // Center the text
            
            // Draw each character of the text
            for(const char* c = VICTORY_TEXT; *c != '\0'; c++) {
                glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
            }
        glPopMatrix();
    glEndList();	


	// create the axes:
	AxesList = glGenLists( 1 );
	glNewList( AxesList, GL_COMPILE );
		glLineWidth( AXES_WIDTH );
			Axes( 1.5 );
		glLineWidth( 1. );
	glEndList( );
}


// the keyboard callback:

void
Keyboard( unsigned char c, int x, int y )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Keyboard: '%c' (0x%0x)\n", c, c );

	switch( c )
	{

		case 'n':
        case 'N':
            // Reset everything to starting state
            currentLevel = 0;
            score = 0;
            moves = 0;
            selectedNode = -1;
            // Reset colors for all nodes in all levels
            for(int l = 0; l < NUM_LEVELS; l++) {
                for(int n = 0; n < levels[l].numNodes; n++) {
                    levels[l].nodes[n].color = -1;
                }
            }
            Reset();
            break;

		case 'r':
        case 'R':
            if(selectedNode != -1) {
                levels[currentLevel].nodes[selectedNode].color = RED;
				moves++;
        		provideFeedback(); 
                glutPostRedisplay();
            }
            break;

        case 'y':
        case 'Y':
            if(selectedNode != -1) {
                levels[currentLevel].nodes[selectedNode].color = YELLOW;
				moves++;
        		provideFeedback(); 
                glutPostRedisplay();
            }
            break;

        case 'g':
        case 'G':
            if(selectedNode != -1) {
                levels[currentLevel].nodes[selectedNode].color = GREEN;
				moves++;
        		provideFeedback(); 
                glutPostRedisplay();
            }
            break;

        case 'c':
        case 'C':
            if(selectedNode != -1) {
                levels[currentLevel].nodes[selectedNode].color = CYAN;
				moves++;
        		provideFeedback(); 
                glutPostRedisplay();
            }
            break;

        case 'b':
        case 'B':
            if(selectedNode != -1) {
                levels[currentLevel].nodes[selectedNode].color = BLUE;
				moves++;
        		provideFeedback(); 
                glutPostRedisplay();
            }
            break;

        case 'm':
        case 'M':
            if(selectedNode != -1) {
                levels[currentLevel].nodes[selectedNode].color = MAGENTA;
				moves++;
        		provideFeedback(); 
                glutPostRedisplay();
            }
            break;
		case 'o':
		case 'O':
			NowProjection = ORTHO;
			break;

		case 'p':
		case 'P':
			NowProjection = PERSP;
			break;

		case 'q':
		case 'Q':
		case ESCAPE:
			DoMainMenu( QUIT );	// will not return here
			break;				// happy compiler

		default:
			fprintf( stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c );
	}

	// force a call to Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// called when the mouse button transitions down or up:

void
MouseButton( int button, int state, int x, int y )
{
    // Only handle left button for node selection
    if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        pickNode(x, y);
    }

    // Do not handle other buttons or scroll wheel
    // This effectively disables rotation, scaling, and any other mouse-based manipulations

    glutSetWindow(MainWindow);
    glutPostRedisplay();
}



// called when the mouse moves while a button is down:

void
MouseMotion( int x, int y )
{
	int dx = x - Xmouse;		// change in mouse coords
	int dy = y - Ymouse;

	if( ( ActiveButton & LEFT ) != 0 )
	{
		Xrot += ( ANGFACT*dy );
		Yrot += ( ANGFACT*dx );
	}

	if( ( ActiveButton & MIDDLE ) != 0 )
	{
		Scale += SCLFACT * (float) ( dx - dy );

		// keep object from turning inside-out or disappearing:

		if( Scale < MINSCALE )
			Scale = MINSCALE;
	}

	Xmouse = x;			// new current position
	Ymouse = y;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene

void
Reset( )
{
	ActiveButton = 0;
	AxesOn = 1;
	DebugOn = 0;
	DepthBufferOn = 1;
	DepthFightingOn = 0;
	DepthCueOn = 0;
	Scale  = 1.0;
	ShadowsOn = 0;
	NowColor = YELLOW;
	NowProjection = PERSP;
	Xrot = Yrot = 0.;
	CameraY = StartCameraY;  // Reset camera Y position
	gameCompleted = false;
    currentLevel = 0;
    score = 0;
    moves = 0;
    


	initializeLevels();

	// Add some debug output
    printf("Reset called, initialized %d nodes in level %d\n", 
           levels[currentLevel].numNodes, currentLevel);
}


// called when user resizes the window:

void
Resize( int width, int height )
{
	// don't really need to do anything since window size is
	// checked each time in Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// handle a change to the window's visibility:

void
Visibility ( int state )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Visibility: %d\n", state );

	if( state == GLUT_VISIBLE )
	{
		glutSetWindow( MainWindow );
		glutPostRedisplay( );
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}



///////////////////////////////////////   HANDY UTILITIES:  //////////////////////////


// the stroke characters 'X' 'Y' 'Z' :

static float xx[ ] = { 0.f, 1.f, 0.f, 1.f };

static float xy[ ] = { -.5f, .5f, .5f, -.5f };

static int xorder[ ] = { 1, 2, -3, 4 };

static float yx[ ] = { 0.f, 0.f, -.5f, .5f };

static float yy[ ] = { 0.f, .6f, 1.f, 1.f };

static int yorder[ ] = { 1, 2, 3, -2, 4 };

static float zx[ ] = { 1.f, 0.f, 1.f, 0.f, .25f, .75f };

static float zy[ ] = { .5f, .5f, -.5f, -.5f, 0.f, 0.f };

static int zorder[ ] = { 1, 2, 3, 4, -5, 6 };

// fraction of the length to use as height of the characters:
const float LENFRAC = 0.10f;

// fraction of length to use as start location of the characters:
const float BASEFRAC = 1.10f;

//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)

void
Axes( float length )
{
	glBegin( GL_LINE_STRIP );
		glVertex3f( length, 0., 0. );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., length, 0. );
	glEnd( );
	glBegin( GL_LINE_STRIP );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., 0., length );
	glEnd( );

	float fact = LENFRAC * length;
	float base = BASEFRAC * length;

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 4; i++ )
		{
			int j = xorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( base + fact*xx[j], fact*xy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 5; i++ )
		{
			int j = yorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( fact*yx[j], base + fact*yy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 6; i++ )
		{
			int j = zorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( 0.0, fact*zy[j], base + fact*zx[j] );
		}
	glEnd( );

}


// function to convert HSV to RGB
// 0.  <=  s, v, r, g, b  <=  1.
// 0.  <= h  <=  360.
// when this returns, call:
//		glColor3fv( rgb );

void
HsvRgb( float hsv[3], float rgb[3] )
{
	// guarantee valid input:

	float h = hsv[0] / 60.f;
	while( h >= 6. )	h -= 6.;
	while( h <  0. ) 	h += 6.;

	float s = hsv[1];
	if( s < 0. )
		s = 0.;
	if( s > 1. )
		s = 1.;

	float v = hsv[2];
	if( v < 0. )
		v = 0.;
	if( v > 1. )
		v = 1.;

	// if sat==0, then is a gray:

	if( s == 0.0 )
	{
		rgb[0] = rgb[1] = rgb[2] = v;
		return;
	}

	// get an rgb from the hue itself:
	
	float i = (float)floor( h );
	float f = h - i;
	float p = v * ( 1.f - s );
	float q = v * ( 1.f - s*f );
	float t = v * ( 1.f - ( s * (1.f-f) ) );

	float r=0., g=0., b=0.;			// red, green, blue
	switch( (int) i )
	{
		case 0:
			r = v;	g = t;	b = p;
			break;
	
		case 1:
			r = q;	g = v;	b = p;
			break;
	
		case 2:
			r = p;	g = v;	b = t;
			break;
	
		case 3:
			r = p;	g = q;	b = v;
			break;
	
		case 4:
			r = t;	g = p;	b = v;
			break;
	
		case 5:
			r = v;	g = p;	b = q;
			break;
	}


	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
}

void
Cross(float v1[3], float v2[3], float vout[3])
{
	float tmp[3];
	tmp[0] = v1[1] * v2[2] - v2[1] * v1[2];
	tmp[1] = v2[0] * v1[2] - v1[0] * v2[2];
	tmp[2] = v1[0] * v2[1] - v2[0] * v1[1];
	vout[0] = tmp[0];
	vout[1] = tmp[1];
	vout[2] = tmp[2];
}

float
Dot(float v1[3], float v2[3])
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}


float
Unit(float vin[3], float vout[3])
{
	float dist = vin[0] * vin[0] + vin[1] * vin[1] + vin[2] * vin[2];
	if (dist > 0.0)
	{
		dist = sqrtf(dist);
		vout[0] = vin[0] / dist;
		vout[1] = vin[1] / dist;
		vout[2] = vin[2] / dist;
	}
	else
	{
		vout[0] = vin[0];
		vout[1] = vin[1];
		vout[2] = vin[2];
	}
	return dist;
}


float
Unit( float v[3] )
{
	float dist = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	if (dist > 0.0)
	{
		dist = sqrtf(dist);
		v[0] /= dist;
		v[1] /= dist;
		v[2] /= dist;
	}
	return dist;
}
