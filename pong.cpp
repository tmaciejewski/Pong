#include <GL/glut.h>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <unistd.h>

#define PI 3.141592

enum GameStates {STATE_GAME, STATE_AFTER};

GLfloat screenWidth = 640.0, screenHeight = 480.0;
GameStates gameState = STATE_GAME;

class Object
{
    public:

        GLfloat x, y, w, h;
        GLfloat speed;

        Object() {  }
        virtual ~Object() { }

        virtual void setColor()
        {
            glColor3f(1.0, 1.0, 1.0);
        }

        virtual void display()
        {
            setColor();
            glBegin(GL_QUADS);
            glVertex2f(x , y);
            glVertex2f(x + w, y);
            glVertex2f(x + w, y + h);
            glVertex2f(x, y + h);
            glEnd();
        }

        virtual void update() = 0;

        virtual void reset() = 0;

        virtual bool isCollision(float x1, float y1, float x2, float y2)
        {
            GLfloat nearest_x = (fabs(x - x1) < fabs(x + w - x1) ? x : x + w);

            if (x < x1 && x + w > x1 && y < y1 && y + h > y1)
                return false;

            if (x < x2 && x + w > x2 && y < y2 && y + h > y2)
                return true;

            if (y2 > y && y2 < y+h)
                if ((nearest_x - x1)*(nearest_x - x2) < 0)
                    return true;

            return false;
        }
};

class Player : public Object
{
    public:

        GLfloat boost;

        Player(GLfloat _x)
        {
            x = _x;
            reset();
            boost = 3.0;
            w = 20;
            h = 100;
            y = screenHeight / 2 - h / 2;
        }

        void reset()
        {
            speed = 0.0;
        }

        void update()
        {
            GLfloat ds = 0.0, decel = 0.1;

            if (speed < decel && speed > -decel)
                ds = -speed;
            else
                ds = (speed > 0 ? -decel : decel);

            y += speed;
            speed += ds;

            if (y > screenHeight - h)
            {
                y = screenHeight - h;
                speed = 0.0;
            }

            if (y < 0)
            {
                y = 0;
                speed = 0.0;
            }
        }

        void up()
        {
            speed += boost;
        }

        void down()
        {
            speed -= boost;
        }

} p1(0), p2(300);

class Ball : public Object
{
    public:

        GLfloat speed, angle;
        unsigned returns;

        Ball()
        {
            srand(time(0));
            reset();
            w = 8;
            h = 8;
        }

        void reset()
        {
            x = int(screenWidth / 2);
            y = rand() % int(screenHeight / 2) + (screenHeight / 4);
            speed = 3.0;
            angle = (rand() / float(RAND_MAX)) * (PI/2) + (PI/4);
            returns = 0;
        }

        void setColor()
        {
            glColor3f(1.0, 0.0, 0.0);
        }

        void update()
        {
            float dx = speed * sin(angle), dy = speed * cos(angle);

            if (p1.isCollision(x, y, x + dx, y + dy) || p2.isCollision(x, y, x + dx, y + dy))
            {
                angle = -angle + rand() / float(RAND_MAX) * PI/8 - PI/16;
                ++returns;
                if (returns % 2 == 0)
                {
                    speed *= 1.1;
                }
            }
            else
            {
                x += dx;
                y += dy;

                if (x < 0 || x > screenWidth)
                {
                    gameState = STATE_AFTER;
                }
            }

            if (y < 0)
            {
                y = 0;
                angle = PI - angle;
            }

            if (y > screenHeight)
            {
                y = screenHeight;
                angle = PI - angle;
            }
        }

        void display()
        {
            setColor();
            glPushMatrix();
            glTranslatef(x, y, 0.0);
            glutSolidSphere(5, 10, 10);
            glPopMatrix();
        }

} ball;

void update()
{
    if (gameState == STATE_GAME)
    {
        ball.update();
    }
    p1.update();
    p2.update();
}

void idle()
{
    update();
    glutPostRedisplay();
    usleep(10000);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    p1.display();
    p2.display();

    // half-way line
    glBegin(GL_QUADS);
    glVertex2f(screenWidth/2 - 1 , 0);
    glVertex2f(screenWidth/2 + 1, 0);
    glVertex2f(screenWidth/2 + 1, screenHeight);
    glVertex2f(screenWidth/2 - 1, screenHeight);
    glEnd();

    ball.display();
    glutSwapBuffers();
}

void reshape(int w, int h)
{
    screenHeight = h;
    screenWidth = w;
    p2.x = w - p2.w;

    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, (GLdouble) w, 0.0, (GLdouble) h, -10, 10);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 27:
        exit(0);
        break;
    case 'a':
        p1.up();
        glutPostRedisplay();
        break;
    case 'z':
        p1.down();
        glutPostRedisplay();
        break;
    case 'm':
        p2.down();
        glutPostRedisplay();
        break;
    case 'k':
        p2.up();
        glutPostRedisplay();
        break;
    case 'r':
        if (gameState == STATE_AFTER)
        {
            ball.reset();
            p1.reset();
            p2.reset();
            gameState = STATE_GAME;
        }
    }
}


int main(int argc,char *argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(screenWidth, screenHeight);
    glutCreateWindow("Pong");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(idle);
    glutMainLoop();
    return 0;
}
