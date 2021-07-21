#include "gtest/gtest.h"
#include <jde/App.h>
#include "../../Windows/source/WindowsWorker.h"
//#include "../../Framework/source/Settings.h"
//#include "../../Framework/source/Cache.h"
#define var const auto
template<class T> using sp = std::shared_ptr<T>;
template<typename T>
constexpr auto ms = std::make_shared<T>;

int main( int argc, char **argv )
{
	using namespace Jde;
	Windows::WindowsWorkerMain::Start( std::nullopt );
	::testing::InitGoogleTest( &argc, argv );

	OSApp::Startup( argc, argv, "Tests.Odbc"sv );

	::testing::GTEST_FLAG(filter) = "OdbcTests.Main";//QLTests.DefTestsFetch
	auto result = RUN_ALL_TESTS();
	Windows::WindowsWorkerMain::Stop();
	IApplication::Instance().Wait();
	IApplication::CleanUp();
	return result;
}