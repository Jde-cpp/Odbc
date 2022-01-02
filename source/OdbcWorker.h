#pragma once
#include "../../Framework/source/threading/Mutex.h"

#define var const auto
namespace Jde::DB::Odbc
{
	struct IWorker /*abstract*/
	{
		virtual bool Poll()noexcept=0;
		ⓣ static ThreadCount()noexcept->uint8;
	};

	struct OdbcWorker final: IWorker
	{
		bool Poll()noexcept override;
		Ω Push( std::coroutine_handle<>&& h, HANDLE hEvent, bool closeEvent=false )noexcept->void;
		static uint8 _threadCount;
		static constexpr sv Name="Odbc"sv;
	protected:

	private:
	};

	struct WorkerManager
	{
		ⓣ static Start( sv workerName )noexcept->sp<T>;
	private:
		static vector<up<IWorker>> _workers;  static std::atomic_flag _objectLock;
		static flat_set<sv> _workerNames; static std::atomic_flag _nameLock;
	};

	ⓣ static IWorker::ThreadCount()noexcept->uint8
	{
		if( T::_threadCount==std::numeric_limits<uint8>::max() )
		{
			T::_threadCount = Settings::Get<uint8>( format("workders/{}/threads", T::Name) ).value_or( 0 );
		}
		return T::_threadCount;
	}

	ⓣ WorkerManager::Start( sv workerName )noexcept->sp<T>
	{
		sp<T> p;
		AtomicGuard l{ _nameLock };
		if( _workerNames.emplace( workerName ).second )
		{
			l.unlock();
			AtomicGuard l2{ _objectLock };
			//TODO:  load settings, if thread count==0 then work with it here, else create worker.
			var pSettings = Settings::TryGetSubcontainer<Settings::Container>( "workers", workerName );
			var threads = pSettings ? pSettings->Get<uint8>( "threads" ).value_or(0) : 0;
		}
		return p;
	}
}
#undef var