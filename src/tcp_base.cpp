#include "tcp_base.h"


tcp_base::tcp_base(boost::asio::io_service& io_service, unsigned int buff_size)
	: _io_service(io_service), _socket(io_service), _buff_size(buff_size)
{
	_read_buff = (char*)malloc(buff_size);
}

void tcp_base::connect(const string& ipStr, unsigned short port)
{
	tcp::resolver resolver(_io_service);
	ostringstream oss;
	oss << port;
	tcp::resolver::query query(ipStr, oss.str());
	tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
	
	boost::asio::async_connect(_socket, endpoint_iterator, 
		boost::bind(&tcp_base::handleConnect, shared_from_this(), boost::asio::placeholders::error));
}

tcp_base::~tcp_base()
{
	doClose();
	free(_read_buff);
}

void tcp_base::setReadHandler(msg_handler handler)
{
	_read_handler = handler;
}

void tcp_base::setWriteHandler(msg_handler handler)
{
	_write_handler = handler;
}

void tcp_base::setConnectHandler(connect_handler handler)
{
	_connect_handler = handler;
}

void tcp_base::write(const string& msg)
{
	_io_service.post(boost::bind(&tcp_base::doWrite, shared_from_this(), msg));
}

void tcp_base::close()
{
	_io_service.post(boost::bind(&tcp_base::doClose, shared_from_this()));
}

void tcp_base::handleConnect(const boost::system::error_code& error)
{
	if(!error)
	{
		if(_connect_handler)
			_connect_handler();
		postRead0();
	}
	else
	{
		fprintf(stderr, "[Error]: %s\n", error.message().c_str());
	}
}

void tcp_base::postRead0()
{
	_socket.async_read_some(boost::asio::null_buffers(), 
		boost::bind(&tcp_base::handleRead, shared_from_this(), boost::asio::placeholders::error, 
		boost::asio::placeholders::bytes_transferred));
}

void tcp_base::handleRead(const boost::system::error_code& error, std::size_t bytes_transferred)
{
	if(!error)
	{
		std::size_t len = 0;
		try
		{
			len = _socket.read_some(boost::asio::buffer(_read_buff, _buff_size));
			if(_read_handler)
			{
				string str(_read_buff, len);
				_read_handler(str);
			}
			postRead0();
		}
		catch(boost::system::system_error exception)
		{
			//fprintf(stderr, "[Error]: %s\n", exception.what().c_str());
			doClose();
		}
	}
	else
	{
		fprintf(stderr, "[Error]: %s\n", error.message().c_str());
		doClose();
	}
}

void tcp_base::doWrite(const string& str)
{
	bool write_in_progress = !_write_queue.empty();
	_write_queue.push_back(str);
	if(!write_in_progress)
	{
		boost::asio::async_write(_socket, boost::asio::buffer(_write_queue.front().c_str(), _write_queue.front().size()),
			boost::bind(&tcp_base::handleWrite, shared_from_this(), boost::asio::placeholders::error));
	}
}

void tcp_base::handleWrite(const boost::system::error_code& error)
{
	if(!error)
	{
		if(_write_handler)
			_write_handler(_write_queue.front());
		_write_queue.pop_front();
		if(!_write_queue.empty())
		{
			boost::asio::async_write(_socket, boost::asio::buffer(_write_queue.front().c_str(), _write_queue.front().size()),
				boost::bind(&tcp_base::handleWrite, shared_from_this(), boost::asio::placeholders::error));
		}
	}
	else
	{
		fprintf(stderr, "[Error]: %s\n", error.message().c_str());
		doClose();
	}
}

void tcp_base::doClose()
{
	_socket.close();
	printf("socket close\n");
}


