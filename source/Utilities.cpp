#include "Utilities.h"
#include <sqlext.h>
#include "../../Framework/source/db/DBException.h"
#include <jde/Log.h>

#define var const auto
namespace Jde::DB::Odbc
{
	static const LogTag& _logLevel = Logging::TagLevel( "dbDriver" );
	α HandleDiagnosticRecord( sv functionName, SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE retCode, SL sl )noexcept(false)->string
	{
		THROW_IF( retCode==SQL_INVALID_HANDLE, "({}) {} - Invalid handle {}", functionName, hType, retCode );
		SQLSMALLINT iRec = 0;
		SQLINTEGER iError;
		SQLCHAR szMessage[SQL_MAX_MESSAGE_LENGTH];
		SQLCHAR szState[SQL_SQLSTATE_SIZE + 1];
		SQLSMALLINT msgLen;
		string y;
		while( SQLGetDiagRec(hType, hHandle, ++iRec, szState, &iError, szMessage, (SQLSMALLINT)(sizeof(szMessage) / sizeof(char)),  &msgLen) == SQL_SUCCESS )
		{
			var level{ retCode!=1 && strncmp( (char*)szState, "01004", SQL_SQLSTATE_SIZE ) ? ELogLevel::Error : ELogLevel::Debug };
			var msg{ format("[{:<5}] {} {}", (char*)szState, (char*)szMessage, iError) };
			if( y.size() )
				y += '\n';
			y += msg;
			if( strncmp((char*)szState, "01000", SQL_SQLSTATE_SIZE)==0  && iError!=3621 )//3621=The statement has been terminated.
				Logging::LogOnce( Logging::Message{ELogLevel::Information, msg, SRCE_CUR} );
			else
			{
				LOGX( msg );
				if( functionName=="SQLDriverConnect" && level==ELogLevel::Error )
					throw Exception{ sl, ELogLevel::Critical, "[{:<5}] {} {}", (char*)szState, (char*)szMessage, iError };
			}
		}
		return y;
	}
}