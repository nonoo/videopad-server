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
#include "Client.h"

#include <sys/socket.h>

CClient::CClient()
{
    m_fConnectionClosed = false;
    m_tTimeConnected = time( NULL );
    m_fLoggedin = false;
    m_fMOTDSent = false;
    m_fOperator = false;
    m_fPingSent = false;
    m_tLastPingTime = time( NULL );
    m_nSocket = 0;
    m_pDataConnection = NULL;
    m_pVideoStream = new CVideoStream;
    m_pAudioStream = new CAudioStream;

    if( g_SettingsFile_Server.Get( "server settings", "server-password", "" ) == "" )
    {
	m_fPassGiven = true;
    }
    else
    {
	m_fPassGiven = false;
    }
}

CClient::~CClient()
{
    SAFE_DELETE( m_pDataConnection );
    SAFE_DELETE( m_pVideoStream );
    SAFE_DELETE( m_pAudioStream );

    shutdown( m_nSocket, SHUT_RDWR );
    close( m_nSocket );
}

void CClient::SetIP( string szIP )
{
    m_szIP = szIP;
}

const string& CClient::GetIP()
{
    return m_szIP;
}

void CClient::SetHost( string szHost )
{
    m_szHost = szHost;
}

const string& CClient::GetHost()
{
    return m_szHost;
}

void CClient::SetSocket( unsigned int nSocket )
{
    m_nSocket = nSocket;
}

const unsigned int& CClient::GetSocket()
{
    return m_nSocket;
}

const time_t& CClient::GetTimeConnected()
{
    return m_tTimeConnected;
}

const bool& CClient::IsLoggedin()
{
    return m_fLoggedin;
}

void CClient::SetLoggedin( bool fState )
{
    m_fLoggedin = fState;
}

const bool& CClient::IsMOTDSent()
{
    return m_fMOTDSent;
}

void CClient::SetMOTDSent( bool fState )
{
    m_fMOTDSent = fState;
}

const bool& CClient::IsConnectionClosed()
{
    return m_fConnectionClosed;
}

void CClient::SendMessage( string szMessage )
{
    if( szMessage[szMessage.size()-1] != '\n' )
    {
	szMessage += "\r\n";
    }

    int res = send( m_nSocket, szMessage.c_str(), szMessage.size(), 0 );

    if( res <= 0 )
    {
	m_fConnectionClosed = true;
    }
    if( res < 0 )
    {
	m_szQuitMessage = "Connection reset by peer";
    }
}

void CClient::SendData( char* pData, unsigned int nDataSize )
{
    int res = send( m_pDataConnection->GetSocket(), pData, nDataSize, 0 );

    if( res <= 0 )
    {
	m_fConnectionClosed = true;
    }
    if( res < 0 )
    {
	m_szQuitMessage = "Data connection reset by peer";
    }
}

void CClient::SetQuitMessage( string szQuitMessage )
{
    m_szQuitMessage = szQuitMessage;
}

const string& CClient::GetQuitMessage()
{
    return m_szQuitMessage;
}

const string& CClient::GetNick()
{
    return m_szNick;
}

int CClient::SetNick( string szNick )
{
    if( szNick.size() > MAXNICKLENGTH )
    {
	return -1;
    }

    // checking for invalid characters
    for( string::iterator it = szNick.begin(); it != szNick.end(); it++ )
    {
	if( *it < 48 )
	{
	    return -2;
	}

	if( ( *it > 57 ) && ( *it < 65 ) )
	{
	    return -2;
	}

	if( *it > 125 )
	{
	    return -2;
	}
    }

    m_szNick = szNick;

    return 0;
}

void CClient::SetOperator( bool fState )
{
    m_fOperator = fState;
}

const bool& CClient::IsOperator()
{
    return m_fOperator;
}

const bool& CClient::GetPassGiven()
{
    return m_fPassGiven;
}

void CClient::SetPassGiven( bool fState )
{
    m_fPassGiven = fState;
}

const string& CClient::GetTagline()
{
    return m_szTagline;
}

void CClient::SetTagline( string szTagline )
{
    m_szTagline = szTagline;
}

const string& CClient::GetRealName()
{
    return m_szRealName;
}

void CClient::SetRealName( string szRealName )
{
    m_szRealName = szRealName;
}

void CClient::SetLastPingTime( time_t tLastPingTime )
{
    m_tLastPingTime = tLastPingTime;
}

const time_t& CClient::GetLastPingTime()
{
    return m_tLastPingTime;
}

const bool& CClient::IsPingSent()
{
    return m_fPingSent;
}

void CClient::SetPingSent( bool fState )
{
    m_fPingSent = fState;
}

CVideoStream* CClient::GetVideoStream()
{
    return m_pVideoStream;
}

CAudioStream* CClient::GetAudioStream()
{
    return m_pAudioStream;
}

void CClient::SetDataConnection( CDataConnection* pDataConnection )
{
    m_pDataConnection = pDataConnection;
}

CDataConnection* CClient::GetDataConnection()
{
    return m_pDataConnection;
}

COggDecoder* CClient::GetOggDecoder()
{
    if( m_pDataConnection != NULL )
    {
	return m_pDataConnection->GetOggDecoder();
    }
    g_Logger.Error( "can't get ogg decoder, this shouldn't happen" );
    return NULL;
}

unsigned int CClient::GetDataSocket()
{
    if( m_pDataConnection != NULL )
    {
	return m_pDataConnection->GetSocket();
    }
    return 0;
}
