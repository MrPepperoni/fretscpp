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

#include "utils.h"

#include "includes.h"
#include "message.h"
#include "verbosity.h"

double gettimefloat()
{
	timeval t;
#ifdef _WINDOWS
	return SDL_GetTicks()*0.001;
#else
	gettimeofday(&t,NULL);
	return t.tv_sec*1.0+t.tv_usec*0.000001;
#endif
}

double origtime=gettimefloat();

double gettimenow()
{
	double v=gettimefloat();
	return v-origtime;
}

void mcolor::set()
{
	glColor4f(Red,Green,Blue,Alpha);
}

string toString_int(int v)
{
	char a[32];
	sprintf(a,"%d",v);
	return a;
}

vector<string> split_string(string s, char c)
{
	int pos=0,mark;
	vector<string> res;
	while (pos<s.length())
	{
		while (pos<s.length() && s[pos]==c) pos++;
		if (pos>=s.length()) break;
		mark=pos+1;
		while (mark<s.length() && s[mark]!=c) mark++;
		res.push_back(s.substr(pos,mark-pos));
		pos=mark;
	}
	return res;
}

