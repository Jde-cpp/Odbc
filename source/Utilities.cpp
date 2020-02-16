#include "Utilities.h"
#include <sqlext.h>

#define var const auto

namespace Jde::DB::Odbc
{
	void HandleDiagnosticRecord( string_view functionName, SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE retCode )
	{
		if( retCode==SQL_INVALID_HANDLE )
			THROW( DBException("({}) {} - Invalid handle {:x}", functionName, hType, retCode) );

		SQLSMALLINT iRec = 0;
		SQLINTEGER iError;
		SQLCHAR szMessage[SQL_MAX_MESSAGE_LENGTH];
		SQLCHAR szState[SQL_SQLSTATE_SIZE + 1];

		//ostringstream os;
		SQLSMALLINT msgLen;
		while( SQLGetDiagRec(hType, hHandle, ++iRec, szState, &iError, szMessage, (SQLSMALLINT)(sizeof(szMessage) / sizeof(char)),  &msgLen) == SQL_SUCCESS )
		{
			var level = strncmp((char*)szState, "01004", 5) ? ELogLevel::Error : ELogLevel::Debug;
			LOG( level, "[{:<5}] {} {}"sv, szState, szMessage, iError );
		}

	}
}