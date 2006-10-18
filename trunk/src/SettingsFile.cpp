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
#include <fstream>
#include "SettingsFile.h"

#include <cstdlib>

void CSettingsFile::Set( string Section, string Key, string Value )
{
    m_Settings[Section][Key] = Value;
}

string CSettingsFile::Get( string Section, string Key, string DefaultValue )
{
    if( m_Settings.count( Section ) > 0 )
    {
	if( m_Settings[Section].count( Key ) > 0 )
	{
	    return m_Settings[Section][Key];
	}
    }
    return DefaultValue;
}

int CSettingsFile::GetInt( string Section, string Key, const int& DefaultValue )
{
    if( m_Settings.count( Section ) > 0 )
    {
	if( m_Settings[Section].count( Key ) > 0 )
	{
	    char* p;
	    int res = strtol( m_Settings[Section][Key].c_str(), &p, 0 );
	    if( *p != 0 ) // the whole string was not valid
	    {
		return DefaultValue;
	    }
	    return res;
	}
    }
    return DefaultValue;
}

string CSettingsFile::TrimLeft( string szString )
{
    string out="";

    if( szString.size() == 0 )
    {
	return out;
    }

    bool bStarted = false;

    for( string::iterator it = szString.begin(); it != szString.end(); it++ )
    {
	if( ( !bStarted ) && ( *it != ' ' ) )
	{
	    bStarted = true;
	}

	if( bStarted )
	{
	    out += *it;
	}
    }

    return out;
}

string CSettingsFile::TrimRight( string szString )
{
    string out="";

    if( szString.size() == 0 )
    {
	return out;
    }

    // searching for the first non-space char from the end of the string
    //
    string::iterator it = szString.end()-1;
    unsigned int i = szString.size();
    while( ( *it == ' ' ) && ( it != szString.begin() ) )
    {
	it--;
	i--;
    }

    out = szString.substr( 0, i );

    return out;
}

void CSettingsFile::LoadConfig()
{
    string tmp;

    if( m_szConfigFile.size() == 0 )
    {
	g_Logger.Error( "LoadConfig(): no config file specified!" );
	return;
    }

    if( m_szConfigPath.size() == 0 )
    {
	SearchForConfigFile();
    }

    string szConfigPath = m_szConfigPath + "/" + m_szConfigFile;
    ifstream FileStream( szConfigPath.c_str() );
    if( FileStream.fail() )
    {
	FileStream.close();
	tmp = "can't open config file: " + szConfigPath;
	g_Logger.Error( tmp );
	return;
    }


    string szCurrentSection = "";
    m_Settings.clear();


    char buf[500];
    memset( buf, 0, 500 );

    while( !FileStream.eof() )
    {
	FileStream.getline( buf, 499 );
	tmp = buf;

	tmp = TrimLeft( tmp );

	if( tmp[0] == '#' ) // comments
	{
	    continue;
	}
	if( tmp.size() == 0 ) // empty lines
	{
	    continue;
	}

	if( tmp[0] == '[' ) // section
	{
	    string::size_type loc = tmp.find( ']', 0 );
	    if( loc == string::npos )
	    {
		tmp = tmp.substr( 1, tmp.size()-1 );
	    }
	    else
	    {
		tmp = tmp.substr( 1, loc-1 );
	    }

	    tmp = TrimRight( TrimLeft( tmp ) );

	    if( tmp.size() == 0 ) // invalid section
	    {
		continue;
	    }

	    szCurrentSection = tmp;
	    continue;
	}

	if( szCurrentSection.size() == 0 ) // we're not in a section
	{
	    continue;
	}

	// we're in a section, reading key & value pairs
	string::size_type loc = tmp.find( '=', 0 );
	if( loc == string::npos ) // no '=' in current line -> invalid line
	{
	    continue;
	}

	string key = tmp.substr( 0, loc );
	key = TrimRight( key );
	if( key.size() == 0 )
	{
	    continue;
	}
	string value = tmp.substr( loc+1, tmp.size()-loc-1 );
	value = TrimRight( TrimLeft( value ) );
	if( value.size() == 0 )
	{
	    continue;
	}

	m_Settings[szCurrentSection][key] = value;
    }

    FileStream.close();
}

void CSettingsFile::SearchForConfigFile()
{
    // opening config file in current directory
    //
    string tmp = m_szInitialHomeDir + "/" + m_szConfigFile;
    ifstream FileStream( tmp.c_str() );
    if( FileStream.fail() )
    {
	// trying in $HOME/.videopad-server
	//
	FileStream.close();

	char* pHomeDir = getenv( "HOME" );
	string szHomeDir = pHomeDir;
	tmp = szHomeDir + "/." + PACKAGE + "/" + m_szConfigFile;
	free( pHomeDir );

	FileStream.open( tmp.c_str(), ios::in );
	if( FileStream.fail() )
	{
	    // trying in /etc/videopad-server
	    //
	    FileStream.close();

	    tmp = "/etc/";
	    tmp += PACKAGE;
	    tmp += "/" + m_szConfigFile;
	    FileStream.open( tmp.c_str(), ios::in );
	    if( FileStream.fail() )
	    {
		tmp = "can't find config file: " + m_szConfigFile;
		g_Logger.Error( tmp );

		return;
	    }

	    m_szConfigPath = "/etc/";
	    m_szConfigPath += PACKAGE;
	    FileStream.close();
	    return;
	}

	m_szConfigPath = szHomeDir + "/." + PACKAGE;
	FileStream.close();
	return;
    }

    m_szConfigPath = m_szInitialHomeDir;
    FileStream.close();
}

void CSettingsFile::SetConfigFile( string& szConfigFile )
{
    m_szConfigFile = szConfigFile;
}

void CSettingsFile::SetConfigPath( string& szConfigPath )
{
    m_szConfigPath = szConfigPath;
}

CSettingsFile::CSettingsFile()
{
    char pBuffer[500];

    getcwd( pBuffer, 500 );
    m_szInitialHomeDir = pBuffer;
}

void CSettingsFile::SaveConfig()
{
    ofstream FileStream;
    string tmp;

    if( m_szConfigPath.size() == 0 )
    // we try to search directory where we can write the config
    {
	tmp = "/etc/";
	tmp += PACKAGE;
	tmp += "/" + m_szConfigFile;
	FileStream.open( tmp.c_str() );
	if( FileStream.fail() )
	{
	    FileStream.close();
	    char* pHomeDir = getenv( "HOME" );
	    string szHomeDir = pHomeDir;
	    tmp = szHomeDir + "/." + PACKAGE + "/" + m_szConfigFile;
	    free( pHomeDir );
	    FileStream.open( szHomeDir.c_str() );
	    if( FileStream.fail() )
	    {
		FileStream.close();
		tmp = m_szInitialHomeDir + "/" + m_szConfigFile;
		FileStream.open( tmp.c_str() );
		if( FileStream.fail() )
		{
		    tmp = "can't save config file: ";
		    tmp += m_szConfigFile;
		    g_Logger.Error( tmp );
		    return;
		}
		m_szConfigPath = m_szInitialHomeDir;
	    }
	    else
	    {
		m_szConfigPath = szHomeDir + "/." + PACKAGE;
	    }
	}
	else
	{
	    m_szConfigPath = "/etc/";
	    m_szConfigPath += PACKAGE;
	}
    }
    else
    {
	tmp = m_szConfigPath + "/" + m_szConfigFile;
	FileStream.open( tmp.c_str() );
	if( FileStream.fail() )
	{
	    tmp = "can't save config file: ";
	    tmp += m_szConfigPath + "/" + m_szConfigFile;
	    g_Logger.Error( tmp );
	    return;
	}
    }

    bool fLeadingLine = false;

    for( map<string, t_mKeys>::iterator it1 = m_Settings.begin(); it1 != m_Settings.end(); it1++ )
    {
	if( fLeadingLine )
	{
	    FileStream << endl;
	}
	fLeadingLine = true;

	FileStream << '['  << it1->first << ']' << endl;

	t_mKeys mKeys = it1->second;
	for( t_mKeys::iterator it2 = mKeys.begin(); it2 != mKeys.end(); it2++ )
	{
	    FileStream << it2->first << "=" << it2->second << endl;
	}
    }

    FileStream.close();
}
