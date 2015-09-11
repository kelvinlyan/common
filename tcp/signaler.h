#ifndef _SIGNALER_H
#define _SIGNALER_H

class signaler
{
	public:
		signaler();
		~signaler();

		void send();
		void recv();
		int get_fd();

	private:
		int _w;
		int _r;
};

#endif
