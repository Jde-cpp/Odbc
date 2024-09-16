#pragma once
#include "../../Framework/source/db/DataType.h"
#include "../../Framework/source/db/DBException.h"

namespace Jde::DB::Odbc{
#pragma warning( push )
#pragma warning (disable: 4716)
	struct Binding{
		Binding( SQLSMALLINT type, SQLSMALLINT cType, SQLLEN output=0 )ι:
			DBType{ type },
			CodeType{cType},
			Output{output}
		{}
		virtual ~Binding()=default;
		Ω GetBinding( SQLSMALLINT type ) ->up<Binding>;
		Ω Create( object parameter )ι->up<Binding>;

		β Data()ι->void* = 0;
		β GetDataValue()Ι->object=0;
#define $ [[noreturn]] β
		$ GetBit()Ι->bool{ THROW( "{} not implemented for DBType={} CodeType={}", "bit", DBType, CodeType ); }
		$ to_string()Ε->string{ THROW( "to_string not implemented for DBType='{}' CodeType='{}' {}", DBType, CodeType, "GetTypeName<decltype(this)>()" ); }
		$ GetInt()Ε->int64_t{ THROW( "{} not implemented for DBType={} CodeType={}", "GetInt", DBType, CodeType ); }
		$ GetInt32()Ε->int32_t{ THROW( "{} not implemented for DBType={} CodeType={}", "GetInt32", DBType, CodeType ); }
		$ GetIntOpt()Ε->std::optional<_int>{ THROW( "{} not implemented for DBType={} CodeType={} {}", "GetIntOpt", DBType, CodeType, "GetTypeName<decltype(this)>()" ); }
		$ GetDouble()Ε->double{ THROW( "{} not implemented for DBType={} CodeType={}", "GetDouble", DBType, CodeType ); }
		β GetFloat()Ε->float{ return static_cast<float>( GetDouble() ); }
		$ GetDoubleOpt()Ε->std::optional<double>{ THROW( "{} not implemented for DBType={} CodeType={}", "GetDoubleOpt", DBType, CodeType ); }
		$ GetDateTime()Ε->DBTimePoint{ THROW( "{} not implemented for DBType={} CodeType={}", "GetDateTime", DBType, CodeType ); }
		$ GetDateTimeOpt()Ε->std::optional<DBTimePoint>{ THROW( "{} not implemented for DBType={} CodeType={}", "GetDateTimeOpt", DBType, CodeType); }
		$ GetUInt()Ε->uint{ THROW( "{} not implemented for DBType={} CodeType={}", "GetUInt", DBType, CodeType); }
		β GetUInt32(uint)Ε->uint32_t{ return static_cast<uint32_t>(GetUInt()); }
		$ GetUIntOpt()Ε->std::optional<uint>{ THROW( "{} not implemented for DBType={} CodeType={} - {}", "GetUIntOpt", DBType, CodeType, "GetTypeName<decltype(this)>()" ); };
		α IsNull()Ι->bool{ return Output==SQL_NULL_DATA; }
		β Size()Ι->SQLULEN{return 0;}
		β DecimalDigits()Ι->SQLSMALLINT{return 0;}
		β BufferLength()Ι->SQLLEN{return 0;}
		SQLSMALLINT DBType{0};
		SQLSMALLINT CodeType{0};
		SQLLEN Output;//SqlBindParameter wants SQLLEN
	};
#undef $
#pragma warning( pop )
	template <class T, SQLSMALLINT TSql, SQLSMALLINT TC>
	struct TBinding : Binding{
		TBinding()ι:Binding{TSql,TC}{}
		α Data()ι->void* override{ return &_data; }
	protected:
		SQL_NUMERIC_STRUCT _data;
	};


	struct BindingNull final : Binding{
		BindingNull( SQLSMALLINT type=SQL_VARCHAR )ι:
			Binding{ type, SQL_C_CHAR, SQL_NULL_DATA }
		{}
		void* Data()ι override{ return nullptr; }
		object GetDataValue()Ι override{ return object{nullptr}; }
	};

	struct BindingString final: Binding{
		BindingString( SQLSMALLINT type, SQLLEN size )ι:Binding{ type, SQL_C_CHAR, size }, _buffer{std::make_unique<char[]>( size )}{}
		//BindingString( str value )ι:Binding{ SQL_VARCHAR, SQL_C_CHAR, (SQLLEN)value.size() },_buffer( value.begin(), value.end() ){}
		BindingString( sv value )ι:Binding{ SQL_VARCHAR, SQL_C_CHAR, (SQLLEN)value.size() }, _buffer{ std::make_unique<char[]>(value.size()) }{std::copy(value.begin(), value.end(), _buffer.get()); }
		α Data()ι->void* override{ return _buffer.get(); }
		α GetDataValue()Ι->DB::object override{ return Output==-1 ? DB::object{} : DB::object{ to_string() }; }
		α to_string()Ι->string override{ return Output==-1 ? string{} : string{ _buffer.get(), _buffer.get()+Output }; }

		α BufferLength()Ι->SQLLEN override{ return std::max(Output,(SQLLEN)0); }
		α Size()Ι->SQLULEN override{ return (SQLULEN)BufferLength(); }
	private:
		up<char[]> _buffer;
	};

	struct BindingBit final : Binding{
		BindingBit()ι:BindingBit{ (SQLSMALLINT)SQL_BIT }{}//-7
		BindingBit( SQLSMALLINT type )ι:Binding{ type, SQL_C_BIT }{}
		BindingBit( bool value )ι: Binding{ SQL_CHAR, SQL_C_BIT },_data{value ? '\1' : '\0'}{}
		α Data()ι->void* override{ return &_data; }
		α GetDataValue()Ι->object override{ return object{_data!=0}; }
		α GetBit()Ι->bool override{ return _data!=0; }
		α Size()Ι->SQLULEN{return 1;}
		α GetInt()Ι->int64_t override{ return static_cast<int64_t>(_data); }
		α to_string()Ι->string override{ return _data ? "true" : "false"; }
	private:
		char _data;
	};
	struct BindingInt8 final : Binding{
		BindingInt8( SQLSMALLINT type=SQL_TINYINT )ι: Binding{ type, SQL_C_TINYINT }{}
		BindingInt8( int8_t value )ι: Binding{ SQL_TINYINT, SQL_C_TINYINT },_data{value}{}
		α Data()ι->void* override{ return &_data; }
		α GetDataValue()Ι->object override{ return object{_data}; }
		α GetInt32()Ι->int32_t override{ return _data; }
		
		int64_t GetInt()Ι override{ return GetInt32(); }
		uint GetUInt()Ι override{ return static_cast<uint>(GetInt32()); }
		std::optional<_int> GetIntOpt()Ι{ std::optional<_int> value; if( !IsNull() )value=GetInt(); return value; }
		std::optional<uint> GetUIntOpt()Ι override{ std::optional<uint> optional; if( !IsNull() ) optional=GetUInt(); return optional; };
	private:
		int8_t _data;
	};
	struct BindingInt32 final : Binding{
		BindingInt32( SQLSMALLINT type=SQL_INTEGER )ι: Binding{ type, SQL_C_SLONG }{}
		BindingInt32( int value )ι: Binding{ SQL_INTEGER, SQL_C_SLONG },_data{value}{}
		α Data()ι->void* override{ return &_data; }
		α GetDataValue()Ι->object override{ return object{_data}; }
		α GetInt32()Ι->int32_t override{ return _data; }
		int64_t GetInt()Ι override{ return GetInt32(); }
		uint GetUInt()Ι override{ return static_cast<uint>(GetInt32()); }
		std::optional<_int> GetIntOpt()Ι{ std::optional<_int> value; if( !IsNull() )value=GetInt(); return value; }
		std::optional<uint> GetUIntOpt()Ι override{ std::optional<uint> optional; if( !IsNull() ) optional=GetUInt(); return optional; };
	private:
		int _data;
	};

	struct BindingDecimal final : Binding
	{};

	struct BindingInt final : Binding{
		BindingInt( SQLSMALLINT type=SQL_C_SBIGINT )ι: Binding{ type, SQL_C_SBIGINT }{}
		BindingInt( _int value )ι: Binding{ SQL_BIGINT, SQL_C_SBIGINT },_data{value}{}
		α Data()ι->void* override{ return &_data; }
		α GetDataValue()Ι->object override{ return object{_data}; }
		int64_t GetInt()Ι override{ return _data; }
		uint GetUInt()Ι override{ return static_cast<uint>( GetInt() ); }
		std::optional<uint> GetUIntOpt()Ι{ std::optional<uint> value; if(!IsNull())value=GetUInt(); return value; }
		β GetDateTime()Ι->DBTimePoint{ return Clock::from_time_t(_data); }
	private:
		_int _data ;
	};

	struct BindingTimeStamp final:  Binding{
		BindingTimeStamp( SQLSMALLINT type=SQL_C_TYPE_TIMESTAMP )ι: Binding{ type, SQL_C_TYPE_TIMESTAMP }{}
		BindingTimeStamp( SQL_TIMESTAMP_STRUCT value )ι: Binding{ SQL_TYPE_TIMESTAMP, SQL_C_TYPE_TIMESTAMP },_data{value}{}
		α Data()ι->void* override{ return &_data; }
		object GetDataValue()Ι override{ return IsNull() ? object(nullptr) : object{GetDateTime()}; };
		DBTimePoint GetDateTime()Ι{ return IsNull() ? DBTimePoint{} : Jde::DateTime( _data.year, (uint8)_data.month, (uint8)_data.day, (uint8)_data.hour, (uint8)_data.minute, (uint8)_data.second, Duration(_data.fraction) ).GetTimePoint(); }
		std::optional<DBTimePoint> GetDateTimeOpt()Ι override{ return IsNull() ? std::nullopt : std::make_optional(GetDateTime()); }
	private:
		SQL_TIMESTAMP_STRUCT _data;
	};


	struct BindingUInt final : Binding{
		BindingUInt( SQLSMALLINT type )ι:	Binding{ type, SQL_C_UBIGINT }{}
		BindingUInt( uint value )ι: Binding{ SQL_INTEGER, SQL_C_SLONG },_data{value}{}
		α Data()ι->void* override{ return &_data; }
		α GetDataValue()Ι->object override{ return object{_data}; }
		uint GetUInt()Ι override{ return _data; }
	private:
		uint _data ;
	};

	struct BindingDateTime final : Binding{
		BindingDateTime( SQLSMALLINT type=SQL_TYPE_TIMESTAMP )ι:Binding{ type, SQL_C_TIMESTAMP, sizeof(SQL_TIMESTAMP_STRUCT) }{}

		BindingDateTime( const optional<DBTimePoint>& value )ι;

		α Data()ι->void* override{ return &_data; }
		object GetDataValue()Ι override{ return IsNull() ? object{nullptr} : object{GetDateTime()}; }
		SQLLEN BufferLength()Ι override{ return sizeof(_data); }
		//https://learn.microsoft.com/en-us/sql/odbc/reference/appendixes/column-size?view=sql-server-ver16&redirectedfrom=MSDN
		//https://wezfurlong.org/blog/2005/Nov/calling-sqlbindparameter-to-bind-sql-timestamp-struct-as-sql-c-type-timestamp-avoiding-a-datetime-overflow/
		SQLULEN Size()Ι override{ return 23; }//23 works with 0
		SQLSMALLINT DecimalDigits()Ι{ return 3; }//https://stackoverflow.com/questions/40918607/cannot-bind-a-sql-type-timestamp-value-using-odbc-with-ms-sql-server-hy104-inv
		DBTimePoint GetDateTime()Ι override{ return IsNull() ? DBTimePoint() : Jde::DateTime( _data.year, (uint8)_data.month, (uint8)_data.day, (uint8)_data.hour, (uint8)_data.minute, (uint8)_data.second, Duration(_data.fraction) ).GetTimePoint();}
		std::optional<DBTimePoint> GetDateTimeOpt()Ι override{
			std::optional<DBTimePoint> value;
			if( !IsNull() )
				value = GetDateTime();
			return value;
		}
		SQL_TIMESTAMP_STRUCT _data;
	};

	struct BindingDouble final : Binding{
		BindingDouble( SQLSMALLINT type=SQL_DOUBLE )ι:	Binding{ type, SQL_C_DOUBLE }{}
		BindingDouble( double value )ι: Binding{ SQL_DOUBLE, SQL_C_DOUBLE },_data{value}{}
		BindingDouble( const optional<double>& value )ι: Binding{ SQL_DOUBLE, SQL_C_DOUBLE },_data{value.has_value() ? value.value() : 0.0}{ if( !value.has_value() ) Output=SQL_NULL_DATA; }

		α Data()ι->void* override{ return &_data; }
		α GetDataValue()Ι->object override{ return object{_data}; }

		double GetDouble()Ι override{ return _data; }
		std::optional<double> GetDoubleOpt()Ι{ std::optional<double> value; if( !IsNull() ) value = GetDouble();	return value; }
	private:
		double _data;
	};
	struct BindingNumeric final : TBinding<SQL_NUMERIC_STRUCT,SQL_NUMERIC,SQL_C_NUMERIC>{
		α GetDataValue()Ι->object override{ return object{GetDouble()}; }
		α GetDouble()Ι->double override//https://docs.microsoft.com/en-us/sql/odbc/reference/appendixes/retrieve-numeric-data-sql-numeric-struct-kb222831?view=sql-server-ver15
		{
			uint divisor = (uint)std::pow( 1, _data.scale );
			_int value = 0, last=1;
			for( uint i=0; i<SQL_MAX_NUMERIC_LEN; ++i )
			{
				const int current = _data.val[i];
				const int a = current % 16;
				const int b = current / 16;
				value += last * a;
				last *= 16;
	         value += last * b;
				last *= 16;
			}
			return (_data.sign ? 1 : -1)*(double)value/divisor;
		}
		α GetDoubleOpt()Ι->std::optional<double> override{ std::optional<double> value; if( !IsNull() ) value = GetDouble(); return value; }
		α GetInt()Ι->_int override{ return (_int)GetDouble(); }
		α GetUInt()Ι->uint override{ return (uint)GetDouble(); }
	};

	struct BindingFloat final : Binding{
		BindingFloat( SQLSMALLINT type=SQL_FLOAT )ι:	Binding{ type, SQL_C_FLOAT }{}
		BindingFloat( float value )ι: Binding{ SQL_FLOAT, SQL_C_FLOAT },_data{value}{}

		α Data()ι->void* override{ return &_data; }
		α GetDataValue()Ι->object override{ return object{_data}; }

		double GetDouble()Ι override{ return _data; }
		std::optional<double> GetDoubleOpt()Ι{ std::optional<double> value; if( !IsNull() ) value = GetDouble();	return value; }
	private:
		float _data;
	};

	struct BindingInt16 final : Binding{
		BindingInt16()ι:Binding{ SQL_SMALLINT, SQL_C_SSHORT }{}
		BindingInt16( int16_t value )ι: Binding{ SQL_SMALLINT, SQL_C_SSHORT },_data{value}{}

		α Data()ι->void* override{ return &_data; }
		α GetDataValue()Ι->object override{ return Output==-1 ? object{nullptr} : object{GetInt()}; }
		α GetUInt()Ι->uint override{ return static_cast<uint>(_data); }
		α GetInt()Ι->_int override{ return static_cast<_int>(_data); }
		α GetIntOpt()Ι->optional<_int> override{ std::optional<_int> value; if( !IsNull() ) value = GetInt(); return value; }
		α GetDouble()Ι->double override{ return _data; }
		α GetDoubleOpt()Ι->std::optional<double>{ std::optional<double> value; if( !IsNull() ) value = GetDouble(); return value; }
	private:
		int16_t _data;
	};

	struct BindingUInt8 final : Binding{
		BindingUInt8()ι:Binding{ SQL_TINYINT, SQL_C_TINYINT }{}
		BindingUInt8( uint8_t value )ι: Binding{ SQL_TINYINT, SQL_C_TINYINT },_data{value}{}

		α Data()ι->void* override{ return &_data; }
		α GetDataValue()Ι->object override{ return object{_data}; }

		α GetUInt()Ι->uint override{ return static_cast<uint>(_data); }
		α GetUIntOpt()Ι->std::optional<uint>{ std::optional<_int> value; if( !IsNull() ) value = GetUInt(); return value; }
		α GetInt32()Ι->int32_t override{ return static_cast<int32_t>(_data); }
		α GetIntOpt()Ι->std::optional<_int> override{ std::optional<_int> value; if( !IsNull() ) value = GetInt(); return value; }
		α GetInt()Ι->_int override{ return static_cast<_int>(_data); }
		α GetDouble()Ι->double override{ return _data; }
		α GetDoubleOpt()Ι->std::optional<double>{ std::optional<double> value; if( !IsNull() ) value = GetDouble();	return value; }
	private:
		uint8_t _data;
	};

	Ξ Binding::GetBinding( SQLSMALLINT type )ε->up<Binding>{
		up<Binding> pBinding;
		if( type==SQL_BIT )
			pBinding = mu<BindingBit>();
		else if( type==SQL_TINYINT )
			pBinding = mu<BindingUInt8>();
		else if( type==SQL_INTEGER )
			pBinding = mu<BindingInt32>( type );
		else if( type==SQL_DECIMAL )
			pBinding = mu<BindingDouble>( type );
		else if( type==SQL_SMALLINT )
			pBinding = mu<BindingInt16>();
		else if( type==SQL_FLOAT )
			pBinding = mu<BindingFloat>( type );
		else if( type==SQL_REAL )
			pBinding = mu<BindingDouble>( type );
		else if( type==SQL_DOUBLE )
			pBinding = mu<BindingDouble>( type );
		else if( type==SQL_DATETIME )
			pBinding = mu<BindingDateTime>( type );
		else if( type==SQL_BIGINT )
			pBinding = mu<BindingInt>( type );
		else if( type==SQL_TYPE_TIMESTAMP )
			pBinding = mu<BindingTimeStamp>( type );
		else if( type==SQL_BIT )
			pBinding = mu<BindingBit>();
		else if( type==SQL_NUMERIC )
			pBinding = mu<BindingNumeric>();
		else
			THROW( "Binding type '{}' is not implemented.", type );
		return pBinding;
	}
	using std::get;
	Ξ Binding::Create( object parameter )ι->up<Binding>{
		up<Binding> pBinding;
		switch( (EObject)parameter.index() ){
		case EObject::Null:
			pBinding = mu<BindingNull>();
		break;
		case EObject::String:
			pBinding = mu<BindingString>( get<string>(parameter) );
		break;
		case EObject::StringView:
			pBinding = mu<BindingString>( get<sv>(parameter) );
		break;
		case EObject::StringPtr:
			pBinding = mu<BindingString>( *get<sp<string>>(parameter) );
		break;
		case EObject::Bool:
			pBinding = mu<BindingBit>( get<bool>(parameter) );
		break;
		case EObject::Int8:
			pBinding = mu<BindingInt8>( get<int8_t>(parameter) );
		break;
		case EObject::Int32:
			pBinding = mu<BindingInt32>( get<int>(parameter) );
		break;
		case EObject::Int64:
			pBinding = mu<BindingInt>( get<_int>(parameter) );
		break;
		case EObject::UInt32:
			pBinding = mu<BindingInt32>( (int)get<uint32>(parameter) );
		break;
		case EObject::UInt64:
			pBinding = mu<BindingInt>( (_int)get<uint>(parameter) );
		break;
		case EObject::Double:
			pBinding = mu<BindingDouble>( get<double>(parameter) );
		break;
		case EObject::Time:
			pBinding = mu<BindingDateTime>( get<DBTimePoint>(parameter) );
		break;
		default:
			ASSERT( false );
		}
		return pBinding;
	}
	inline BindingDateTime::BindingDateTime( const optional<DBTimePoint>& value )ι:
		BindingDateTime{}{
		if( !value.has_value() )
			Output = SQL_NULL_DATA;
		else{
			Jde::DateTime date( value.value() );
			_data.year = (SQLSMALLINT)date.Year();
			_data.month = date.Month();
			_data.day = date.Day();
			_data.hour = date.Hour();
			_data.minute = date.Minute();
			_data.second = date.Second();
			_data.fraction = Chrono::MillisecondsSinceEpoch( date )%1000*1'000'000;
		}
	}
}