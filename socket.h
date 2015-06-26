/*
 * $Id: socket.h,v 1.4 2005/06/20 16:44:47 cipher Exp $
 */
#ifndef _SOCKET_H_
#define _SOCKET_H_

#include "config.h"

#ifdef _WIN32

#include "winsock2.h"

#elif _OS_LINUX

#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#endif


#ifdef _WIN32
//typedef UINT_PTR SOCKET;
#else
typedef int SOCKET;
#endif

class Socket
{
	public:
		Socket( void );
		virtual ~Socket( void );

		SOCKET getfd( void ) const { return _fd; }
		void setfd( int fd ) { _fd = fd; }
		
		const string getpeername( void ) const;
		
		bool close( void );
		
		
		bool is_open( void ) const { return ( _fd ? true : false ); }
		string Socket::gethostname( void ) const;


	protected:
		bool create( void );
		
	protected:
		SOCKET _fd;


	private:

};

void hexdump( int len, const char *buf );



#endif