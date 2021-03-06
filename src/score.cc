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

#include "score.h"

#include "sha1.h"
#include "player.h"
#include "message.h"
#include "configuration.h"
#include "verbosity.h"

tscores scores;

int scorepromille(int tot, int good, int xmiss)
{
	int res;
	res=good*1000/tot;
	if (good>=1000 && xmiss) good=999;
	return res;
}

string scoreline::percentage()
{
	char a[16];
	int v=promille();
	if (v<1000) sprintf(a,"%d.%d%%",v/10,v%10);
	else sprintf(a,"100%%");
	return a;
}

int scoreline::promille()
{
	return scorepromille(notestotal,notesgood,xmisses);
}

void tscores::setCurrent()
{
	int numbests=crtSong.trk_notes.size()*4;
	int i,a,b;
	uint64_t date;
	date=time(NULL);
	crtids.resize(0);
	best.resize(0);
	best.resize(numbests);
	for (i=0; i<numbests; i++)
	{
		best[i]=(crtSong.trk_difficulties[i/4] & (1<<(i%4)))?-1:-2;
	}
	for (i=0; i<data.size(); i++)
	{
		if (0!=memcmp(crtSong.hash,data[i].midhash,20)) continue;
		crtids.push_back(i);
		a=data[i].instrument*4+data[i].difficulty;
		if (best[a]<0 || data[best[a]].promille()<data[i].promille()) best[a]=i;
	}
	history.resize(0);
	for (i=crtids.size()-1; i>=0; i--)
	{
		int v=crtids[i];
		char a[256];
		int64_t tt=(date-data[v].date);
		if (tt<0) tt=0;
		tt/=86400;
		if (tt>9999) tt=9999;
		sprintf(a,"%d days %s/%s   %s   %s",
			(int) (tt),
			difficultynames[data[v].difficulty],
			crtSong.trk_instrument[data[v].instrument].substr(0,11).c_str(),
			data[v].percentage().c_str(),
			data[v].playername);
		history.push_back(a);
	}
}

string scoreline::toStringDump()
{
	int i;
	string res="";
	char a[512];
	for (i=0; i<sizeof(scoreline); i++)
	{
		sprintf(a+i*2,"%02x",midhash[i]);
	}
	res+=a;
	return res;
}

void tscores::init()
{
	data.resize(0);
	load(scoresFile);
}

void tscores::load(string file)
{
	FILE * fin=fopen(file.c_str(),"r");
	if (!fin) return;
	scoreline a;
	INFO(SCORES,"Reading scores\n");
	while (!feof(fin))
	{
		if (a.read(fin))
			data.push_back(a);
	}
	fclose(fin);
}

void scoreline::readfromplayer(int num)
{
	memcpy(midhash,crtSong.hash,20);
	date=time(NULL);
	memset(playername,0,36);
	strncpy((char *) playername,player[num].name.c_str(),32);
	playername[32]=0;
	instrument  = player[num].instrument;
	difficulty  = player[num].difficulty;
	score       = player[num].score;
	streak      = player[num].stat_longeststreak;
	notestotal  = player[num].stat_notestotal;
	notesgood   = player[num].stat_noteshit;
	xmisses     = player[num].stat_xmiss;
	noteshopo   = player[num].stat_hopos;
	scorenomult = player[num].score_nomult;
	scorehits   = player[num].score_fromhits;
	scorehold   = player[num].score_hold;
	scorewham   = player[num].score_whammy;
	flags       = 0;
}

int gethex(char c)
{
	if (c>='0' && c<='9') return c-'0';
	if (c>='a' && c<='f') return c-'a'+10;
	if (c>='A' && c<='F') return c-'A'+10;
	return -1;
}

int readhash(const char *c, uint8_t *hash)
{
	int i,a,b;
	for (i=0; i<20; i++)
	{
		a=gethex(c[i*2]);
		b=gethex(c[i*2+1]);
		if (a<0 || b<0)
		{
			DBG(SCORES,"Error reading hash at %d\n", i);
			return 0;
		}
		hash[i]=a*16+b;
	}
	return 1;
}

void myprintll(FILE *f, uint64_t val)
{
	char a[20];
	int i=14;
	a[16]=0;
	a[15]=val%10+'0';
	while (val>=10)
	{
		val/=10;
		a[i--]=val%10+'0';
	}
	a[i]=' ';
	fprintf(f,"%s",a+i);
}

void scoreline::write(FILE *f)
{
	int i;
	long long int tmpdate=date;
	for (i=0; i<20; i++) fprintf(f,"%02x",midhash[i]);
	fprintf(f," ");
	sha_buffer((char *) midhash,scorehash-midhash,scorehash);
	for (i=0; i<20; i++) fprintf(f,"%02x",scorehash[i]);
	myprintll(f, tmpdate);
	fprintf(f," %d %d %d %d %d %d %d %d %d %d %d %d %d %s\n",
		instrument,difficulty,flags,
		score,streak,notesgood,notestotal,xmisses,noteshopo,
		scorenomult,scorehits,scorehold,scorewham,
		playername);
//	DBG(SCORES,"%s" &toStringDump());
}

// read long int manually since scanf(%lld) proved not to be portable
uint64_t readnum(char *&p)
{
	uint64_t res=0;
	while (*p==' ' || *p=='\t') p++;
	while (*p>='0' && *p<='9') res=res*10+(*p++)-'0';
	while (*p==' ' || *p=='\t') p++;
	return res;
}

int scoreline::read(FILE *f, int ver)
{
	char s[256],*p,*pe;
	long long int tmpdate;
	uint8_t check[20];
	int res,rr;
	playername[0]=0;
	s[0]=0;
	if (!fgets(s,255,f))
	{
		DBG(SCORES,"Scores: finished reading\n");
		return 0;
	}
	s[255]=0;
	res=readhash(s,midhash) | readhash(s+41,scorehash);
	if (!res)
	{
		DBG(SCORES,"Scores: no hash on this line\n");
		return 0;
	}
	p=s+82;
	date=readnum(p);
	res=sscanf(p," %d %d %d %d %d %d %d %d %d %d %d %d %d %n",
		&instrument, &difficulty, &flags,
		&score, &streak, &notesgood, &notestotal, &xmisses, &noteshopo,
		&scorenomult,&scorehits,&scorehold,&scorewham,
		&rr);
	p+=rr;
	if (res<8)
	{
		DBG(SCORES,"Scores: too few items (%d) on line, chars:%d\n", res, rr);
		DBG(SCORES,"Line was:%s\n", (s+41));
		return 0;
	}
	if (*p==' ') p++;
	pe=p;
	while (*pe)
	{
		if (*pe==0x0a || *pe==0x0d) *pe=0;
		else pe++;
	}
	memset(playername,0,36);
	strncpy((char *)playername,p,32);
	playername[32]=0;
	sha_buffer((char *) midhash,scorehash-midhash,check);

	INFO(SCORES,"Read score: %d %d %s ", score, notesgood, (char *)playername);
	if (0!=memcmp((char *)check,(char *)scorehash,20))
	{
		WARN(SCORES,"-- wrong score hash\n");
//		DBG(SCORES,"%s\n" &toStringDump());
		return 0;
	}
	else INFO(SCORES,"\n");
//	DBG(SCORES,"%s\n" &toStringDump());
	return 1;
}



