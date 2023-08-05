﻿#include "Utilities.h"
#include <sqlext.h>
#include "../../Framework/source/db/DBException.h"
#include <jde/Log.h>

#define var const auto
namespace Jde::DB::Odbc
{
	static const LogTag& _logLevel = Logging::TagLevel( "dbDriver" );
	α HandleDiagnosticRecord( sv functionName, SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE retCode, SL sl )ε->string
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
			const string state{ (const char*)szState, SQL_SQLSTATE_SIZE };
			var level{ (retCode!=1 && state=="01004") ? ELogLevel::Error : ELogLevel::Debug };
			var msg{ format("[{:<5}] {} {}", state, (char*)szMessage, iError) };
			if( y.size() )
				y += '\n';
			y += msg;
			if( state=="01000" && iError!=3621 )//3621=The statement has been terminated.
				Logging::LogOnce( Logging::Message{ELogLevel::Information, msg, SRCE_CUR} );
			else
			{
				LOGX( msg );
				if( functionName=="SQLDriverConnect" && level==ELogLevel::Error )
					throw Exception{ sl, ELogLevel::Critical, "[{:<5}] {} {}", state, (char*)szMessage, iError };
				if( retCode==1 && state=="23000" )//23000=Integrity constraint violation.  multiple statements why retCode==1.
					throw DBException{ "", nullptr, msg, sl };
			}
		}
		return y;
	}
}