#include "MsSqlSchemaProc.h"
#include <jde/Str.h>
#include "../../../Framework/source/db/Row.h"
#include "MsSqlStatements.h"
#include "../OdbcDataSource.h"
#define var const auto

namespace Jde::DB::MsSql
{

	α MsSqlSchemaProc::LoadTables( sv schema )noexcept(false)->flat_map<string,Table>
	{
		if( schema.empty() )
			schema = "dbo"sv;/*_pDataSource->Catalog( MsSql::Sql::CatalogSql )*/;
		flat_map<string,Table> tables;
		auto result2 = [&]( sv tableName, sv name, _int ordinalPosition, sv dflt, sv isNullable, sv type, optional<_int> maxLength, _int isIdentity, _int isId, optional<_int> numericPrecision, optional<_int> numericScale )
		{
			auto& table = tables.emplace( tableName, Table{schema,tableName} ).first->second;
			table.Columns.resize( ordinalPosition );
			var dataType = ToType(type);
			string defaultParsed;
			if( !dflt.empty() )
			{
				if( dataType==EType::Int )
				{
					var start = dflt.find_first_of( '\'' );
					var end = dflt.find_last_of( '\'' );
					if( end-start>1 )
						defaultParsed = dflt.substr( start+1, end-start );
					else if( start==string::npos && end==string::npos && dflt.size()>4 )//"((?))
						defaultParsed = dflt.substr( 2, dflt.size()-4 );
				}
			}
			table.Columns[ordinalPosition-1] = Column{ name, (uint)ordinalPosition, defaultParsed, isNullable!="false", dataType, maxLength.value_or(0), isIdentity!=0, isId!=0, numericPrecision, numericScale };
		};
		auto result = [&]( const IRow& row )
		{
			result2( row.GetString(0), row.GetString(1), row.GetInt(2), row.GetString(3), row.GetString(4), row.GetString(5), row.GetIntOpt(6), row.GetInt(7), row.GetInt(8), row.GetIntOpt(9), row.GetIntOpt(10) );
		};
		var sql = Sql::ColumnSql( false );
		_pDataSource->Select( sql, result, {schema} );
		return tables;
	}

	vector<Index> MsSqlSchemaProc::LoadIndexes( sv schema, sv tableName )noexcept(false)
	{
		if( schema.empty() )
			schema = "dbo"sv;// _pDataSource->Catalog( MsSql::Sql::CatalogSql );

		vector<Index> indexes;
		auto result = [&]( const IRow& row )
		{
			uint i=0;
			var tableName = row.GetString(i++); var indexName = row.GetString(i++); var columnName = row.GetString(i++); var unique = row.GetBit(i++)==0;

			vector<string>* pColumns;
			auto pExisting = std::find_if( indexes.begin(), indexes.end(), [&](auto index){ return index.Name==indexName && index.TableName==tableName; } );
			if( pExisting==indexes.end() )
			{
				bool clustered = false;//Boolean.Parse( row["CLUSTERED"].ToString() );
				bool primaryKey = indexName=="PRIMARY";//Boolean.Parse( row["PRIMARY_KEY"].ToString() );
				pColumns = &indexes.emplace_back( indexName, tableName, primaryKey, nullptr, unique, clustered ).Columns;
			}
			else
				pColumns = &pExisting->Columns;
			pColumns->push_back( columnName );
		};

		std::vector<object> values{schema};
		if( tableName.size() )
			values.push_back( tableName );
		var sql = Sql::IndexSql( tableName.size() );
		_pDataSource->Select( sql, result, values );

		return indexes;
	}

	flat_map<string,Procedure> MsSqlSchemaProc::LoadProcs( sv schema )noexcept(false)
	{
		if( schema.empty() )
			schema = "dbo"sv;// _pDataSource->Catalog( MsSql::Sql::CatalogSql );
		flat_map<string,Procedure> values;
		auto fnctn = [&values]( const IRow& row )
		{
			string name = row.GetString(0);
			values.try_emplace( name, Procedure{name} );
		};
		_pDataSource->Select( Sql::ProcSql(true), fnctn, {schema} );
		return values;
	}

	EType MsSqlSchemaProc::ToType( sv typeName )noexcept
	{
		EType type{ EType::None };
		if(typeName=="datetime")
			type=EType::DateTime;
		else if( typeName=="smalldatetime" )
			type=EType::SmallDateTime;
		else if(typeName=="float")
			type=EType::Float;
		else if(typeName=="real")
			type=EType::SmallFloat;
		else if( typeName=="int" )
			type = EType::Int;
		else if( Str::StartsWith(typeName, "bigint") )
			type=EType::Long;
		else if( typeName=="nvarchar" || typeName=="sysname" )
			type=EType::VarWChar;
		else if(typeName=="nchar")
			type=EType::WChar;
		else if( Str::StartsWith(typeName, "smallint") )
			type=EType::Int16;
		else if(typeName=="tinyint")
			type=EType::Int8;
		else if( typeName=="tinyint unsigned" )
			type=EType::UInt8;
		else if( typeName=="uniqueidentifier" )
			type=EType::Guid;
		else if(typeName=="varbinary")
			type=EType::VarBinary;
		else if( Str::StartsWithInsensitive(typeName, "varchar") )
			type=EType::VarChar;
		else if(typeName=="ntext")
			type=EType::NText;
		else if(typeName=="text")
			type=EType::Text;
		else if(typeName=="char")
			type=EType::Char;
		else if(typeName=="image")
			type=EType::Image;
		else if(Str::StartsWith(typeName, "bit") )
			type=EType::Bit;
		else if( Str::StartsWith(typeName, "binary") )
			type=EType::Binary;
		else if( Str::StartsWith(typeName, "decimal") )
			type=EType::Decimal;
		else if(typeName=="numeric")
			type=EType::Numeric;
		else if(typeName=="money")
			type=EType::Money;
		else
			WARN( "Unknown datatype({}).  need to implement, no big deal if not our table.", typeName );
		return type;
	}

	α MsSqlSchemaProc::LoadForeignKeys( sv schema )noexcept(false)->flat_map<string,ForeignKey>
	{
		if( schema.empty() )
			schema = "dbo"sv; //_pDataSource->Catalog( MsSql::Sql::CatalogSql );
		flat_map<string,ForeignKey> fks;
		auto result = [&]( const IRow& row )
		{
			uint i=0;
			var name = row.GetString(i++); var fkTable = row.GetString(i++); var column = row.GetString(i++); var pkTable = row.GetString(i++); //var pkColumn = row.GetString(i++); var ordinal = row.GetUInt(i);
			auto pExisting = fks.find( name );
			if( pExisting==fks.end() )
				fks.emplace( name, ForeignKey{name, fkTable, {column}, pkTable} );
			else
				pExisting->second.Columns.push_back( column );
		};
		_pDataSource->Select( Sql::ForeignKeySql(schema.size()), result, {schema} );
		return fks;
	}
}