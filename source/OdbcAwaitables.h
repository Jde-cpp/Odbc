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
		ConnectAwaitable( sv connectionString, bool asynchronous )noexcept(false): Session{}, Asynchronous{asynchronous}, ConnectionString{ connectionString }{}
		α await_ready()noexcept->bool;
		α await_suspend( std::coroutine_handle<> h )noexcept->void;
		α await_resume()noexcept->TaskResult;
	private:
		std::exception_ptr ExceptionPtr;
		HandleSessionAsync Session;
		bool Asynchronous;
		sv ConnectionString;
	};
	struct ExecuteAwaitable : IAwaitable
	{
		ExecuteAwaitable( HandleSessionAsync&& session, string&& sql, up<vector<up<Binding>>> pBindings, bool log )noexcept:Statement{move(session)},_sql{move(sql)}, _pBindings{move(pBindings)}, _log{log}{}
		α await_ready()noexcept->bool;
		α await_suspend( std::coroutine_handle<> h )noexcept->void;
		α await_resume()noexcept->TaskResult;
	private:
		bool IsAsynchronous()const noexcept{ return Statement.IsAsynchronous(); }
		std::exception_ptr ExceptionPtr;
		string _sql;
		up<vector<up<Binding>>> _pBindings;
		bool _log;
		HandleStatementAsync Statement;
	};

	struct FetchAwaitable : IAwaitable
	{
		FetchAwaitable( HandleStatementAsync&& statement, std::function<void(const IRow&)> f )noexcept:Statement{ move(statement) }, _function{f}{}
		α await_ready()noexcept->bool;
		α await_suspend( std::coroutine_handle<> h )noexcept->void;
		α await_resume()noexcept->TaskResult;
	private:
		bool IsAsynchronous()const noexcept{ return Statement.IsAsynchronous(); }
		std::exception_ptr ExceptionPtr;
		std::function<void(const IRow&)> _function;
		HandleStatementAsync Statement;
	};
}