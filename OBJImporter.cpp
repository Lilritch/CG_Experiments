#include "OBJImporter.h"
#include <cmath>

OBJImporter::OBJImporter(string path) : path(path) {
    ifstream ifs(path, ifstream::in);
    if (!ifs.is_open()) { cerr << "Cannot open: " << path << "\n"; return; }
    parseStream(ifs);
    ifs.close();
    smoothNormals();
}

void OBJImporter::parseStream(ifstream& ifs) {
    string line, currentMtl;
    while (getline(ifs, line)) {
        if (line.empty()) continue;
        switch (line[0]) {
            case '#': break;
            case 'm': readMaterial(line); break;
            case 'u': useMaterial(line, currentMtl); break;
            case 'v': readVertex(line); break;
            case 'f': readFace(line, currentMtl); break;
        }
    }
}

void OBJImporter::readMaterial(string& lineBuffer) {
    stringstream ss(lineBuffer);
    string skip, filename;
    ss >> skip >> filename;
    string mtlPath = path.substr(0, path.rfind('/') + 1) + filename;
    ifstream mtl(mtlPath);
    if (!mtl.is_open()) return;
    string line, currentName;
    while (getline(mtl, line)) {
        if (line.empty()) continue;
        stringstream ls(line);
        string token;
        ls >> token;
        if (token == "newmtl") ls >> currentName;
        else if (token == "Ka") for (int i=0;i<3;i++) ls >> materialTable[currentName].ambient[i];
        else if (token == "Kd") for (int i=0;i<3;i++) ls >> materialTable[currentName].diffuse[i];
        else if (token == "Ks") for (int i=0;i<3;i++) ls >> materialTable[currentName].specular[i];
        else if (token == "Ns") {
            ls >> materialTable[currentName].shininess;
            materialTable[currentName].shininess = materialTable[currentName].shininess / 1000.0f * 128.0f;
        }
    }
}

void OBJImporter::useMaterial(string& line, string& currentMtl) {
    stringstream ss(line); string skip;
    ss >> skip >> currentMtl;
}

void OBJImporter::readVertex(string& line) {
    stringstream ss(line); string skip;
    if (line[1] == ' ') {
        OBJVertex v; ss >> skip >> v.x >> v.y >> v.z; vertexList.push_back(v);
    } else if (line[1] == 'n') {
        OBJNormal n; ss >> skip >> n.x >> n.y >> n.z; normalList.push_back(n);
    }
}

void OBJImporter::readFace(string& line, string& currentMtl) {
    // detect format
    int flag = 0;
    for (int i=0;i<(int)line.size();i++) {
        if (line[i]=='/') {
            flag = (i+1<(int)line.size() && line[i+1]=='/') ? 1 : 2;
            if (flag==2) { for(int j=i+1;j<(int)line.size();j++) { if(line[j]=='/'){flag=3;break;} if(line[j]==' ')break; } }
            break;
        }
    }
    for (auto& c : line) if (c=='/') c=' ';
    stringstream ss(line); string skip; ss >> skip;
    auto pushFace = [&](int a, int b, int c, int an, int bn, int cn) {
        faceList.push_back({a-1,b-1,c-1,an-1,bn-1,cn-1,currentMtl});
    };
    int a,b,c,d,an=1,bn=1,cn=1,dn=1,t;
    switch(flag) {
        case 0: ss>>a>>b>>c; pushFace(a,b,c,1,1,1);
                if(ss>>d) pushFace(a,c,d,1,1,1); break;
        case 1: ss>>a>>an>>b>>bn>>c>>cn; pushFace(a,b,c,an,bn,cn);
                if(ss>>d>>dn) pushFace(a,c,d,an,cn,dn); break;
        case 2: ss>>a>>t>>b>>t>>c; pushFace(a,b,c,1,1,1);
                if(ss>>d>>t) pushFace(a,c,d,1,1,1); break;
        case 3: ss>>a>>t>>an>>b>>t>>bn>>c>>t>>cn; pushFace(a,b,c,an,bn,cn);
                if(ss>>d>>t>>dn) pushFace(a,c,d,an,cn,dn); break;
    }
}

void OBJImporter::smoothNormals() {
    int nv = vertexList.size(), nf = faceList.size();
    vector<OBJNormal> faceNormals(nf);
    for (int i=0;i<nf;i++) {
        auto& f = faceList[i]; auto& fn = faceNormals[i];
        auto& va = vertexList[f.a]; auto& vb = vertexList[f.b]; auto& vc = vertexList[f.c];
        float ux=vb.x-va.x,uy=vb.y-va.y,uz=vb.z-va.z;
        float wx=vc.x-va.x,wy=vc.y-va.y,wz=vc.z-va.z;
        fn={uy*wz-uz*wy, uz*wx-ux*wz, ux*wy-uy*wx};
        float l=sqrt(fn.x*fn.x+fn.y*fn.y+fn.z*fn.z);
        if(l>0){fn.x/=l;fn.y/=l;fn.z/=l;}
    }
    normalList.clear(); normalList.resize(nv,{0,0,0});
    vector<int> count(nv,0);
    for (int i=0;i<nf;i++) {
        auto& f=faceList[i]; auto& fn=faceNormals[i];
        for (int idx : {f.a,f.b,f.c}) {
            normalList[idx].x+=fn.x; normalList[idx].y+=fn.y; normalList[idx].z+=fn.z;
            count[idx]++;
        }
    }
    for (int i=0;i<nv;i++) {
        float l=sqrt(normalList[i].x*normalList[i].x+normalList[i].y*normalList[i].y+normalList[i].z*normalList[i].z);
        if(l>0){normalList[i].x/=l;normalList[i].y/=l;normalList[i].z/=l;}
        // update face normal indices to use vertex index
    }
    for (auto& f : faceList) { f.na=f.a; f.nb=f.b; f.nc=f.c; }
}