#pragma once

namespace Jde::DB::Odbc
{
	α HandleDiagnosticRecord( sv functionName, SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE retCode, SRCE )ε->string;
	Ξ Call( SQLHANDLE handle, SQLSMALLINT handleType, std::function<int()> func, sv functionName, SRCE )ε
	{
		if( const int retCode = func(); retCode!=SQL_SUCCESS )
			HandleDiagnosticRecord( functionName, handle, handleType, retCode, sl );
	}
	#define CALL( handle, handleType, function, functionName ) Call( handle, handleType, [&](){ return function; }, functionName )
}