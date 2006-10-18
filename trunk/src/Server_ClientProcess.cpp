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

#include <sys/types.h>
#include <sys/socket.h>
#include <cerrno>
#include <sstream>

void CServer::ReadClientMessage( CClient*& pClient )
{
    int res = recv( pClient->GetSocket(), m_pReadBuf+m_nReadBufPos, MAXMESSAGELENGTH-m_nReadBufPos, MSG_DONTWAIT );

    if( ( res <= 0 ) && ( errno != EAGAIN ) )
    {
	DeleteClient( pClient );
	return;
    }

    m_nReadBufPos += res;

    if( ( m_pReadBuf[m_nReadBufPos-1] == '\n' ) ||
	( m_nReadBufPos == MAXMESSAGELENGTH ) )
    {
	ProcessClientMessage( pClient, m_pReadBuf );

	memset( m_pReadBuf, 0, MAXMESSAGELENGTH+1 );
	m_nReadBufPos = 0;

	// if the client has been dropped
	if( pClient == NULL )
	{
	    return;
	}
    }
}

void CServer::ProcessClientMessage( CClient*& pClient, string szLine )
{
    vector<string> vTokens = Tokenize( szLine, ' ' );

    if( vTokens.size() == 0 )
    {
	return;
    }
    
    string szCommand = vTokens[0];
    string szParam;

    if( vTokens.size() > 1 )
    {
	szParam = vTokens[1];
    }

    // szText is the string after : from the last token
    string szText;
    if( vTokens[vTokens.size()-1][0] == ':' )
    {
	szText = vTokens[vTokens.size()-1].substr( 1, vTokens[vTokens.size()-1].size()-1 );
    }

    if( pClient->IsLoggedin() == false )
    {
        if( Compare( szCommand, "PASS" ) )
        {
	    if( szParam == g_SettingsFile_Server.Get( "server settings", "server-password", "" ) )
	    {
	        // pass ok
	        pClient->SetPassGiven( true );
	    }
	    else
	    {
		// wrong pass
		pClient->SendMessage( "007 :Wrong password, bye" );
		DeleteClient( pClient );
		return;
	    }
	}

	if( Compare( szCommand, "NICK" ) )
	{
	    NickChange( pClient, szParam );
	}

	if( Compare( szCommand, "QUIT" ) )
	{
	    DeleteClient( pClient );
	    return;
	}

	if( Compare( szCommand, "PONG" ) )
	{
	    pClient->SetPingSent( false );
	    pClient->SetLastPingTime( time( NULL ) );
	    return;
	}

	if( ( pClient->GetNick().size() > 0 ) && ( pClient->GetPassGiven() ) )
	{
	    // sending allocated serial numbers
	    //
	    stringstream s1;
	    stringstream s2;
	    int r1 = m_SerialMapper.GetNewVideoSerial( pClient );
	    int r2 = m_SerialMapper.GetNewAudioSerial( pClient );
	    s1 << r1;
	    s2 << r2;
	    string sz1;
	    string sz2;
	    s1 >> sz1;
	    s2 >> sz2;
	    
	    pClient->SendMessage( "008 " + sz1 + " " + sz2 );

	    pClient->SetLoggedin( true );
	}
	return;
    }

    if( Compare( szCommand, "QUIT" ) )
    {
	if( szText.size() > 0 )
	{
	    pClient->SetQuitMessage( szText );
	}

	DeleteClient( pClient );
	return;
    }

    if( Compare( szCommand, "JOIN" ) )
    {
	JoinChannel( pClient, ToLower( szParam ) );
	return;
    }

    if( Compare( szCommand, "PART" ) )
    {
	PartChannel( pClient, szParam );
	return;
    }

    // channel msg
    if( Compare( szCommand, "MSG" ) )
    {
	if( szText.size() == 0 ) // no message
	{
	    return;
	}

	// channel names are always lowercase
	szParam = ToLower( szParam );

	// channel exists?
	if( m_ChannelMap.count( szParam ) == 0 )
	{
	    string err = "702 " + szParam;
	    pClient->SendMessage( err );
	    return;
	}

	// sending channel msg
	string msg = "500 " + m_ChannelMap[szParam]->GetName() + " " + pClient->GetNick() + " :";
	msg += szText;
	m_ChannelMap[szParam]->SendMessage2( msg, pClient );

	return;
    }

    // private msg
    if( Compare( szCommand, "PRIVMSG" ) )
    {
	if( szText.size() == 0 ) // no message
	{
	    return;
	}

	// channel names are always lowercase
	szParam = ToLower( szParam );

	// searching for the destination's *CClient
	bool fMSGSent = false;
	for( vector<CClient*>::iterator it = m_ClientVector.begin(); it != m_ClientVector.end(); it++ )
	{
	    if( Compare( (*it)->GetNick(), szParam ) )
	    {
		string msg = "501 " + pClient->GetNick() + " :" + szText;
		(*it)->SendMessage( msg );
		fMSGSent = true;
		break;
	    }
	}

	if( !fMSGSent )
	{
	    // user does not exist
	    pClient->SendMessage( "703 " + szParam );
	    return;
	}

	return;
    }

    if( Compare( szCommand, "NICK" ) )
    {
	NickChange( pClient, szParam );
	return;
    }

    if( Compare( szCommand, "WHOIS" ) )
    {
	Whois( pClient, szParam );
    }

    if( Compare( szCommand, "TAGLINE" ) )
    {
	pClient->SetTagline( szText );
    }

    if( Compare( szCommand, "REALNAME" ) )
    {
	pClient->SetRealName( szText );
    }

    if( Compare( szCommand, "PONG" ) )
    {
	pClient->SetPingSent( false );
	pClient->SetLastPingTime( time( NULL ) );
    }
}

void CServer::DeleteChannel( CChannel*& pChannel )
{
    // deleting channel from channelmap
    for( tChannelMap::iterator it = m_ChannelMap.begin(); it != m_ChannelMap.end(); it++ )
    {
	if( it->second == pChannel )
	{
	    m_ChannelMap.erase( it );
	    break;
	}
    }

    SAFE_DELETE( pChannel );
}

void CServer::DeleteClient( CClient*& pClient )
{
    // deleting client from channels it is on
    for( tChannelMap::iterator it = m_ChannelMap.begin(); it != m_ChannelMap.end(); it++ )
    {
        CChannel* pChannel = it->second;
        if( pChannel->IsOn( pClient ) )
        {
    	    // sending message to the channel that the client has left
    	    string qmsg = "304 " + pChannel->GetName() + " " + pClient->GetNick() + " :" + pClient->GetQuitMessage();
    	    pChannel->SendMessage( qmsg );

	    pChannel->DeleteClient( pClient );
	    if( pChannel->ClientNum() == 0 )
	    {
		DeleteChannel( pChannel );
		if( m_ChannelMap.size() == 0 )
		{
		    break;
		}
		it = m_ChannelMap.begin();
	    }
	}
    }


    // deleting client from clientvector
    for( vector<CClient*>::iterator it = m_ClientVector.begin(); it != m_ClientVector.end(); it++ )
    {
	if( *it == pClient )
	{
	    m_ClientVector.erase( it );
	    break;
	}
    }

    // deleting client's serials
    m_SerialMapper.DeleteClientSerials( pClient );

    SAFE_DELETE( pClient );
}

void CServer::CheckForConnectionStatus( CClient*& pClient )
{
    if( pClient->IsConnectionClosed() )
    {
	DeleteClient( pClient );
	return;
    }

    if( ( pClient->IsLoggedin() ) && ( !pClient->IsMOTDSent() ) )
    {
	SendMOTD( pClient );
	pClient->SetMOTDSent( true );
	pClient->SendMessage( "009 :Login process completed" );
	return;
    }

    if( pClient->IsPingSent() )
    {
	if( pClient->GetLastPingTime() + g_SettingsFile_Server.GetInt( "server settings", "timeout-interval", 30 ) < time( NULL ) )
	{
	    pClient->SetQuitMessage( "Ping timeout" );
	    DeleteClient( pClient );
	    return;
	}
    }
    else
    {
	if( pClient->GetLastPingTime() + g_SettingsFile_Server.GetInt( "server settings", "ping-interval", 180 ) < time( NULL ) )
	{
	    pClient->SetPingSent( true );
	    pClient->SetLastPingTime( time( NULL ) );
	    pClient->SendMessage( "503" ); // ping
	}
    }
}

void CServer::CheckForLoginTimeout( CClient*& pClient )
{
    if( g_SettingsFile_Server.Get( "server settings", "server-password", "" ).size() > 0 )
    {
	if( ( !pClient->IsLoggedin() ) &&
	    ( time(NULL) - pClient->GetTimeConnected() > g_SettingsFile_Server.GetInt( "server settings", "login-timeout", 60 ) ) )
	{
	    pClient->SendMessage( "011 :Login timeout, bye" );
	    DeleteClient( pClient );
	}
    }
}
