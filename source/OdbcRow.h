#pragma once
#include "../../Framework/source/db/Row.h"
#include "Bindings.h"
#include <jde/Assert.h>

namespace Jde::DB::Odbc
{
	struct OdbcRow : public IRow
	{
		OdbcRow( const vector<up<Binding>>& bindings );
		void Reset()const{_index=0;}

		DataValue operator[]( uint value )const override;
		bool GetBit( uint position )const override;
		std::string GetString( uint position )const override;
		CIString GetCIString( uint i )const override{ return CIString{GetString(i)}; }
		int64_t GetInt( uint position )const override;
		int32_t GetInt32( uint position )const override;
		std::optional<_int> GetIntOpt( uint position )const override;
		double GetDouble( uint position )const override;
		float GetFloat( uint position )const{ return static_cast<float>( GetDouble(position) ); }
		std::optional<double> GetDoubleOpt( uint position )const override;
		DBDateTime GetDateTime( uint position )const override;
		std::optional<DBDateTime> GetDateTimeOpt( uint position )const override;
		uint GetUInt( uint position )const override;
		std::optional<uint> GetUIntOpt( uint position )const override;
	private:
		const vector<up<Binding>>& _bindings;
	};
	struct OdbcRow2 : public IRow
	{
		OdbcRow2( const vector<Binding>& bindings )noexcept:_bindings{bindings}{};
		void Reset()const noexcept{_index=0;}

		DataValue operator[]( uint position )const noexcept override{ ASSERT( position<_bindings.size() );  return _bindings[position].GetDataValue(); }
		bool GetBit( uint position )const noexcept override{ ASSERT( position<_bindings.size() ); return _bindings[position].GetBit(); }
		std::string GetString( uint position )const noexcept override{ ASSERT( position<_bindings.size() ); return _bindings[position].to_string(); }
		CIString GetCIString( uint i )const noexcept override{ return CIString{GetString(i)}; }
		int64_t GetInt( uint position )const noexcept override{ ASSERT( position<_bindings.size() ); return _bindings[position].GetInt(); }
		int32_t GetInt32( uint position )const noexcept override{ ASSERT( position<_bindings.size() ); return _bindings[position].GetInt32(); }
		std::optional<_int> GetIntOpt( uint position )const noexcept override{ ASSERT( position<_bindings.size() ); return _bindings[position].GetIntOpt(); }
		double GetDouble( uint position )const noexcept override{ ASSERT( position<_bindings.size() ); return _bindings[position].GetDouble(); }
		float GetFloat( uint position )const noexcept{ return static_cast<float>( GetDouble(position) ); }
		std::optional<double> GetDoubleOpt( uint position )const noexcept override{ ASSERT( position<_bindings.size() ); return _bindings[position].GetDoubleOpt(); }
		DBDateTime GetDateTime( uint position )const noexcept override{ ASSERT( position<_bindings.size() ); return _bindings[position].GetDateTime(); }
		std::optional<DBDateTime> GetDateTimeOpt( uint position )const noexcept override{ ASSERT( position<_bindings.size() ); return _bindings[position].GetDateTimeOpt(); }
		uint GetUInt( uint position )const noexcept override{ ASSERT( position<_bindings.size() ); return _bindings[position].GetUInt(); }
		std::optional<uint> GetUIntOpt( uint position )const noexcept override{ ASSERT( position<_bindings.size() ); return _bindings[position].GetUIntOpt(); }
	private:
		const vector<Binding>& _bindings;
	};
}