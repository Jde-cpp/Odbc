#pragma once
#include "../../Framework/source/db/DataType.h"
#include "../../Framework/source/db/DBException.h"

namespace Jde::DB::Odbc
{
#pragma warning( push )
#pragma warning (disable: 4716)
	struct Binding
	{
		Binding( SQLSMALLINT type, SQLSMALLINT cType, SQLLEN output=0 ):
			DBType{ type },
			CodeType{cType},
			Output{output}
		{}
		virtual ~Binding()=default;
		Ω GetBinding( SQLSMALLINT type )->up<Binding>;
		Ω Create( object parameter )->up<Binding>;

		β Data()noexcept->void* = 0;
		β GetDataValue()const->object=0;
#define $ [[noreturn]] β
		$ GetBit()const->bool{ THROW( "{} not implemented for DBType={} CodeType={}", "bit", DBType, CodeType ); }
		$ to_string()Ε->string{ THROW( "to_string not implemented for DBType='{}' CodeType='{}' {}", DBType, CodeType, "GetTypeName<decltype(this)>()" ); }
		[[noreturn]] virtual int64_t GetInt()const{ THROW( "{} not implemented for DBType={} CodeType={}", "GetInt", DBType, CodeType ); }
		$ GetInt32()Ε->int32_t{ THROW( "{} not implemented for DBType={} CodeType={}", "GetInt32", DBType, CodeType ); }
		[[noreturn]] virtual std::optional<_int> GetIntOpt()const{ THROW( "{} not implemented for DBType={} CodeType={} {}", "GetIntOpt", DBType, CodeType, "GetTypeName<decltype(this)>()" ); }
		[[noreturn]] virtual double GetDouble()const{ THROW( "{} not implemented for DBType={} CodeType={}", "GetDouble", DBType, CodeType ); }
		virtual float GetFloat()const{ return static_cast<float>( GetDouble() ); }
		$ GetDoubleOpt()Ε->std::optional<double>{ THROW( "{} not implemented for DBType={} CodeType={}", "GetDoubleOpt", DBType, CodeType ); }
		$ GetDateTime()Ε->DBTimePoint{ THROW( "{} not implemented for DBType={} CodeType={}", "GetDateTime", DBType, CodeType ); }
		$ GetDateTimeOpt()Ε->std::optional<DBTimePoint>{ THROW( "{} not implemented for DBType={} CodeType={}", "GetDateTimeOpt", DBType, CodeType); }
		[[noreturn]] virtual uint GetUInt()const{ THROW( "{} not implemented for DBType={} CodeType={}", "GetUInt", DBType, CodeType); }
		virtual uint32_t GetUInt32(uint position )const{ return static_cast<uint32_t>(GetUInt()); }
		[[noreturn]] virtual std::optional<uint> GetUIntOpt()const{ THROW( "{} not implemented for DBType={} CodeType={} - {}", "GetUIntOpt", DBType, CodeType, "GetTypeName<decltype(this)>()" ); };
		bool IsNull()const{ return Output==SQL_NULL_DATA; }
		virtual SQLULEN Size()Ι{return 0;}
		virtual SQLSMALLINT DecimalDigits()Ι{return 0;}
		virtual SQLLEN BufferLength()Ι{return 0;}
		SQLSMALLINT DBType{0};
		SQLSMALLINT CodeType{0};
		SQLLEN Output;
	};
#undef $
#pragma warning( pop )
	template <class T, SQLSMALLINT TSql, SQLSMALLINT TC>
	struct TBinding : Binding
	{
		TBinding()noexcept:Binding{TSql,TC}{}
		void* Data()noexcept override{ return &_data; }
	protected:
		SQL_NUMERIC_STRUCT _data;
	};


	struct BindingNull final : Binding
	{
		BindingNull( SQLSMALLINT type=SQL_VARCHAR ):
			Binding{ type, SQL_C_CHAR, SQL_NULL_DATA }
		{}
		void* Data()noexcept override{ return nullptr; }
		object GetDataValue()const override{ return object{nullptr}; }
	};

	struct BindingString final: Binding
	{
		BindingString( SQLSMALLINT type, SQLLEN size ):Binding{ type, SQL_C_CHAR, size }{ _buffer.reserve( size ); }
		BindingString( str value ):Binding{ SQL_VARCHAR, SQL_C_CHAR, value.size() },_buffer( value.begin(), value.end() ){}
		BindingString( sv value ):Binding{ SQL_VARCHAR, SQL_C_CHAR, value.size() },_buffer( value.begin(), value.end() ){}
		α Data()noexcept->void* override{ return _buffer.data(); }
		α GetDataValue()const->DB::object override{ return Output==-1 ? DB::object{} : DB::object{ to_string() }; }
		α to_string()const->string override{ return Output==-1 ? string{} : string{ _buffer.data(), _buffer.data()+Output }; }

		α BufferLength()Ι->SQLLEN override{return _buffer.size();}
		α Size()Ι->SQLULEN override{ return _buffer.size(); }
	private:
		vector<char> _buffer;
	};

	struct BindingBit final : Binding
	{
		BindingBit():BindingBit{ (SQLSMALLINT)SQL_BIT }{}//-7
		BindingBit( SQLSMALLINT type ):Binding{ type, SQL_C_BIT }{}
		BindingBit( bool value ): Binding{ SQL_CHAR, SQL_C_BIT },_data{value ? '\1' : '\0'}{}
		void* Data()noexcept override{ return &_data; }
		object GetDataValue()const override{ return object{_data!=0}; }
		bool GetBit()const override{ return _data!=0; }
		int64_t GetInt()const override{ return static_cast<int64_t>(_data); }
		string to_string()const override{ return _data ? "true" : "false"; }
	private:
		char _data;
	};
	struct BindingInt8 final : Binding
	{
		BindingInt8( SQLSMALLINT type=SQL_TINYINT ): Binding{ type, SQL_C_TINYINT }{}
		BindingInt8( int8_t value ): Binding{ SQL_TINYINT, SQL_C_TINYINT },_data{value}{}
		void* Data()noexcept override{ return &_data; }
		object GetDataValue()const override{ return object{_data}; }
		α GetInt32()Ι->int32_t override{ return _data; }
		
		int64_t GetInt()const override{ return GetInt32(); }
		uint GetUInt()const override{ return static_cast<uint>(GetInt32()); }
		std::optional<_int> GetIntOpt()const{ std::optional<_int> value; if( !IsNull() )value=GetInt(); return value; }
		std::optional<uint> GetUIntOpt()const override{ std::optional<uint> optional; if( !IsNull() ) optional=GetUInt(); return optional; };
	private:
		int8_t _data;
	};
	struct BindingInt32 final : Binding
	{
		BindingInt32( SQLSMALLINT type=SQL_INTEGER ): Binding{ type, SQL_C_SLONG }{}
		BindingInt32( int value ): Binding{ SQL_INTEGER, SQL_C_SLONG },_data{value}{}
		void* Data()noexcept override{ return &_data; }
		object GetDataValue()const override{ return object{_data}; }
		α GetInt32()Ι->int32_t override{ return _data; }
		int64_t GetInt()const override{ return GetInt32(); }
		uint GetUInt()const override{ return static_cast<uint>(GetInt32()); }
		std::optional<_int> GetIntOpt()const{ std::optional<_int> value; if( !IsNull() )value=GetInt(); return value; }
		std::optional<uint> GetUIntOpt()const override{ std::optional<uint> optional; if( !IsNull() ) optional=GetUInt(); return optional; };
	private:
		int _data;
	};

	struct BindingDecimal final : Binding
	{};

	struct BindingInt final : Binding
	{
		BindingInt( SQLSMALLINT type=SQL_C_SBIGINT ): Binding{ type, SQL_C_SBIGINT }{}
		BindingInt( _int value ): Binding{ SQL_BIGINT, SQL_C_SBIGINT },_data{value}{}
		void* Data()noexcept override{ return &_data; }
		object GetDataValue()const override{ return object{_data}; }
		int64_t GetInt()const override{ return _data; }
		uint GetUInt()const override{ return static_cast<uint>( GetInt() ); }
		std::optional<uint> GetUIntOpt()const{ std::optional<uint> value; if(!IsNull())value=GetUInt(); return value; }
		virtual DBTimePoint GetDateTime()const{ return Clock::from_time_t(_data); }
	private:
		_int _data ;
	};

	struct BindingTimeStamp final:  Binding
	{
		BindingTimeStamp( SQLSMALLINT type=SQL_C_TYPE_TIMESTAMP ): Binding{ type, SQL_C_TYPE_TIMESTAMP }{}
		BindingTimeStamp( SQL_TIMESTAMP_STRUCT value ): Binding{ SQL_TYPE_TIMESTAMP, SQL_C_TYPE_TIMESTAMP },_data{value}{}
		void* Data()noexcept override{ return &_data; }
		object GetDataValue()const override{ return IsNull() ? object(nullptr) : object{GetDateTime()}; };
		DBTimePoint GetDateTime()const{ return IsNull() ? DBTimePoint{} : Jde::DateTime( _data.year, (uint8)_data.month, (uint8)_data.day, (uint8)_data.hour, (uint8)_data.minute, (uint8)_data.second, Duration(_data.fraction) ).GetTimePoint(); }
		std::optional<DBTimePoint> GetDateTimeOpt()const override{ return IsNull() ? std::nullopt : std::make_optional(GetDateTime()); }
	private:
		SQL_TIMESTAMP_STRUCT _data;
	};


	struct BindingUInt final : Binding
	{
		BindingUInt( SQLSMALLINT type ):	Binding{ type, SQL_C_UBIGINT }{}
		BindingUInt( uint value ): Binding{ SQL_INTEGER, SQL_C_SLONG },_data{value}{}
		void* Data()noexcept override{ return &_data; }
		object GetDataValue()const override{ return object{_data}; }
		uint GetUInt()const override{ return _data; }
	private:
		uint _data ;
	};

	struct BindingDateTime final : Binding
	{//{ SQL_DATETIME, SQL_C_TYPE_TIMESTAMP }
		BindingDateTime( SQLSMALLINT type=SQL_TYPE_TIMESTAMP ):Binding{ type, SQL_C_TIMESTAMP, sizeof(SQL_TIMESTAMP_STRUCT) }{}

		BindingDateTime( const optional<DBTimePoint>& value );

		void* Data()noexcept override{ return &_data; }
		object GetDataValue()const override{ return IsNull() ? object{nullptr} : object{GetDateTime()}; }
		SQLLEN BufferLength()Ι override{ return sizeof(_data); }
		//https://learn.microsoft.com/en-us/sql/odbc/reference/appendixes/column-size?view=sql-server-ver16&redirectedfrom=MSDN
		//https://wezfurlong.org/blog/2005/Nov/calling-sqlbindparameter-to-bind-sql-timestamp-struct-as-sql-c-type-timestamp-avoiding-a-datetime-overflow/
		SQLULEN Size()Ι override{ return 23; }//23 works with 0
		SQLSMALLINT DecimalDigits()Ι{ return 3; }//https://stackoverflow.com/questions/40918607/cannot-bind-a-sql-type-timestamp-value-using-odbc-with-ms-sql-server-hy104-inv
		DBTimePoint GetDateTime()const override
		{
			return IsNull() ? DBTimePoint() : Jde::DateTime( _data.year, (uint8)_data.month, (uint8)_data.day, (uint8)_data.hour, (uint8)_data.minute, (uint8)_data.second, Duration(_data.fraction) ).GetTimePoint();
		}
		std::optional<DBTimePoint> GetDateTimeOpt()const override
		{
			std::optional<DBTimePoint> value;
			if( !IsNull() )
				value = GetDateTime();
			return value;
		}
	//private:
		SQL_TIMESTAMP_STRUCT _data;
	};

	struct BindingDouble final : Binding
	{
		BindingDouble( SQLSMALLINT type=SQL_DOUBLE ):	Binding{ type, SQL_C_DOUBLE }{}
		BindingDouble( double value ): Binding{ SQL_DOUBLE, SQL_C_DOUBLE },_data{value}{}
		BindingDouble( const optional<double>& value ): Binding{ SQL_DOUBLE, SQL_C_DOUBLE },_data{value.has_value() ? value.value() : 0.0}{ if( !value.has_value() ) Output=SQL_NULL_DATA; }

		void* Data()noexcept override{ return &_data; }
		object GetDataValue()const override{ return object{_data}; }

		double GetDouble()const override{ return _data; }
		std::optional<double> GetDoubleOpt()const{ std::optional<double> value; if( !IsNull() ) value = GetDouble();	return value; }
	private:
		double _data;
	};
	struct BindingNumeric final : TBinding<SQL_NUMERIC_STRUCT,SQL_NUMERIC,SQL_C_NUMERIC>
	{
		α GetDataValue()const->object override{ return object{GetDouble()}; }
		α GetDouble()const->double override//https://docs.microsoft.com/en-us/sql/odbc/reference/appendixes/retrieve-numeric-data-sql-numeric-struct-kb222831?view=sql-server-ver15
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
		α GetDoubleOpt()const->std::optional<double> override{ std::optional<double> value; if( !IsNull() ) value = GetDouble(); return value; }
		α GetInt()const->_int override{ return (_int)GetDouble(); }
	};

	struct BindingFloat final : Binding
	{
		BindingFloat( SQLSMALLINT type=SQL_FLOAT ):	Binding{ type, SQL_C_FLOAT }{}
		BindingFloat( float value ): Binding{ SQL_FLOAT, SQL_C_FLOAT },_data{value}{}

		void* Data()noexcept override{ return &_data; }
		object GetDataValue()const override{ return object{_data}; }

		double GetDouble()const override{ return _data; }
		std::optional<double> GetDoubleOpt()const{ std::optional<double> value; if( !IsNull() ) value = GetDouble();	return value; }
	private:
		float _data;
	};

	struct BindingInt16 final : Binding
	{
		BindingInt16():Binding{ SQL_SMALLINT, SQL_C_SSHORT }{}
		BindingInt16( int16_t value ): Binding{ SQL_SMALLINT, SQL_C_SSHORT },_data{value}{}

		α Data()noexcept->void* override{ return &_data; }
		α GetDataValue()const->object override{ return Output==-1 ? object{nullptr} : object{GetInt()}; }
		α GetUInt()Ι->uint override{ return static_cast<uint>(_data); }
		α GetInt()Ι->_int override{ return static_cast<_int>(_data); }
		α GetIntOpt()Ι->optional<_int> override{ std::optional<_int> value; if( !IsNull() ) value = GetInt(); return value; }
		α GetDouble()const->double override{ return _data; }
		α GetDoubleOpt()const->std::optional<double>{ std::optional<double> value; if( !IsNull() ) value = GetDouble(); return value; }
	private:
		int16_t _data;
	};

	struct BindingUInt8 final : Binding
	{
		BindingUInt8():Binding{ SQL_TINYINT, SQL_C_TINYINT }{}
		BindingUInt8( uint8_t value ): Binding{ SQL_TINYINT, SQL_C_TINYINT },_data{value}{}

		void* Data()noexcept override{ return &_data; }
		object GetDataValue()const override{ return object{_data}; }

		α GetUInt()Ι->uint override{ return static_cast<uint>(_data); }
		std::optional<uint> GetUIntOpt()const{ std::optional<_int> value; if( !IsNull() ) value = GetUInt(); return value; }
		α GetInt32()Ι->int32_t override{ return static_cast<int32_t>(_data); }
		std::optional<_int> GetIntOpt()const override{ std::optional<_int> value; if( !IsNull() ) value = GetInt(); return value; }
		_int GetInt()const override{ return static_cast<_int>(_data); }
		double GetDouble()const override{ return _data; }
		std::optional<double> GetDoubleOpt()const{ std::optional<double> value; if( !IsNull() ) value = GetDouble();	return value; }
	private:
		uint8_t _data;
	};

	inline up<Binding> Binding::GetBinding( SQLSMALLINT type )noexcept(false)
	{
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
	inline up<Binding> Binding::Create( object parameter )
	{
		up<Binding> pBinding;
		switch( (EObject)parameter.index() )
		{
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
	inline BindingDateTime::BindingDateTime( const optional<DBTimePoint>& value ):
		BindingDateTime{}
	{
		if( !value.has_value() )
			Output = SQL_NULL_DATA;
		else
		{
			Jde::DateTime date( value.value() );
			_data.year = date.Year();
			_data.month = date.Month();
			_data.day = date.Day();
			_data.hour = date.Hour();
			_data.minute = date.Minute();
			_data.second = date.Second();
			_data.fraction = Chrono::MillisecondsSinceEpoch( date )%1000*1'000'000;
		}
	}
}