#include <stdio.h>
#include "lib.h"
#include "loader.h"
#include <vector>

#define scale 30

using namespace std;

struct Dot{
	float x,y,m;
	Dot(Loader ld){
		x=ld.parseint();
		y=ld.parseint();
		m=ld.parseint();
	}
	void draw(Framework*w,bool selected){
		// XXX make this mass-dependent?
		for(int x=-2;x<2;x++)
		for(int y=-2;y<2;y++)
		w->p(selected?0xfff:0xf00,this->x+x,this->y+y);
	}
};

struct Spring{
	int a,b;
	float len;
	Spring(Loader ld,vector<Dot>dots){
		a=(int)ld.parseint();
		b=(int)ld.parseint();
		float x=dots[a].x-dots[b].x,y=dots[a].y-dots[b].y;
		len=sqrt(x*x+y*y);
	}
	void draw(Framework*w,vector<Dot>dots){
		w->line(0x0f0,dots[a].x,dots[b].x,dots[a].y,dots[b].y);
  }
};

struct Obj{
	vector<Dot>dots;
	vector<Spring>springs;

	Obj(){}
	Obj(const char*fname){
		Loader ld(fname);
		ld.white();
		while(ld.a!=EOF){
			switch(ld.a){
				case'd':dots.push_back(Dot(ld));break;
				case's':springs.push_back(Spring(ld,dots));break;
				default:break;
			}
			ld.get();
		}
		ld.close();
	}

	void relax(){
		// this seems suspiciously easy...
		for(Spring&s:springs){
			Dot&a=dots[s.a],&b=dots[s.b];
			float x=a.x-b.x,y=a.y-b.y;
			float len = sqrtf(x*x+y*y);
			x/=len,y/=len;
			float d=len-s.len;
			d*=.5; // dampening
			float m=a.m+b.m;
			a.x-=x*d*b.m/m,a.y-=y*d*b.m/m;
			b.x+=x*d*a.m/m,b.y+=y*d*a.m/m;
		}
	}

	void draw(Framework*w,int selected){
		for(Spring&s:springs)s.draw(w,dots);
		for(int i=0;i<dots.size();i++)dots[i].draw(w,selected==i);
	}
};

Obj m;

bool on;int selected;

int main(){
	Framework w=Framework(640,480,2);

	m=Obj("save");

	w.userfunc=[](Framework*w){
		if(w->key==0x1b)return false;
		m.draw(w,selected);
		if(on)m.relax();
		float inc=.5;
		if(SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LSHIFT])inc=5;
		if(SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LCTRL])inc=.1;

		switch(w->key){
			case' ':on^=1;break;
			case'w':m.dots[selected].y-=inc;break;
			case'a':m.dots[selected].x-=inc;break;
			case's':m.dots[selected].y+=inc;break;
			case'd':m.dots[selected].x+=inc;break;
			case'q':selected-=1;break;
			case'e':selected+=1;break;
		}
		selected%=m.dots.size();

		return true;
	};
	w.run();
	return 0;
}
