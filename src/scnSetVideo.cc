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

#include "scnSetVideo.h"
#include "scnSetBase.h"
#include "message.h"

class tScnSetVideo:public tScnSetBase
{
public:
	tScnSetVideo() { scenes.add(this,SCN_SETVIDEO); }
	void init();
	void enter();
	void itemAdjusted(int i);
} scnSetVideo;

#include "configuration.h"
#include "scene.h"
#include "video.h"

void tScnSetVideo::init()
{
	additem(new tSIcheckbox("Fullscreen",&video_dofullscreen));
	additem(new tSIclist("Fullscreen Mode",videomodes,(tConfigurable **)&fsvideomode));
	additem(new tSIcheckbox("VSync (N/A)",&video_vsync));
	additem(new tSIcheckbox("glFlush",&video_glflush));
	additem(new tSIcheckbox("glFinish",&video_glfinish));
	additem(new tSIcheckbox("Frame Limiter (N/A)",&video_limiter));
	additem(new tSIrange("FPS (N/A)",25,100,&video_fpslim,1));
	tScnSetBase::init();
}

void tScnSetVideo::enter()
{
	tScnSetBase::enter();
}

void tScnSetVideo::itemAdjusted(int i)
{
	switch (i)
	{
	case 0:
		if (video_dofullscreen) videoToFullscreen();
		else videoExitFullscreen();
		break;
	case 1:
		MESSAGE("current fs res %d %d\n", fsvideomode->width, fsvideomode->height);
	}
}



