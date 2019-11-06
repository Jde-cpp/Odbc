#pragma once

namespace Jde::DB::Odbc
{
	void HandleDiagnosticRecord( string_view functionName, SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE retCode );
	inline void Call( SQLHANDLE handle, SQLSMALLINT handleType, std::function<int()> func, std::string_view functionName )
	{
		const int retCode = func();
		if( retCode!=SQL_SUCCESS )
			HandleDiagnosticRecord( functionName, handle, handleType, retCode);
		//THROW(Exception("{} - error ({}) - {}", functionName, result, XGBGetLastError()));
	}
	#define CALL(handle, handleType, function,functionName) Call( handle, handleType, [&](){ return function; }, functionName )
}