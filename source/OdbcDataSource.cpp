#include "OdbcDataSource.h"
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
#pragma region AllocateBindings
	sp<vector<sp<Binding>>> AllocateBindings( const HandleStatement& statement,  SQLSMALLINT columnCount )noexcept
	{ 
		auto pBindings = make_shared<vector<sp<Binding>>>(); pBindings->reserve( columnCount );
		for( SQLSMALLINT iCol = 1; iCol <= columnCount; iCol++ )
		{
			SQLLEN ssType;
			CALL( statement, SQL_HANDLE_STMT, SQLColAttribute(statement, iCol, SQL_DESC_CONCISE_TYPE, NULL, 0, NULL, &ssType), "SQLColAttribute::Concise" );

			SQLLEN bufferSize = 0;
			sp<Binding> pBinding;
			if( ssType == SQL_CHAR || ssType == SQL_VARCHAR || ssType == SQL_LONGVARCHAR )
			{
				CALL( statement, SQL_HANDLE_STMT, SQLColAttribute(statement, iCol, SQL_DESC_DISPLAY_SIZE, NULL, 0, NULL, &bufferSize), "SQLColAttribute::Display" );
				pBinding = make_shared<BindingString>( (SQLSMALLINT)ssType, ++bufferSize );
			}
			else
				pBinding = Binding::GetBinding( (SQLSMALLINT)ssType );
		
			CALL( statement, SQL_HANDLE_STMT, SQLBindCol( statement, iCol, (SQLSMALLINT)pBinding->CodeType, pBinding->Data(), bufferSize, &pBinding->Output), "SQLBindCol" );
			pBindings->push_back( pBinding );
		}
		return pBindings;
	}
#pragma endregion
	uint OdbcDataSource::Execute( string_view sql )noexcept(false){ return Query( sql, true ); }
	uint OdbcDataSource::Execute(string_view sql, const std::vector<DataValue>& parameters, bool log)noexcept(false){ return Query(sql, log, nullptr, &parameters); }
	uint OdbcDataSource::Execute(string_view sql, const std::vector<DataValue>& parameters, std::function<void(const IRow&)> f, bool log)noexcept(false){ return Query(sql, log, &f, &parameters); }
	uint OdbcDataSource::ExecuteProc(string_view sql, const std::vector<DataValue>& parameters, bool log )noexcept(false){ return Query(sql, log, nullptr, &parameters); }
	uint OdbcDataSource::ExecuteProc(string_view sql, const std::vector<DataValue>& parameters, std::function<void(const IRow&)> f, bool log)noexcept(false){ return Query(sql, log, &f, &parameters); }
	void OdbcDataSource::Select(string_view sql, std::function<void(const IRow&)> f, const std::vector<DataValue>& values, bool log)noexcept(false){ Query( sql, log, &f, &values ); }
	void OdbcDataSource::Select(string_view sql, std::function<void(const IRow&)> f )noexcept(false){ Query( sql, false, &f ); }

	uint OdbcDataSource::Query( string_view sql, bool log, std::function<void(const IRow&)>* pResultFunction, const std::vector<DataValue>* pValues )noexcept(false)
	{
		HandleStatement statement{ ConnectionString };
		std::vector<sp<Binding>> parameters;
		if( pValues )
		{
			parameters.reserve( pValues->size() );
			SQLUSMALLINT iParameter = 0;
			for( var& param : *pValues )
			{
				auto pBinding = Binding::Create( param );
				parameters.push_back( pBinding );
				SQLBindParameter( statement, ++iParameter, SQL_PARAM_INPUT, pBinding->CodeType, pBinding->DBType, pBinding->Size(), 0/*decimalDigits*/,  pBinding->Data(), pBinding->BufferLength(), &pBinding->Output );
			}
		}
		uint resultCount = 0;
		var retCode = SQLExecDirect( statement, (SQLCHAR*)sql.data(), static_cast<SQLINTEGER>(sql.size()) );
		switch( retCode )
		{
		case SQL_SUCCESS_WITH_INFO:
			HandleDiagnosticRecord( "SQLExecDirect", statement, SQL_HANDLE_STMT, retCode );
		case SQL_SUCCESS:
		{
			SQLSMALLINT columnCount;
			CALL( statement, SQL_HANDLE_STMT, SQLNumResultCols(statement,&columnCount), "SQLNumResultCols" );
			if( columnCount > 0 )
			{
				var pBindings = AllocateBindings( statement, columnCount );
				OdbcRow row{*pBindings};
				for(;;)
				{
					var retCode2 = ::SQLFetch( statement );
					if( retCode2==SQL_NO_DATA_FOUND )
						break;
					if( !pResultFunction )
					{
						WARN( "{} is returning results, but no function to handle.", sql );
						break;
					}
					row.Reset();
					(*pResultFunction)( row );
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
			THROW( DBException( "SQL_ERROR {}.", sql) );
		default:
			THROW( DBException( "Unexpected return code:  {:x}.", retCode) );
		}
		return resultCount;
	}
}