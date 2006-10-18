//  This file is part of videopad-server.
//    
//  videopad-server is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  videopad-server is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with videopad-server; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include "Main.h"
#include "StringExt.h"

vector<string> Tokenize( string szLine, unsigned char cDelimiter )
{
    vector<string> vTokens;
    string szToken;
    unsigned int i = 0;
    unsigned int nLastSpacePos = 0;

    for( ; i < szLine.size(); i++ )
    {
	if( ( szLine[i] == cDelimiter ) || ( szLine[i] == '\n' ) ||
	    ( szLine[i] == '\r' ) || ( szLine[i] == ':' ) )
	{
	    szToken = szLine.substr( nLastSpacePos, i-nLastSpacePos );
	    if( szToken.size() > 0 )
	    {
		vTokens.push_back( szToken );
	    }

	    // return the line to the end
	    if( szLine[i] == ':' )
	    {
		break;
	    }

	    nLastSpacePos = i+1;
	}
    }

    // copy the rest of the string
    if( szLine[szLine.size()-1] == '\n' )
    {
	if( szLine[szLine.size()-2] == '\r' )
	{
	    szToken = szLine.substr( nLastSpacePos, szLine.size()-nLastSpacePos-2 );
	}
	else
	{
	    szToken = szLine.substr( nLastSpacePos, szLine.size()-nLastSpacePos-1 );
	}
    }
    else
    {
	szToken = szLine.substr( nLastSpacePos, szLine.size()-nLastSpacePos-1 );
    }

    if( szToken.size() > 0 )
    {
	vTokens.push_back( szToken );
    }
    return vTokens;
}

bool Compare( string s1, string s2 )
{
    string::const_iterator it1=s1.begin();
    string::const_iterator it2=s2.begin();

    while ( (it1 != s1.end() ) && ( it2 != s2.end() ) ) 
    { 
	if ( ::toupper(*it1) != ::toupper(*it2) )
	{
	    return false;
	}

	++it1;
	++it2;
    }

    if ( s1.size() == s2.size() )
    {
	return true;
    }
    return false;
}

string ToLower( string szString )
{
    string res;

    for( string::iterator it = szString.begin(); it != szString.end(); it++ )
    {
	res += tolower( *it );
    }

    return res;
}
