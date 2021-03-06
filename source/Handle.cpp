﻿#include "Handle.h"
#include <sql.h>
#include <sqlext.h>
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
			var rc=SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &handle ); THROW_IF( rc==SQL_ERROR, DBException("({}) - Unable to allocate an environment handle", rc) );
			CALL( handle, SQL_HANDLE_ENV, SQLSetEnvAttr(handle, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0), "SQLSetEnvAttr(SQL_ATTR_ODBC_VERSION)" );
			_handle = shared_ptr<void>{ handle, [](SQLHENV h)
			{
			}};
		}
	}

	//HDBC HandleSession::s_handle{nullptr};
	//mutex HandleSession::_mutex;
	//atomic<uint> index=0;
	HandleSession::HandleSession()noexcept(false)
	{
		CALL( _env, SQL_HANDLE_ENV, SQLAllocHandle(SQL_HANDLE_DBC, _env, &_handle), "SQLAllocHandle" );
	}
	HandleSession::HandleSession( sv connectionString )noexcept(false):
		HandleSession{}
	{
		Connect( connectionString );
	}
	auto HandleSession::Connect( sv connectionString )noexcept(false)->void
	{
		SQLCHAR connectionStringResult[8192]; 
		SQLSMALLINT connectionStringLength;
		CALL( _handle, SQL_HANDLE_DBC, SQLDriverConnect(_handle, nullptr, (SQLCHAR*)string(connectionString).c_str(), SQL_NTS, connectionStringResult, 8192, &connectionStringLength, SQL_DRIVER_NOPROMPT), "SQLDriverConnect" );
		if( connectionStringLength>0 )
			INFO_ONCE( string((char*)connectionStringResult) );
		else
			CRITICAL( "connectionString Length={}"sv, connectionStringLength );
	}
	
	auto HandleSessionAsync::Connect( sv connectionString )noexcept(false)->void
	{
		if( IsAsynchronous() )
		{
			CALL( _handle, SQL_HANDLE_DBC, SQLSetConnectAttr(_handle, SQL_ATTR_ASYNC_DBC_FUNCTIONS_ENABLE, (SQLPOINTER)SQL_ASYNC_DBC_ENABLE_ON, SQL_IS_INTEGER), "SQLSetConnectAttr(SQL_ATTR_ASYNC_DBC_FUNCTIONS_ENABLE)" );
			_event = CreateEvent( nullptr, false, false, nullptr ); THROW_IF( !_event, "CreateEvent - {}"sv, GetLastError() );
			CALL( _handle, SQL_HANDLE_DBC, SQLSetConnectAttr(_handle, SQL_ATTR_ASYNC_DBC_EVENT, _event, SQL_IS_POINTER), "SQLSetConnectAttr(SQL_ATTR_ASYNC_DBC_FUNCTIONS_ENABLE)" );
		}
		else
			HandleSession::Connect( connectionString );
	}

	HandleSession::~HandleSession()
	{
		if( _handle )
		{
			SQLDisconnect( _handle );
			SQLFreeHandle( SQL_HANDLE_DBC, _handle );
		}
	}

	HandleStatement::HandleStatement( sv connectionString )noexcept(false):
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
	HandleStatementAsync::~HandleStatementAsync()
	{
		WARN_IF( _event && !::CloseHandle(_event), "CloseHandle returned {}", ::GetLastError() );
		var rc = _handle ? ::SQLFreeHandle( SQL_HANDLE_STMT, _handle ) : SQL_SUCCESS;
		WARN_IF( rc!=SQL_SUCCESS, "SQLFreeHandle(SQL_HANDLE_STMT) returned {} - {}", rc, ::GetLastError() );
	}
	α HandleStatementAsync::Bindings()noexcept(false)->const vector<up<Binding>>&
	{
		if( !_bindings.size() )
		{
			SQLSMALLINT columnCount;
			CALL( _handle, SQL_HANDLE_STMT, SQLNumResultCols(_handle,&columnCount), "SQLNumResultCols" );
			_bindings.reserve( columnCount );
			for( SQLSMALLINT iCol = 1; iCol <= columnCount; ++iCol )
			{
				SQLLEN ssType;
				CALL( _handle, SQL_HANDLE_STMT, ::SQLColAttribute(_handle, iCol, SQL_DESC_CONCISE_TYPE, NULL, 0, NULL, &ssType), "SQLColAttribute::Concise" );

				SQLLEN bufferSize = 0;
				if( ssType == SQL_CHAR || ssType == SQL_VARCHAR || ssType == SQL_LONGVARCHAR || ssType == -9/*varchar(max)?*/ )
				{
					CALL( _handle, SQL_HANDLE_STMT, ::SQLColAttribute(_handle, iCol, SQL_DESC_DISPLAY_SIZE, NULL, 0, NULL, &bufferSize), "SQLColAttribute::Display" );
					if( ssType==-9 && bufferSize==0 )
						bufferSize = (1 << 14) - 1;//TODO handle varchar(max).
					_bindings.emplace_back( make_unique<BindingString>((SQLSMALLINT)ssType, ++bufferSize) );
				}
				else
					_bindings.emplace_back( Binding::GetBinding((SQLSMALLINT)ssType) );
			}
		}
		return _bindings;
	}
}