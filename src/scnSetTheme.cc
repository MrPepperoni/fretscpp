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

#include "scnSetTheme.h"

#include "configuration.h"
#include "scene.h"
#include "message.h"
#include "verbosity.h"
#include "stage.h"
#include "font.h"
#include "texManager.h"
#include "fileio.h"
#include "scnGuitar.h"

tScnSetTheme scnSetTheme;

void initNeckname()
{
	int i;
	themeNeckfilenames=listDirContents(datadir+"/necks");
	themeNeckidx=0;
	themeNecknames.resize(0);
	themeNecknames.push_back("Default");
	for (i=0; i<themeNeckfilenames.size(); i++)
	{
		string s=themeNeckfilenames[i];
		themeNeckfilenames[i]="necks/"+s;
		s=s.substr(0,s.length()-4);
		themeNecknames.push_back(s);
		if (s==themeNeckname) themeNeckidx=i+1;
	}
	themeNeckname=themeNecknames[themeNeckidx];
	themeNeckfilenames.insert(themeNeckfilenames.begin(),"neck.png");
	themeNeckfilename=themeNeckfilenames[themeNeckidx];
}

void theme_init()
{
	initNeckname();
}

void tScnSetTheme::init()
{
	themeStages=listDirContents(datadir+"/stages",3);
	themesMainm=listDirContents(datadir+"/st_mainm",3);
	themesScore=listDirContents(datadir+"/st_score",3);
	themesSlist=listDirContents(datadir+"/st_slist",3);
	themesSopts=listDirContents(datadir+"/st_sopts",3);
	themesSettm=listDirContents(datadir+"/st_settm",3);
	additem(new tSIlist("Neck",themeNecknames,&themeNeckidx));
	additem(new tSIlist("Stage",themeStages,&themeStage));
	additem(new tSIlist("Main Menu"    ,themesMainm,&themeMainm));
	additem(new tSIlist("Song list"    ,themesSlist,&themeSlist));
	additem(new tSIlist("Song Options" ,themesSopts,&themeSopts));
	additem(new tSIlist("Score Screen" ,themesScore,&themeScore));
	additem(new tSIlist("Settings Menu",themesSettm,&themeSettm));
	additem(new tSIintrange("Neck transparency",0,90,&themeNecktransparency,10));
	tScnSetBase::init();
}

void tScnSetTheme::itemAdjusted(int i)
{
	switch(i)
	{
	case 0:
		themeNeckname=themeNecknames[themeNeckidx];
		themeNeckfilename=themeNeckfilenames[themeNeckidx];
		texRelease(sp_neck);
		sp_neck=texLoad(themeNeckfilename);
		break;
	case 1:
		stagePlay->cleanup();
		stagePlay->readDir(datadir+"/stages/"+themeStage);
		break;
	case 2:
		st_mainm->cleanup();
		st_mainm->readDir(datadir+"/st_mainm/"+themeMainm);
		break;
	case 3:
		st_slist->cleanup();
		st_slist->readDir(datadir+"/st_slist/"+themeSlist);
		break;
	case 4:
		st_sopts->cleanup();
		st_sopts->readDir(datadir+"/st_sopts/"+themeSopts);
		break;
	case 5:
		st_score->cleanup();
		st_score->readDir(datadir+"/st_score/"+themeScore);
		break;
	case 6:
		st_settm->cleanup();
		st_settm->readDir(datadir+"/st_settm/"+themeSettm);
		break;
	}
}

void tScnSetTheme::render()
{
	int cplayer;

	glPushMatrix();
	glLoadIdentity();
	glTranslatef(8,0,0);
	glScalef(0.75,1,1);
	stagePlay->render();
	glPopMatrix();

	scene_setNeck(0,5);
	guitarScene.renderNeck();
	guitarScene.renderTracks();

	scene_setOrtho();
	guitarScene.activeneckpos=10;
	guitarScene.renderMultiplierVal();
	guitarScene.renderStats();

	glPushMatrix();
	glLoadIdentity();
	glTranslatef(8,0,0);
	glScalef(0.75,1,1);
	stagePlay->renderForeground();
	glPopMatrix();

	rendertext();
}


