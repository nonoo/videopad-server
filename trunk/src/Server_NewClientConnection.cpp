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
#include "Server.h"

#include <sstream>
#include <fstream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

void CServer::SendMOTD( CClient*& pClient )
{
    string szMOTDFile = g_SettingsFile_Server.Get( "server settings", "motd-file", "/etc/videopad-server/motd" );

    if( szMOTDFile.size() > 0 )
    {
	ifstream MOTDFileStream( szMOTDFile.c_str() );
	if( MOTDFileStream.fail() )
	{
	    MOTDFileStream.close();
	    string err = "can't open MOTD file: " + szMOTDFile;
	    g_Logger.Error( err );
	    return;
	}

	pClient->SendMessage( "003 :MOTD" );
	while( !MOTDFileStream.eof() )
	{
	    string szLine;
	    char buf[500];
    	    memset( buf, 0, 500 );
	    MOTDFileStream.getline( buf, 499 );

	    szLine = buf;
	    pClient->SendMessage( "004 :" + szLine );
	}
	pClient->SendMessage( "005 :End of MOTD" );

	MOTDFileStream.close();
    }
}

void CServer::NewClientConnect()
{
    struct sockaddr_in ClientAddr;
cout<<"newclient"<<endl;
    // accepting new client connection
    //
    memset( &ClientAddr, 0, sizeof(ClientAddr) );
    unsigned int l = sizeof(ClientAddr);
    int nNewSocket = accept( m_pControlSocket->GetSocket(), (struct sockaddr *)&ClientAddr, &l );
cout<<"accepted: "<<nNewSocket<<endl;
    // resolving client's address
    //
    struct hostent* pNewClientHostent = NULL;
    pNewClientHostent = gethostbyaddr( &ClientAddr.sin_addr.s_addr, sizeof(ClientAddr.sin_addr.s_addr), AF_INET );

    struct in_addr inaddr;
    inaddr.s_addr = ClientAddr.sin_addr.s_addr;
    string szIP = inet_ntoa( inaddr );

    if ( nNewSocket < 0 ) {
	string tmp;

	if( pNewClientHostent != NULL )
	{
	    tmp = "can't accept client connection from ";
	    tmp += pNewClientHostent->h_name;
	    tmp += " (" + szIP + ")";
	}
	else
	{
	    tmp = "can't accept client connection from " + szIP;
	}

	g_Logger.Error( tmp );
	return;
    }

    // creating new client class
    //
    CClient* pNewClient = new CClient;
    m_ClientVector.push_back( pNewClient );

    pNewClient->SetControlSocket( nNewSocket );
    if( pNewClientHostent != NULL )
    {
	pNewClient->SetHost( pNewClientHostent->h_name );
	pNewClient->SetIP( szIP );
    }

    // welcome message
    //
    string tmp = "001 :Welcome to ";
    tmp += g_SettingsFile_Server.Get( "server settings", "server-name", "VideoPad Server" );
    pNewClient->SendMessage( tmp );

    tmp = "002 :Running ";
    tmp += PACKAGE;
    tmp += " v";
    tmp += VERSION;
    pNewClient->SendMessage( tmp );

    if( g_SettingsFile_Server.Get( "server settings", "server-password", "" ).size() > 0 )
    {
	pNewClient->SendMessage( "006 :Please login" );
    }
}
