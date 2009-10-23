pong: pong.cpp
	g++ -o pong pong.cpp -lSDL -lGL -lGLU -O2 -Wall

clean:
	rm pong 
