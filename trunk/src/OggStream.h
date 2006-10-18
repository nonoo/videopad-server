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

#ifndef __OGGSTREAM_H
#define __OGGSTREAM_H

#include <vector>

#include "ogg/ogg.h"

class COggStream
{
public:
    COggStream();
    ~COggStream();

    const unsigned int&	GetSerial();
    void 		SetSerial( const unsigned int& nSerial );

    void		FeedPage( ogg_page& OggPage );

			// returns ogg_header+ogg_packet
			// returned pointer must be deleted after use
			//
    char*		GetRawPage();
    unsigned int	GetRawPageSize();

protected:
    bool		IsHeaderPacket( ogg_packet* pOggPacket );
    void		StoreHeaderPacket( ogg_packet* pOggPacket );




    unsigned int		m_nSerial;
    ogg_stream_state*		m_pStreamState;
    ogg_page*			m_pOggPage;
    vector< ogg_packet* >	m_HeaderVector;
    // becomes true when we process the first non-header packet
    // this means we got all of the header packets in our header vector
    bool			m_fInitialized;
};

#endif
