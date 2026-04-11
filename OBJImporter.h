#pragma once
#include <GLUT/glut.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
using namespace std;

struct OBJVertex { GLfloat x, y, z; };
struct OBJNormal { GLfloat x, y, z; };
struct OBJFace {
    GLint a, b, c;
    GLint na, nb, nc;
    string mtl_name;
};
struct OBJMaterial {
    GLfloat ambient[4]  = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat diffuse[4]  = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat specular[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    GLfloat shininess   = 32.0f;
};

class OBJImporter {
public:
    OBJImporter(string path);
    string path;
    vector<OBJVertex>   vertexList;
    vector<OBJNormal>   normalList;
    vector<OBJFace>     faceList;
    unordered_map<string, OBJMaterial> materialTable;
private:
    void parseStream(ifstream& ifs);
    void readMaterial(string& line);
    void useMaterial(string& line, string& currentMtl);
    void readVertex(string& line);
    void readFace(string& line, string& currentMtl);
    void smoothNormals();
};