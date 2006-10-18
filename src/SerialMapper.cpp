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
#include "SerialMapper.h"

CSerialMapper::CSerialMapper()
{
    srand( time( NULL ) );
}

const unsigned int CSerialMapper::GetNewVideoSerial( CClient* pClient )
{
    unsigned int r = rand();

    while( m_SerialMap.count( r ) > 0 )
    {
	r = rand();
    }

    pClient->GetVideoStream()->SetSerial( r );
    m_SerialMap[r].first = pClient;
    m_SerialMap[r].second = pClient->GetVideoStream();
    
    return r;
}

const unsigned int CSerialMapper::GetNewAudioSerial( CClient* pClient )
{
    unsigned int r = rand();

    while( m_SerialMap.count( r ) > 0 )
    {
	r = rand();
    }

    pClient->GetAudioStream()->SetSerial( r );
    m_SerialMap[r].first = pClient;
    m_SerialMap[r].second = pClient->GetAudioStream();
    
    return r;
}

void CSerialMapper::DeleteClientSerials( CClient* pClient )
{
    for( tSerialMap::iterator it = m_SerialMap.begin();
	it != m_SerialMap.end(); it++ )
    {
	if( it->second.first == pClient )
	{
	    m_SerialMap.erase( it );
	}
    }
}

CClient* CSerialMapper::GetClient( const unsigned int& nSerial )
{
    return m_SerialMap[nSerial].first;
}

COggStream* CSerialMapper::GetOggStream( const unsigned int& nSerial )
{
    return m_SerialMap[nSerial].second;
}
