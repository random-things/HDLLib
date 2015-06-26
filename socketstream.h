#ifndef _SOCKETSTREAM_H_
#define _SOCKETSTREAM_H_

#include "config.h"
#include "socket.h"


class Socketstream : public Socket
{
	public:
		Socketstream( ) : Socket( )
		{}
		~Socketstream( );

		bool attach( SOCKET socketfd );
		bool connect( const char* addr, u16 _port );

		int can_recv( void );

		Socketstream& operator >> (string& data );
		Socketstream& operator << ( const string& data );
		
		//void recv( std::string& s ) const;
		int recv( void* data, int len ) const;
		//int recv( char* buf, int len ) const;
		
		void send( const char* buf, int len ) const;
		void send( const string& pkt ) const;
		void send( const void* data,  int len ) const;
		
		
};

#ifdef _WIN32
bool WinsockStartup();
bool WinsockCleanup();
#endif

#endif
