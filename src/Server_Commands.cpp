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
#include "Server.h"
#include "StringExt.h"

#include <sstream>

void CServer::Whois( CClient*& pClient, string szParam )
{
    CClient* pWhoisClient = NULL;

    for( vector<CClient*>::iterator it = m_ClientVector.begin(); it != m_ClientVector.end(); it++ )
    {
        if( Compare( (*it)->GetNick(), szParam ) )
        {
	    pWhoisClient = *it;
	    break;
	}
    }

    if( pWhoisClient == NULL )
    {
	pClient->SendMessage( "707 " + szParam );
	return;
    }

    pClient->SendMessage( "400 " + szParam + " " + pWhoisClient->GetIP() );
    pClient->SendMessage( "401 " + szParam + " " + pWhoisClient->GetHost() );

    if( pWhoisClient->GetRealName().size() > 0 )
    {
	pClient->SendMessage( "402 :" + szParam + " " + pWhoisClient->GetRealName() );
    }

    if( pWhoisClient->GetTagline().size() > 0 )
    {
	pClient->SendMessage( "403 :" + szParam + " " + pWhoisClient->GetTagline() );
    }

    // constructing channel list
    string szChanList;
    for( tChannelMap::iterator it = m_ChannelMap.begin(); it != m_ChannelMap.end(); it++ )
    {
	CChannel* pChannel = it->second;

	if( pChannel->IsOn( pWhoisClient ) )
	{
	    szChanList += pChannel->GetName() + " ";
	}
    }

    if( szChanList.size() > 0 )
    {
	pClient->SendMessage( "404 " + szParam + " :" + szChanList );
    }

    stringstream ss;
    ss << pWhoisClient->GetTimeConnected();
    string res;
    ss >> res;
    pClient->SendMessage( "405 " + szParam + " " + res );
}

void CServer::NickChange( CClient*& pClient, string szParam )
{
    if( szParam.size() == 0 )
    {
	return;
    }

    // if the client don't want to change the case of it's nick
    // (wants to change to another name)
    if( !Compare( pClient->GetNick(), szParam ) )
    {
	// checking nick collision
	for( vector<CClient*>::iterator it = m_ClientVector.begin(); it != m_ClientVector.end(); it++ )
	{
	    if( Compare( (*it)->GetNick(), szParam ) )
	    {
		pClient->SendMessage( "705 " + szParam );
		return;
	    }
	}
    }    

    // if the old and the new nick is exactly the same
    if( pClient->GetNick() == szParam )
    {
	pClient->SendMessage( "705 " + szParam );
	return;
    }

    string szOldNick = pClient->GetNick();
    int res = pClient->SetNick( szParam );

    if( res == 0 )
    {
	// valid nick, sending a nick change message to the client
	pClient->SendMessage( "502 " + pClient->GetNick() );

	// sending nick change message to the channels user is on
	//
	for( tChannelMap::iterator it = m_ChannelMap.begin(); it != m_ChannelMap.end(); it++ )
	{
    	    CChannel* pChannel = it->second;
    	    if( pChannel->IsOn( pClient ) )
    	    {
		string msg = "306 " + pChannel->GetName() + " " + szOldNick + " " + pClient->GetNick();
		pChannel->SendMessage( msg );
	    }
	}
    }
    else
    {
	if( res == -1 )
	{
	    // nick is too long
	    string err;
	    stringstream ss;
	    ss << MAXNICKLENGTH;
	    ss >> err;
	    err = "706 " + szParam + " " + err;
	    pClient->SendMessage( err );
	    return;
	}
	// invalid chars in nick
	string err = "704 " + szParam;
	pClient->SendMessage( err );
    }
}

void CServer::JoinChannel( CClient*& pClient, string szChannelName )
{
    if( ( szChannelName.size() <= 1 ) ||
	( szChannelName[0] != '#' ) )
    {
	string err = "700 " + szChannelName;
	pClient->SendMessage( err );
	return;
    }

    // if the client has already joined a channel
/*    for( tChannelMap::iterator it = m_ChannelMap.begin(); it != m_ChannelMap.end(); it++ )
    {
        CChannel* pChannel = it->second;

	if( pChannel->IsOn( pClient ) )
	{
	    // can't join to the same channel again
	    if( Compare( pChannel->GetName(), szChannelName ) )
	    {
		return;
	    }

	    PartChannel( pClient, pChannel->GetName() );
	    break;
	}
    }*/

    if( m_ChannelMap.count( szChannelName ) == 0 )
    {
	// channel does not exist, we create it
	CChannel* pNewChannel = new CChannel( szChannelName );
	m_ChannelMap[szChannelName] = pNewChannel;
    }
    m_ChannelMap[szChannelName]->AddClient( pClient );
}

void CServer::PartChannel( CClient*& pClient, string szChannelName )
{
    if( m_ChannelMap.count( szChannelName ) == 0 )
    {
	// user is not on that channel
	string err = "701 " + szChannelName;
	pClient->SendMessage( err );
	return;
    }

    string pmsg = "305 " + szChannelName + " " + pClient->GetNick();
    m_ChannelMap[szChannelName]->SendMessage( pmsg );

    m_ChannelMap[szChannelName]->DeleteClient( pClient );

    // if the channel became empty
    if( m_ChannelMap[szChannelName]->ClientNum() == 0 )
    {
	DeleteChannel( m_ChannelMap[szChannelName] );
    }
}
