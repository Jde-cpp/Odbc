#pragma once

namespace Jde::DB::Odbc
{
	void HandleDiagnosticRecord( sv functionName, SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE retCode )noexcept(false);
	inline void Call( SQLHANDLE handle, SQLSMALLINT handleType, std::function<int()> func, sv functionName )noexcept(false)
	{
		if( const int retCode = func(); retCode!=SQL_SUCCESS )
			HandleDiagnosticRecord( functionName, handle, handleType, retCode );
	}
	#define CALL( handle, handleType, function, functionName ) Call( handle, handleType, [&](){ return function; }, functionName )
}