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
    m_pStreamState = (ogg_stream_state *) malloc( sizeof( ogg_stream_state ) );
    memset( m_pStreamState, 0, sizeof( ogg_stream_state ) );
}

COggStream::~COggStream()
{
    ogg_stream_destroy( m_pStreamState );

    // freeing elements of headervector
    for( vector< ogg_page* >::iterator it = m_HeaderVector.begin();
	it != m_HeaderVector.end(); it++ )
    {
	ogg_page* tmpOggPage = *it;
	SAFE_DELETE_ARRAY( tmpOggPage->header );
	SAFE_DELETE_ARRAY( tmpOggPage->body );
	SAFE_DELETE( tmpOggPage );
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

    if( !m_fInitialized )
    {
	// processing packets if the stream hasn't been initalized yet
	// (we haven't got the first non-header packet yet)
	//
	ogg_stream_pagein( m_pStreamState, &OggPage );

	ogg_packet* pOggPacket = (ogg_packet *) malloc( sizeof( ogg_packet ) );
	memset( pOggPacket, 0, sizeof( ogg_packet ) );
	pOggPacket->granulepos = 1; // because packetout doesn't fill out this
	ogg_stream_packetout( m_pStreamState, pOggPacket );

        // got a packet
	if( pOggPacket->granulepos == 0 )
	{
	    StoreHeaderPage( m_pOggPage );
	    cout << m_nSerial << ": got header" << endl;
	}
	else
	{
	    cout << m_nSerial << ": got non-header" << endl;
	    // we got a non-header packet
	    // this means we have all header pages in our header vector
	    //
	    m_fInitialized = true;
	}
	free( pOggPacket );
    }
}

bool COggStream::IsHeaderPacket( ogg_packet* pOggPacket )
{
    return ( pOggPacket->granulepos == 0 ) ? true : false;
}

void COggStream::StoreHeaderPage( ogg_page* pOggPage )
{
    ogg_page* newOggPage;
    newOggPage = new ogg_page;
    newOggPage->header = new unsigned char[pOggPage->header_len];
    newOggPage->body = new unsigned char[pOggPage->body_len];
    newOggPage->header_len = pOggPage->header_len;
    newOggPage->body_len = pOggPage->body_len;

    memcpy( newOggPage->header, pOggPage->header, pOggPage->header_len );
    memcpy( newOggPage->body, pOggPage->body, pOggPage->body_len );

    m_HeaderVector.push_back( newOggPage );
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
