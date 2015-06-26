#ifndef _ERROR_H_
#define _ERROR_H_

#include <string>

typedef std::string string;
typedef unsigned short u16;

class Exception
{
	public:
		Exception( void );
		Exception( string s ) : m_s( s )
		{}
		virtual ~Exception( void )
		{}

		virtual string& description( void )
		{
			return m_s;
		}
	protected:
		std::string m_s;
};

class ClientException : public Exception
{
	public:
		ClientException( string s ) : Exception( s )
		{ }
		ClientException( string s, u16 p ) : Exception( s ) , _p( p )
		{ }
	private:
		u16 _p;
};

class SocketException : public Exception
{
	public:
		SocketException( string s ) : Exception( s )
		{ }
		SocketException( string s, u16 p ) : Exception( s ) , _p( p )
		{ }
	private:
		u16 _p;
};

class ServerException : public Exception
{
	public:
		ServerException( std::string s ) : Exception( s )
		{}
		ServerException( std::string s, u16 p ) : Exception( s ), _p( p )
		{ }
	private:
		u16 _p;
};

/*
namespace ClientError
{
	struct BufferOverflow {};
}
 
namespace NetError
{
	struct DeadSocket {};
	struct BadConnect {
		const char* a;
		u16 p;
		BadConnect( const char* _a, u16 _p ) : a(_a), p(_p) {}
	};
	struct BadHost {
		const char *q;
		BadHost( const char* _q ) : q(_q) {}
	};
	struct SendError {};
	struct SocketExists {};
	struct BindError {
		u16 p;
		BindError( u16 q ) : p(q) {}
	};
}
 
namespace ServerError
{
	struct ListenError{};
	struct SockOpt {};
	struct SelectError {};
	struct AcceptError {};
	struct RecvError {};
	struct SendError {};
}
*/

#endif /* _ERROR_H_ */
