/*******************************************************************
(C) 2011,2012 by Radu Stefan
radu124@gmail.com

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*******************************************************************/

#include "stage.h"

#include "configuration.h"
#include "texManager.h"
#include "stagefx.h"

tStageElem *Stage::findElem(string name)
{
	int v=-1,i;
	for (i=0; i<elem.size(); i++)
		if (elem[i]->name==name) v=i;
	if (v<0)
	{
		tStageElem *a=new tStageElem();
		a->name=name;
		a->isBackground=(a->name=="background");
		elem.push_back(a);
		DBG(STAGE,"Stage: new layer %s\n", name);
		a->parent=this;
		return a;
	}
	return elem[v];
}

tStageFx *Stage::findFX(string name, string typ)
{
	int v=-1,i;
	for (i=0; i<fx.size(); i++)
		if (fx[i]->name==name) v=i;
	if (v<0)
	{
		tStageFx *a=createStageFx(typ);
		a->name=name;
		fx.push_back(a);
		DBG(STAGE,"Stage: new fx %s, type:%s\n", name, typ);
		return a;
	}
	return fx[v];
}


void Stage::load(string dir, string filename)
{
	char line[1024];
	int mode=0;
	int i;
	cleanup();
	tStageElem *crtl=NULL;
	tStageFx *crtf=NULL;
	FILE *fc=fopen((dir+"/"+filename).c_str(),"r");
	if (!fc) return;

	while (!feof(fc))
	{
		fgets(line,999,fc);
		char *p, *sav, *fxs;
		// eliminate CR,LF at the end of line
		p=line+strlen(line)-1;
		while (p>=line)
		{
			if (*p!=10 && *p!=13) break;
			*p=0;
		}
		p=line;
		while (*p==' ' || *p=='\t') p++;
		if (*p=='[')
		{
			sav=++p;
			while (*p!=']' && *p) p++;
			*p=0;
			if (*sav==':')
			{
				sav++;
				fxs=strchr(sav,' ');
				const char *fxnam="unnamed";
				if (!fxs)
				{
					WARN(STAGE,"Invalid effect %s\n",line);
				}
				else
				{
					fxnam=fxs+1;
					*fxs=0;
				}

				mode=2;
				crtf=findFX(fxnam,sav);
			} else {
				mode=1;
				crtl=findElem(sav);
			}
			continue;
		}
		if (*p==';' || *p=='#')
			continue;
		if (mode==1) crtl->read(p);
		if (mode==2) crtf->read(p);
	}
	for (i=0; i<elem.size(); i++) elem[i]->texid=-1;
	for (i=0; i<elem.size(); i++) if (elem[i]->lv_texture!="")
	{
		string txname=elem[i]->lv_texture;
		int pos=txname.rfind(".svg");
		if (pos!=string::npos)
			txname.replace(pos,4,".png");
		else if (txname[0]=='/') elem[i]->texid=texLoad(txname.substr(1,999));
		else elem[i]->texid=texLoad(dir+"/"+txname,1);

		elem[i]->lv_yscale=elem[i]->lv_yscale*texAspect(elem[i]->texid);
		elem[i]->lv_yscale*=elem[i]->lv_scale;
		elem[i]->lv_xscale*=elem[i]->lv_scale;
	}
	for (i=0; i<elem.size(); i++) if (elem[i]->lv_effects!="")
	{
		vector<string> effects=split_string(elem[i]->lv_effects,' ');
		int j;
		for (j=0; j<effects.size(); j++)
		{
			INFO(STAGE,"Binding effect %s to %s\n",effects[j],elem[i]->name);
			elem[i]->fx.push_back(findFX(effects[j]));
		}
	}
	fclose(fc);
}

void Stage::cleanup()
{
	int i;
	for (i=0; i<elem.size(); i++)
	{
		if (elem[i]->texid>=0) texRelease(elem[i]->texid);
		delete elem[i];
	}
	elem.resize(0);
	for (i=0; i<fx.size(); i++)
	{
		delete fx[i];
	}
	fx.resize(0);
}

void Stage::render()
{
	int i;
	for (i=0; i<elem.size(); i++)
	{
		if (elem[i]->lv_foreground) continue;
		if (!(elem[i]->lv_players & (1<<(numplayers-1)))) continue;
		elem[i]->render();
	}
}

void Stage::renderForeground()
{
	int i;
	for (i=0; i<elem.size(); i++)
	{
		if (elem[i]->lv_foreground!=1) continue;
		if (!(elem[i]->lv_players & (1<<(numplayers-1)))) continue;
		elem[i]->render();
	}
}



