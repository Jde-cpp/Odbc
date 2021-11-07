#include "OdbcDataSource.h"
#include "../../Framework/source/db/Database.h"
#include "../../Framework/source/db/DBException.h"
#include "MsSql/MsSqlSchemaProc.h"
#include "Bindings.h"
#include "OdbcRow.h"
#include "Handle.h"
#include "Utilities.h"

#define var const auto

Jde::DB::IDataSource* GetDataSource()
{
	return new Jde::DB::Odbc::OdbcDataSource();
}

namespace Jde::DB::Odbc
{
	void OdbcDataSource::SetConnectionString( sv x )noexcept
	{
		_connectionString = format( "{};APP={}", x, IApplication::ApplicationName() );
		DBG( "connectionString={}"sv, _connectionString );
	}
	void OdbcDataSource::SetAsynchronous()noexcept(false)
	{
		HandleSession session{ _connectionString };
		SQLUINTEGER infoValue;
		var returnValue = ::SQLGetInfo( session, SQL_ASYNC_NOTIFICATION, &infoValue, sizeof(infoValue), nullptr );
		if( !SQL_SUCCEEDED(returnValue) )
			ERR( "::SQLGetInfo(SQL_ASYNC_NOTIFICATION) returned {}  - {}"sv, returnValue, ::GetLastError() );
		else
			Asynchronous = SQL_ASYNC_NOTIFICATION_CAPABLE == infoValue;
	}

	vector<up<Binding>> AllocateBindings( const HandleStatement& statement,  SQLSMALLINT columnCount )noexcept(false)
	{ 
		vector<up<Binding>> bindings; bindings.reserve( columnCount );
		for( SQLSMALLINT iCol = 1; iCol <= columnCount; ++iCol )
		{
			SQLLEN ssType;
			CALL( statement, SQL_HANDLE_STMT, ::SQLColAttribute(statement, iCol, SQL_DESC_CONCISE_TYPE, NULL, 0, NULL, &ssType), "SQLColAttribute::Concise" );

			SQLLEN bufferSize = 0;
			up<Binding> pBinding;
			if( ssType == SQL_CHAR || ssType == SQL_VARCHAR || ssType == SQL_LONGVARCHAR || ssType == -9/*varchar(max)?*/ || ssType == -10/*nvarchar(max)?*/ )
			{
				CALL( statement, SQL_HANDLE_STMT, ::SQLColAttribute(statement, iCol, SQL_DESC_DISPLAY_SIZE, NULL, 0, NULL, &bufferSize), "SQLColAttribute::Display" );
				if( (ssType==-9 || ssType==-10) && bufferSize==0 )
					bufferSize = (1 << 14) - 1;//TODO handle varchar(max).
				pBinding = make_unique<BindingString>( (SQLSMALLINT)ssType, ++bufferSize );
			}
			else
				pBinding = Binding::GetBinding( (SQLSMALLINT)ssType );
		
			CALL( statement, SQL_HANDLE_STMT, ::SQLBindCol( statement, iCol, (SQLSMALLINT)pBinding->CodeType, pBinding->Data(), bufferSize, &pBinding->Output), "SQLBindCol" );
			bindings.push_back( move(pBinding) );
		}
		return bindings;
	}

	uint OdbcDataSource::Execute( sv sql )noexcept(false){ return Select( sql, nullptr, nullptr, true ); }
	uint OdbcDataSource::Execute( sv sql, const std::vector<DataValue>& parameters, bool log)noexcept(false){ return Execute(sql, &parameters, nullptr, false, log); }
	uint OdbcDataSource::Execute( sv sql, const std::vector<DataValue>* pParameters, std::function<void(const IRow&)>* f, bool isStoredProc, bool log, const source_location& sl )noexcept(false){  return Select( sql, f, pParameters, log, sl );  }
	uint OdbcDataSource::ExecuteProc( sv sql, const std::vector<DataValue>& parameters, bool log, const source_location& sl )noexcept(false){ return Select( format("{{call {} }}", sql), nullptr, &parameters, log, sl); }
	uint OdbcDataSource::ExecuteProc( sv sql, const std::vector<DataValue>& parameters, std::function<void(const IRow&)> f, bool log, const source_location& sl )noexcept(false){ return Select(format( "{{call {} }}", sql), f, &parameters, log, sl); }

	sp<ISchemaProc> OdbcDataSource::SchemaProc()noexcept
	{
		return make_shared<MsSql::MsSqlSchemaProc>( shared_from_this() );
	}
	α OdbcDataSource::Select( sv sql, std::function<void(const IRow&)>* f, const std::vector<DataValue>* pParameters, bool log, const source_location& sl )noexcept(false)->uint
	{
		HandleStatement statement{ _connectionString };
		vector<SQLUSMALLINT> paramStatusArray;
		vector<up<Binding>> parameters;
		void* pData = nullptr;
		if( pParameters )
		{
			parameters.reserve( pParameters->size() );
			SQLUSMALLINT iParameter = 0;
			for( var& param : *pParameters )
			{
				auto pBinding = Binding::Create( param );
				pData = pBinding->Data();
				var size = pBinding->Size(); var bufferLength = pBinding->BufferLength();
				if( pBinding->DBType==SQL_DATETIME )
					DBG( "fractions={}"sv, dynamic_cast<const BindingDateTime*>(pBinding.get())->_data.fraction );
				var result = SQLBindParameter( statement, ++iParameter, SQL_PARAM_INPUT, pBinding->CodeType, pBinding->DBType, size, pBinding->DecimalDigits(),  pData, bufferLength, &pBinding->Output );
				THROW_IFX( result<0, DBException(result, sql, pParameters, format("parameter {}", (uint)iParameter-1), sl)  );
				parameters.push_back( move(pBinding) );
			}
		}
		uint resultCount = 0;
		DB::Log( sql, pParameters, sl );
		var retCode = SQLExecDirect( statement, (SQLCHAR*)sql.data(), static_cast<SQLINTEGER>(sql.size()) );
		switch( retCode )
		{
		case SQL_SUCCESS_WITH_INFO:
			HandleDiagnosticRecord( "SQLExecDirect", statement, SQL_HANDLE_STMT, retCode );
		case SQL_SUCCESS:
		{
			SQLSMALLINT columnCount=0;
			if( f )
				CALL( statement, SQL_HANDLE_STMT, SQLNumResultCols(statement,&columnCount), "SQLNumResultCols" );
			if( columnCount > 0 )
			{
				var bindings = AllocateBindings( statement, columnCount );
				OdbcRow row{ bindings };
				while( ::SQLFetch(statement)!=SQL_NO_DATA_FOUND )
				{
					row.Reset();
					(*f)( row );
				}
			} 
			else
			{
				SQLLEN count;
				CALL( statement, SQL_HANDLE_STMT, SQLRowCount(statement,&count), "SQLRowCount" );
				resultCount = count;
			}
			break;
		}
		case SQL_INVALID_HANDLE:
			throw DBException( retCode, sql, pParameters, "SQL_INVALID_HANDLE" );
			break;
		case SQL_ERROR:
			throw DBException{ retCode, sql, pParameters, HandleDiagnosticRecord("SQLExecDirect", statement, SQL_HANDLE_STMT, retCode, sl), sl };
		default:
			throw DBException( retCode, sql, pParameters, "Unknown error" );
		}
		return resultCount;
	}
	
	α OdbcDataSource::SelectCo( string&& ql, std::function<void(const IRow&)> f, const std::vector<DataValue>&& parameters, bool log )noexcept->up<IAwaitable>
	{ 
		return make_unique<FunctionAwaitable>( [sql=move(ql),f,parms=move(parameters),log,this]( coroutine_handle<Task2::promise_type> h )mutable->Task2
		{
			try
			{
				auto pBindings = parms.size() ? make_unique<vector<up<Binding>>>() : up<vector<up<Binding>>>{};
				if( pBindings )
				{
					pBindings->reserve( parms.size() );
					for( var& param : parms )
						pBindings->push_back( Binding::Create(param) );
				}
				auto pSession = (co_await Connect()).Get<HandleSessionAsync>();
				auto pStatement = ( co_await Execute(move(*pSession), string(sql), move(pBindings), log) ).Get<HandleStatementAsync>();
				if( f )
					pStatement = ( co_await Fetch(move(*pStatement), f) ).Get<HandleStatementAsync>();
				else
					CALL( *pStatement, SQL_HANDLE_STMT, ::SQLRowCount(*pStatement, (SQLLEN*)&pStatement->_result), "SQLRowCount" );

				h.promise().get_return_object().SetResult( make_shared<uint>(pStatement->_result) );
			}
			catch( const std::exception& e )
			{
				h.promise().get_return_object().SetResult( std::make_exception_ptr(e) );
			}
			h.resume();
		}); 
	}
}
