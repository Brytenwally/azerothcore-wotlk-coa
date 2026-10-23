from pathlib import Path
import subprocess,tempfile

def main():
    source=Path('/workspace/src/common/Collision/BoundingIntervalHierarchy.h').read_text()
    start=source.index('    template<typename RayCallback>')
    method=source[start:source.index('    template<typename IsectCallback>',start)]
    code=r'''#include <algorithm>
#include <cmath>
#include <vector>
#include <cstring>
#include <cassert>
#include <cstdint>
using uint32=uint32_t;
uint32 floatToRawIntBits(float f){uint32 n;memcpy(&n,&f,4);return n;}
float intBitsToFloat(uint32 n){float f;memcpy(&f,&n,4);return f;}
namespace G3D {
struct Vector3{float v[3];float& operator[](int i){return v[i];}float operator[](int i)const{return v[i];}};
struct Ray{Vector3 o,d;Vector3 origin()const{return o;}Vector3 direction()const{return d;}};
bool fuzzyNe(float a,float b){return std::abs(a-b)>1e-6f;}
}
constexpr int MAX_STACK_SIZE=64;
struct Box{G3D::Vector3 low()const{return {{-10,-10,-10}};}G3D::Vector3 high()const{return {{10,10,10}};}};
struct BIH{struct StackNode{int node;float tnear,tfar;};Box bounds;std::vector<uint32> tree,objects;
''' + method + r'''};
int main(){
 for(unsigned axis=0;axis<3;++axis)for(bool first:{false,true}){
  BIH b;b.tree={axis<<30,0,0,3u<<30,0,0};
  G3D::Ray r{{{0,0,0}},{{0,0,0}}};r.d[(axis+1)%3]=1;
  int calls=0;auto callback=[&](auto const&,unsigned,float&,bool){++calls;return true;};
  float distance=100;b.intersectRay(r,callback,distance,first);assert(calls==0&&distance==100);
 }
 BIH b;b.tree={3u<<30,2,0};b.objects={7,8};
 for(bool first:{false,true}){int calls=0;auto cb=[&](auto const&,unsigned id,float&,bool){assert(id==7||id==8);++calls;return true;};
 float d=100;b.intersectRay(G3D::Ray{{{0,0,0}},{{1,1,1}}},cb,d,first);assert(calls==(first?1:2));}
 BIH split;split.tree={3, floatToRawIntBits(1),floatToRawIntBits(-1),3u<<30,1,0,(3u<<30)|1,1,0};split.objects={7,8};
 int calls=0;auto cb=[&](auto const&,unsigned,float&,bool){++calls;return false;};float d=100;
 split.intersectRay(G3D::Ray{{{0,0,0}},{{1,1,1}}},cb,d,false);assert(calls==2);
 calls=0;d=1;split.intersectRay(G3D::Ray{{{-20,0,0}},{{1,1,1}}},cb,d,false);assert(calls==0);
}
'''
    with tempfile.TemporaryDirectory() as tmp:
        p=Path(tmp);(p/'test.cpp').write_text(code)
        subprocess.run(['g++','-std=c++17','-fsanitize=address','-fno-omit-frame-pointer',str(p/'test.cpp'),'-o',str(p/'test')],check=True)
        subprocess.run([str(p/'test')],check=True,timeout=10)
    print('PASS: extracted native traversal with AddressSanitizer; three NaN/self-loop axes, both early-exit modes, leaf callbacks, two-node traversal and distance rejection. Geometry adapters; no gameplay.')

if __name__=='__main__':main()
