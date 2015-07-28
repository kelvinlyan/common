#ifndef _LOG_WRITER_H
#define _LOG_WRITER_H

namespace nLog
{
	class writer : public iLink
	{
		public:
			writer(logger& l)
			{
			}
			
			virtual const char* handle(const char* pstr)
			{
				_handle(pstr);
				return pstr;
			}

		private:
			string getFileName();
			void _handle(const char* pstr)
			{
				string name = getFileName();
				fwrite(pstr, strlen(pstr), 1, _fptr);
			}

		private:
			const time_t* _now;
			FILE* _fptr;
			string _fname;


			int _vbuf_type;
			char* _vbuf_ptr;
			unsigned _vbuf_size;
	};
}

#endif
