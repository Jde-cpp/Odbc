#include "OdbcDataSource.h"
#include "../../Framework/source/db/Database.h"
#include "../../Framework/source/db/DBException.h"
#include "MsSql/MsSqlSchemaProc.h"
#include "Bindings.h"
#include "OdbcRow.h"
#include "Handle.h"
#include "Utilities.h"

#define var const auto
#define _logTag LogTag()

Jde::DB::IDataSource* GetDataSource(){
	return new Jde::DB::Odbc::OdbcDataSource();
}

namespace Jde::DB::Odbc{

	α AllocateBindings( const HandleStatement& statement,  SQLSMALLINT columnCount )ε->vector<up<Binding>>;

	α ExecDirect( string cs, string sql, const RowΛ* f, const vector<object>* pParams, SL sl, bool log=true )ε->uint{
		HandleStatement statement{ move(cs) };
		vector<SQLUSMALLINT> paramStatusArray;
		vector<up<Binding>> parameters;
		void* pData = nullptr;
		if( pParams ){
			parameters.reserve( pParams->size() );
			SQLUSMALLINT iParameter = 0;
			for( var& param : *pParams ){
				auto pBinding = Binding::Create( param );
				pData = pBinding->Data();
				var size = pBinding->Size(); var bufferLength = pBinding->BufferLength();
				if( pBinding->DBType==SQL_DATETIME )
					TRACE( "fractions={}", dynamic_cast<const BindingDateTime*>(pBinding.get())->_data.fraction );
				var decimals = pBinding->DecimalDigits();
				var result = SQLBindParameter( statement, ++iParameter, SQL_PARAM_INPUT, pBinding->CodeType, pBinding->DBType, size, decimals, pData, bufferLength, &pBinding->Output );
				THROW_IFX(result < 0, DBException(result, move(sql), pParams, HandleDiagnosticRecord("SQLBindParameter", statement, SQL_HANDLE_STMT, result, sl), sl) );
				parameters.push_back( move(pBinding) );
			}
		}
		uint resultCount = 0;
		if( log )
			DB::Log( sql, pParams, sl );
		//DEBUG_IF( sql.find("call")!=string::npos );
		var retCode = ::SQLExecDirect( statement, (SQLCHAR*)sql.data(), static_cast<SQLINTEGER>(sql.size()) );
		switch( retCode ){
		case SQL_NO_DATA://update with no records effected...
			DBG( "noData={}", sql );
		case SQL_SUCCESS_WITH_INFO:
			try{
				HandleDiagnosticRecord( "SQLExecDirect", statement, SQL_HANDLE_STMT, retCode );
			}
			catch( const DBException& e ){
				throw DBException{ retCode, sql, pParams, e.what(), sl };
			}
		case SQL_SUCCESS:{
			SQLSMALLINT columnCount=0;
			if( f )
				CALL( statement, SQL_HANDLE_STMT, SQLNumResultCols(statement,&columnCount), "SQLNumResultCols" );
			if( columnCount>0 ){
				var bindings = AllocateBindings( statement, columnCount );
				OdbcRow row{ bindings };
				while( ::SQLFetch(statement)!=SQL_NO_DATA_FOUND ){
					row.Reset();
					(*f)( row );
				}
			}
			else{
				SQLLEN count;
				CALL( statement, SQL_HANDLE_STMT, SQLRowCount(statement,&count), "SQLRowCount" );
				resultCount = count;
			}
			break;
		}
		case SQL_INVALID_HANDLE:
			throw DBException( retCode, sql, pParams, "SQL_INVALID_HANDLE" );
			break;
		case SQL_ERROR:
			throw DBException{ retCode, sql, pParams, HandleDiagnosticRecord("SQLExecDirect", statement, SQL_HANDLE_STMT, retCode, sl), sl };
		default:
			throw DBException( retCode, sql, pParams, "Unknown error" );
		}
		return resultCount;
	}

	void OdbcDataSource::SetConnectionString( string x )ι{
		_connectionString = Ƒ( "{};APP={}", move(x), Process::ApplicationName() );
		DBG( "connectionString={}"sv, _connectionString );
	}

	vector<up<Binding>> AllocateBindings( const HandleStatement& statement,  SQLSMALLINT columnCount )ε{
		vector<up<Binding>> bindings; bindings.reserve( columnCount );
		for( SQLSMALLINT iCol = 1; iCol <= columnCount; ++iCol ){
			SQLLEN ssType;
			CALL( statement, SQL_HANDLE_STMT, ::SQLColAttribute(statement, iCol, SQL_DESC_CONCISE_TYPE, NULL, 0, NULL, &ssType), "SQLColAttribute::Concise" );

			SQLLEN bufferSize = 0;
			up<Binding> pBinding;
			if( ssType == SQL_CHAR || ssType == SQL_VARCHAR || ssType == SQL_LONGVARCHAR || ssType == -9/*varchar(max)?*/ || ssType == -10/*nvarchar(max)?*/ ){
				CALL( statement, SQL_HANDLE_STMT, ::SQLColAttribute(statement, iCol, SQL_DESC_DISPLAY_SIZE, NULL, 0, NULL, &bufferSize), "SQLColAttribute::Display" );
				if( (ssType==-9 || ssType==-10) && bufferSize==0 )
					bufferSize = (1 << 14) - 1;//TODO handle varchar(max).
				pBinding = mu<BindingString>( (SQLSMALLINT)ssType, ++bufferSize );
			}
			else
				pBinding = Binding::GetBinding( (SQLSMALLINT)ssType );

			CALL( statement, SQL_HANDLE_STMT, ::SQLBindCol(statement, iCol, (SQLSMALLINT)pBinding->CodeType, pBinding->Data(), bufferSize, &pBinding->Output), "SQLBindCol" );
			bindings.push_back( move(pBinding) );
		}
		return bindings;
	}

	α OdbcDataSource::Execute( string sql, SL sl )ε->uint{ return Select( sql, nullptr, nullptr, sl ); }
	α OdbcDataSource::Execute( string sql, const vector<object>& parameters, SL sl)ε->uint{ return Execute(sql, &parameters, nullptr, false, sl); }
	α OdbcDataSource::Execute( string sql, const vector<object>* pParameters, const RowΛ* f, bool /*isStoredProc*/, SL sl )ε->uint{ return ExecDirect( CS(), sql, f, pParameters, sl );  }
	α OdbcDataSource::ExecuteCo( string sql, vector<object> p, SL sl )ι->up<IAwait>{
		return SelectCo( nullptr, move(sql), move(p), sl );
	}
	α OdbcDataSource::ExecuteCo( string sql_, vector<object> p, bool proc_, RowΛ f, SL sl )ε->up<IAwait>{
		return mu<TPoolAwait<uint>>( [sql=move(sql_), params=move(p), sl, proc=proc_, func=f, this]()ε{
			return mu<uint>( Execute(move(sql), &params, &func, proc, sl) );
			}, "ExecuteCo", sl );
	}
	α OdbcDataSource::ExecuteNoLog( string sql, const vector<object>* p, RowΛ* f, bool, SL sl )ε->uint{ return ExecDirect( CS(),move(sql), f, p, sl, false );  }
	α OdbcDataSource::ExecuteProcNoLog( string sql, vec<object> v, SL sl )ε->uint{ return ExecDirect( CS(), Ƒ("{{call {} }}", move(sql)), nullptr, &v, sl, false ); }
	α OdbcDataSource::ExecuteProc( string sql, const vector<object>& parameters, SL sl )ε->uint{ return ExecDirect( CS(), Ƒ("{{call {} }}", sql), nullptr, &parameters, sl); }
	α OdbcDataSource::ExecuteProc( string sql, const vector<object>& parameters, RowΛ f, SL sl )ε->uint{ return Select(Ƒ("{{call {} }}", move(sql)), f, &parameters, sl); }
	α OdbcDataSource::ExecuteProcCo( string sql, vector<object> p, SL sl )ι->up<IAwait>{
		return ExecuteCo(Ƒ("{{call {} }}", move(sql)), move(p), sl );
	}
	
	α OdbcDataSource::ExecuteProcCo( string sql, vector<object> params, RowΛ f, SL sl )ε->up<IAwait>{
		return ExecuteCo( Ƒ("{{call {} }}", move(sql)), move(params), true, f, sl );
	}

	sp<ISchemaProc> OdbcDataSource::SchemaProc()ι{
		return ms<MsSql::MsSqlSchemaProc>( shared_from_this() );
	}
	α OdbcDataSource::SelectNoLog( string sql, RowΛ f, const vector<object>* p, SL sl )ε->uint{ return ExecDirect(CS(), move(sql), &f, p, sl, false); }
	α OdbcDataSource::Select( string sql, RowΛ f, const vector<object>* p, SL sl )ε->uint{ return ExecDirect( CS(), move(sql), &f, p, sl ); }

	α OdbcDataSource::SelectCo( ISelect* pAwait, string sql_, vector<object>&& params_, SL sl_ )ι->up<IAwait>{
		return mu<TPoolAwait<uint>>( [cs=CS(),pAwait,sql=move(sql_),params=move(params_), sl=sl_]()->up<uint>{
			uint y;
			if( pAwait ){
				function<void( const IRow& )> f = [pAwait](var& r){pAwait->OnRow(r);};
				y = ExecDirect( cs, sql, &f, &params, sl );
			}
			else
				y = ExecDirect( cs, sql, nullptr, &params, sl );
			return mu<uint>( y );
			//return y;
		});
/*		return mu<AsyncAwait>( [pAwait,sql=move(sql_),params=move(params_), sl=sl_,this]( HCoroutine h )mutable->Task
		{
			try
			{
				auto pBindings = params.size() ? mu<vector<up<Binding>>>() : up<vector<up<Binding>>>{};
				if( pBindings )
				{
					pBindings->reserve( params.size() );
					for( var& param : params )
						pBindings->push_back( Binding::Create(param) );
				}
				auto pSession =  (co_await Connect() ).SP<HandleSessionAsync>();
				DB::Log( sql, &params, sl );
				auto pStatement = ( co_await Execute(move(*pSession), move(sql), move(pBindings), move(params), sl) ).SP<HandleStatementAsync>();
				if( pAwait )
					pStatement = ( co_await Fetch(move(*pStatement), pAwait) ).SP<HandleStatementAsync>();
				else
				{
					CALL( *pStatement, SQL_HANDLE_STMT, ::SQLRowCount(*pStatement, (SQLLEN*)&pStatement->_result), "SQLRowCount" );
					h.promise().get_return_object().SetResult( mu<uint>(pStatement->_result) );
				}
			}
			catch( IException& e )
			{
				h.promise().get_return_object().SetResult( e.Clone() );
			}
			h.resume();
		}, sl_);*/
	}
}