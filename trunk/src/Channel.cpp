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
#include "Channel.h"

#include <sstream>

CChannel::CChannel( string szName )
{
    m_tTimeCreated = time( NULL );
    m_szName = szName;
    m_pCreator = NULL;
}

void CChannel::AddClient( CClient*& pClient )
{
    if( !IsOn( pClient ) )
    {
	// setting channel creator
	if( m_ClientVector.size() == 0 )
	{
	    m_pCreator = pClient;
	    m_FlagMap[pClient] = '.';
	}
	else
	{
	    if( pClient->IsOperator() )
	    {
		m_FlagMap[pClient] = '@';
	    }
	    else
	    {
		// default flag
		m_FlagMap[pClient] = ' ';
	    }
	}

	m_ClientVector.push_back( pClient );

	// sending a join message to all of the channel's members
	string msg = "300 " + m_szName + " " + pClient->GetNick();
	SendMessage( msg );
    }

    string msg;
    stringstream ss;
    ss << m_tTimeCreated;
    ss >> msg;
    msg = "301 " + m_szName + " " + msg;
    pClient->SendMessage( msg );

    SendNickList( pClient );

    if( m_pCreator != NULL )
    {
	msg = "303 " + m_szName + " ";
	msg += m_pCreator->GetNick();
	pClient->SendMessage( msg );
    }
}

void CChannel::SendNickList( CClient*& pClient )
{
    string msg = "302 " + m_szName + " :";

    // constructing nicklist
    for( tClientVector::iterator it = m_ClientVector.begin(); it != m_ClientVector.end(); it++ )
    {
	CClient*& itClient = *it;
	char cClientFlag = m_FlagMap[itClient];
	if( cClientFlag != ' ' )
	{
	    msg += cClientFlag + itClient->GetNick() + " ";
	}
	else
	{
	    msg += itClient->GetNick() + " ";
	}
    }

    pClient->SendMessage( msg );
}

void CChannel::DeleteClient( CClient*& pClient )
{
    // deleting from clientvector
    for( tClientVector::iterator it = m_ClientVector.begin(); it != m_ClientVector.end(); it++ )
    {
	if( *it == pClient )
	{
	    m_ClientVector.erase( it );
	    break;
	}
    }

    // deleting from flagmap
    for( tFlagMap::iterator it = m_FlagMap.begin(); it != m_FlagMap.end(); it++ )
    {
	if( (*it).first == pClient )
	{
	    m_FlagMap.erase( it );
	    break;
	}
    }

    if( m_pCreator == pClient )
    {
	m_pCreator = NULL;
    }
}

bool CChannel::IsOn( CClient*& pClient )
{
    for( tClientVector::iterator it = m_ClientVector.begin(); it != m_ClientVector.end(); it++ )
    {
	if( *it == pClient )
	{
	    return true;
	}
    }
    return false;
}

void CChannel::SendMessage( string szMessage )
{
    for( tClientVector::iterator it = m_ClientVector.begin(); it != m_ClientVector.end(); it++ )
    {
	(*it)->SendMessage( szMessage );
    }
}

void CChannel::SendMessage2( string szMessage, CClient*& pClient )
{
    for( tClientVector::iterator it = m_ClientVector.begin(); it != m_ClientVector.end(); it++ )
    {
	if( *it != pClient )
	{
	    (*it)->SendMessage( szMessage );
	}
    }
}

int CChannel::ClientNum()
{
    return m_ClientVector.size();
}

const string& CChannel::GetName()
{
    return m_szName;
}

void CChannel::BroadcastData( char* pData, unsigned int nDataSize, CClient* pSenderClient )
{
    for( tClientVector::iterator it = m_ClientVector.begin(); it != m_ClientVector.end(); it++ )
    {
	if( *it != pSenderClient )
	{
	    (*it)->SendData( pData, nDataSize );
	}
    }
}
