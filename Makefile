CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wno-deprecated
TARGET = bird_scene
SRCS = main.cpp OBJImporter.cpp

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
LIBS = -framework OpenGL -framework GLUT
else ifneq ($(findstring MINGW,$(UNAME_S)),)
LIBS = -lopengl32 -lglu32 -lfreeglut
else ifneq ($(findstring MSYS,$(UNAME_S)),)
LIBS = -lopengl32 -lglu32 -lfreeglut
else
LIBS = -lGL -lGLU -lglut
endif

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS) $(LIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
