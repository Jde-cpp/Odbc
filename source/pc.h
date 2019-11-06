#include <string_view>
#if _MSC_VER
	#ifndef __INTELLISENSE__ 
		#include	<windows.h>
//		#include <basetsd.h>
		#include <sqltypes.h>
		#include <sql.h>
		#include <sqlext.h>
	#endif
#else
//	#include <iodbc/iodbcunix.h>
	#include <sqltypes.h>
	#include <sql.h>
//	#include <iodbc/sqlucode.h>
	#include <sqlext.h>
#endif
#pragma warning( disable : 4245) 
#include <boost/crc.hpp> 
#pragma warning( default : 4245) 
#include <boost/noncopyable.hpp>
#include <boost/system/error_code.hpp>
#ifndef __INTELLISENSE__
	#include <spdlog/spdlog.h>
	#include <spdlog/sinks/basic_file_sink.h>
	#include <spdlog/fmt/ostr.h>
#endif

#include "TypeDefs.h"
#include "../../Framework/source/DateTime.h"
#include "../../Framework/source/db/DBException.h"
#include "../../Framework/source/db/DataType.h"
#include "../../Framework/source/db/DataSource.h"

