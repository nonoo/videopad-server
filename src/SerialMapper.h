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

#ifndef __SERIALMAPPER_H
#define __SERIALMAPPER_H

#include <map>

#include "Client.h"
#include "OggStream.h"

// this class manages unique serial numbers
// when the user logs in, the server associates him 2 unique serial numbers
// (video+audio) using CSerialMapper.
//
class CSerialMapper
{
public:
    CSerialMapper();

    // sets a new unique serial to the given client and returns with the
    // number
    //
    const unsigned int	GetNewVideoSerial( CClient* pClient );
    const unsigned int	GetNewAudioSerial( CClient* pClient );

    void 		DeleteClientSerials( CClient* pClient );

    CClient*		GetClient( const unsigned int& nSerial );
    COggStream*		GetOggStream( const unsigned int& nSerial );

private:
    typedef map< unsigned int, pair< CClient*, COggStream* > > tSerialMap;
    tSerialMap		m_SerialMap;
};

#endif
