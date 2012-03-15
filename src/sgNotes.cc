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

#include "sgNotes.h"

#include "scnGuitar.h"
#include "midiparser.h"
#include "globals.h"
#include "configuration.h"
#include "player.h"
#include "texManager.h"

GLfloat tSceneGuitar::notePos(int timestamp)
{
	if (timestamp<0) return -boardline;
	return (timestamp-timenow)/44100.0*2.2-boardline;
}

void tSceneGuitar::noteRegion()
{
	tPlayer &pp=player[cplayer];
	pp.nextnote=pp.crtnote;
	while (pp.nextnote<crtSong.trk_notes[cinstrument].size())
	{
		if (crtSong.trk_notes[cinstrument][pp.nextnote].timestamp>timenow) break;
		pp.nextnote++;
	}
	//INFO(SCNGUITAR,"note %d\n" &crtnote);
	for (pp.farrnote=pp.nextnote; pp.farrnote<crtSong.trk_notes[cinstrument].size(); pp.farrnote++)
	{
		if (crtSong.trk_notes[cinstrument][pp.farrnote].timestamp>timenow+120000) break;
	}
}

int tSceneGuitar::renderNote(int col, int ts, int flags)
{
	GLfloat y,sz,nfade;
	GLfloat c1,c2,c0;
	y=notePos(ts);
	nfade=1.0f;
	if (y>1) nfade=2-y;
	//if (pos>0) nfade=1-pos;
	if (nfade<0) nfade=0;
	if (y>2) return 1;
	sz=0.4;
	c0=c1=c2=1;

	int tt=0;
	//if (y>-2) tt=1;
	//if (y>-1) tt=2;
	//if (y>0) tt=3;

	glColor4f(c0,c1,c2,nfade);
	glPushMatrix();
	glTranslatef(col-2,y,0);
	glRotatef(60,1,0,0);
	glScalef(sz,sz,sz);
	texDraw(sp_note[col]);
	glPopMatrix();
	return 0;
}

void tSceneGuitar::renderNoteLine(int col, GLfloat from, GLfloat to, int flags)
{
	tPlayer &pp=player[cplayer];
	int i;
	GLfloat c0,c1,c2;
	c0=keycolors[col][0];
	c1=keycolors[col][1];
	c2=keycolors[col][2];

	if (flags==2 && !pp.hitactive) c0=c1=c2=0.5;
	if (flags==2 && pp.hitactive)
	{
		c0=1; c1=1; c2=0;
	}
	texBind(sp_noteline);
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	GLfloat lw=0.1;
	col-=2;
	glBegin(GL_TRIANGLE_STRIP);
	glColor4f(c0,c1,c2,0.7f);
	if (flags==2 && pp.hitactive)
	{
		glTexCoord2f(0.0f,1); glVertex3f(col-lw,from,0);
		glTexCoord2f(1.0f,1); glVertex3f(col+lw,from,0);
		for (i=0; i<20; i++)
		{
			if (from+i*0.2+0.2>to) break;
			GLfloat a1=0.1*sin(timenow*0.0001+i*0.3)+0.1*sin(timenow*0.0001754-i*0.3)+0.1*sin(timenow*0.0001754);
			GLfloat a2=0;
			if (pp.whammyon) a2=0.1*sin(i*0.5-timenow*0.0001);
			glTexCoord2f(0.0f,1); glVertex3f(col-lw-a1+a2,from+i*0.2,0);
			glTexCoord2f(1.0f,1); glVertex3f(col+lw+a1+a2,from+i*0.2,0);
		}
		glTexCoord2f(0.0f,0); glVertex3f(col-lw,to,0);
		glTexCoord2f(1.0f,0); glVertex3f(col+lw,to,0);
		//u = ((t - time) * -.1 + pos - time) / 64.0 + .0001
        //return (math.sin(event.number + self.time * -.01 + t * .03) + math.cos(event.number + self.time * .01 + t * .02)) * .1 + .1 + math.sin(u) / (5 * u)

	}
	else
	{
		glTexCoord2f(0.0f,0); glVertex3f(col-lw,to,0);
		glTexCoord2f(1.0f,0); glVertex3f(col+lw,to,0);

		glTexCoord2f(0.0f,1); glVertex3f(col-lw,from,0);
		glTexCoord2f(1.0f,1); glVertex3f(col+lw,from,0);
	}
	glEnd();

	texUnbind();
}

void tSceneGuitar::renderNoteLines()
{
	tPlayer &pp=player[cplayer];
	GLfloat startNote[5];
	int active[5];
	int activecount=0;
	int i,j;
	startNote[0]=notePos(-1);
	notestatus v;
	if (pp.nextnote>0)
		v=cnotes[0][pp.nextnote-1];
	for (i=0; i<5; i++)
	{
		char ch=v.stat[12*cdifficulty+i];
		active[i]=2*(ch=='O' || ch=='-');
		if (active[i]) activecount++;
		startNote[i]=startNote[0];
	}
	if (!activecount) pp.hitactive=0;
	for (j=pp.nextnote; j<pp.farrnote; j++)
	{
		v=cnotes[0][j];
		GLfloat lend=notePos(v.timestamp);
		if (lend>necktop) break;
		for (i=0; i<5; i++)
		{
			char ch=v.stat[12*cdifficulty+i];
			if (active[i] && ch!='-')
			{
				renderNoteLine(i,startNote[i],lend,active[i]);
				active[i]=0;
			}
			if (ch=='O') {
				active[i]=1;
				startNote[i]=notePos(v.timestamp);
			}
		}
	}
	for (i=0; i<5; i++)
		if (active[i])
			renderNoteLine(i,startNote[i],necktop,active[i]);

}

void tSceneGuitar::renderNotes()
{
	int i,j;
	tPlayer &pp=player[cplayer];
	//MESSAGE("notes: %d %d" &pp.nextnote &pp.farrnote);
	for (i=pp.farrnote-1; i>=pp.nextnote; i--)
	{
		notestatus v=crtSong.trk_notes[cinstrument][i];
		int res;
		for (j=0; j<5; j++)
		{
			char ch=v.stat[j+12*pp.difficulty];
			res=2;
			if (ch=='O' || ch=='B')
			{
				res=renderNote(j,v.timestamp);
			}
			if (res) ch='!';
		//	MESSAGE("%c" &ch);
		}
	}
	//MESSAGE("\n");
}


