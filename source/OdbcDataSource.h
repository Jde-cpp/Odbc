﻿#pragma once
#include "Exports.h"
#include "../../Framework/source/coroutine/Awaitable.h"
#include "../../Framework/source/db/DataSource.h"
#include "OdbcAwaitables.h"
#include "Binding.h"

extern "C" ΓODBC Jde::DB::IDataSource* GetDataSource();

#define var const auto
namespace Jde::DB::Odbc
{
	using namespace Coroutine;
	struct OdbcDataSource : IDataSource{
		α SchemaProc()ι->sp<ISchemaProc> override;
		α Execute( string sql, SRCE )ε->uint override;
		α Execute( string sql, vec<object>& params, SRCE)ε->uint override;
		α ExecuteCo( string sql, vector<object> params, SRCE )ι->up<IAwait> override;
		α ExecuteCo( string sql, vector<object> p, bool proc, RowΛ f, SRCE )ε->up<IAwait>;
		α Execute( string sql, const vector<object>* params, const RowΛ* f, bool isStoredProc=false, SRCE )ε->uint override;
		α ExecuteProc( string sql, const vector<object>& params, SRCE )ε->uint override;
		α ExecuteProc( string sql, const vector<object>& params, RowΛ f, SRCE )ε->uint override;

		α Select( string sql, RowΛ f, const vector<object>* pValues, SRCE )ε->uint override;

		α SelectCo( ISelect* pAwait, string sql, vector<object>&& params, SRCE )ι->up<IAwait> override;
		α SetConnectionString( string x )ι->void override;

		α ExecuteNoLog( string sql, const vector<object>* params, RowΛ* f=nullptr, bool isStoredProc=false, SRCE )ε->uint override;
		α ExecuteProcNoLog( string sql, vec<object> params, SRCE )ε->uint override;
		α ExecuteProcCo( string sql, vector<object> params, RowΛ f, SRCE )ε->up<IAwait> override;
		α ExecuteProcCo( string sql, vector<object> params, SRCE )ι->up<IAwait> override;
		α SelectNoLog( string sql, RowΛ f, const vector<object>* pValues, SRCE )ε->uint override;

	private:
		α Connect()ε{ return ConnectAwaitable{_connectionString/*, Asynchronous*/}; }
		Ω Execute( HandleSessionAsync&& session, string&& sql, up<vector<up<Binding>>> pBindings, vector<object> params, SL& sl )ι{ return ExecuteAwaitable( move(session), move(sql), move(pBindings), move(params), sl ); }
		Ω Fetch( HandleStatementAsync&& h, ISelect* p )ι{ return FetchAwaitable( move(h), p ); }

		/*SQLHDBC*/ α GetSession()ε->sp<void>;
		α GetEnvironment()ε->sp<void>;
	};

/*	ⓣ OdbcDataSource::ScalerCo( sv sql, const vector<object>&& parameters )ε->FunctionAwaitable//sp<T>
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