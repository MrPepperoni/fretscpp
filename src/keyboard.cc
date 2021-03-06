/*******************************************************************
(C) 2011 by Radu Stefan
radu124@gmail.com

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*******************************************************************/

#include "keyboard.h"

#include "configuration.h"
#include "includes.h"
#include "player.h"
#include "configuration.h"
#include "joystick.h"
#include "scene.h"
#include "message.h"
#include "verbosity.h"
#include "scnGuitar.h"

string keyname(int id)
{
	char a[32];
	if (id<1000000)
	{
		string res=SDL_GetKeyName((SDLKey) id);
		if (res=="unknown") res="-n/a-";
		return res;
	}
	if (id<2000000)
	{
		id-=1000000;
		sprintf(a,"Joy #%d %c",id/256,'a'+id%256);
	} else if (id<3000000) {
		id-=2000000;
		sprintf(a,"Joy #%d hat%d%c,",id/1024,(id&1023)/4,"NSWE"[id&3]);
	} else if (id<4000000) {
		id-=3000000;
		sprintf(a,"Joy #%d axis%d",id/256,id%256);
	} else if (id<5000000) {
		id-=4000000;
		sprintf(a,"Joy #%d ball%d%c",id/512,(id&511)/2,"XY"[id&1]);
	} else return "config err";
	return a;
}

const int JOY_BALL_SENSITIVITY=10000;
const int JOY_AXIS_DETECT_SENSITIVITY=5000;
// perform translation before sending to scene
void translateandhandleevent(SDL_Event &event)
{
	SDL_Event e=event;

	// translate hat to buttons
	// a single hat event may generate multiple button events
	if (e.type==SDL_JOYAXISMOTION && joydetect_enable)
	{
		int joy=e.jaxis.which;
		int axis=e.jaxis.axis;
		int value=e.jaxis.value;
		int oldval=joysticks[joy].axes[axis];
		DBG(JOYSTICK, "Joy axis %d %d %d\n", (int) e.jaxis.which, (int) e.jaxis.axis, (int) e.jaxis.value);
		joysticks[joy].axes[axis]=value;
		scenehandleevent(e);
		if (joydetect_enable && oldval!=-1000000 && abs(oldval-value)>JOY_AXIS_DETECT_SENSITIVITY)
		{
			joydetect_enable=0;
			e.type=SDL_KEYDOWN;
			e.key.keysym.sym=(SDLKey) (3000000+e.jaxis.which*256+e.jaxis.axis);
			scenehandleevent(e);
		}
		return;
	}
	if (e.type==SDL_JOYBALLMOTION)
	{
		DBG(JOYSTICK, "Ball motion %d %d %d %d\n", (int) e.jball.which, (int) e.jball.ball, (int) e.jball.xrel, (int) e.jball.yrel);
		// motion detect for the power-up
		if (e.jball.xrel<-JOY_BALL_SENSITIVITY || e.jball.xrel>JOY_BALL_SENSITIVITY)
		{
			e.type=SDL_KEYDOWN;
			e.key.keysym.sym=(SDLKey) (4000000+e.jball.which*512+e.jball.ball*2);
			scenehandleevent(e);
		}
		if (e.jball.yrel<-JOY_BALL_SENSITIVITY || e.jball.yrel>JOY_BALL_SENSITIVITY)
		{
			e.type=SDL_KEYDOWN;
			e.key.keysym.sym=(SDLKey) (4000000+e.jball.which*512+e.jball.ball*2+1);
			scenehandleevent(e);
		}
		return;
	}

	if (e.type==SDL_JOYHATMOTION)
	{
		int joy=e.jhat.which;
		int hat=e.jhat.hat;
		int hval=e.jhat.value;
		int prev=joysticks[joy].hats[hat];
		int base=2000000+joy*1024+hat*4;
		e.type=SDL_KEYDOWN;
		if ((hval & SDL_HAT_UP   ) && !(prev & SDL_HAT_UP   )) { e.key.keysym.sym=(SDLKey) base;     scenehandleevent(e); }
		if ((hval & SDL_HAT_DOWN ) && !(prev & SDL_HAT_DOWN )) { e.key.keysym.sym=(SDLKey) (base+1); scenehandleevent(e); }
		if ((hval & SDL_HAT_LEFT ) && !(prev & SDL_HAT_LEFT )) { e.key.keysym.sym=(SDLKey) (base+2); scenehandleevent(e); }
		if ((hval & SDL_HAT_RIGHT) && !(prev & SDL_HAT_RIGHT)) { e.key.keysym.sym=(SDLKey) (base+3); scenehandleevent(e); }
		e.type=SDL_KEYUP;
		if (!(hval & SDL_HAT_UP   ) && (prev & SDL_HAT_UP   )) { e.key.keysym.sym=(SDLKey) base;     scenehandleevent(e); }
		if (!(hval & SDL_HAT_DOWN ) && (prev & SDL_HAT_DOWN )) { e.key.keysym.sym=(SDLKey) (base+1); scenehandleevent(e); }
		if (!(hval & SDL_HAT_LEFT ) && (prev & SDL_HAT_LEFT )) { e.key.keysym.sym=(SDLKey) (base+2); scenehandleevent(e); }
		if (!(hval & SDL_HAT_RIGHT) && (prev & SDL_HAT_RIGHT)) { e.key.keysym.sym=(SDLKey) (base+3); scenehandleevent(e); }
		joysticks[joy].hats[hat]=hval;
		return;
	}
	if (e.type==SDL_JOYBUTTONDOWN)
	{
		e.type=SDL_KEYDOWN;
		e.key.keysym.sym=(SDLKey) (1000000+256*e.jbutton.which+e.jbutton.button);
	}
	if (e.type==SDL_JOYBUTTONUP)
	{
		e.type=SDL_KEYUP;
		e.key.keysym.sym=(SDLKey) (1000000+256*e.jbutton.which+e.jbutton.button);
	}
	scenehandleevent(e);
}

void guitarkey(int ev, int key, int other)
{
	int i,pl,q;
	for (pl=0; pl<numplayers; pl++)
	{
		for (q=0; q<keydefs.size(); q++)
		{
			if (!(keydefselector[pl] & (1<<q))) continue;
			int id=-1;
			for (i=0; i<9; i++)
			{
				if (key==keydefs[q].key[i]) id=i;
			}
			if (id==6) id=5;
			if (id>=0 && ev==SDL_KEYDOWN) player[pl].presskey(guitarScene.timenow, id);
			if (id>=0 && ev==SDL_KEYUP)   player[pl].releasekey(guitarScene.timenow, id);
			if (id==7 && ev==SDL_JOYAXISMOTION) player[pl].whammyaxis(other);
		}
	}
}

void menukey(SDL_Event &event, int flags)
{
	if (event.type!=SDL_KEYDOWN && event.type!=SDL_KEYUP) return;
	switch (event.key.keysym.sym)
	{
		// these keys are never remapped
		case SDLK_ESCAPE:
		case SDLK_RETURN:
		case SDLK_UP:
		case SDLK_DOWN:
		case SDLK_LEFT:
		case SDLK_RIGHT:
		case SDLK_HOME:
		case SDLK_END:
		case SDLK_PAGEUP:
		case SDLK_PAGEDOWN:
		return;
		default:;
	}
	// string input
	if (event.key.keysym.sym>='a' && event.key.keysym.sym<='z' && (flags&1))
		return;

	int i,pl,q,key=event.key.keysym.sym;
	pl=0; // by default only player controller is remapped
	if (!redefining) for (q=0; q<keydefs.size(); q++)
	{
		if (!(keydefselector[pl] & (1<<q))) continue;
		if (key==keydefs[q].key[0]) event.key.keysym.sym=SDLK_RETURN;
		if (key==keydefs[q].key[1]) event.key.keysym.sym=SDLK_ESCAPE;
		if (key==keydefs[q].key[2]) event.key.keysym.sym=SDLK_LEFT;
		if (key==keydefs[q].key[3]) event.key.keysym.sym=SDLK_RIGHT;
		if (key==keydefs[q].key[5]) event.key.keysym.sym=SDLK_UP;
		if (key==keydefs[q].key[6]) event.key.keysym.sym=SDLK_DOWN;
	}
}



