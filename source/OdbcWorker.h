﻿#pragma once
#include "../../Framework/source/threading/Mutex.h"

#define var const auto
namespace Jde::DB::Odbc
{
	struct IWorker /*abstract*/
	{
		//IWorker(){}
		virtual bool Poll()noexcept=0;
		//virtual sv Name()noexcept=0;
		ⓣ static ThreadCount()noexcept->uint8;
	protected:
		//uint8 _threadCount{ std::numeric_limits<uint8>::max() };
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
		//ⓣ static Push( std::coroutine_handle<>&& hCoroutine, HANDLE&& hEvent )noexcept->void;
	private:
		static vector<up<IWorker>> _workers;  static atomic<bool> _objectLock;
		static flat_set<sv> _workerNames; static atomic<bool> _nameLock;
	};

	ⓣ static IWorker::ThreadCount()noexcept->uint8
	{
		if( T::_threadCount==std::numeric_limits<uint8>::max() )
		{
			var pSettings = Settings::TryGetSubcontainer<Settings::Container>( "workers", T::Name );
			T::_threadCount = pSettings ? pSettings->Get2<uint8>( "threads" ).value_or(0) : 0;
		}
		return T::_threadCount;
	}

	ⓣ WorkerManager::Start( sv workerName )noexcept->sp<T>
	{
		sp<T> p;
		Threading::AtomicGuard l{ _nameLock };
		if( _workerNames.emplace( workerName ).second )
		{
			l.unlock();
			Threading::AtomicGuard l2{ _objectLock };
			//TODO:  load settings, if thread count==0 then work with it here, else create worker.
			var pSettings = Settings::TryGetSubcontainer<Settings::Container>( "workers", workerName );
			var threads = pSettings ? pSettings->Get2<uint8>( "threads" ).value_or(0) : 0;
		}
		return p;
	}
/*	ⓣ WorkerManager::Push( std::coroutine_handle<>&& hCoroutine, HANDLE&& hEvent )noexcept->void
	{
		
	}*/
}
#undef var 