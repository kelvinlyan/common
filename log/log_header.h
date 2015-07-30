#ifndef _LOG_LOGHEADER_H
#define _LOG_LOGHEADER_H

#include "logger.h"
#include "formatter.h"
#include "printer.h"
#include "writer.h"

namespace nLog
{
	static inline logger* getLogger()
	{
		logger* lg = new logger;
		formatter* f = new formatter;
		*f << nAttr::nTime::create("[%02H:%02M:%02S]", nColor::YELLOW)
		  << nAttr::severity::create(*lg, "[%s]: ", nColor::RED)
		  << nAttr::message::create(nColor::GREEN);
		printer* p = new printer;
		writer* w = new writer;
		w->setTimeSize(DAY);
		w->setByteSize(100 * MB);
		*w << nAttr::nTime::create("./log/LOG-%Y-%02m-%02d");
		w->open();

		lg->push(f);
		f->push(p);
		f->push(w);
		return lg;
	}
}

#endif

