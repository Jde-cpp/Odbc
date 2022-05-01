#pragma once
#include "../../../Framework/source/db/types/Schema.h"
#include "../../../Framework/source/db/types/Table.h"
#include "../../../Framework/source/db/SchemaProc.h"

namespace Jde::DB
{
	struct IDataSource;
namespace MsSql
{
	struct MsSqlSchemaProc final : ISchemaProc
	{
		MsSqlSchemaProc( sp<IDataSource> pDataSource )ι:
			ISchemaProc{ pDataSource }
		{}
		α LoadTables( sv catalog )ε->flat_map<string,Table> override;
		α ToType( sv typeName )ι->EType override;
		α LoadIndexes( sv schema, sv tableName )ε->vector<Index> override;
		α LoadForeignKeys( sv catalog )ε->flat_map<string,ForeignKey> override;
		α LoadProcs( sv catalog={} )ε->flat_map<string,Procedure> override;
	};
}}