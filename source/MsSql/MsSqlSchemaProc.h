#pragma once
#include "../../../Framework/source/db/types/Schema.h"
#include "../../../Framework/source/db/types/Table.h"
#include "../../../Framework/source/db/SchemaProc.h"

namespace Jde::DB
{
	struct IDataSource;
namespace MsSql
{
	struct MsSqlSchemaProc final : public ISchemaProc
	{
		MsSqlSchemaProc( sp<IDataSource> pDataSource ):
			ISchemaProc{ pDataSource }
		{}
		α LoadTables( sv catalog )noexcept(false)->up<flat_map<string,Table>> override;
		//object ToObject( sv name )noexcept override;
		α ToType( sv typeName )noexcept->EType override;
		vector<Index> LoadIndexes( sv schema, sv tableName )noexcept(false) override;
		flat_map<string,ForeignKey> LoadForeignKeys( sv catalog )noexcept(false) override;
		flat_map<string,Procedure> LoadProcs( sv catalog={} )noexcept(false) override;
	};
}}