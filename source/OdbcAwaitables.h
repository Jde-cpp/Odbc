#pragma once

#include <jde/coroutine/Task.h>
#include "../../Framework/source/coroutine/Awaitable.h"
#include "../../Framework/source/db/DataSource.h"
#include "Handle.h"

namespace Jde::DB::Odbc
{
	using namespace Coroutine;
	struct ConnectAwaitable : IAwaitable
	{
		ConnectAwaitable( sv connectionString )noexcept(false): Session{}, ConnectionString{ connectionString }{}
		α await_ready()noexcept->bool;
		α await_suspend( std::coroutine_handle<> h )noexcept->void;
		α await_resume()noexcept->TaskResult;
	private:
		sp<IException> ExceptionPtr;
		HandleSessionAsync Session;
		sv ConnectionString;
	};
	struct ExecuteAwaitable : IAwaitable
	{
		ExecuteAwaitable( HandleSessionAsync&& session, string&& sql, up<vector<up<Binding>>> pBindings )noexcept:Statement{move(session)},_sql{move(sql)}, _pBindings{move(pBindings)}{}
		α await_ready()noexcept->bool;
		α await_suspend( std::coroutine_handle<> h )noexcept->void;
		α await_resume()noexcept->TaskResult;
	private:
		bool IsAsynchronous()const noexcept{ return Statement.IsAsynchronous(); }
		sp<IException> ExceptionPtr;
		string _sql;
		up<vector<up<Binding>>> _pBindings;
		bool _log{true};
		HandleStatementAsync Statement;
	};

	struct FetchAwaitable : IAwaitable
	{
		FetchAwaitable( HandleStatementAsync&& statement, ISelect* p )noexcept:Statement{ move(statement) }, _function{p}{}
		α await_ready()noexcept->bool;
		α await_suspend( std::coroutine_handle<> h )noexcept->void;
		α await_resume()noexcept->TaskResult;
	private:
		bool IsAsynchronous()const noexcept{ return Statement.IsAsynchronous(); }
		sp<IException> ExceptionPtr;
		ISelect* _function;
		HandleStatementAsync Statement;
	};
}