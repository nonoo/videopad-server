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

#ifndef __DATACONNECTION_H
#define __DATACONNECTION_H

#include "OggDecoder.h"

class CDataConnection
{
public:
    CDataConnection( int nSocket );
    ~CDataConnection();

    char*		GetOggDecoderBuffer( const unsigned int& nBufferSize );
    void		OggDecoderWrote( const unsigned int& nBytes );
    ogg_page*		OggDecoderPageOut();
    int			GetCurrentPageSerial();
    int			GetSocket();
    int			Send( char* pData, unsigned int nDataSize );

private:
    COggDecoder*	m_pOggDecoder;
    int			m_nSocket;
};

#endif
