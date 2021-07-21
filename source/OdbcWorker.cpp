#include "OdbcWorker.h"
#include "../../Windows/source/WindowsWorker.h"

namespace Jde::DB::Odbc
{
	
	vector<up<IWorker>> WorkerManager::_workers;  
	atomic<bool> WorkerManager::_objectLock{false};
	flat_set<sv> WorkerManager::_workerNames; 
	atomic<bool> WorkerManager::_nameLock{ false };
	uint8 OdbcWorker::_threadCount{ std::numeric_limits<uint8>::max() };

	α OdbcWorker::Push( std::coroutine_handle<>&& h, HANDLE hEvent, bool close )noexcept->void
	{
		if( IWorker::ThreadCount<OdbcWorker>()==0 )
			Windows::WindowsWorkerMain::Push( move(h), hEvent, close );
	}
}