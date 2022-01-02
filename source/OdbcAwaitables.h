#pragma once

#include <jde/coroutine/Task.h>
#include "../../Framework/source/coroutine/Awaitable.h"
#include "../../Framework/source/db/DataSource.h"
#include "Binding.h"
#include "Handle.h"

namespace Jde::DB::Odbc
{
	using namespace Coroutine;
	struct ConnectAwaitable final: IAwait
	{
		ConnectAwaitable( sv connectionString )noexcept(false): Session{}, ConnectionString{ connectionString }{}
		α await_ready()noexcept->bool;
		α await_suspend( std::coroutine_handle<> h )noexcept->void;
		α await_resume()noexcept->AwaitResult;
	private:
		sp<IException> ExceptionPtr;
		HandleSessionAsync Session;
		sv ConnectionString;
	};
	struct ExecuteAwaitable final: IAwait
	{
		ExecuteAwaitable( HandleSessionAsync&& session, string&& sql, up<vector<up<Binding>>> pBindings, vector<object> params, SL& sl )noexcept:IAwait{sl}, Statement{move(session)},_sql{move(sql)}, _pBindings{move(pBindings)}, _params{move(params)}{}
		α await_ready()noexcept->bool;
		α await_suspend( std::coroutine_handle<> h )noexcept->void;
		α await_resume()noexcept->AwaitResult;
	private:
		bool IsAsynchronous()const noexcept{ return Statement.IsAsynchronous(); }
		sp<IException> ExceptionPtr;
		string _sql;
		up<vector<up<Binding>>> _pBindings;
		vector<object> _params;
		bool _log{true};
		HandleStatementAsync Statement;
	};

	struct FetchAwaitable final : IAwait
	{
		FetchAwaitable( HandleStatementAsync&& statement, ISelect* p )noexcept:Statement{ move(statement) }, _function{p}{}
		α await_ready()noexcept->bool{return true;}
		α await_resume()noexcept->AwaitResult;
	private:
		bool IsAsynchronous()const noexcept{ return Statement.IsAsynchronous(); }
		sp<IException> ExceptionPtr;
		ISelect* _function;
		HandleStatementAsync Statement;
	};
}