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

#ifndef __SETTINGSFILE_H
#define __SETTINGSFILE_H

#include <string>
#include <map>

using namespace std;

// config file settings manager class
//
class CSettingsFile
{
public:
    CSettingsFile();

    // if config path is not set, loadconfig tries to
    // figure out where the config file is
    //
    void	SetConfigPath( string& szConfigPath );
    void	SetConfigFile( string& szConfigFile );

    void	LoadConfig();
    void	SaveConfig();

    string	Get( string Section, string Key, string DefaultValue );
    int		GetInt( string Section, string Key, const int& DefaultValue );
    void	Set( string Section, string Key, string Value );

private:
    // removes whitespaces
    //
    string	TrimLeft( string szString );
    string	TrimRight( string szString );

    void	SearchForConfigFile();




    string	m_szInitialHomeDir;
    string	m_szConfigFile;
    string	m_szConfigPath;

    // ini structure in memory
    // m_Settings[section][key] == value
    //
    typedef map< string, string >	t_mKeys;
    map< string, t_mKeys >		m_Settings;
};

#endif
