#include "OdbcDataSource.h"
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
	vector<up<Binding>> AllocateBindings( const HandleStatement& statement,  SQLSMALLINT columnCount )noexcept(false)
	{ 
		vector<up<Binding>> bindings; bindings.reserve( columnCount );
		for( SQLSMALLINT iCol = 1; iCol <= columnCount; iCol++ )
		{
			SQLLEN ssType;
			CALL( statement, SQL_HANDLE_STMT, SQLColAttribute(statement, iCol, SQL_DESC_CONCISE_TYPE, NULL, 0, NULL, &ssType), "SQLColAttribute::Concise" );

			SQLLEN bufferSize = 0;
			up<Binding> pBinding;
			if( ssType == SQL_CHAR || ssType == SQL_VARCHAR || ssType == SQL_LONGVARCHAR || ssType == -9/*varchar(max)?*/ )
			{
				CALL( statement, SQL_HANDLE_STMT, SQLColAttribute(statement, iCol, SQL_DESC_DISPLAY_SIZE, NULL, 0, NULL, &bufferSize), "SQLColAttribute::Display" );
				if( ssType==-9 && bufferSize==0 )
					bufferSize = (1 << 14) - 1;//TODO handle varchar(max).
				pBinding = make_unique<BindingString>( (SQLSMALLINT)ssType, ++bufferSize );
			}
			else
				pBinding = Binding::GetBinding( (SQLSMALLINT)ssType );
		
			CALL( statement, SQL_HANDLE_STMT, SQLBindCol( statement, iCol, (SQLSMALLINT)pBinding->CodeType, pBinding->Data(), bufferSize, &pBinding->Output), "SQLBindCol" );
			bindings.push_back( move(pBinding) );
		}
		return bindings;
	}

	uint OdbcDataSource::Execute( sv sql )noexcept(false){ return Select( sql, nullptr, nullptr, true ); }
	uint OdbcDataSource::Execute( sv sql, const std::vector<DataValue>& parameters, bool log)noexcept(false){ return Execute(sql, &parameters, nullptr, false, log); }
	uint OdbcDataSource::Execute( sv sql, const std::vector<DataValue>* pParameters, std::function<void(const IRow&)>* f, bool isStoredProc, bool log )noexcept(false){ return Select( sql, f, pParameters, log ); }

	//uint OdbcDataSource::Execute(sv sql, const std::vector<DataValue>& parameters, std::function<void(const IRow&)> f, bool log)noexcept(false){ return Query(sql, log, &f, &parameters); }
	uint OdbcDataSource::ExecuteProc( sv sql, const std::vector<DataValue>& parameters, bool log )noexcept(false){ return Select( format( "{{call {} }}", sql), nullptr, &parameters, log); }
	uint OdbcDataSource::ExecuteProc( sv sql, const std::vector<DataValue>& parameters, std::function<void(const IRow&)> f, bool log )noexcept(false){ return Select(format( "{{call {} }}", sql), f, &parameters, log); }
	//void OdbcDataSource::Select(sv sql, std::function<void(const IRow&)> f, const std::vector<DataValue>& values, bool log)noexcept(false){ Query( sql, log, &f, &values ); }
	//void OdbcDataSource::Select(sv sql, std::function<void(const IRow&)> f )noexcept(false){ Query( sql, false, &f ); }

	sp<ISchemaProc> OdbcDataSource::SchemaProc()noexcept
	{
		return make_shared<MsSql::MsSqlSchemaProc>( shared_from_this() );
		//return {};
	}
	uint OdbcDataSource::Select( sv sql, std::function<void(const IRow&)>* f, const std::vector<DataValue>* pParameters, bool log )noexcept(false)
	{
		HandleStatement statement{ ConnectionString };
		vector<SQLUSMALLINT> paramStatusArray;
		vector<up<Binding>> parameters;
		void* pData = nullptr;
		//SQLINTEGER paramsProcessed=0;
		if( pParameters )
		{
			//0x000001e557a560a0 "{call log_application_instance_insert2(?,?,?) }"
	//		auto retcode = SQLSetStmtAttr( statement, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER)pParameters->size(), 0 ); THROW_IF( retcode<0, DBException(format("{} - SQL_ATTR_PARAMSET_SIZE returned {}", sql, retcode)) );
//			paramStatusArray.reserve( pParameters->size() );
	//		retcode = SQLSetStmtAttr( statement, SQL_ATTR_PARAM_STATUS_PTR, paramStatusArray.data(), pParameters->size() ); THROW_IF( retcode<0, DBException(format("{} - SQL_ATTR_PARAM_STATUS_PTR returned {}", sql, retcode)) );
			//retcode = SQLSetStmtAttr( statement, SQL_ATTR_PARAMS_PROCESSED_PTR, &paramsProcessed, 0 ); THROW_IF( retcode<0, DBException(format("{} - SQL_ATTR_PARAMS_PROCESSED_PTR returned {}", sql, retcode)) );
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
				THROW_IF( result<0, DBException( "{} - parameter {} returned {}", sql, iParameter-1, result) );
				parameters.push_back( move(pBinding) );
			}
		}
		uint resultCount = 0;
		DBG( sql );
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
				for(;;)
				{
					var retCode2 = ::SQLFetch( statement );
					if( retCode2==SQL_NO_DATA_FOUND )
						break;
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
			THROW( DBException( "Invalid Handle:  {:x}.", retCode) );
			break;
		case SQL_ERROR:
			HandleDiagnosticRecord( "SQLExecDirect", statement, SQL_HANDLE_STMT, retCode );
			THROW( DBException(sql, pParameters) );
		default:
			THROW( DBException(retCode, sql, pParameters) );
		}
		return resultCount;
	}
}
