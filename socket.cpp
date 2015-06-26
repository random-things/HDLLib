/*
* $Id: socket.cpp,v 1.4 2005/06/20 16:44:47 cipher Exp $
*
*/
#include "socket.h"

#ifdef _WIN32
#include "winsock2.h"
#pragma comment(lib, "ws2_32.lib")
#endif


Socket::Socket( void ) :
	_fd( 0 )
{
}

Socket::~Socket( void )
{
	return;
}


/**
 * Protected member function to create a valid socket
 * @param  
 * @return true if the socket was created successfully, false otherwise
 */
bool Socket::create( void )
{
	close();

	_fd = socket(AF_INET, SOCK_STREAM, 0);
	return _fd ? true: false;
}

bool Socket::close( void )
{
	if ( !is_open() )
	{
		return true;
	}
	std::cout << "in Socket::close() && closing()\n"; //only print this if we're going to close
	int r = 0;
	
	#ifdef _WIN32

	r = closesocket( _fd );
	
	#else /* UNIX */
	r = ::close( _fd );
	
	#endif

	_fd = 0;

	return r ? false : true;
}

string Socket::gethostname( ) const
{
	sockaddr_in hostAddress;
	int         addressLength = sizeof hostAddress;

	if(::getsockname(_fd, (sockaddr*)&hostAddress, &addressLength) == 0)
	{
		return string(inet_ntoa(hostAddress.sin_addr));
	}

	return string();
}


void hexdump( int len, const char *buf )
{
	char mybuf[ 256 ];

	int i, offset, stop;
	unsigned char *p;

	for ( offset = 0; offset < len; offset += 16 )
	{

		// Offset
		sprintf( mybuf, "  %04X   ", offset );
		std::cout << mybuf;

		// Adjust start
		p = ( unsigned char * ) & buf[ offset ];
		stop = ( ( offset + 16 ) > len ) ? len - offset : 16;

		// Drop numbers
		for ( i = 0; i < stop; i++ )
		{
			if ( i == 8 )
				std::cout << "- ";
			sprintf( mybuf, "%02X ", p[ i ] );
			std::cout << mybuf;
		}

		// Fill
		while ( i < 16 )
		{
			if ( i++ == 8 )
				std::cout << "- ";
			std::cout << "   ";
		}

		// Space
		std::cout << "  ";

		// Drop characters
		for ( i = 0; i < stop; i++ )
			if ( isgraph( p[ i ] ) )
			{
				sprintf( mybuf, "%c", p[ i ] );
				std::cout << mybuf;
			}
			else
			{
				if ( p[ i ] == ' ' )
					std::cout << " ";
				else
					std::cout << ".";
			}

		// Finish
		std::cout << std::endl;
	}
}