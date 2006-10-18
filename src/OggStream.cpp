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
#include "OggStream.h"

COggStream::COggStream()
{
    m_fInitialized = false;
    m_pStreamState = (ogg_stream_state *)malloc( sizeof(ogg_stream_state) );
}

COggStream::~COggStream()
{
    ogg_stream_destroy( m_pStreamState );

    // freeing elements of headervector
    for( vector< ogg_packet* >::iterator it = m_HeaderVector.begin();
	it != m_HeaderVector.end(); it++ )
    {
	free( *it );
    }
}

const unsigned int& COggStream::GetSerial()
{
    return m_nSerial;
}

void COggStream::SetSerial( const unsigned int& nSerial )
{
    m_nSerial = nSerial;
    ogg_stream_init( m_pStreamState, m_nSerial );
}

void COggStream::FeedPage( ogg_page& OggPage )
{
    m_pOggPage = &OggPage;
    ogg_stream_pagein( m_pStreamState, &OggPage );

    if( !m_fInitialized )
    {
	// processing packets if the stream hasn't been initalized yet
	// (we haven't got the header packets yet)
	//
	ogg_packet* pOggPacket = (ogg_packet *)malloc( sizeof( ogg_packet ) );
	while( ogg_stream_packetout( m_pStreamState, pOggPacket ) == 1 )
        {
	    // got a packet
	    if( IsHeaderPacket( pOggPacket ) )
	    {
		StoreHeaderPacket( pOggPacket );
	    }
	    else
	    {
		// we got a non-header packet
		// this means we have all header packets in our header vector
		//
		SAFE_DELETE( pOggPacket );
		m_fInitialized = true;
	    }
	}
    }
}

bool COggStream::IsHeaderPacket( ogg_packet* pOggPacket )
{
/*    if( m_pOggPacket->packetno == 0 )
    {
	return true;
    }
    return false;*/
    return ( pOggPacket->packet[0] & 0x80 ) ? 1 : 0;
}

void COggStream::StoreHeaderPacket( ogg_packet* pOggPacket )
{
    m_HeaderVector.push_back( pOggPacket );
}

char* COggStream::GetRawPage()
{
    char* pData = new char[ m_pOggPage->header_len + m_pOggPage->body_len ];
    memcpy( pData, m_pOggPage->header, m_pOggPage->header_len );
    memcpy( pData+m_pOggPage->header_len, m_pOggPage->body, m_pOggPage->body_len );
    return pData;
}

unsigned int COggStream::GetRawPageSize()
{
    return m_pOggPage->header_len + m_pOggPage->body_len;
}
