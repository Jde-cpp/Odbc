#pragma once
#include "Exports.h"
#include "../../Framework/source/coroutine/Awaitable.h"
#include "../../Framework/source/db/DataSource.h"
#include "OdbcAwaitables.h"

extern "C" JDE_ODBC_VISIBILITY Jde::DB::IDataSource* GetDataSource(); 

#define var const auto
namespace Jde::DB::Odbc
{
	using namespace Coroutine;
	struct OdbcDataSource : IDataSource
	{
		sp<ISchemaProc> SchemaProc()noexcept;
		uint Execute( sv sql )noexcept(false) override;

		uint Execute( sv sql, const std::vector<DataValue>& parameters, bool log)noexcept(false) override;
		uint Execute( sv sql, const std::vector<DataValue>* pParameters, std::function<void(const IRow&)>* f, bool isStoredProc=false, bool log=true )noexcept(false);
		uint ExecuteProc(sv sql, const std::vector<DataValue>& parameters, bool log=true)noexcept(false) override;
		uint ExecuteProc(sv sql, const std::vector<DataValue>& parameters, std::function<void(const IRow&)> f, bool log)noexcept(false) override;
		
		uint Select( sv sql, std::function<void(const IRow&)> f, const vector<DataValue>* pValues, bool log )noexcept(false)override{ return Select(sql, &f, pValues, log); }
		uint Select( sv sql, std::function<void(const IRow&)>* f, const std::vector<DataValue>* values, bool log )noexcept(false);
		ⓣ ScalerCo( sv sql, const vector<DataValue>&& parameters={} )noexcept(false)->FunctionAwaitable;//sp<T>
		//ⓣ ScalerNullCo( string&& sql, const vector<DataValue>&& parameters )noexcept(false)->Task2;//sp<optional<T>>
		
		α SelectCo( string&& sql, std::function<void(const IRow&)> f, const std::vector<DataValue>&& parameters, bool log )noexcept->up<IAwaitable> override;
		α SetAsynchronous()noexcept(false)->void override;
		α SetConnectionString( sv x )noexcept->void override;
	private:
		α Connect()noexcept(false){ return ConnectAwaitable{_connectionString, Asynchronous}; }
		Ω Execute( HandleSessionAsync&& session, string&& sql, up<vector<up<Binding>>> pBindings, bool log )noexcept{ return ExecuteAwaitable( move(session), move(sql), move(pBindings), log ); }
		Ω Fetch( HandleStatementAsync&& h, std::function<void(const IRow&)> f )noexcept{ return FetchAwaitable( move(h), f ); }

		/*SQLHDBC*/ std::shared_ptr<void> GetSession()noexcept(false);
		std::shared_ptr<void> GetEnvironment()noexcept(false);
	};

	ⓣ OdbcDataSource::ScalerCo( sv sql, const vector<DataValue>&& parameters )noexcept(false)->FunctionAwaitable//sp<T>
	{
		return FunctionAwaitable{ [sql,params=move(parameters),this]( coroutine_handle<Task2::promise_type> h )mutable->Task2
		{
			T result;
			std::function<void(const IRow&)> f = [&result](const IRow& row){ result = row.Get<T>(0); DBG("result={}"sv, result); };
			auto result2 = co_await SelectCo( sql, &f, &params, true );
			//h.promise().get_return_object().SetResult( move(result2) );
			if( result2.HasError() )
				h.promise().get_return_object().SetResult( move(result2) );
			else
				h.promise().get_return_object().SetResult( make_shared<T>(result) );
			h.resume();
		}};
	}
#undef var
}