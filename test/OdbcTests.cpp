#include <condition_variable>
#include <shared_mutex>
#include <jde/coroutine/Task.h>
#include "../source/OdbcDataSource.h"
#include "../../Framework/source/db/Database.h"

namespace Jde::DB::Odbc
{
	using namespace Coroutine;
	struct OdbcTests : public ::testing::Test
	{
	protected:
		OdbcTests() {}
		~OdbcTests() override{}

		void SetUp() override {}
		void TearDown() override {}
	public:
	};

	std::condition_variable_any cv;
	std::shared_mutex mtx;

	α Select()->Task2
	{
		auto pDataSource = dynamic_pointer_cast<OdbcDataSource>( DataSource() );
		auto x = co_await pDataSource->ScalerCo<uint>( "select count(*) from sys.dm_exec_sessions sess join sys.dm_exec_connections conn on sess.session_id = conn.session_id where program_name='Tests.Odbc'" );
		uint connectionCount = *x.Get<uint>();
		ASSERT( connectionCount==1 );
		std::shared_lock l{ mtx };
		cv.notify_one();
	}

	TEST_F( OdbcTests, Main ) 
	{
		Select();
		std::shared_lock l{ mtx };
		cv.wait( l );
		DBG( "Done"sv );
	}
}