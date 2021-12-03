#pragma once
#include <jde/Log.h>
#include <boost/noncopyable.hpp>
#include "Bindings.h"
#include "../../Framework/source/Settings.h"

namespace Jde::DB{ struct IRow; }
namespace Jde::DB::Odbc
{
	struct FetchAwaitable; struct OdbcDataSource;

	struct HandleEnvironment final: boost::noncopyable
	{
		HandleEnvironment();
		HandleEnvironment( HandleEnvironment&& rhs )noexcept{};
//		~HandleEnvironment();

		operator SQLHENV()const noexcept{ return _handle.get(); }
	private:
		static sp<void> _handle;
	};

	struct HandleSession : boost::noncopyable
	{
		HandleSession()noexcept(false);
		HandleSession( sv connectionString )noexcept(false);
		HandleSession( HandleSession&& rhs )noexcept:_handle{move(rhs._handle)}, _env{move(rhs._env)}{ rhs._handle=nullptr; };
		~HandleSession();
		virtual auto Connect( sv connectionString )noexcept(false)->void;
		operator HDBC()const noexcept{ return _handle; }
	protected:
		HDBC _handle{nullptr};
		HandleEnvironment _env;
	};
	
	struct HandleSessionAsync : HandleSession
	{
		HandleSessionAsync( /*sv connectionString, bool asynchronous*/ )noexcept(false):HandleSession{}{}
		HandleSessionAsync( HandleSessionAsync&& rhs )noexcept:HandleSession{move(rhs)}, _event{ rhs._event }{ rhs._event=nullptr; };
		~HandleSessionAsync(){ if(_event) ::CloseHandle(_event); }
		void Connect( sv connectionString )noexcept(false) override;
		auto IsAsynchronous()const noexcept{ return _event!=nullptr; } 
		HANDLE Event()noexcept{ if( !_event ) _event = ::CreateEvent( nullptr, false, false, nullptr );
			return _event; }
		HANDLE MoveEvent()noexcept{ auto y=_event; _event=nullptr; return y; }
	protected:
		HANDLE _event{ nullptr };
	};	
	struct HandleStatement : boost::noncopyable
	{
		HandleStatement( string connectionString )noexcept(false);
		HandleStatement( HandleStatement&& rhs )noexcept:_handle{move(rhs._handle)}, _session{move(rhs._session)}{ rhs._handle=nullptr; };
		~HandleStatement();
		operator SQLHSTMT()const noexcept{ return _handle; }
	private:
		SQLHSTMT _handle{nullptr};
		HandleSession _session;
	};
	struct HandleStatementAsync : boost::noncopyable
	{
		HandleStatementAsync( HandleSessionAsync&& session )noexcept(false):_rowStatus{ new SQLUSMALLINT[ChunkSize] }, _event{ session.IsAsynchronous() ? ::CreateEvent(nullptr, false, false, nullptr) : nullptr}, _session{ move(session) }{};
		HandleStatementAsync( HandleStatementAsync&& rhs )noexcept:_bindings{move(rhs._bindings)}, _rowStatus{move(rhs._rowStatus)}, _result{rhs._result}, _moreRows{rhs._moreRows}, _event{move(rhs._event)}, _handle{move(rhs._handle)}, _session{move(rhs._session)}{ rhs._handle=nullptr; }
		~HandleStatementAsync();
		
		α SetHandle( SQLHSTMT h )noexcept{ _handle=h; }
		α Event()noexcept{ return _event; }
		operator SQLHSTMT()const noexcept{ return _handle; }
		α& Session()noexcept{ return _session; }
		α OBindings()noexcept(false)->const vector<up<IBindings>>&;
		α RowStatuses()noexcept->SQLUSMALLINT*{ return _rowStatus.get();}
		α RowStatusesSize()const noexcept->uint{ return ChunkSize; }
		bool IsAsynchronous()const noexcept{ return _session.IsAsynchronous(); }
	private:
		vector<up<IBindings>> _bindings;
		up<SQLUSMALLINT[]> _rowStatus;
		uint _result{0};
		bool _moreRows{true};
		HANDLE _event{ nullptr };
		SQLHSTMT _handle{nullptr};
		HandleSessionAsync _session;
		static uint ChunkSize;
		friend FetchAwaitable; friend OdbcDataSource;
	};
}