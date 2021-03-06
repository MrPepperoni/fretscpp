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

#include "scnSetSongOpt.h"

#include "sprites.h"
#include "scnTable.h"
#include "globals.h"
#include "player.h"
#include "configuration.h"
#include "songini.h"

void tScnSetSongOpt::regroupPlayers()
{
	int i, l;
	long ks;
	tPlayer tmp;
	player[0].enabled=1;
	l=1;
	for (i=1; i<MAX_PLAYERS; i++)
	{
		if (!player[i].enabled) continue;
		if (i!=l)
		{
			tmp=player[i];
			player[i]=player[l];
			player[l]=tmp;
			ks=keydefselector[i];
			keydefselector[i]=keydefselector[l];
			keydefselector[l]=ks;
		}
		l++;
	}
	numplayers=l;
}

void tScnSetSongOpt::enableDisable(int pl)
{
	int i;
	if (!player[pl].enabled)
	{
		player[pl].enabled=1;
		for (i=1; i<opt[1].size(); i++)
			opt[pl][i]=0;
		opt[pl][0]=1;
		selInstrument(pl);
		setKeySels(pl);
	} else {
		player[pl].enabled=0;
		numplayers=1;
		for (i=1; i<opt[1].size(); i++)
			opt[pl][i]=2;
		opt[pl][0]=0;
	}
}

void tScnSetSongOpt::selInstrument(int i, int j)
{
	int base,q;
	if (j>=crtSong.trk_instrument.size()) j=-1;
	if (j<0)
	{
		j=0;
		for (q=0; q<crtSong.trk_instrument.size(); q++)
			if (player[i].lastInstr==crtSong.trk_instrument[q]) j=q;
	} else
		player[i].lastInstr=crtSong.trk_instrument[j];
	base=groupStart(i,1);
	DBG(SCNSONGOPT,"col: %d base:%d\n", i, base);
	for (q=0; q<crtSong.trk_instrument.size(); q++)
		opt[i][base+q]=(j==q);
	player[i].instrument=j;
	selDifficulty(i);
}

/**
 * return currently selected instrument for player i(+1)
 */
int tScnSetSongOpt::getInstrument(int i)
{
	int base,q;
	base=groupStart(i,1);
	for (q=0; q<crtSong.trk_instrument.size(); q++)
		if (opt[i][base+q]==1) return q;
	return 0;
}

/**
 * return currently selected difficulty for player i(+1)
 */
int tScnSetSongOpt::getDifficulty(int i)
{
	int base,q;
	base=groupStart(i,2);
	for (q=0; q<4; q++)
		if (opt[i][base+q]==1) return q;
	return 0;
}

void tScnSetSongOpt::selDifficulty(int i, int j)
{
	int q,base, instr, dff;
	instr=getInstrument(i);
	base=groupStart(i,2);
	DBG(SCNSONGOPT,"col: %d instr:%d base:%d\n", i, instr, base);
	dff=1;
	for (q=0; q<4; q++)
	{
		opt[i][base+q]=(crtSong.trk_difficulties[instr]&dff)?0:2;
		dff<<=1;
	}
	if (j==-1) j=player[i].lastDiffi;
	else player[i].lastDiffi=j;
	// this difficulty level does not exist for this song/instrument
	if (opt[i][base+j])
	{
		j=0;
		for (q=0; q<4; q++)
			if (!opt[i][base+q]) { j=q; break; }
	}
	opt[i][base+j]=1;
	player[i].difficulty=j;
}

void tScnSetSongOpt::enter()
{
	int res,ires,i;
	int delay=0;
	res=crtSong.openfile((selectedsong+"/notes.mid").c_str());
	colsvisible=4;
	crtIni=tSongini();
	ires=crtIni.load(selectedsong+"/song.ini");
	if (crtIni.delay!=0)
	{
		INFO(SONGINI,"Applying delay as read from song.ini (%d)\n", crtIni.delay);
		crtSong.applydelay(crtIni.delay);
	}

	if (!res) jumptoscene(1);
	makeempty();
	additem("OStart");
	additem("");
	for (i=0; i<crtSong.trk_instrument.size(); i++)
		additem(string("O")+crtSong.trk_instrument[i]);
	additem("");
	additem("ASuperEasy");
	additem("AEasy");
	additem("AMedium");
	additem("AHard");
	additem("");
	additem("|Controller:");
	for (i=0; i<keydefs.size(); i++)
	{
		additem(string("C")+keydefs[i].name);
	}
	dupcol();
	dupcol();
	dupcol();
	entries[1][0]="OPlayer 2";
	entries[2][0]="OPlayer 3";
	entries[3][0]="OPlayer 4";
	optinit();
	setKeySels(0);
	setKeySels(1);
	setKeySels(2);
	setKeySels(3);
	// we initialize with the opposite value
	// because enableDisable will invert it
	// players 2 and 3 keep their previous state
	// if multiplayer is enabled
	player[1].enabled=numplayers==1;
	player[2].enabled=numplayers==1 || !player[2].enabled;
	player[3].enabled=numplayers==1 || !player[3].enabled;
	enableDisable(1);
	enableDisable(2);
	enableDisable(3);
	selInstrument(0);
}

void tScnSetSongOpt::setKeySels(int i)
{
	int j,q;
	int base=groupStart(i,3);
	for (j=0; j<keydefs.size(); j++)
		opt[i][base+1+j]=(keydefselector[i] & (1<<j))!=0;
}

void tScnSetSongOpt::itemClickedSep(int i, int k, int j, int &o)
{
	DBG(SCNSONGOPT,"SEPclick %d %d %d\n", i, k, j);

	if (i==-1)
	{
		fadetoscene(SCN_SONGS);
		return;
	}
	if (i==0 && k==0)
	{
		regroupPlayers();
		fadetoscene(SCN_PLAYING);
		return;
	}
	if (i>0 && k==0)
	{
		enableDisable(i);
		return;
	}
	if (k==1)
	{
		selInstrument(i,j);
		return;
	}
	if (k==2)
	{
		selDifficulty(i,j);
		return;
	}
	if (k==3)
	{
		MESSAGE("%d \n", o);
		if (o) keydefselector[i] |= (1<<(j-1));
		else keydefselector[1] &= ~(1<<(j-1));
	}
}

tScnSetSongOpt SongOpt;



