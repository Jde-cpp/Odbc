#include "OdbcAwaitables.h"
#include "OdbcWorker.h"
#include "../../Framework/source/db/DBException.h"
#include "Utilities.h"
#include "OdbcRow.h"

#define var const auto
namespace Jde::DB::Odbc
{
	α ConnectAwaitable::await_ready()noexcept->bool
	{
		try
		{
/*			if( Asynchronous )
			{
				CALL( Session, SQL_HANDLE_DBC, SQLSetConnectAttr(Session, SQL_ATTR_ASYNC_DBC_FUNCTIONS_ENABLE, (SQLPOINTER)SQL_ASYNC_DBC_ENABLE_ON, SQL_IS_INTEGER), "SQLSetConnectAttr(SQL_ATTR_ASYNC_DBC_FUNCTIONS_ENABLE)" );
				CALL( Session, SQL_HANDLE_DBC, SQLSetConnectAttr(Session, SQL_ATTR_ASYNC_DBC_EVENT, Session.Event(), SQL_IS_POINTER), "SQLSetConnectAttr(SQL_ATTR_ASYNC_DBC_FUNCTIONS_ENABLE)" );
				CALL( Session, SQL_HANDLE_DBC, SQLDriverConnect(Session, nullptr, (SQLCHAR*)string(ConnectionString).c_str(), SQL_NTS, nullptr, 0,nullptr, SQL_DRIVER_NOPROMPT), "SQLDriverConnect" );
			}
			else*/
				Session.Connect( ConnectionString );
		}
		catch( DBException& e )
		{
			ExceptionPtr = e.Clone();
		}
		catch( ... )
		{
			CRITICAL( "Unexpected Exception"sv );
		}
		return ExceptionPtr!=nullptr;
	}
	α ConnectAwaitable::await_suspend( std::coroutine_handle<> h )noexcept->void
	{
/*		if( Asynchronous )
			OdbcWorker::Push( move(h), Session.Event(), false );
		else*/
			CoroutinePool::Resume( move(h) );
	}
	α ConnectAwaitable::await_resume()noexcept->TaskResult
	{
		/*if( Asynchronous && !ExceptionPtr  )
		{
			try
			{
				RETCODE connectResult;
				var completeAsyncResult = ::SQLCompleteAsync( SQL_HANDLE_DBC, Session, &connectResult );
				THROW_IF( !SQL_SUCCEEDED(completeAsyncResult), "SQLCompleteAsync returned {} - {}", completeAsyncResult, ::GetLastError() );
				THROW_IF( !SQL_SUCCEEDED(connectResult), "SQLDriverConnect retunred {} - {}", connectResult, ::GetLastError() );
			}
			catch( const Exception& e )
			{
				ExceptionPtr = std::make_exception_ptr( move(e) );
			}
		}*/
		return ExceptionPtr ? TaskResult{ ExceptionPtr } : TaskResult{ make_shared<HandleSessionAsync>(move(Session)) };
	}

	α ExecuteAwaitable::await_ready()noexcept->bool
	{
		try
		{
			SQLHSTMT h;
			CALL( Statement.Session(), SQL_HANDLE_DBC, SQLAllocHandle(SQL_HANDLE_STMT, Statement.Session(), &h), "SQLAllocHandle" );
			Statement.SetHandle( h );
			CALL( Statement.Session(), SQL_HANDLE_DBC, SQLSetStmtAttr(h, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)Statement.RowStatusesSize(), 0), "SQLSetStmtAttr(SQL_ATTR_ROW_ARRAY_SIZE)" );
			CALL( Statement.Session(), SQL_HANDLE_DBC, SQLSetStmtAttr(h, SQL_ATTR_ROW_STATUS_PTR, (SQLPOINTER)Statement.RowStatuses(), 0), "SQLSetStmtAttr(SQL_ATTR_ROW_STATUS_PTR)" );
			if( Statement.IsAsynchronous() )
			{
				CALL( Statement.Session(), SQL_HANDLE_DBC, SQLSetStmtAttr(h, SQL_ATTR_ASYNC_ENABLE, (SQLPOINTER)SQL_ASYNC_ENABLE_ON, SQL_IS_POINTER), "SQLSetStmtAttr(SQL_ASYNC_ENABLE_ON)" );
				CALL( Statement.Session(), SQL_HANDLE_DBC, SQLSetStmtAttr(h, SQL_ATTR_ASYNC_STMT_EVENT, Statement.Event(), SQL_IS_POINTER), "SQLSetStmtAttr(SQL_ATTR_ASYNC_STMT_EVENT)" );
			}
			if( _pBindings )
			{
				SQLUSMALLINT i = 0;
				for( var& pBinding : *_pBindings )
				{
					var result = ::SQLBindParameter( Statement, ++i, SQL_PARAM_INPUT, pBinding->CodeType, pBinding->DBType, pBinding->Size(), pBinding->DecimalDigits(),  pBinding->Data(), pBinding->BufferLength(), &pBinding->Output );  THROW_IFX( result<0, DBException(result, _sql, nullptr, format("parameter {} returned {} - {}", i-1, result, ::GetLastError()), SRCE_CUR) );
					THROW_IFX( !SUCCEEDED(result), DBException(move(_sql), nullptr, format("SQLBindParameter {}", result)) );
				}
			}
			var retCode = ::SQLExecDirect( Statement, (SQLCHAR*)_sql.data(), (SQLINTEGER)_sql.size() );
			if( SUCCEEDED(retCode) )
			{
				HandleDiagnosticRecord( "SQLExecDirect", Statement, SQL_HANDLE_STMT, retCode );
				SQLLEN count;
				CALL( Statement, SQL_HANDLE_STMT, ::SQLRowCount(Statement,&count), "SQLRowCount" );
				Statement._result = count;
			}
			else if( retCode==SQL_INVALID_HANDLE )
				throw DBException( retCode, move(_sql), &_params, "SQL_INVALID_HANDLE" );
			else if( retCode==SQL_ERROR )
				throw DBException{ retCode, move(_sql), &_params, HandleDiagnosticRecord("SQLExecDirect", Statement, SQL_HANDLE_STMT, retCode, _sl), _sl };
			else
				throw DBException( retCode, move(_sql), &_params, "Unknown error" );
		}
		catch( DBException& e )
		{
			ExceptionPtr = e.Clone();
		}
		return !Statement.IsAsynchronous() || ExceptionPtr!=nullptr;
	}
	α ExecuteAwaitable::await_suspend( std::coroutine_handle<> h )noexcept->void
	{
		OdbcWorker::Push( move(h), Statement.Event(), false );//never gets called.
	}
	α ExecuteAwaitable::await_resume()noexcept->TaskResult
	{
		if( Statement.IsAsynchronous() && !ExceptionPtr )//never asynchronous
		{
			try
			{
				RETCODE statementResult;
				var completeAsyncResult = ::SQLCompleteAsync( SQL_HANDLE_STMT, Statement, &statementResult );
				THROW_IF( !SQL_SUCCEEDED(completeAsyncResult), "SQLCompleteAsync returned {} - {}", completeAsyncResult, ::GetLastError() );
				THROW_IF( !SQL_SUCCEEDED(statementResult), "SQLDriverConnect retunred {} - {}", statementResult, ::GetLastError() );
			}
			catch( Exception& e )
			{
				ExceptionPtr = e.Clone();
			}
		}
		return ExceptionPtr ? TaskResult{ ExceptionPtr } : TaskResult{ make_shared<HandleStatementAsync>(move(Statement)) };
	}

	α FetchAwaitable::await_ready()noexcept->bool
	{
		return !IsAsynchronous();
		//try
		//{
		//	var hr = ::SQLFetch( Statement );
		//	THROW_IF( !SQL_SUCCEEDED(hr), "SQLFetch returned {} - {}", hr, GetLastError() );
		//}
		//catch( const Exception& e )
		//{
		//	ExceptionPtr = std::make_exception_ptr( e );
		//}
		//return !IsAsynchronous() || ExceptionPtr!=nullptr;
	}
	α FetchAwaitable::await_suspend( std::coroutine_handle<> h )noexcept->void
	{
		OdbcWorker::Push( move(h), Statement.Event(), false );
	}
	α FetchAwaitable::await_resume()noexcept->TaskResult
	{
		if( !ExceptionPtr )
		{
			try
			{
				var& bindings = Statement.OBindings();

/*				BindingInt32s ib{5};
				//SQLLEN ids[50]={0};
				CALL( Statement, SQL_HANDLE_STMT, ::SQLBindCol(Statement, 1, (SQLSMALLINT)ib.CodeType(), ib.Data(), ib.BufferLength(), ib.OutputPtr()), "SQLBindCol" );
				BindingStrings<SQL_CHAR> sb{5, 256};
				SQLLEN l = sb.BufferLength();
				SQLLEN x[50]={0};
				char sz[256][256];
				//DBG( "sizeof={:x} data={:x}, DBType={:x}, CodeType={:x} Output={:x} Buffer={:x} size={:x}", sizeof(BindingString), (uint16)&sb, (uint16)&sb.DBType, (uint16)&sb.CodeType, (uint16)&sb.Output, (uint16)&sb._pBuffer, (uint16)&sb._size );
				var hr2 = ::SQLBindCol( Statement, 2, (SQLSMALLINT)sb.CodeType(), sz, 10, x );//sb.BufferLength()
				//CALL( Statement, SQL_HANDLE_STMT, ::SQLBindCol(Statement, 2, (SQLSMALLINT)sb.CodeType, sb.Data(), sb.BufferLength(), &sb.Output), "SQLBindCol" );
				var hr3 = ::SQLFetch( Statement );
				*/
				SQLUSMALLINT i=0;
				for( var& p : bindings )
					CALL( Statement, SQL_HANDLE_STMT, ::SQLBindCol(Statement, ++i, (SQLSMALLINT)p->CodeType(), p->Data(), p->BufferLength(), p->OutputPtr()), "SQLBindCol" );

				OdbcRowMulti row( bindings );
				HRESULT hr;
				while( (hr = ::SQLFetch(Statement))==SQL_SUCCESS || hr==SQL_SUCCESS_WITH_INFO )
				{
					uint i=0;
					for( ; i<Statement.RowStatusesSize() && Statement.RowStatuses()[i]==SQL_ROW_SUCCESS; ++i )
					{
						_function->OnRow( row );
						row.Reset();
					}
					if( i<Statement.RowStatusesSize() && Statement.RowStatuses()[i]==SQL_ROW_NOROW )
						break;
					ASSERT( i==Statement.RowStatusesSize() || Statement.RowStatuses()[i]!=SQL_ROW_ERROR );//allocate more space.
					row.ResetRowIndex();
				}
				THROW_IF( hr!=SQL_NO_DATA && !SQL_SUCCEEDED(hr), "SQLFetch returned {} - {}", hr, GetLastError() );
			}
			catch( IException& e )
			{
				ExceptionPtr = e.Clone();
			}
		}
		return ExceptionPtr ? TaskResult{ ExceptionPtr } : TaskResult{ make_shared<HandleStatementAsync>(move(Statement)) };
	}
}