#include "MsSqlSchemaProc.h"
#include <jde/Str.h>
#include "../../../Framework/source/db/Row.h"
#include "MsSqlStatements.h"
#include "../OdbcDataSource.h"
#define var const auto

namespace Jde::DB::MsSql
{

	MapPtr<string,Table> MsSqlSchemaProc::LoadTables( sv schema )noexcept(false)
	{
		if( schema.empty() )
			schema = "dbo"sv;/*_pDataSource->Catalog( MsSql::Sql::CatalogSql )*/;
		auto pTables = make_shared<map<string,Table>>();
		//std::function<void(str name, str COLUMN_NAME, int ordinalPosition, str dflt, int isNullable, str type, int maxLength, int isIdentity, int isId, int NumericPrecision, int NumericScale)>
		auto result2 = [&]( sv tableName, sv name, _int ordinalPosition, sv dflt, sv isNullable, sv type, optional<_int> maxLength, _int isIdentity, _int isId, optional<_int> numericPrecision, optional<_int> numericScale )
		{
			auto& table = pTables->emplace( tableName, Table{schema,tableName} ).first->second;
			table.Columns.resize( ordinalPosition );
			var dataType = ToDataType(type);
			string defaultParsed;
			if( !dflt.empty() )
			{
				if( dataType==DataType::Int )
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
		return pTables;
	}

	vector<Index> MsSqlSchemaProc::LoadIndexes( sv schema, sv tableName )noexcept(false)
	{
		if( schema.empty() )
			schema = "dbo"sv;// _pDataSource->Catalog( MsSql::Sql::CatalogSql );

		vector<Index> indexes;
		//std::function<void(str indexName, str tableName, bool unique, str columnName, bool primaryKey)>
		auto result = [&]( const IRow& row )
		{
			uint i=0;
			var tableName = row.GetString(i++); var indexName = row.GetString(i++); var columnName = row.GetString(i++); var unique = row.GetBit(i++)==0;

			//var ordinal = row.GetUInt(i++); var dflt = row.GetString(i++);  //var primaryKey = row.GetBit(i);
			vector<CIString>* pColumns;
			auto pExisting = std::find_if( indexes.begin(), indexes.end(), [&](auto index){ return index.Name==indexName && index.TableName==tableName; } );
			if( pExisting==indexes.end() )
			{
				bool clustered = false;//Boolean.Parse( row["CLUSTERED"].ToString() );
				bool primaryKey = indexName=="PRIMARY";//Boolean.Parse( row["PRIMARY_KEY"].ToString() );
				//var pTable = tables.find( tableName );
				//if( pTable==tables.end() )
				//	THROW2( LogicException("Could not find table '{}' for index '{}'.", tableName, indexName) );
				pColumns = &indexes.emplace_back( indexName, tableName, primaryKey, nullptr, unique, clustered ).Columns;
			}
			else
				pColumns = &pExisting->Columns;
			pColumns->push_back( columnName );
		};

		std::vector<DataValue> values{schema};
		if( tableName.size() )
			values.push_back( tableName );
		var sql = Sql::IndexSql( tableName.size() );
		_pDataSource->Select( sql, result, values, true );

		return indexes;
	}

	flat_map<string,Procedure> MsSqlSchemaProc::LoadProcs( sv schema )noexcept(false)
	{
		if( schema.empty() )
			schema = "dbo"sv;// _pDataSource->Catalog( MsSql::Sql::CatalogSql );
		//std::vector<DataValue> params;
		//if( schema.size() )
		//	params.emplace_back( schema );
		flat_map<string,Procedure> values;
		auto fnctn = [&values]( const IRow& row )
		{
			string name = row.GetString(0);
			values.try_emplace( name, Procedure{name} );
		};
		_pDataSource->Select( Sql::ProcSql(true), fnctn, {schema}, true );
		return values;
	}

	DataType MsSqlSchemaProc::ToDataType( sv typeName )noexcept
	{
		DataType type{ DataType::None };
		if(typeName=="datetime")
			type=DataType::DateTime;
		else if( typeName=="smalldatetime" )
			type=DataType::SmallDateTime;
		else if(typeName=="float")
			type=DataType::Float;
		else if(typeName=="real")
			type=DataType::SmallFloat;
		else if( typeName=="int" )
			type = DataType::Int;
		else if( Str::StartsWith(typeName, "bigint") )
			type=DataType::Long;
		else if( typeName=="nvarchar" )
			type=DataType::VarWChar;
		else if(typeName=="nchar")
			type=DataType::WChar;
		else if( Str::StartsWith(typeName, "smallint") )
			type=DataType::Int16;
		else if(typeName=="tinyint")
			type=DataType::Int8;
		else if( typeName=="tinyint unsigned" )
			type=DataType::UInt8;
		else if( typeName=="uniqueidentifier" )
			type=DataType::Guid;
		else if(typeName=="varbinary")
			type=DataType::VarBinary;
		else if( Str::StartsWithInsensitive(typeName, "varchar") )
			type=DataType::VarChar;
		else if(typeName=="ntext")
			type=DataType::NText;
		else if(typeName=="text")
			type=DataType::Text;
		else if(typeName=="char")
			type=DataType::Char;
		else if(typeName=="image")
			type=DataType::Image;
		else if(Str::StartsWith(typeName, "bit") )
			type=DataType::Bit;
		else if( Str::StartsWith(typeName, "binary") )
			type=DataType::Binary;
		else if( Str::StartsWith(typeName, "decimal") )
			type=DataType::Decimal;
		else if(typeName=="numeric")
			type=DataType::Numeric;
		else if(typeName=="money")
			type=DataType::Money;
		else
			GetDefaultLogger()->warn( "Unknown datatype({}).  need to implement, no big deal if not our table.", typeName );
		return type;
	}

	flat_map<string,ForeignKey> MsSqlSchemaProc::LoadForeignKeys( sv schema )noexcept(false)
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
		_pDataSource->Select( Sql::ForeignKeySql(schema.size()), result, {schema}, true );
		return fks;
	}
/*	Schema LoadSchema( IDataSource& ds, sv schema )noexcept(false) override;
	{

	}

	Table MySqlSchemaProc::ToTable( const mysqlx::Table& mysqlTable )noexcept
	{

		//return Table( mysqlTable.);
	}
*/
}