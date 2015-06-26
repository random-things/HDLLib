#include "socketstream.h"



#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#endif
/**
 * close the socket once the stream goes out of scope
 * @return 
 */
Socketstream::~Socketstream( )
{
	close();
}

/**
 * attach the stream to a socket
 * @param socketfd 
 * @return returns 1 on success, -1 on failure
 */
bool Socketstream::attach( SOCKET socketfd )
{
	if(is_open() == true) return false;

	_fd = socketfd;
	return _fd ? true : false;
}


/**
 * connect the stream to a remote address and port
 * @param addr 
 * @param port 
 * @return true on success, false on failure
 */
bool Socketstream::connect( const char* addr, u16 port )
{
	if ( is_open() )
		return false;
		
	std::cout << "attempting to connect to : " << addr << " " << port << std::endl;
	struct sockaddr_in sock;
	struct hostent *he;
	

	if ( ( he = gethostbyname( addr ) ) == NULL )
		throw SocketException( std::string( "Socketstream::connect: Bad host: " ) + std::string( addr ) );

	sock.sin_family = AF_INET;
	sock.sin_port = htons( port );
	sock.sin_addr = *( ( struct in_addr * ) he->h_addr );
	memset( &( sock.sin_zero ), '\0', 8 ); // zero the rest of the struct
	
	if( create() == false ) 
		return false;
		
	if ( ::connect( _fd, ( struct sockaddr * ) & sock, sizeof( struct sockaddr ) ) == -1 )
		return false;
		//throw SocketException( "Socketstream::connect: Unable to open connection to hostname." );
		
	return true;
}



/**
 * Stream receive data from the network socket
 * @param data 
 * @return 
 */
Socketstream& Socketstream::operator >> ( std::string& data )
{
	char c;
	std::string t;

	while ( is_open() )
	{
		if ( recv( &c, 1 ) == 0 )
			close();
			
		if( c == '\x0A' ) 
			break;			
			
		t += c;
	}

	data = t;

	return *this;
}


/**
 * stream data over the network socket
 * @param data data stored in a std::string to be streamed over the socket
 * @return 
 */
Socketstream& Socketstream::operator << ( const std::string& data )
{
	this->send( data );
	return *this;
}

/**
 * determine if the stream has data to receive
 *
 * @param  
 * @return number of bytes received
 */
int Socketstream::can_recv( void )
{
	if ( is_open() )
	{
		char data;

		int bytesPending = ::recv( _fd, &data, 1, MSG_PEEK );

		if ( bytesPending == 0 )
			close();
		else
			// return the number of bytes pending.
			return bytesPending;
	}

	// If we get here, the socket doesn't have anything to tell us,
	// and therefore no bytes can be read.
	return 0;
}

int Socketstream::recv( void* data, int len ) const
{
	int bytes_recvd = 0;

	while ( is_open() )
	{
		bytes_recvd += ::recv( _fd, ( ( char* ) data ) + bytes_recvd, len - bytes_recvd, 0 );
		if( bytes_recvd == len 
			|| bytes_recvd == 0 
			|| bytes_recvd == SOCKET_ERROR )
			break;
	}

	return bytes_recvd;
}


void Socketstream::send( const void* data, int len ) const
{
	Socketstream::send( ( const char* ) data, len );
}

void Socketstream::send( const char* buf, int len ) const
{
	if ( is_open() )
	{
		int total = 0;
		int bytesleft = len;
		int n;

		while ( total < len )
		{
			n = ::send( _fd, buf + total, bytesleft, 0 );
			if ( n == -1 )
				throw SocketException( "Socketstream::send(buf,len): All data could not be sent." );
			total += n;
			bytesleft -= n;
		}
	}
}

void Socketstream::send( const std::string& pkt ) const
{
	if ( is_open() )
	{
		int total = 0;
		int len = (int)pkt.size();
		int bytesleft = (int)pkt.size();
		int n;
		const char* buf = pkt.c_str();

		while ( total < len )
		{
			n = ::send( _fd, buf + total, bytesleft, 0 );
			if ( n == -1 )
				throw SocketException( "Socketstream::send(string): All data could not be sent." );
			total += n;
			bytesleft -= n;
		}
		if ( n == -1 )
			throw SocketException( "Socketstream::send(string): All data could not be sent." );
	}
}


/* non-member */
#ifdef _WIN32

bool WinsockStartup()
{
	WSADATA versionData;
	int versionRequested = MAKEWORD(1, 0);

	return (WSAStartup(versionRequested, &versionData) == 0) ? true : false;
}

bool WinsockCleanup()
{
	return (WSACleanup() == 0) ? true : false;
}

#endif // _WIN32


