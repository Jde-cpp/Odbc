#pragma once
#include "Exports.h"
#include "../../Framework/source/coroutine/Awaitable.h"
#include "../../Framework/source/db/DataSource.h"
#include "OdbcAwaitables.h"
#include "Binding.h"

extern "C" JDE_ODBC_VISIBILITY Jde::DB::IDataSource* GetDataSource();

#define var const auto
namespace Jde::DB::Odbc
{
	using namespace Coroutine;
	struct OdbcDataSource : IDataSource
	{
		α SchemaProc()noexcept->sp<ISchemaProc> override;
		α Execute( string sql, SRCE )noexcept(false)->uint override;
		α Execute( string sql, vec<object>& parameters, SRCE)noexcept(false)->uint override;
		α Execute( string sql, const std::vector<object>* pParameters, RowΛ* f, bool isStoredProc=false, SRCE )noexcept(false)->uint  override;
		α ExecuteProc( string sql, const std::vector<object>& parameters, SRCE )noexcept(false)->uint override;
		α ExecuteProc( string sql, const std::vector<object>& parameters, RowΛ f, SRCE )noexcept(false)->uint override;

		α Select( string sql, RowΛ f, const vector<object>* pValues, SRCE )noexcept(false)->uint override;

		α SelectCo( ISelect* pAwait, string sql, vector<object>&& params, SRCE )noexcept->up<IAwait> override;
		α SetConnectionString( string x )noexcept->void override;

		α ExecuteNoLog( string sql, const vector<object>* pParameters, RowΛ* f=nullptr, bool isStoredProc=false, SRCE )noexcept(false)->uint override;
		α ExecuteProcNoLog( string sql, vec<object> parameters, SRCE )noexcept(false)->uint override;
		α ExecuteProcCo( string sql, vector<object> parameters, SRCE )noexcept->up<IAwait> override;
		α SelectNoLog( string sql, RowΛ f, const vector<object>* pValues, SRCE )noexcept(false)->uint override;

	private:
		α Connect()noexcept(false){ return ConnectAwaitable{_connectionString/*, Asynchronous*/}; }
		Ω Execute( HandleSessionAsync&& session, string&& sql, up<vector<up<Binding>>> pBindings, vector<object> params, SL& sl )noexcept{ return ExecuteAwaitable( move(session), move(sql), move(pBindings), move(params), sl ); }
		Ω Fetch( HandleStatementAsync&& h, ISelect* p )noexcept{ return FetchAwaitable( move(h), p ); }

		/*SQLHDBC*/ α GetSession()noexcept(false)->sp<void>;
		α GetEnvironment()noexcept(false)->sp<void>;
	};

/*	ⓣ OdbcDataSource::ScalerCo( sv sql, const vector<object>&& parameters )noexcept(false)->FunctionAwaitable//sp<T>
	{
		return FunctionAwaitable{ [sql,params=move(parameters),this]( coroutine_handle<Task2::promise_type> h )mutable->Task2
		{
			T result;
			RowΛ f = [&result](const IRow& row){ result = row.Get<T>(0); DBG("result={}"sv, result); };
			auto result2 = co_await SelectCo( sql, &f, &params, true );
			if( result2.HasError() )
				h.promise().get_return_object().SetResult( move(result2) );
			else
				h.promise().get_return_object().SetResult( make_shared<T>(result) );
			h.resume();
		}};
	}
	*/
#undef var
}