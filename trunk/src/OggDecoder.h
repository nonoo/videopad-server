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

#ifndef __OGGDECODER_H
#define __OGGDECODER_H

#include "ogg/ogg.h"

class COggDecoder
{
public:
    COggDecoder();
    ~COggDecoder();

		// returns the buffer where raw ogg data should be stored
		//
    char*	GetBuffer( const unsigned int& nBufferSize );

		// call this after writing into the buffer
		// returns -1 if no ogg page is available
		// otherwise returns the serial number of the ogg page
		// available
		//
    void	Wrote( const unsigned int& nBytes );
    ogg_page*	PageOut();
    int		GetCurrentPageSerial();

private:
    ogg_sync_state*	m_pSyncState;
    ogg_page		m_OggPage;
};

#endif
