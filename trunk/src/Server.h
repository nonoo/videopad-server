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

#ifndef __SERVER_H
#define __SERVER_H

#include "TCPListener.h"
#include "UDPListener.h"
#include "Client.h"
#include "Channel.h"
#include "OggDecoder.h"
#include "SerialMapper.h"
#include "MyString.h"

#include <vector>
#include <map>

class CServer
{
public:
    CServer();
    ~CServer();

    // main processing loop
    //
    void Loop();

private:
    // in Server_NewClient.cpp
    void SendMOTD( CClient*& pClient );
    void NewClientConnect();

    // in Server_DataProcess.cpp
    // accepts new data connections if there is a client already logged in
    // from the same IP
    // puts the newly created socket into the unassigned data connection
    // vector
    // data connections become assigned when a client starts to send
    // ogg data to them - this is for supporting multiple clients
    // behind NAT firewalls
    //
    void NewDataConnection();
    // returns -1 if the connection has been dropped
    // returns -2 if there was no error but an ogg page couldn't produced
    // otherwise returns the serial number of the ogg page read from
    // the connection
    //
    int ReadUnassignedDataSocket( int& nSocket );
    void ReadUDPData( CUDPListener* pUDPListener );
    void ReadClientDataSocket( CClient*& pClient );
    void ProcessClientData( CClient*& pClient );

    // in Server_ClientProcess.cpp
    void ReadClientMessage( CClient*& pClient );
    void ProcessClientMessage( CClient*& pClient, CMyString szLine );
    void CheckForConnectionStatus( CClient*& pClient );
    void CheckForLoginTimeout( CClient*& pClient );
    void DeleteChannel( CChannel*& pChannel );
    void DeleteClient( CClient*& pClient );

    // in Server_Commands.cpp
    void JoinChannel( CClient*& pClient, string szChannelName );
    void PartChannel( CClient*& pClient, string szChannelName );
    void NickChange( CClient*& pClient, string szParam );
    void Whois( CClient*& pClient, string szParam );
    void SendChanList( CClient*& pClient );



    CTCPListener*			m_pControlSocket;
    CTCPListener*			m_pDataSocket_TCP;
    CUDPListener*			m_pDataSocket_UDP;
    vector< CClient* >			m_ClientVector;
    typedef map< string, CChannel* >	tChannelMap;
    tChannelMap				m_ChannelMap;
    // data connection sockets not assigned to a client yet
    vector< int >			m_UnassignedDataSocketVector;
    CSerialMapper			m_SerialMapper;
    COggDecoder*			m_pOggDecoder;
};

#endif
