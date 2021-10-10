#include "OdbcRow.h"
#include <jde/Assert.h>

namespace Jde::DB::Odbc
{
	OdbcRow::OdbcRow( const vector<up<Binding>>& bindings ):
		_bindings{bindings}
	{}
	
	α OdbcRow::operator[]( uint position )const->DataValue
	{
		ASSERT( position<_bindings.size() );
		return _bindings[position]->GetDataValue();
	}
	bool OdbcRow::GetBit( uint position )const{ ASSERT( position<_bindings.size() ); return _bindings[position]->GetBit(); }
	std::string OdbcRow::GetString( uint position )const{ ASSERT( position<_bindings.size() ); return _bindings[position]->to_string(); }
	int64_t OdbcRow::GetInt( uint position )const{ ASSERT( position<_bindings.size() ); return _bindings[position]->GetInt(); }
	int32_t OdbcRow::GetInt32( uint position )const{ ASSERT( position<_bindings.size() ); return _bindings[position]->GetInt32(); }
	std::optional<_int> OdbcRow::GetIntOpt( uint position )const{ ASSERT( position<_bindings.size() ); return _bindings[position]->GetIntOpt(); }
	double OdbcRow::GetDouble( uint position )const{ ASSERT( position<_bindings.size() ); return _bindings[position]->GetDouble(); }
	std::optional<double> OdbcRow::GetDoubleOpt( uint position )const{ ASSERT( position<_bindings.size() ); return _bindings[position]->GetDoubleOpt(); }
	DBDateTime OdbcRow::GetDateTime( uint position )const{ ASSERT( position<_bindings.size() ); return _bindings[position]->GetDateTime(); }
	std::optional<DBDateTime> OdbcRow::GetDateTimeOpt( uint position )const{ ASSERT( position<_bindings.size() ); return _bindings[position]->GetDateTimeOpt(); }
	uint OdbcRow::GetUInt( uint position )const{ ASSERT( position<_bindings.size() ); return _bindings[position]->GetUInt(); }
	std::optional<uint> OdbcRow::GetUIntOpt( uint position )const{ ASSERT( position<_bindings.size() ); return _bindings[position]->GetUIntOpt(); }
}