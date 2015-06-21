#include "LUtil.h"

bool initGL()
{
    //Initialize Projection Matrix
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
	glOrtho(0, MAP_WIDTH, MAP_HEIGHT, 0, 1.0, -1.0);

    //Initialize Modelview Matrix
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    //Save the default modelview matrix
    glPushMatrix();

    //Initialize clear color
    glClearColor(0, 0, 0, 0.f);

    //Check for error
    GLenum error = glGetError();
    if( error != GL_NO_ERROR )
    {
        printf( "Error initializing OpenGL! %s\n", gluErrorString( error ) );
        return false;
    }

    return true;
}


void render()
{
    //Clear color buffer
    glClear( GL_COLOR_BUFFER_BIT );

    //Take saved matrix off the stack and reset it
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();
    glLoadIdentity();

    //Render Test square
    /*glColor3f(1, 0.5, 0);
    glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(0, 100);
        glVertex2f(100, 100);
        glVertex2f(100, 0);
    glEnd();*/

	renderGame();

    //Save default matrix again
    glPushMatrix();

    //Move to center of the screen
    glTranslatef(0.f, 0.f, 0.f );

    glutSwapBuffers();
}

void handleKeys( unsigned char key, int x, int y )
{
    if(key == 27) //Escape
    {
		close();
        exit(0);
    }
    else if(key == ' ')
    {
        glutFullScreenToggle();
    }
	else if(key == 'p')
	{
		system("PAUSE");
	}
	else if(key == 'i')
	{
		close();
		init();
	}
	else if(key == 'c')
	{
		outputPlayerColorCodes();
	}
}

void handleKeysUp(unsigned char key, int x, int y)
{

}

void handleSpecialKeys(int key, int x, int y)
{

}

void handleSpecialKeysUp(int key, int x, int y)
{

}

void handleMouse(int button, int state, int x, int y)
{
    
}

void handlePassiveMouseMotion(int x, int y)
{

}

void handleMouseMotion(int x, int y)
{
    
}

void drawCircle(float cx, float cy, float r, int num_segments, int begin_type)
{
	float theta = 2 * 3.1415926 / float(num_segments);
	float tangetial_factor = tan(theta);//calculate the tangential factor

	float radial_factor = cos(theta);//calculate the radial factor

	float x = r;//we start at angle = 0

	float y = 0;

	glBegin(begin_type);
	for(int ii = 0; ii < num_segments; ii++)
	{
		glVertex2f(x + cx, y + cy);//output vertex

		//calculate the tangential vector
		//remember, the radial vector is (x, y)
		//to get the tangential vector we flip those coordinates and negate one of them

		float tx = -y;
		float ty = x;

		//add the tangential vector

		x += tx * tangetial_factor;
		y += ty * tangetial_factor;

		//correct using the radial factor

		x *= radial_factor;
		y *= radial_factor;
	}
	glEnd();
}

void textToScreen(int x, int y, float r, float g, float b, void *font, std::string str)
{
    glColor3f( r, g, b );
    glRasterPos2f(x, y);
    int len, i;
    len = str.size();
    for (i = 0; i < len; i++)
    {
        glutBitmapCharacter(font, str[i]);
    }
}