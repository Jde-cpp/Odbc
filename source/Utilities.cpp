#include "Utilities.h"
#include <sqlext.h>
#include "../../Framework/source/db/DBException.h"
#include <jde/Log.h>

#define var const auto
namespace Jde::DB::Odbc
{
	α HandleDiagnosticRecord( sv functionName, SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE retCode, const source_location& sl )noexcept(false)->string
	{
		THROW_IF( retCode==SQL_INVALID_HANDLE, "({}) {} - Invalid handle {:x}", functionName, hType, retCode );
		SQLSMALLINT iRec = 0;
		SQLINTEGER iError;
		SQLCHAR szMessage[SQL_MAX_MESSAGE_LENGTH];
		SQLCHAR szState[SQL_SQLSTATE_SIZE + 1];
		SQLSMALLINT msgLen;
		ostringstream os;
		while( SQLGetDiagRec(hType, hHandle, ++iRec, szState, &iError, szMessage, (SQLSMALLINT)(sizeof(szMessage) / sizeof(char)),  &msgLen) == SQL_SUCCESS )
		{
			var level = retCode!=1 && strncmp( (char*)szState, "01004", SQL_SQLSTATE_SIZE ) ? ELogLevel::Error : ELogLevel::Debug;
			if( strncmp((char*)szState, "01000", SQL_SQLSTATE_SIZE)==0  )
			{
				const string msg{ format("[{:<5}] {} {}", (char*)szState, (char*)szMessage, iError) };
				os << msg;
				Logging::LogOnce( Logging::Message{ELogLevel::Information, msg, SRCE_CUR} );
			}
			else
			{
				Logging::LogNoServer( Logging::Message{ level, "[{:<5}] {} {}", sl}, szState, szMessage, iError );
				THROW_IF( functionName=="SQLDriverConnect" && level==ELogLevel::Error, "[{:<5}] {} {}"sv, szState, szMessage, iError );
			}
		}
		return os.str();
	}
}