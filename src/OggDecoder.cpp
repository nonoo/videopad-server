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
#include "OggDecoder.h"

COggDecoder::COggDecoder()
{
    m_pSyncState = (ogg_sync_state *)malloc( sizeof(ogg_sync_state) );
    ogg_sync_init( m_pSyncState );
}

COggDecoder::~COggDecoder()
{
    free( m_pSyncState->data );
    ogg_sync_init( m_pSyncState );
    ogg_sync_destroy( m_pSyncState );
}

char* COggDecoder::GetBuffer( const unsigned int& nBufferSize )
{
    //ogg_sync_clear( m_pSyncState );
    //ogg_sync_reset( m_pSyncState );
    return ogg_sync_buffer( m_pSyncState, nBufferSize );
}

void COggDecoder::Wrote( const unsigned int& nBytes )
{
    ogg_sync_wrote( m_pSyncState, nBytes );
}

ogg_page* COggDecoder::PageOut()
{
    if( ogg_sync_pageout( m_pSyncState, &m_OggPage ) == 1 )
    {
	return &m_OggPage;
    }
    return NULL;
}

int COggDecoder::GetCurrentPageSerial()
{
    return ogg_page_serialno( &m_OggPage );
}
