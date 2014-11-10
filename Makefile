PROJECT = visualizer_linux
SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
CFLAGS  = -c -O2 -Wall -pedantic
INCLUDES = -I./fmod_include
LIBRARIES = `pkg-config --libs libglfw` -lGLEW -lGL ./libfmodex64-4.44.32.so

all: $(PROJECT)

%.o: %.cpp
	g++ $(CFLAGS) $(INCLUDES) $< -o $@

$(PROJECT): $(OBJECTS)
	g++ -s $(OBJECTS) -o $(PROJECT) $(LIBRARIES)

clean:
	rm $(OBJECTS) -f

