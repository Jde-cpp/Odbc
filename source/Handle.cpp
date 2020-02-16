#include "Handle.h"
#include <sql.h>
#include <sqlext.h>
#include "Utilities.h"

#define var const auto
namespace Jde::DB::Odbc
{
	HandleEnvironment::HandleEnvironment()
	{
		RETCODE rc;
		if( rc=SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &_handle) == SQL_ERROR )
			THROW( DBException("({}) - Unable to allocate an environment handle", rc) );
		try
		{
			CALL( _handle, SQL_HANDLE_ENV, SQLSetEnvAttr(_handle, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0), "SQLSetEnvAttr" );
		}
		catch( const DBException& e )
		{
			SQLFreeHandle( SQL_HANDLE_ENV, _handle );
			throw e;
		}
	}
	HandleEnvironment::~HandleEnvironment()
	{
		if( _handle )
			SQLFreeHandle( SQL_HANDLE_ENV, _handle );
	}
	HDBC HandleSession::s_handle{nullptr};
	mutex HandleSession::_mutex;
	HandleSession::HandleSession( string_view connectionString )://const HandleEnvironment& env
		_lock(_mutex)
	{
		//DBG0( "HandleSession::HandleSession()" );
		auto& handle = _handle;
		if( !handle )
		{
			CALL( _env, SQL_HANDLE_ENV, SQLAllocHandle(SQL_HANDLE_DBC, _env, &handle), "SQLAllocHandle" );
			SQLCHAR connectionStringResult[8192]; 
			SQLSMALLINT connectionStringLength;
			CALL( handle, SQL_HANDLE_DBC, SQLDriverConnect(handle, nullptr, (SQLCHAR*)string(connectionString).c_str(), SQL_NTS, connectionStringResult, 8192, &connectionStringLength, SQL_DRIVER_COMPLETE), "SQLDriverConnect" );
			//DBG0( "Leave1 HandleSession::HandleSession()" );
			if( connectionStringLength>0 )
			{
				INFO0_ONCE( string((char*)connectionStringResult) );
				//_connectionString = connectionString;
			}
			else
				ERR( "connectionString Length={}"sv, connectionStringLength );
		}
		//DBG0( "Leave HandleSession::HandleSession()" );
	}
	HandleSession::~HandleSession()
	{
		if( _handle ) //TODO close on Exit
		{
			SQLDisconnect( _handle );
			SQLFreeHandle( SQL_HANDLE_DBC, _handle );
		}
	}

	HandleStatement::HandleStatement( string_view connectionString ):
		_session{ connectionString }
	{
		CALL( _session, SQL_HANDLE_DBC, SQLAllocHandle(SQL_HANDLE_STMT, _session, &_handle), "SQLAllocHandle" );
	}
	HandleStatement::~HandleStatement()
	{
		if( _handle )
		{
			if( var rc=SQLFreeHandle( SQL_HANDLE_STMT, _handle )!=SQL_SUCCESS )
				DBG( "SQLFreeHandle( SQL_HANDLE_STMT, {} ) returned {}"sv, _handle, rc );
		}
	}
}