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

#include "scnTable.h"

#include "songsdb.h"
#include "sprites.h"
#include "audio.h"
#include "font.h"
#include "globals.h"
#include "stage.h"

tScnTable::tScnTable()
{
	selx=0;
	sely=0;
	baseh=0;
	fixedcols=0;
	xpos=20;
	colsvisible=2;
	mustregenopt=0;
}

void tScnTable::additem(string s)
{
	if (!entries.size()) entries.resize(1);
	entries.back().push_back(s);
	mustregenopt=1;
}

void tScnTable::cleancol()
{
	if (!entries.size()) entries.resize(1);
	entries.back().resize(0);
	mustregenopt=1;
}

void tScnTable::newcol()
{
	entries.push_back(vstring());
	mustregenopt=1;
}

void tScnTable::dupcol()
{
	entries.push_back(entries.back());
	mustregenopt=1;
}

void tScnTable::makeempty()
{
	entries.resize(0);
	mustregenopt=1;
}


void tScnTable::enter()
{
	SDL_EnableKeyRepeat(133,33);
}

/**
 * Make sure the table of flags is as large as the menu
 * and initialize all flags with zero
 */
void tScnTable::optinit()
{
	int i;
	mustregenopt=0;
	opt.resize(0);
	opt.resize(entries.size());
	for (i=0; i<entries.size(); i++)
		if (opt[i].size()!=entries[i].size())
			opt[i].resize(entries[i].size());

}

/**
 * Determine if the current element is disabled or empty
 */
char tScnTable::getMarker(int selx, int sely)
{
	char c;
	if (opt[selx][sely] & 6) return '|';
	if (entries[selx][sely]=="") c='|';
		else c=entries[selx][sely][0];
	return c;
}

/**
 * default virtual function for table menu
 * called when one item is clicked (keyboard selected)
 * INPUTS:
 *    i=column
 *    j=row
 *
 * empty rows are seen as separators
 *
 * CALLS: itemClickedSep(column, group, item)
 */
void tScnTable::itemClicked(int i, int j)
{
	int q, k=0, ii=0;
	if (i<0 || j<0)
	{
		itemClickedSep(-1,-1,-1,q);
		return;
	}
	for (q=0; q<j && q<entries[i].size(); q++)
	{
		ii++;
		if (entries[i][q]=="") { k++; ii=0; }
	}
	itemClickedSep(i,k,ii,opt[i][j]);
}

/**
 * return index of row where a group starts
 */
int tScnTable::groupStart(int i, int g)
{
	int q;
	for (q=0; q<entries[i].size(); q++)
	{
		if (!g) break;
		if (entries[i][q]=="") g--;
	}
	if (q>=entries[i].size()) q=0;
	return q;
}

/**
 * Fix selection for menu items when selection falls outside the table
 * boundaries or the current item is disabled
 */
void tScnTable::selfix()
{
	int i,steps=0;
	if (opt.size()!=entries.size()) optinit();
	while (1)
	{
		steps++;
		if (steps>100)
		{
			//avoid looping forever
			selx=0;
			sely=0;
			baseh=fixedcols;
			return;
		}
		if (selx<0) { selx=0; lastlr=1; continue; }
		if (selx>=entries.size()) { selx=entries.size()-1; lastlr=-1; continue; }
		if (sely<0) { sely=0; lastud=1; continue; }
		if (sely>=entries[selx].size()) { sely=entries[selx].size()-1; lastud=-1; continue; }
		//if (xpos<baseh) baseh=xpos;
		//if (xpos>=baseh+colsvisible) baseh=xpos-colsvisible+1;
		switch (getMarker(selx,sely))
		{
		case 'O':
		case 'A':
		case 'B':
		case 'C':
		case 'E':
		case 'F':
		case 'X':
		case 'Y':
			return;
		case '|':
			sely+=lastud;
			continue;
		case '-':
			selx+=lastlr;
			continue;
		case '<':
			selx--;
			lastlr=-1;
			continue;
		case '>':
			selx++;
			lastlr=1;
			continue;
		case 'V':
		case 'v':
			sely++;
			lastud=1;
			continue;
		case '^':
			sely--;
			lastud=-1;
			continue;
		}
	}
}

void tScnTable::rendertext()
{
	if (mustregenopt) optinit();

	int i,j,cc;
	GLfloat x,y,scale;
	colsize=80/(fixedcols+colsvisible);
	// (w-xpos*2)/... why?
	scale=3;
	glLoadIdentity();

	for (j=0; j<fixedcols+colsvisible; j++)
	{
		if (j<fixedcols) cc=j;
		else cc=j-fixedcols+baseh;
		y=2*scale-30;
		x=colsize*j-37;
		if (cc>=entries.size()) continue;
		for (i=0; i<entries[cc].size(); i++)
		{
			string item=entries[cc][i];

			if (item=="")
			{
				y+=scale/2;
				continue;
			}
			if (opt[cc][i] & 2) continue;

			glColor3f(0.0f,0.0f,0.0f);
			GLfloat dy=0;

			if (selx==cc && sely==i && scn.fade==0)
			{
				dy=sin(scn.time*8)*scale*0.1;
				if (opt[cc][i] & 1) glColor3f(1.0f,0.1f,0.0f);
				else glColor3f(1.0f,0.7f,0.0f);
			}
			else
			{
				if (opt[cc][i] & 1) glColor3f(0.9f,0.0f,0.0f);
				else glColor3f(0.8f,0.3f,0.0f);
			}

			if (opt[cc][i] & 4)
			{
				glColor3f(0.4f,0.4f,0.4f);
			}

			deffont.displayString(item.c_str()+1, x, y+dy ,scale);
			y+=scale;
		}
	}
}

void tScnTable::touch(int i, int j)
{
	char c=getMarker(i,j);
	//DBG(TABLE,"%d %d %c\n" &i &j &c);
	int q;
	switch (c)
	{
	case 'O':
		itemClicked(i,j);
		break;
	case 'Q':
		itemClicked(-1,-1);
		break;
	case 'A':
	case 'B':
		// these work like radio buttons
		for (q=0; q<entries[i].size(); q++)
			if (getMarker(i,q)==c) opt[i][q]=0;
		opt[i][j]=1;
		itemClicked(i,j);
		break;
	case 'C':
		opt[i][j]^=1;
		itemClicked(i,j);
		break;
	}
}

void tScnTable::handleevent(SDL_Event &event)
{
	int actkey;
	if (mustregenopt) optinit();
	switch (event.type)
	{
	case SDL_KEYDOWN:
		actkey=event.key.keysym.sym;
		// may need to process this one to intercept other keys
		switch (actkey)
		{
		case SDLK_ESCAPE:
			itemClicked(-1,-1);
			break;
		case SDLK_DOWN:
			lastud=1;
			sely++;
			break;
		case SDLK_UP:
			lastud=-1;
			sely--;
			break;
		case SDLK_RIGHT:
			lastlr=1;
			selx++;
			break;
		case SDLK_LEFT:
			lastlr=-1;
			selx--;
			break;
		case SDLK_END:
		case SDLK_PAGEDOWN:
			lastud=1;
			sely=entries[selx].size()-1;
			break;
		case SDLK_HOME:
		case SDLK_PAGEUP:
			lastud=-1;
			sely=0;
			break;
		case SDLK_SPACE:
		case SDLK_RETURN:
			touch(selx, sely);
			break;
		}
		break;
	}
	selfix();
	if (selx>=fixedcols)
	{
		if (selx<baseh) baseh=selx;
		if (selx>=baseh+colsvisible) baseh=selx-colsvisible+1;
	}
}

void tScnTable::render()
{
	st_sopts->render();
	rendertext();
	st_sopts->renderForeground();
}




