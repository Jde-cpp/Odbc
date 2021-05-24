#include "Utilities.h"
#include <sqlext.h>
#include "../../Framework/source/db/DBException.h"
#include <jde/Log.h>


#define var const auto

namespace Jde::DB::Odbc
{
	void HandleDiagnosticRecord( sv functionName, SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE retCode )noexcept(false)
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
			var level = retCode!=1 && strncmp( (char*)szState, "01004", SQL_SQLSTATE_SIZE ) ? ELogLevel::Error : ELogLevel::Debug;
			if( strncmp((char*)szState, "01000", SQL_SQLSTATE_SIZE)==0  )
				INFO_ONCE( format("[{:<5}] {} {}"sv, szState, szMessage, iError) );
			else
			{
				LOGX( level, "[{:<5}] {} {}"sv, szState, szMessage, iError );
				if( functionName=="SQLDriverConnect" && level==ELogLevel::Error )
					THROW( DBException("[{:<5}] {} {}"sv, szState, szMessage, iError) );
			}
		}

	}
}