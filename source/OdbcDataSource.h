#pragma once
#include "Exports.h"

extern "C" JDE_ODBC_VISIBILITY Jde::DB::IDataSource* GetDataSource(); 

namespace Jde::DB::Odbc
{
	struct OdbcDataSource : public IDataSource
	{

		//std::variant Fetch( string_view sql, std::variant parameters )noexcept(false) override;
		//template<typename ...TColumns, typename ...TParameters>
		//void Select( string_view sql, std::function<void(TColumns...)> f, TParameters... );
		uint Execute( string_view sql )noexcept(false) override;

		uint Execute(string_view sql, const std::vector<DataValue>& parameters, bool log)noexcept(false) override;
		uint Execute(string_view sql, const std::vector<DataValue>& parameters, std::function<void(const IRow&)> f, bool log)noexcept(false) override;
		uint ExecuteProc(string_view sql, const std::vector<DataValue>& parameters, bool log=true)noexcept(false) override;
		uint ExecuteProc(string_view sql, const std::vector<DataValue>& parameters, std::function<void(const IRow&)> f, bool log)noexcept(false) override;
		
		void Select( string_view sql, std::function<void(const IRow&)> f, const std::vector<DataValue>& values, bool log )noexcept(false) override;
		void Select( string_view sql, std::function<void(const IRow&)> f )noexcept(false) override;
		uint Query( string_view sql, bool log, std::function<void(const IRow&)>* pResultFunction=nullptr, const std::vector<DataValue>* pValues=nullptr )noexcept(false);

	private:
		/*SQLHDBC*/ std::shared_ptr<void> GetSession()noexcept(false);
		std::shared_ptr<void> GetEnvironment()noexcept(false);
		//SQLHDBC _hDbc;
		//uint Execute2(string_view sql, const std::vector<DataValue>* pParameters = nullptr, std::function<void(const IRow&)>* pFunction = nullptr, bool isStoredProcedure = false);
	};
}