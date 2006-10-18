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

#ifndef __MAIN_H
#define __MAIN_H

#ifndef DEBUG
#define DEBUG 0
#endif

#ifdef DEBUG
// for simple "cout<<debugmessage<<endl;"
#include <iostream>
using namespace std;
#endif

// max length of one line sent to the control port
#define MAXMESSAGELENGTH 1024
#define MAXNICKLENGTH 20
// how many bytes to read from a data connection in one step
#define MAXDATAREAD 5000

// some common macros
//
#define SAFE_DELETE(p)       { delete (p); (p)=NULL; }
#define SAFE_DELETE_ARRAY(p) { delete[] (p); (p)=NULL; }
#define MAX(x,y) ( (x) > (y) ? (x) : (y) )


#include "SettingsFile.h"
#include "Logger.h"

// globals
//
extern CSettingsFile g_SettingsFile_Server;
extern CLogger g_Logger;
extern bool g_fDaemonMode;
extern bool g_fTerminate;

#endif
