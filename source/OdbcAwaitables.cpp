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
			if( Asynchronous )
			{
				CALL( Session, SQL_HANDLE_DBC, SQLSetConnectAttr(Session, SQL_ATTR_ASYNC_DBC_FUNCTIONS_ENABLE, (SQLPOINTER)SQL_ASYNC_DBC_ENABLE_ON, SQL_IS_INTEGER), "SQLSetConnectAttr(SQL_ATTR_ASYNC_DBC_FUNCTIONS_ENABLE)" );
				CALL( Session, SQL_HANDLE_DBC, SQLSetConnectAttr(Session, SQL_ATTR_ASYNC_DBC_EVENT, Session.Event(), SQL_IS_POINTER), "SQLSetConnectAttr(SQL_ATTR_ASYNC_DBC_FUNCTIONS_ENABLE)" );
				CALL( Session, SQL_HANDLE_DBC, SQLDriverConnect(Session, nullptr, (SQLCHAR*)string(ConnectionString).c_str(), SQL_NTS, nullptr, 0,nullptr, SQL_DRIVER_NOPROMPT), "SQLDriverConnect" );
			}
			else
				Session.Connect( ConnectionString );
		}
		catch( const DBException& e )
		{
			ExceptionPtr = std::make_exception_ptr( e );
		}
		catch( ... )
		{
			DBG( "here"sv );
		}
		return ExceptionPtr!=nullptr;
	}
	α ConnectAwaitable::await_suspend( std::coroutine_handle<> h )noexcept->void
	{
		if( Asynchronous )
			OdbcWorker::Push( move(h), Session.Event(), false );
		else
			CoroutinePool::Resume( move(h) );
	}
	α ConnectAwaitable::await_resume()noexcept->TaskResult
	{
		if( Asynchronous && !ExceptionPtr  )
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
		}
		return ExceptionPtr ? TaskResult{ ExceptionPtr } : TaskResult{ make_shared<HandleSessionAsync>(move(Session)) };
	}

	α ExecuteAwaitable::await_ready()noexcept->bool
	{
		try
		{
			SQLHSTMT h;
			CALL( Statement.Session(), SQL_HANDLE_DBC, SQLAllocHandle(SQL_HANDLE_STMT, Statement.Session(), &h), "SQLAllocHandle" );
			Statement.SetHandle( h );
			CALL( Statement.Session(), SQL_HANDLE_DBC, SQLSetStmtAttr(h, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)Statement.RowStatuses().size(), 0), "SQLSetStmtAttr(SQL_ATTR_ROW_ARRAY_SIZE)" );
			CALL( Statement.Session(), SQL_HANDLE_DBC, SQLSetStmtAttr(h, SQL_ATTR_ROW_STATUS_PTR, (SQLPOINTER)Statement.RowStatuses().data(), 0), "SQLSetStmtAttr(SQL_ATTR_ROW_STATUS_PTR)" );
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
					var result = ::SQLBindParameter( Statement, ++i, SQL_PARAM_INPUT, pBinding->CodeType, pBinding->DBType, pBinding->Size(), pBinding->DecimalDigits(),  pBinding->Data(), pBinding->BufferLength(), &pBinding->Output );  THROW_IF( result<0, DBException( "{} - parameter {} returned {} - {}", _sql, i-1, result, ::GetLastError()) );
				}
			}
			CALL( Statement.Session(), SQL_HANDLE_DBC, SQLExecDirect(h, (SQLCHAR*)_sql.data(), static_cast<SQLINTEGER>(_sql.size())), "SQLExecDirect" ); 
		}
		catch( const DBException& e )
		{
			ExceptionPtr = std::make_exception_ptr( e );
		}
		return !Statement.IsAsynchronous() || ExceptionPtr!=nullptr;
	}
	α ExecuteAwaitable::await_suspend( std::coroutine_handle<> h )noexcept->void
	{
		OdbcWorker::Push( move(h), Statement.Event(), false );
	}
	α ExecuteAwaitable::await_resume()noexcept->TaskResult
	{
		if( Statement.IsAsynchronous() && !ExceptionPtr )
		{
			try
			{
				RETCODE statementResult;
				var completeAsyncResult = ::SQLCompleteAsync( SQL_HANDLE_STMT, Statement, &statementResult ); 
				THROW_IF( !SQL_SUCCEEDED(completeAsyncResult), "SQLCompleteAsync returned {} - {}", completeAsyncResult, ::GetLastError() );
				THROW_IF( !SQL_SUCCEEDED(statementResult), "SQLDriverConnect retunred {} - {}", statementResult, ::GetLastError() );
			}
			catch( const Exception& e )
			{
				ExceptionPtr = std::make_exception_ptr( move(e) );
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
				auto& bindings = Statement.Bindings();
				SQLUSMALLINT i=0;
				for( var& pBinding : bindings )
					CALL( Statement, SQL_HANDLE_STMT, ::SQLBindCol(Statement, ++i, (SQLSMALLINT)pBinding->CodeType, pBinding->Data(), pBinding->BufferLength(), &pBinding->Output), "SQLBindCol" );

				OdbcRow row{ bindings };
				for(;;)
				{
					var hr = ::SQLFetch( Statement ); 
					if( hr==SQL_NO_DATA_FOUND )
						break;
					THROW_IF( !SQL_SUCCEEDED(hr), "SQLFetch returned {} - {}", hr, GetLastError() );
					row.Reset();
					_function( row );
				}
				//for( ; i<statuses.size() && SQL_SUCCEEDED( statuses[i] ); ++Statement._result )
				//{
				//	//CALL( Statement, SQL_HANDLE_STMT, ::SQLSetPos(Statement, ++i, SQL_POSITION, SQL_LOCK_NO_CHANGE), "SQLSetPos" ); 
				//	//var hr = ;
				//	//THROW_IF( !SQL_SUCCEEDED(hr), "SQLSetPos returned {} - {}", hr, GetLastError() );
				//	SQLUSMALLINT j=0;
				//	for( auto& p : bindings )
				//	{
				//		CALL( Statement, SQL_HANDLE_STMT, ::SQLGetData(Statement, ++j, p->CodeType, p->Data(), p->Size(), &p->Output), "SQLGetData" ); 
				//	}
				//	row.Reset();
				//	_function( row );
				//}
				//Statement._moreRows = statuses.size() && i==statuses.size();
			}
			catch( const Exception& e )
			{
				ExceptionPtr = std::make_exception_ptr( move(e) );
			}
		}
		return ExceptionPtr ? TaskResult{ ExceptionPtr } : TaskResult{ make_shared<HandleStatementAsync>(move(Statement)) };
	}
}