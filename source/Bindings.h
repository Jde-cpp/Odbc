#pragma once
#include "../../Framework/source/db/DataType.h"
#include "../../Framework/source/db/DBException.h"

namespace Jde::DB::Odbc
{
#pragma warning( push )
#pragma warning (disable: 4716)
#define $ [[noreturn]] β
#define ρ const noexcept
	struct IBindings
	{
		IBindings( uint rowCount, uint size=0 ): _output{ new SQLLEN[rowCount] }, RowCount{ rowCount }, _size{ size }{}
		virtual ~IBindings()=0;
		Ω Create( SQLSMALLINT type, uint rowCount )->up<IBindings>;
		Ω Create( SQLSMALLINT type, uint rowCount, uint size )noexcept(false)->up<IBindings>;
		β Data()noexcept->void* = 0;
		β Object( uint i )ρ->object=0;
		β CodeType()ρ->SQLSMALLINT = 0;
		β DBType()ρ->SQLSMALLINT = 0;

		$ Bit( uint i )ρ->bool{ THROW( "{} not implemented for DBType={} CodeType={}", "bit", DBType(), CodeType() ); }
		$ ToString( uint i )ρ->string{ THROW( "ToString not implemented for DBType='{}' CodeType='{}' {}", DBType(), CodeType(), "GetTypeName<decltype(this)>()" ); }
		$ Int( uint i )ρ->int64_t{ THROW( "{} not implemented for DBType={} CodeType={}", "GetInt", DBType(), CodeType() ); }
		$ Int32( uint i )ρ->int32_t{ THROW( "{} not implemented for DBType={} CodeType={}", "Int32", DBType(), CodeType() ); }
		$ IntOpt( uint i )ρ->optional<_int>{ THROW( "{} not implemented for DBType={} CodeType={} {}", "IntOpt", DBType(), CodeType(), "GetTypeName<decltype(this)>()" ); }
		$ Double( uint i )ρ->double{ THROW( "{} not implemented for DBType={} CodeType={}", "Double", DBType(), CodeType() ); }
		β GetFloat( uint i )ρ->float{ return static_cast<float>( Double(i) ); }
		$ DoubleOpt( uint i )ρ->optional<double>{ THROW( "{} not implemented for DBType={} CodeType={}", "DoubleOpt", DBType(), CodeType() ); }
		$ DateTime( uint i )ρ->DBTimePoint{ THROW( "{} not implemented for DBType={} CodeType={}", "DateTime", DBType(), CodeType() ); }
		$ DateTimeOpt( uint i )ρ->optional<DBTimePoint>{ THROW( "{} not implemented for DBType={} CodeType={}", "DateTimeOpt", DBType(), CodeType()); }
		$ UInt( uint i )ρ->uint{ THROW( "{} not implemented for DBType={} CodeType={}", "UInt", DBType(), CodeType()); }
		//β GetUInt32(uint position )ρ->uint32_t{ return static_cast<uint32_t>(UInt()); }
		$ UIntOpt( uint i )ρ->optional<uint>{ THROW( "{} not implemented for DBType={} CodeType={} - {}", "UIntOpt", DBType(), CodeType(), "GetTypeName<decltype(this)>()" ); };
		α IsNull( uint i )ρ->bool{ return _output[i]==SQL_NULL_DATA; }
		α Output( uint i )ρ->SQLLEN{ return _output[i]; }
		α OutputPtr()noexcept->SQLLEN*{ return _output.get(); }
		β Size()ρ->SQLULEN{return _size;}
		β DecimalDigits()ρ->SQLSMALLINT{return 0;}
		β BufferLength()ρ->SQLLEN{return 0;}
		
	private:
		up<SQLLEN[]> _output;
		uint RowCount;
		SQLULEN _size;
	};
#undef $
	inline IBindings::~IBindings(){};
#pragma warning( pop )
	template <class T, SQLSMALLINT TSql, SQLSMALLINT TC>
	struct TBindings : IBindings
	{
		TBindings( uint rowCount, SQLLEN size ):IBindings{ rowCount, size }, _pBuffer{ new T[rowCount*size]() }{}
		TBindings( uint rowCount )noexcept:TBindings{ rowCount, 1 }{}
		void* Data()noexcept override{ return _pBuffer.get(); }
		static consteval α SqlType()->SQLSMALLINT{ return TSql; }
		β CodeType()ρ->SQLSMALLINT override{ return TC; }
		β DBType()ρ->SQLSMALLINT override{ return SqlType(); }
	protected:
		up<T[]> _pBuffer;
	};

	#define base TBindings<char, TSql, SQL_C_CHAR>
	template<SQLSMALLINT TSql>
	struct BindingStrings final : base
	{
		BindingStrings( uint rowCount, SQLLEN size ):base{ rowCount, size }{}
		α Object( uint i )ρ->object override{ return base::IsNull( i ) ? DB::object{} : DB::object{ ToString(i) }; }
		α ToString( uint i )ρ->string override{ const char* p=base::_pBuffer.get(); return base::IsNull( i ) ? string{} : string{ p+base::Size()*i, p+base::Size()*i+base::Output(i) }; }

		α BufferLength()ρ->SQLLEN override{ return base::Size(); }
	};

	struct BindingBits final: TBindings<bool,SQL_BIT, SQL_C_BIT>
	{
		BindingBits( uint rowCount ):TBindings<bool,SQL_BIT, SQL_C_BIT>{ rowCount }{}//-7
		α Object( uint i )ρ->object override{ return object{ Bit(i) }; }
		α Bit( uint i )ρ->bool override{ return _pBuffer[i]!=0; }
		α Int( uint i )ρ->int64_t override{ return static_cast<int64_t>(Bit(i)); }
		α ToString( uint i )ρ->string override{ return Bit( i ) ? "true" : "false"; }
	};

	struct BindingInt32s final: TBindings<int,SQL_INTEGER, SQL_C_SLONG>
	{
		BindingInt32s( uint rowCount ): TBindings<int,SQL_INTEGER, SQL_C_SLONG>{ rowCount }{}
		α Object( uint i )ρ->object override{ return object{Int32(i)}; }
		α Int32( uint i )ρ->int32_t override{ return _pBuffer[i]; }
		α Int( uint i )ρ->int64_t override{ return Int32(i); }
		α UInt( uint i )ρ->uint override{ return static_cast<uint>(Int32(i)); }
		α IntOpt( uint i )ρ->optional<_int>{ optional<_int> value; if( !IsNull(i) )value=Int( i ); return value; }
		α UIntOpt( uint i )ρ->optional<uint> override{ optional<uint> optional; if( !IsNull(i) ) optional = UInt( i ); return optional; };
	};
	
	struct BindingInts final: TBindings<_int,SQL_BIGINT, SQL_C_SBIGINT>
	{
		BindingInts( uint rowCount ): TBindings<_int,SQL_BIGINT, SQL_C_SBIGINT>{ rowCount }{}
		α Object( uint i )ρ->object override{ return object{Int(i)}; };
		α Int( uint i )ρ->int64_t override{ return _pBuffer[i]; }
		α UInt( uint i )ρ->uint override{ return static_cast<uint>( Int(i) ); }
		α UIntOpt( uint i )ρ->optional<uint>{ optional<uint> value; if(!IsNull(i))value=UInt( i ); return value; };
		α DateTime( uint i )ρ->DBTimePoint{ return Clock::from_time_t(Int(i)); }
	};
#pragma warning(push)
#pragma warning(disable:4005)
#define base TBindings<SQL_TIMESTAMP_STRUCT, TSql, SQL_C_TYPE_TIMESTAMP>
	template<SQLSMALLINT TSql>
	struct BindingTimes : base
	{
		BindingTimes( uint rowCount ):base{ rowCount }{}
		
		α Object( uint i )ρ->object override{ return base::IsNull( i ) ? object{ nullptr } : object{ DateTime(i) }; }
		α BufferLength()ρ->SQLLEN override{ return sizeof(SQL_TIMESTAMP_STRUCT); }
		α Size()ρ->SQLULEN override{ return 27; }//https://wezfurlong.org/blog/2005/Nov/calling-sqlbindparameter-to-bind-sql-timestamp-struct-as-sql-c-type-timestamp-avoiding-a-datetime-overflow/
		α DecimalDigits()ρ->SQLSMALLINT override{ return 7; }
		α DateTime( uint i )ρ->DBTimePoint
		{
			if( base::IsNull(i) ) return DBTimePoint{};
			SQL_TIMESTAMP_STRUCT data = base::_pBuffer[i];
			return Jde::DateTime( data.year, (uint8)data.month, (uint8)data.day, (uint8)data.hour, (uint8)data.minute, (uint8)data.second, Duration(data.fraction) ).GetTimePoint(); 
		}
		α DateTimeOpt( uint i )ρ->optional<DBTimePoint> override{ return base::IsNull(i) ? nullopt : std::make_optional(DateTime(i)); }
	};
	////////////////////////////////////////////////////////////////////////////
	#define base TBindings<double, TSql, SQL_C_DOUBLE>
	template<SQLSMALLINT TSql>
	struct BindingDoubles : base
	{
		BindingDoubles( uint rowCount )noexcept:base{ rowCount }{}

		α Object( uint i )ρ->object override{ return base::IsNull(i) ? object{} : object{ Double(i) }; }
		α Double( uint i )ρ->double override{ return base::_pBuffer[i]; }
		α DoubleOpt( uint i )ρ->optional<double>{ optional<double> value; if( !base::IsNull(i) ) value = Double(i); return value; }
	};
	////////////////////////////////////////////////////////////////////////////
	#define base TBindings<SQL_NUMERIC_STRUCT,SQL_NUMERIC,SQL_C_NUMERIC>
	#define var const auto
	struct BindingNumerics : public base
	{
		BindingNumerics( uint rowCount )noexcept:base{ rowCount }{}
		α Object( uint i )ρ->object override{ return object{ Double(i) }; }
		α Double( uint i )ρ->double override//https://docs.microsoft.com/en-us/sql/odbc/reference/appendixes/retrieve-numeric-data-sql-numeric-struct-kb222831?view=sql-server-ver15
		{ 
			var data = _pBuffer[i];
			uint divisor = (uint)std::pow( 1, data.scale );
			_int value = 0, last=1;
			for( uint i=0; i<SQL_MAX_NUMERIC_LEN; ++i )
			{
				const int current = data.val[i];
				const int a = current % 16;
				const int b = current / 16;
				value += last * a;   
				last *= 16;
	         value += last * b;
				last *= 16;
			}
			return (data.sign ? 1 : -1)*(double)value/divisor; 
		}
		α DoubleOpt( uint i )ρ->optional<double> override{ optional<double> value; if( !IsNull(i) ) value = Double(i); return value; }
		α Int( uint i )ρ->_int override{ return (_int)Double( i ); }
	};

	////////////////////////////////////////////////////////////////////////////
	#define base TBindings<float, SQL_FLOAT, SQL_C_FLOAT>
	struct BindingFloats : base
	{
		BindingFloats( uint rowCount ): base{ rowCount }{}

		α Object( uint i )ρ->object override{ return object{ Double(i) }; }
		α Double( uint i )ρ->double override{ return _pBuffer[i]; }
		α DoubleOpt( uint i )ρ->optional<double>{ return IsNull(i) ? nullopt : optional<double>{ Double(i) }; }
	};

	////////////////////////////////////////////////////////////////////////////
	#define base TBindings<float, SQL_SMALLINT, SQL_C_SSHORT>
	struct BindingInt16s : base
	{
		BindingInt16s( uint rowCount ): base{ rowCount }{}

		α Object( uint i )ρ->object override{ return IsNull( i ) ? object{ nullptr } : object{ Int(i) }; }
		α UInt( uint i )ρ->uint override{ return static_cast<uint>( Int(i) ); }
		α Int( uint i )ρ->_int override{ return static_cast<_int>( _pBuffer[i] ); }
		α IntOpt( uint i )ρ->optional<_int> override{ return IsNull(i) ? nullopt : optional<_int>( Int(i) ); }
		α Double( uint i )ρ->double override{ return (double)Int(i); }
		α DoubleOpt( uint i )ρ->optional<double>{ return IsNull(i) ? nullopt : optional<double>( Double(i) ); }
	};
////////////////////////////////////////////////////////////////////////////
	#define base TBindings<uint8_t, SQL_TINYINT, SQL_C_TINYINT>
	struct BindingUInt8s : base
	{
		BindingUInt8s( uint rowCount ): base{ rowCount }{}

		α Object( uint i )ρ->object override{ return object{ UInt(i) }; }

		α UInt( uint i )ρ->uint override{ return static_cast<uint>( _pBuffer[i] ); }
		α UIntOpt( uint i )ρ->optional<uint> override{ optional<uint> value; if( !IsNull(i) ) value = UInt( i ); return value; }
		α IntOpt( uint i )ρ->optional<_int> override{ optional<_int> value; if( !IsNull(i) ) value = Int( i ); return value; }
		α Int( uint i )ρ->int64_t override{ return static_cast<int64_t>( UInt(i) ); }
		α Double( uint i )ρ->double override{ return (double)UInt( i ); }
		α DoubleOpt( uint i )ρ->optional<double> override{ optional<double> value; if( !IsNull(i) ) value = Double( i ); return value; }
	};

	Ξ IBindings::Create( SQLSMALLINT type, uint rowCount )noexcept(false)->up<IBindings>
	{
		up<IBindings> pBinding;
		if( type==BindingBits::SqlType() )
			pBinding = mu<BindingBits>( rowCount );
		else if( type==BindingUInt8s::SqlType() )
			pBinding = mu<BindingUInt8s>( rowCount );
		else if( type==BindingInt32s::SqlType() )
			pBinding = mu<BindingInt32s>( rowCount );
		//else if( type==SQL_DECIMAL )
		//	pBinding = mu<BindingDouble>( rowCount );
		else if( type==BindingInt16s::SqlType() )
			pBinding = mu<BindingInt16s>( rowCount );
		else if( type==BindingFloats::SqlType() )
			pBinding = mu<BindingFloats>( rowCount );
		else if( type==SQL_REAL )
			pBinding = mu<BindingDoubles<SQL_REAL>>( rowCount );
		else if( type==SQL_DOUBLE )
			pBinding = mu<BindingDoubles<SQL_DOUBLE>>( rowCount );
		else if( type==SQL_DATETIME )
			pBinding = mu<BindingTimes<SQL_DATETIME>>( rowCount );
		else if( type==BindingInts::SqlType() )
			pBinding = mu<BindingInts>( rowCount );
		else if( type==SQL_TYPE_TIMESTAMP )
			pBinding = mu<BindingTimes<SQL_TYPE_TIMESTAMP>>( rowCount );
		else if( type==BindingNumerics::SqlType() )
			pBinding = mu<BindingNumerics>( rowCount );
		else
			THROW( "Binding type '{}' is not implemented.", type );
		return pBinding;
	}
	Ξ IBindings::Create( SQLSMALLINT type, uint rowCount, uint size )noexcept(false)->up<IBindings>
	{
		up<IBindings> p;
		if( type==SQL_CHAR )
			p = mu<BindingStrings<SQL_CHAR>>( rowCount, size );
		else if( type==SQL_VARCHAR )
			p = mu<BindingStrings<SQL_VARCHAR>>( rowCount, size );
		else if( type==SQL_LONGVARCHAR )
			p = mu<BindingStrings<SQL_LONGVARCHAR>>( rowCount, size );
		else if( type==-9 )//varchar(max)
			p = mu<BindingStrings<SQL_LONGVARCHAR>>( rowCount, size );
		else
			THROW( "Binding type '{}' is not implemented.", type );
		return p;	
	}
#undef base
#undef var
#undef ρ
#pragma warning(pop)
}