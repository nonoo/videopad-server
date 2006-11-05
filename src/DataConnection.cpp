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
#include "DataConnection.h"

#include <sys/socket.h>

CDataConnection::CDataConnection( int nSocket )
{
    m_nSocket = nSocket;
    m_pOggDecoder = new COggDecoder();
}

CDataConnection::~CDataConnection()
{
    if( m_nSocket >= 0 )
    {
	shutdown( m_nSocket, SHUT_RDWR );
	close( m_nSocket );
    }

    SAFE_DELETE( m_pOggDecoder );
}

char* CDataConnection::GetOggDecoderBuffer( const unsigned int& nBufferSize )
{
    return m_pOggDecoder->GetBuffer( nBufferSize );
}

void CDataConnection::OggDecoderWrote( const unsigned int& nBytes )
{
    m_pOggDecoder->Wrote( nBytes );
}

ogg_page* CDataConnection::OggDecoderPageOut()
{
    return m_pOggDecoder->PageOut();
}

int CDataConnection::GetCurrentPageSerial()
{
    return m_pOggDecoder->GetCurrentPageSerial();
}

int CDataConnection::GetSocket()
{
    return m_nSocket;
}

int CDataConnection::Send( char* pData, unsigned int nDataSize )
{
    return send( m_nSocket, pData, nDataSize, 0 );
}
