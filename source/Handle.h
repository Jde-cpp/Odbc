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
	//template<typename T>
	//using SharedHandle = std::shared_ptr<T,std::function<void(T)>>;

	struct HandleEnvironment final: boost::noncopyable
	{
		HandleEnvironment();
//		~HandleEnvironment();

		operator SQLHENV()const noexcept{ return _handle.get(); }
	private:
		static std::shared_ptr<void> _handle;
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
		up<lock_guard<mutex>> _pLock;
		uint i;
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