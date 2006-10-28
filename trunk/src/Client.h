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

#ifndef __CLIENT_H
#define __CLIENT_H

#include "OggStream.h"
#include "VideoStream.h"
#include "AudioStream.h"

class CClient
{
public:
    CClient();
    ~CClient();

    void		SendMessage( string szMessage );
    void		SendData( char* pData, unsigned int nDataSize );

    void		SetControlSocket( int nSocket );
    const int&		GetControlSocket();
    void		SetDataSocket( int nSocket );
    const int&		GetDataSocket();
    void		SetIP( string szIP );
    const string&	GetIP();
    void		SetHost( string szHost );
    const string&	GetHost();
    const bool&		IsLoggedin();
    void		SetLoggedin( bool fState );
    const bool&		IsMOTDSent();
    void		SetMOTDSent( bool fState );
    const bool&		IsConnectionClosed();
    const string&	GetQuitMessage();
    void		SetQuitMessage( string szQuitMessage );
    const bool&		IsOperator();
    void		SetOperator( bool fState );
    const bool&		GetPassGiven();
    void		SetPassGiven( bool fState );
    void		SetLastPingTime( time_t tLastPingTime );
    const time_t&	GetLastPingTime();
    const bool&		IsPingSent(); // has the client been pinged?
    void		SetPingSent( bool fState );
    const string&	GetNick();
			// returns -1 if nick is too long
			// return -2 if nick contains invalid chars
    int			SetNick( string szNick );

    const time_t&	GetTimeConnected();
    const string&	GetRealName();
    void		SetRealName( string szRealName );
    const string&	GetTagline();
    void		SetTagline( string szTagline );
    CVideoStream*	GetVideoStream();
    CAudioStream*	GetAudioStream();

private:
    string		m_szNick;
    int			m_nControlSocket;
    int			m_nDataSocket;
    string		m_szIP;
    string		m_szHost;
    string		m_szRealName;
    string		m_szTagline;
    time_t		m_tTimeConnected;
    bool		m_fPassGiven;
    bool		m_fLoggedin;
    bool		m_fMOTDSent;
    bool		m_fConnectionClosed;
    bool		m_fOperator;
    string		m_szQuitMessage;
    time_t		m_tLastPingTime;
    bool		m_fPingSent;
    CVideoStream*	m_pVideoStream;
    CAudioStream*	m_pAudioStream;
};

#endif
