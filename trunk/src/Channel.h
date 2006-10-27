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

#ifndef __CHANNEL_H
#define __CHANNEL_H

#include "Client.h"

#include <vector>
#include <string>
#include <map>

class CChannel
{
public:
    CChannel( string szName );

    const string& 	GetName();
    void 		AddClient( CClient*& pClient );
    void 		DeleteClient( CClient*& pClient );
    bool 		IsOn( CClient*& pClient );
    int 		ClientNum();

			// this sends message to all of the channel members
			// except given client
			//
    void 		SendMessage2( string szMessage, CClient*& pClient );

    void 		SendMessage( string szMessage );
    void		SendNickList( CClient*& pClient );

			// sends the given data to all of the channel's
			// members except sender
    void		BroadcastData( char* pData, unsigned int nDataSize,
			    CClient* pSenderClient );

private:
    string 		m_szName;
    time_t 		m_tTimeCreated;
    CClient* 		m_pCreator;

    typedef vector<CClient*>	tClientVector;
    tClientVector		m_ClientVector;
    typedef map<CClient*,char>	tFlagMap;
    tFlagMap			m_FlagMap;
};

#endif
