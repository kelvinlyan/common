#ifndef _TCP_BASE_H
#define _TCP_BASE_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <deque>
#include <string>

using boost::asio::ip::tcp;
using namespace std;

class tcp_base
	: public boost::enable_shared_from_this<tcp_base>
{
	public:
		enum{
			_default_buff_size = 10240
		};
		typedef boost::shared_ptr<tcp_base> pointer;
		typedef boost::function<void(string&)> msg_handler;
		typedef boost::function<void()> connect_handler;

		tcp_base(boost::asio::io_service& ios, unsigned int buff_size = _default_buff_size);
		~tcp_base();

		void connect(const string& ipStr, unsigned short port);
		void setReadHandler(msg_handler handler);
		void setWriteHandler(msg_handler handler);
		void setConnectHandler(connect_handler handler);
		void write(const string& str);
		void close();

	private:
		void handleConnect(const boost::system::error_code& error);
		void handleRead(const boost::system::error_code& error, std::size_t bytes_transferred);
		void handleWrite(const boost::system::error_code& error); 

		void postRead0();
		void doWrite(const string& msg);
		void doClose();
		
	private:
		msg_handler _read_handler;
		msg_handler _write_handler;
		connect_handler _connect_handler;
		char* _read_buff;
		unsigned int _buff_size;
		
		typedef deque<string> msg_queue;
		msg_queue _write_queue;

		boost::asio::io_service& _io_service;
		tcp::socket _socket;
};

#endif
