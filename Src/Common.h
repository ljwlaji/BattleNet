#ifndef __COMMON_H__
#define __COMMON_H__

#include <vector>
#include <mutex>
#include <map>
#include <list>
#include "Result.h"
using namespace std;
#define ThreadLocker		std::lock_guard<std::mutex>

#define	int8				signed char				
#define	int16				short
#define	int32				int
#define	int64				signed __int64

#define	uint8				unsigned char		
#define	uint16				unsigned short		
#define	uint32				unsigned int		
#define	uint64				unsigned __int64	

#endif