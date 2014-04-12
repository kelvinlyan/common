#ifndef _COMMON_EXCOMMON_H
#define _COMMON_EXCOMMON_H

typedef unsigned char UInt8;
typedef char Int8;
typedef unsigned short UInt16;
typedef short Int16;
typedef unsigned int UInt32;
typedef int Int32;
typedef unsigned long long UInt64;
typedef long long Int64;


#define DATA_DEFINE(type, name) \
protected: type m_##name; \
public: virtual type& get##name(){ return m_##name; } \
public: virtual void set##name(const type& var){ m_##name = var; } \

#define PROPERTY_PUBLIC_GET_ONLY(type, name) \
protected: type m_##name; \
public: virtual type& get##name(){ return m_##name; }


#endif
