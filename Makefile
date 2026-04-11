CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wno-deprecated
FRAMEWORKS = -framework OpenGL -framework GLUT
TARGET = bird_scene
SRCS = main.cpp OBJImporter.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(FRAMEWORKS) -o $(TARGET) $(SRCS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)