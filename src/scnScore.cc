/*******************************************************************
(C) 2010 by Radu Stefan
radu124@gmail.com

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*******************************************************************/

#include "scnScore.h"

#include "sprites.h"
#include "scnTable.h"
#include "globals.h"
#include "score.h"
#include "seScore.h"
#include "stage.h"
#include "player.h"
#include "font.h"
#include "configuration.h"

tScnScore SCNScore;

void tScnScore::enter()
{
	editing=0;
}

void tScnScore::render()
{
	char a[128];
	st_score->render();
	int percentage=100;
	int i;
	for (i=0; i<numplayers; i++)
	{
		tPlayer &pp=player[i];

		if (pp.stat_notestotal>0) percentage=pp.stat_noteshit*100/pp.stat_notestotal;
		glLoadIdentity();
		glColor4f(1.0,1.0,1.0,1.0);
		sprintf(a,"%d/%d (%d%%)",pp.stat_noteshit,pp.stat_notestotal,percentage);
		if (pp.stat_noteshit==pp.stat_notestotal && pp.stat_xmiss)
		{
			sprintf(a+strlen(a)," and %d extra misses",pp.stat_xmiss);
		}
		GLfloat scoreposx,scoreposy;
		scoreposx=-38+(i%2)*40;
		scoreposy=-26+5*(i&6);
		deffont.displayString(a,scoreposx,scoreposy,3,0);
		glPushMatrix();
		glTranslatef(scoreposx+25,scoreposy+1.6,0);
		glScalef(3,3,1);
		showStars(scorepromille(pp.stat_notestotal,pp.stat_noteshit,pp.stat_xmiss));
		glPopMatrix();
		sprintf(a,"%d points",pp.score);
		deffont.displayString(a,scoreposx,scoreposy+3,3,0);
		string s=pp.name;
		if (i==editing && (((int) (scn.time*2))&1)) s+='_';
		sprintf(a,"name: %s",s.c_str());
		deffont.displayString(a,scoreposx,scoreposy+7,3,0);
	}
	st_score->renderForeground();
}

void tScnScore::writescores()
{
	int i;
	FILE *scf=fopen(scoresFile.c_str(),"a");
	if (!scf) return;
	for (i=0; i<numplayers; i++)
	{
		scoreline a;
		if (player[i].stat_noteshit<2) continue;
		if (!player[i].name.length()) continue;
		a.readfromplayer(i);
		a.write(scf);
		scores.data.push_back(a);
	}
	fclose(scf);
}

void tScnScore::handleevent(SDL_Event &a)
{
	if (a.type==SDL_KEYDOWN)
	{
		int ch=a.key.keysym.sym;
		if (ch==SDLK_ESCAPE || ch==SDLK_RETURN)
		{
			writescores();
			editing=-1;
			fadetoscene(SCN_SONGS);
			return;
		}
		if (editing>=0)
		{
			if (ch==SDLK_TAB) editing=(editing+1)%numplayers;
			if (isalnum(ch) || ch=='.')
			{
				if (player[editing].name.size()<16) player[editing].name+=(char) ch;
			}
			if (ch==SDLK_BACKSPACE)
			{
				int q=player[editing].name.size();
				if (q>0) player[editing].name=player[editing].name.substr(0,q-1);
			}
		}
	}
}



