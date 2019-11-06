#pragma once

namespace Jde::DB::Odbc
{
/*	template<typename T>
	struct Handle
	{
		Handle( function<T()>& constructor, function<void(const T&)>& destructor ):_constructor{destructor}, _constructor{destructor}
	private:
		function<T()> _constructor;
		function<void(const T&)> _destructor;
	};
*/
	struct HandleEnvironment : boost::noncopyable
	{
		HandleEnvironment();
		~HandleEnvironment();

		operator SQLHENV()const noexcept{ return _handle; }
	private:
		SQLHENV _handle{nullptr};
	};

	struct HandleSession : boost::noncopyable
	{
		//HandleSession( const HandleEnvironment& env );
		HandleSession( string_view connectionString );
		~HandleSession();
		operator HDBC()const noexcept{ return _handle; }
	private:
		static HDBC s_handle;
		static mutex _mutex;

		HDBC _handle{nullptr};
		lock_guard<mutex> _lock;
		//string _connectionString;
		HandleEnvironment _env;
	};

	struct HandleStatement : boost::noncopyable
	{
		//HandleStatement( const HandleSession& env );
		HandleStatement( string_view connectionString );
		~HandleStatement();
		operator SQLHSTMT()const noexcept{ return _handle; }
	private:
		SQLHSTMT _handle{nullptr};
		HandleSession _session;
	};
}