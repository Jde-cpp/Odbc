#pragma once
#include "Exports.h"
#include "../../Framework/source/db/DataSource.h"

extern "C" JDE_ODBC_VISIBILITY Jde::DB::IDataSource* GetDataSource(); 

namespace Jde::DB::Odbc
{
	struct OdbcDataSource : public IDataSource
	{
		sp<ISchemaProc> SchemaProc()noexcept;
		uint Execute( string_view sql )noexcept(false) override;

		uint Execute( sv sql, const std::vector<DataValue>& parameters, bool log)noexcept(false) override;
		uint Execute( sv sql, const std::vector<DataValue>* pParameters, std::function<void(const IRow&)>* f, bool isStoredProc=false, bool log=true )noexcept(false);
		uint ExecuteProc(string_view sql, const std::vector<DataValue>& parameters, bool log=true)noexcept(false) override;
		uint ExecuteProc(string_view sql, const std::vector<DataValue>& parameters, std::function<void(const IRow&)> f, bool log)noexcept(false) override;
		
		uint Select( sv sql, std::function<void(const IRow&)> f, const vector<DataValue>* pValues, bool log )noexcept(false)override{ return Select(sql, &f, pValues, log); }
		uint Select( string_view sql, std::function<void(const IRow&)>* f, const std::vector<DataValue>* values, bool log )noexcept(false);
		//uint  Query( string_view sql, bool log, std::function<void(const IRow&)>* pResultFunction=nullptr, const std::vector<DataValue>* pValues=nullptr )noexcept(false);

	private:
		/*SQLHDBC*/ std::shared_ptr<void> GetSession()noexcept(false);
		std::shared_ptr<void> GetEnvironment()noexcept(false);
	};
}