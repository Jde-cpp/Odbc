#include "Handle.h"
#include <sql.h>
#include <sqlext.h>
#include <jde/Log.h>
#include "../../Framework/source/db/DBException.h"
#include "Utilities.h"


#define var const auto
namespace Jde::DB::Odbc
{
	shared_ptr<void> HandleEnvironment::_handle; 
	HandleEnvironment::HandleEnvironment()noexcept(false)
	{
		if( !_handle )
		{
			CALL( nullptr, SQL_HANDLE_ENV, SQLSetEnvAttr(nullptr, SQL_ATTR_CONNECTION_POOLING, (SQLPOINTER)SQL_CP_ONE_PER_HENV, 0), "SQLSetEnvAttr(SQL_ATTR_CONNECTION_POOLING)" );
			SQLHENV handle;
			RETCODE rc=SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &handle); THROW_IF( rc==SQL_ERROR, DBException("({}) - Unable to allocate an environment handle", rc) );
			CALL( handle, SQL_HANDLE_ENV, SQLSetEnvAttr(handle, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0), "SQLSetEnvAttr(SQL_ATTR_ODBC_VERSION)" );
			//CALL( handle, SQL_HANDLE_ENV, SQLSetEnvAttr(handle, SQL_ATTR_CONNECTION_POOLING, (SQLPOINTER)SQL_CP_ONE_PER_HENV, 0), "SQLSetEnvAttr(SQL_ATTR_CONNECTION_POOLING)" );
			_handle = shared_ptr<void>{ handle, [](SQLHENV h)
			{
				//hangs SQLFreeHandle( SQL_HANDLE_ENV, h);
			}};
		}
		//
		//try
		//{
		//}
		//catch( const DBException& e )
		//{
		//	SQLFreeHandle( SQL_HANDLE_ENV, _handle );
		//	throw e;
		//}
	}
	//HandleEnvironment::~HandleEnvironment()
	//{
		//if( _handle )
		//	SQLFreeHandle( SQL_HANDLE_ENV, _handle );
	//}
	HDBC HandleSession::s_handle{nullptr};
	mutex HandleSession::_mutex;
	atomic<uint> index=0;
	HandleSession::HandleSession( sv connectionString )//://const HandleEnvironment& env
		//_lock(_mutex)
	{
		//i = ++index;
		//DBG( "HandleSession::HandleSession({})"sv, i );
		//_pLock = make_unique<lock_guard<mutex>>( _mutex );
		//DBG( "HandleSession::_pLock({})"sv, i );
		auto& handle = _handle;
		//if( !handle )
		{
			CALL( _env, SQL_HANDLE_ENV, SQLAllocHandle(SQL_HANDLE_DBC, _env, &handle), "SQLAllocHandle" );
			SQLCHAR connectionStringResult[8192]; 
			SQLSMALLINT connectionStringLength;
			//connectionString = "DSN=Jde_UM_Connection2"sv;
			CALL( handle, SQL_HANDLE_DBC, SQLDriverConnect(handle, nullptr, (SQLCHAR*)string(connectionString).c_str(), SQL_NTS, connectionStringResult, 8192, &connectionStringLength, SQL_DRIVER_NOPROMPT), "SQLDriverConnect" );
			//DBG0( "Leave1 HandleSession::HandleSession()" );
			if( connectionStringLength>0 )
			{
				INFO_ONCE( string((char*)connectionStringResult) );
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
		//DBG( "HandleSession::~HandleSession({})"sv, i );
	}

	HandleStatement::HandleStatement( sv connectionString ):
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