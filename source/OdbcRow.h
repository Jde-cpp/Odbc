#pragma once
#include "../../Framework/source/db/Row.h"
#include "Bindings.h"

namespace Jde::DB::Odbc
{
	struct OdbcRow : public IRow
	{
		OdbcRow( const vector<up<Binding>>& bindings );
		void Reset()const{_index=0;}

		DataValue operator[]( uint value )const override;
		bool GetBit( uint position )const override;
		const std::string GetString( uint position )const override;
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
}