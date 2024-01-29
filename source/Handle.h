#pragma once
#include <jde/Log.h>
#include <boost/noncopyable.hpp>
#include "Bindings.h"
#include "../../Framework/source/Settings.h"

namespace Jde::DB{ struct IRow; }
namespace Jde::DB::Odbc
{
	struct FetchAwaitable; struct OdbcDataSource; struct ExecuteAwaitable;

	struct HandleEnvironment final: boost::noncopyable
	{
		HandleEnvironment();
		HandleEnvironment( HandleEnvironment&& rhs )ι{};

		operator SQLHENV()Ι{ return _handle.get(); }
	private:
		static sp<void> _handle;
	};

	struct HandleSession : boost::noncopyable
	{
		HandleSession()ε;
		HandleSession( sv connectionString )ε;
		HandleSession( HandleSession&& rhs )ι:_handle{move(rhs._handle)}, _env{move(rhs._env)}{ rhs._handle=nullptr; };
		~HandleSession();
		virtual auto Connect( sv connectionString )ε->void;
		operator HDBC()Ι{ return _handle; }
	protected:
		HDBC _handle{nullptr};
		HandleEnvironment _env;
	};

	struct HandleSessionAsync : HandleSession
	{
		HandleSessionAsync()ε:HandleSession{}{}
		HandleSessionAsync( HandleSessionAsync&& rhs )ι:HandleSession{move(rhs)}, _event{ rhs._event }{ rhs._event=nullptr; };
		~HandleSessionAsync(){ if(_event) ::CloseHandle(_event); }
		void Connect( sv connectionString )ε override;
		auto IsAsynchronous()Ι{ return _event!=nullptr; }
		HANDLE Event()ι{ if( !_event ) _event = ::CreateEvent( nullptr, false, false, nullptr );
			return _event; }
		HANDLE MoveEvent()ι{ auto y=_event; _event=nullptr; return y; }
	protected:
		HANDLE _event{ nullptr };
	};
	struct HandleStatement : boost::noncopyable
	{
		HandleStatement( string connectionString )ε;
		HandleStatement( HandleStatement&& rhs )ι:_handle{move(rhs._handle)}, _session{move(rhs._session)}{ rhs._handle=nullptr; };
		~HandleStatement();
		operator SQLHSTMT()Ι{ return _handle; }
	private:
		SQLHSTMT _handle{nullptr};
		HandleSession _session;
	};
	struct HandleStatementAsync : boost::noncopyable
	{
		HandleStatementAsync( HandleSessionAsync&& session )ε:_rowStatus{ new SQLUSMALLINT[ChunkSize] }, _event{ session.IsAsynchronous() ? ::CreateEvent(nullptr, false, false, nullptr) : nullptr}, _session{ move(session) }{};
		HandleStatementAsync( HandleStatementAsync&& rhs )ι:_bindings{move(rhs._bindings)}, _rowStatus{move(rhs._rowStatus)}, _result{rhs._result}, _moreRows{rhs._moreRows}, _event{move(rhs._event)}, _handle{move(rhs._handle)}, _session{move(rhs._session)}{ rhs._handle=nullptr; }
		~HandleStatementAsync();

		α SetHandle( SQLHSTMT h )ι{ _handle=h; }
		α Event()ι{ return _event; }
		operator SQLHSTMT()Ι{ return _handle; }
		α& Session()ι{ return _session; }
		α OBindings()ε->const vector<up<IBindings>>&;
		α RowStatuses()ι->SQLUSMALLINT*{ return _rowStatus.get();}
		α RowStatusesSize()Ι->uint{ return ChunkSize; }
		bool IsAsynchronous()Ι{ return _session.IsAsynchronous(); }
	private:
		vector<up<IBindings>> _bindings;
		up<SQLUSMALLINT[]> _rowStatus;
		uint _result{0};
		bool _moreRows{true};
		HANDLE _event{ nullptr };
		SQLHSTMT _handle{nullptr};
		HandleSessionAsync _session;
		static uint ChunkSize;
		friend FetchAwaitable; friend OdbcDataSource; friend ExecuteAwaitable;
	};
}