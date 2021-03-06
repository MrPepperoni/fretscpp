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

#include "songsdb.h"

#include "fileio.h"
#include "configuration.h"
#include "message.h"
#include "verbosity.h"

vector<songentry> songsdb;

void readsongsdir(string songsroot)
{
	vector<string> dirs=listDirContents(songsroot);
	int i;
	INFO(SONGDB,"Updating songs database, songs root is %s\n", songsroot);
	sort(dirs.begin(),dirs.end());
	for (i=0; i<dirs.size(); i++)
	{
		int haslib=(fileExists(songsroot+"/"+dirs[i]+"/library.ini"));
		int hasini=(fileExists(songsroot+"/"+dirs[i]+"/song.ini"));
		int hasmid=(fileExists(songsroot+"/"+dirs[i]+"/notes.mid"));

		if (haslib)
		{
			readsongsdir(songsroot+"/"+dirs[i]);
			continue;
		}

		DBG(SONGDB,"Song %s hasini:%d hasmid:%d\n", dirs[i], hasini, hasmid);
		if (hasini && hasmid) songsdb.push_back(songentry(dirs[i],songsroot+"/"+dirs[i],1));
	}
	INFO(SONGDB,"Song database contains %d entries\n", (int)songsdb.size());
}

void updatesongdb()
{
	readsongsdir(homeconfdir+"/data/songs");
	readsongsdir("/usr/share/games/fretsonfire/data/songs");
	readsongsdir("/usr/share/games/fretscpp/data/songs");
	readsongsdir("data/songs");
	if (!songsdb.size()) songsdb.push_back(songentry("- library is empty -"));
}


