#include <GL/glu.h>
#include <GL/gl.h>
#include <SDL/SDL.h>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <unistd.h>
#include <vector>

#define PI 3.141592

enum GameStates {STATE_GAME, STATE_AFTER};

GLsizei screenWidth = 800, screenHeight = 600;
GameStates gameState = STATE_GAME;
std::vector<bool> keyPressed(SDLK_LAST, false);

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
            if (y2 > y && y2 < y+h)
                if ((x - x1)*(x - x2) < 0)
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
            boost = 1.2;
            y = 0.0;
            w = 20;
            h = 100;
        }

        void reset()
        {
            speed = 0.0;
        }

        void update()
        {
            GLfloat ds = 0.0, decel = 1.0;

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
                angle = -angle;
                ++returns;
                if (returns % 2 == 0)
                {
                    speed *= 1.2;
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

} ball;

void initGL()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
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
}

void reshape(int w, int h)
{
    screenHeight = h;
    screenWidth = w;
    p2.x = w - p2.w;

    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, (GLdouble) w, 0.0, (GLdouble) h);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void keyboard()
{
    if(keyPressed[SDLK_ESCAPE])
        exit(0);

    if (keyPressed['a'])
        p1.up();

    if (keyPressed['z'])
        p1.down();

    if (keyPressed['m'])
        p2.down();

    if (keyPressed['k'])
        p2.up();

    if (keyPressed['r'])
        if (gameState == STATE_AFTER)
        {
            ball.reset();
            p1.reset();
            p2.reset();
            gameState = STATE_GAME;
        }
}

void update()
{
    keyboard();
    if (gameState == STATE_GAME)
    {
        p1.update();
        p2.update();
        ball.update();
    }
}

SDL_Surface * setVideoMode()
{
    SDL_Surface *surface;
    int flags = SDL_OPENGL | SDL_RESIZABLE;
    if ((surface = SDL_SetVideoMode(screenWidth, screenHeight, 0, flags)) == NULL)
    {
        std::cerr << "Unable to create OpenGL screen: " << SDL_GetError() << '\n';
        SDL_Quit();
        exit(-1);
    }

    return surface;
}


int main(int argc,char *argv[])
{
    SDL_Surface *surface = NULL;
    SDL_Event event;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "Unable to initialize SDL: " << SDL_GetError() << '\n';
        return -1;
    }

    surface = setVideoMode();

    SDL_WM_SetCaption("Pong", NULL);

    initGL();
    reshape(screenWidth, screenHeight);

    while (!keyPressed[SDLK_ESCAPE] && event.type != SDL_QUIT)
    {
        update();
        display();
        SDL_GL_SwapBuffers();

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
            {
                keyPressed[event.key.keysym.sym] = static_cast<bool>(event.key.state);
            }

            if (event.type == SDL_VIDEORESIZE)
            {
                screenWidth = event.resize.w;
                screenHeight = event.resize.h;

                if (surface)
                    SDL_FreeSurface(surface);

                surface = setVideoMode();
            }
        }

        usleep(10000);
    }

    SDL_Quit();

    return 0;
}

