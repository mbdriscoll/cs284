#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <gl/glut.h>

#define MAX_MSG_LEN 64

GLvoid *font_style = GLUT_BITMAP_TIMES_ROMAN_24;

//-------------------------------------------------------------------------
//  Draws a string at the specified coordinates.
//  from: http://mycodelog.com/2010/03/23/printw/
//-------------------------------------------------------------------------
void printw (float x, float y, float z, char* format, ...) {

    va_list args;   //  Variable argument list
    int i;          //  Iterator
    char * text;    // Text

    //  Initialize a variable argument list
    va_start(args, format);

    //  Allocate memory for a string of the specified size
    text = (char*) malloc(MAX_MSG_LEN * sizeof(char));

    //  Write formatted output using a pointer to the list of arguments
    vsprintf(text, format, args);

    //  End using variable argument list
    va_end(args);

    //  Specify the raster position for pixel operations.
    glRasterPos3f (x, y, z);

    //  Draw the characters one by one
    for (i = 0; text[i] != '\0'; i++)
    glutBitmapCharacter(font_style, text[i]);

    //  Free the allocated memory for the string
    free(text);
}
