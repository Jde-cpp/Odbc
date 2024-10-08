﻿#pragma once
#include "../../Framework/source/db/Row.h"
#include "Binding.h"
#include "Bindings.h"
#include <jde/Assert.h>

namespace Jde::DB::Odbc
{
	struct OdbcRow : public IRow
	{
		OdbcRow( const vector<up<Binding>>& bindings )ι;
		void Reset()const{_index=0;}

		α operator[]( uint value )const->object override;
		bool GetBit( uint position, SRCE )const override;
		std::string GetString( uint position, SRCE )const override;
		CIString GetCIString( uint i, SL )const override{ return CIString{GetString(i)}; }
		int64_t GetInt( uint position, SRCE )const override;
		int32_t GetInt32( uint position, SRCE )const override;
		std::optional<_int> GetIntOpt( uint position, SRCE )const override;
		double GetDouble( uint position, SRCE )const override;
		float GetFloat( uint position )const{ return static_cast<float>( GetDouble(position) ); }
		std::optional<double> GetDoubleOpt( uint position, SRCE )const override;
		DBTimePoint GetTimePoint( uint position, SRCE )const override;
		std::optional<DBTimePoint> GetTimePointOpt( uint position, SRCE )const override;
		uint GetUInt( uint position, SRCE )const override;
		std::optional<uint> GetUIntOpt( uint position, SRCE )const override;
	private:
		const vector<up<Binding>>& _bindings;
	};
	
	struct OdbcRowMulti : public IRow
	{
		OdbcRowMulti( const vector<up<IBindings>>& bindings )ι:_bindings{bindings}{};
		void Reset()Ι{ _index=0; ++_row; }
		void ResetRowIndex()Ι{ _row=0; }

		α operator[]( uint position )Ι->object override{ ASSERT( position<_bindings.size() );  return _bindings[position]->Object(_row); }
		bool GetBit( uint position, SL )Ι override{ ASSERT( position<_bindings.size() ); return _bindings[position]->Bit(_row); }
		std::string GetString( uint position, SL )Ι override{ ASSERT( position<_bindings.size() ); return _bindings[position]->ToString(_row); }
		CIString GetCIString( uint i, SRCE)Ι override{ return CIString{GetString(i, sl)}; }
		int64_t GetInt( uint position, SL )Ι override{ ASSERT( position<_bindings.size() ); return _bindings[position]->Int(_row); }
		int32_t GetInt32( uint position, SL )Ι override{ ASSERT( position<_bindings.size() ); return _bindings[position]->Int32(_row); }
		std::optional<_int> GetIntOpt( uint position, SL )Ι override{ ASSERT( position<_bindings.size() ); return _bindings[position]->IntOpt(_row); }
		double GetDouble( uint position, SL )Ι override{ ASSERT( position<_bindings.size() ); return _bindings[position]->Double(_row); }
		float GetFloat( uint position )Ι{ return static_cast<float>( GetDouble(position, SRCE_CUR) ); }
		std::optional<double> GetDoubleOpt( uint position, SL )Ι override{ ASSERT( position<_bindings.size() ); return _bindings[position]->DoubleOpt(_row); }
		DBTimePoint GetTimePoint( uint position, SL )Ι override{ ASSERT( position<_bindings.size() ); return _bindings[position]->DateTime(_row); }
		std::optional<DBTimePoint> GetTimePointOpt( uint position, SL )Ι override{ ASSERT( position<_bindings.size() ); return _bindings[position]->DateTimeOpt(_row); }
		uint GetUInt( uint position, SL )Ι override{ ASSERT( position<_bindings.size() ); return _bindings[position]->UInt(_row); }
		std::optional<uint> GetUIntOpt( uint position, SL )Ι override{ ASSERT( position<_bindings.size() ); return _bindings[position]->UIntOpt(_row); }
	private:
		const vector<up<IBindings>>& _bindings;
		mutable uint _row{0};
	};
}