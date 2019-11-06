#pragma once
#include "../../Framework/source/db/DataType.h"

namespace Jde::DB::Odbc
{
#pragma warning( push )
#pragma warning (disable: 4716)
	struct Binding
	{
		Binding( SQLSMALLINT type, SQLSMALLINT cType ):
			DBType{ type },
			CodeType{cType}
		{}
		static sp<Binding> GetBinding( SQLSMALLINT type );
		static sp<Binding> Create( DataValue parameter );
		
		virtual void* Data()noexcept=0;
		virtual DataValue GetDataValue()const=0;
		[[noreturn]] virtual bool GetBit()const{ THROW( DBException("{} not implemented for DBType={} CodeType={}", "bit", DBType, CodeType) ); }
		[[noreturn]] virtual const std::string to_string()const{ THROW( DBException("{} not implemented for DBType={} CodeType={}", "to_string", DBType, CodeType) ); }
		[[noreturn]] virtual int64_t GetInt()const{ THROW( DBException("{} not implemented for DBType={} CodeType={}", "GetInt", DBType, CodeType) ); }
		[[noreturn]] virtual int32_t GetInt32()const{ THROW( DBException("{} not implemented for DBType={} CodeType={}", "GetInt32", DBType, CodeType) ); }
		[[noreturn]] virtual std::optional<_int> GetIntOpt()const{ THROW( DBException("{} not implemented for DBType={} CodeType={}", "GetIntOpt", DBType, CodeType) ); }
		[[noreturn]] virtual double GetDouble()const{ THROW( DBException("{} not implemented for DBType={} CodeType={}", "GetDouble", DBType, CodeType) ); }
		virtual float GetFloat()const{ return static_cast<float>( GetDouble() ); }
		[[noreturn]] virtual std::optional<double> GetDoubleOpt()const{ THROW( DBException("{} not implemented for DBType={} CodeType={}", "GetDoubleOpt", DBType, CodeType) ); }
		[[noreturn]] virtual DBDateTime GetDateTime()const{ THROW( DBException("{} not implemented for DBType={} CodeType={}", "GetDateTime", DBType, CodeType) ); }
		[[noreturn]] virtual std::optional<DBDateTime> GetDateTimeOpt()const{ THROW( DBException("{} not implemented for DBType={} CodeType={}", "GetDateTimeOpt", DBType, CodeType) ); }
		[[noreturn]] virtual uint GetUInt()const{ THROW( DBException("{} not implemented for DBType={} CodeType={}", "GetUInt", DBType, CodeType) ); }
		virtual uint32_t GetUInt32(uint position )const{ return static_cast<uint32_t>(GetUInt()); }
		[[noreturn]] virtual std::optional<uint> GetUIntOpt()const{ THROW( DBException("{} not implemented for DBType={} CodeType={} - {}", "GetUIntOpt", DBType, CodeType, GetTypeName<decltype(this)>() ) ); };
		bool IsNull()const{ return Output==SQL_NULL_DATA; }
		virtual SQLULEN Size()const noexcept{return 0;}
		virtual SQLLEN BufferLength()const noexcept{return 0;}
		SQLSMALLINT DBType{0};
		SQLSMALLINT CodeType{0};
		SQLLEN Output{0};
	};
#pragma warning( pop )

	struct BindingNull : public Binding
	{
		BindingNull( SQLSMALLINT type=SQL_UNKNOWN_TYPE ):
			Binding{ type, SQL_C_CHAR }
		{}
		void* Data()noexcept override{ return nullptr; }
		DB::DataValue GetDataValue()const override{ return DataValue{nullptr}; }
	};

	struct BindingString : public Binding
	{
		BindingString( SQLSMALLINT type, SQLLEN size ):Binding{ type, SQL_C_CHAR }{ _buffer.reserve( size ); }
		BindingString( const string& value ):Binding{ SQL_VARCHAR, SQL_C_CHAR },_buffer( value.begin(), value.end() ){}
		BindingString( const string_view& value ):Binding{ SQL_VARCHAR, SQL_C_CHAR },_buffer( value.begin(), value.end() ){Output = value.size();}
		void* Data()noexcept override{ return _buffer.data(); }
		DB::DataValue GetDataValue()const override{ return DataValue{to_string()}; }
		const std::string to_string()const override{ return string( _buffer.data() ); }

		SQLLEN BufferLength()const noexcept override{return _buffer.size();}
		SQLULEN Size()const noexcept override{ return _buffer.size(); }
	private:
		vector<char> _buffer;
	};

	struct BindingBit : public Binding
	{
		BindingBit( SQLSMALLINT type ):	Binding{ type, SQL_C_BIT }{}
		BindingBit( bool value ): Binding{ SQL_CHAR, SQL_C_BIT },_data{value ? '\1' : '\0'}{}
		void* Data()noexcept override{ return &_data; }
		DataValue GetDataValue()const override{ return DataValue{_data!=0}; }
		bool GetBit()const override{ return _data!=0; }
	private:
		char _data;
	};

	struct BindingInt32 : public Binding
	{
		BindingInt32( SQLSMALLINT type=SQL_INTEGER ): Binding{ type, SQL_C_SLONG }{}
		BindingInt32( int value ): Binding{ SQL_INTEGER, SQL_C_SLONG },_data{value}{}
		void* Data()noexcept override{ return &_data; }
		DataValue GetDataValue()const override{ return DataValue{_data}; }
		int32_t GetInt32()const override{ return _data; }
		int64_t GetInt()const override{ return GetInt32(); }
		uint GetUInt()const override{ return static_cast<uint>(GetInt32()); }
		std::optional<_int> GetIntOpt()const{ std::optional<_int> value; if( !IsNull() )value=GetInt(); return value; }
		std::optional<uint> GetUIntOpt()const override{ std::optional<uint> optional; if( !IsNull() ) optional=GetUInt(); return optional; };
	private:
		int _data ;
	};
	
	struct BindingDecimal : public Binding
	{
	};

	struct BindingInt : public Binding
	{
		BindingInt( SQLSMALLINT type=SQL_C_SBIGINT ): Binding{ type, SQL_C_SBIGINT }{}
		BindingInt( _int value ): Binding{ SQL_INTEGER, SQL_C_SLONG },_data{value}{}
		void* Data()noexcept override{ return &_data; }
		DataValue GetDataValue()const override{ return DataValue{_data}; };
		int64_t GetInt()const override{ return _data; }
		uint GetUInt()const override{ return static_cast<uint>( GetInt() ); }
		std::optional<uint> GetUIntOpt()const{ std::optional<uint> value; if(!IsNull())value=GetUInt(); return value; };
		virtual DBDateTime GetDateTime()const{ return Clock::from_time_t(_data); }
	private:
		_int _data ;
	};

	struct BindingTimeStamp : public Binding
	{
		BindingTimeStamp( SQLSMALLINT type=SQL_C_TYPE_TIMESTAMP ): Binding{ type, SQL_C_TYPE_TIMESTAMP }{}
		BindingTimeStamp( SQL_TIMESTAMP_STRUCT value ): Binding{ SQL_TYPE_TIMESTAMP, SQL_C_TYPE_TIMESTAMP },_data{value}{}
		void* Data()noexcept override{ return &_data; }
		DataValue GetDataValue()const override{ return DataValue{GetDateTime()}; };
		//int64_t GetInt()const override{ return _data; }
		//uint GetUInt()const override{ return static_cast<uint>( GetInt() ); }
		//std::optional<uint> GetUIntOpt()const{ std::optional<uint> value; if(!IsNull())value=GetUInt(); return value; };
		virtual DBDateTime GetDateTime()const
		{  
			//const Duration duration = std::chrono::milliseconds( _data.fraction/std::numeric_limits<SQLUINTEGER>::max() );
			//return Jde::DateTime( (uint16)_data.year, (uint8)_data.month, (uint8)_data.day, (uint8)_data.hour, (uint8)_data.minute, (uint8)_data.second, duration ).GetTimePoint(); 
			return IsNull() ? DBDateTime() : Jde::DateTime( _data.year, (uint8)_data.month, (uint8)_data.day, (uint8)_data.hour, (uint8)_data.minute, (uint8)_data.second, Duration(_data.fraction) ).GetTimePoint();
		}
		std::optional<DBDateTime> GetDateTimeOpt()const override
		{ 
			std::optional<DBDateTime> value;
			if( !IsNull() )
				value = GetDateTime();
			return value;
		}

	private:
		SQL_TIMESTAMP_STRUCT _data;
	};

	
	struct BindingUInt : public Binding
	{
		BindingUInt( SQLSMALLINT type ):	Binding{ type, SQL_C_UBIGINT }{}
		BindingUInt( uint value ): Binding{ SQL_INTEGER, SQL_C_SLONG },_data{value}{}
		void* Data()noexcept override{ return &_data; }
		DataValue GetDataValue()const override{ return DataValue{_data}; }
		uint GetUInt()const override{ return _data; }
	private:
		uint _data ;
	};
	
	struct BindingDateTime : public Binding
	{
		BindingDateTime( SQLSMALLINT type=SQL_DATETIME ):Binding{ type, SQL_C_TYPE_TIMESTAMP }{}
		BindingDateTime( const optional<DBDateTime>& value );

		void* Data()noexcept override{ return &_data; }
		DataValue GetDataValue()const override{ return IsNull() ? DataValue{nullptr} : DataValue{GetDateTimeOpt()}; }
		DBDateTime GetDateTime()const override
		{ 
			return IsNull() ? DBDateTime() : Jde::DateTime( _data.year, (uint8)_data.month, (uint8)_data.day, (uint8)_data.hour, (uint8)_data.minute, (uint8)_data.second, Duration(_data.fraction) ).GetTimePoint();
		}
		std::optional<DBDateTime> GetDateTimeOpt()const override
		{ 
			std::optional<DBDateTime> value;
			if( !IsNull() )
				value = GetDateTime();
			return value;
		}
	private:
		SQL_TIMESTAMP_STRUCT _data;
	};

	struct BindingDouble : public Binding
	{
		BindingDouble( SQLSMALLINT type=SQL_DOUBLE ):	Binding{ type, SQL_C_DOUBLE }{}
		BindingDouble( double value ): Binding{ SQL_DOUBLE, SQL_C_DOUBLE },_data{value}{}
		BindingDouble( const Decimal2& value ): Binding{ SQL_DOUBLE, SQL_C_DOUBLE },_data{ (double)value }{}
		BindingDouble( const optional<double>& value ): Binding{ SQL_DOUBLE, SQL_C_DOUBLE },_data{value.has_value() ? value.value() : 0.0}{ if( !value.has_value() ) Output=SQL_NULL_DATA; }
		//BindingDouble( const Decimal2& value ): Binding{ SQL_DOUBLE, SQL_C_DOUBLE },_data{ (double)value }{}

		void* Data()noexcept override{ return &_data; }
		DataValue GetDataValue()const override{ return DataValue{_data}; }

		double GetDouble()const override{ return _data; }
		std::optional<double> GetDoubleOpt()const{ std::optional<double> value; if( !IsNull() ) value = GetDouble();	return value; }
	private:
		double _data;
	};

	struct BindingFloat : public Binding
	{
		BindingFloat( SQLSMALLINT type=SQL_FLOAT ):	Binding{ type, SQL_C_FLOAT }{}
		BindingFloat( float value ): Binding{ SQL_FLOAT, SQL_C_FLOAT },_data{value}{}
		//BindingFloat( const Decimal2& value ): Binding{ SQL_FLOAT, SQL_C_FLOAT },_data{ (double)value }{}
		//BindingFloat( const optional<double>& value ): Binding{ SQL_FLOAT, SQL_C_FLOAT },_data{value.has_value() ? value.value() : 0.0}{ if( !value.has_value() ) Output=SQL_NULL_DATA; }

		void* Data()noexcept override{ return &_data; }
		DataValue GetDataValue()const override{ return DataValue{_data}; }

		double GetDouble()const override{ return _data; }
		std::optional<double> GetDoubleOpt()const{ std::optional<double> value; if( !IsNull() ) value = GetDouble();	return value; }
	private:
		float _data;
	};

	struct BindingInt16 : public Binding
	{
		BindingInt16():Binding{ SQL_SMALLINT, SQL_C_SSHORT }{}
		BindingInt16( int16_t value ): Binding{ SQL_SMALLINT, SQL_C_SSHORT },_data{value}{}
		//BindingInt16( const Decimal2& value ): Binding{ SQL_SMALLINT, SQL_C_SSHORT },_data{ (double)value }{}
		//BindingInt16( const optional<double>& value ): Binding{ SQL_SMALLINT, SQL_C_SSHORT },_data{value.has_value() ? value.value() : 0.0}{ if( !value.has_value() ) Output=SQL_NULL_DATA; }

		void* Data()noexcept override{ return &_data; }
		DataValue GetDataValue()const override{ return DataValue{_data}; }
		uint GetUInt()const noexcept override{ return static_cast<uint>(_data); }
		double GetDouble()const override{ return _data; }
		std::optional<double> GetDoubleOpt()const{ std::optional<double> value; if( !IsNull() ) value = GetDouble();	return value; }
	private:
		int16_t _data;
	};

	struct BindingUInt8 : public Binding
	{
		BindingUInt8():Binding{ SQL_TINYINT, SQL_C_TINYINT }{}
		BindingUInt8( uint8_t value ): Binding{ SQL_TINYINT, SQL_C_TINYINT },_data{value}{}
		//BindingInt16( const Decimal2& value ): Binding{ SQL_TINYINT, SQL_C_SSHORT },_data{ (double)value }{}
		//BindingInt16( const optional<double>& value ): Binding{ SQL_TINYINT, SQL_C_SSHORT },_data{value.has_value() ? value.value() : 0.0}{ if( !value.has_value() ) Output=SQL_NULL_DATA; }

		void* Data()noexcept override{ return &_data; }
		DataValue GetDataValue()const override{ return DataValue{_data}; }

		uint GetUInt()const noexcept override{ return static_cast<uint>(_data); }
		double GetDouble()const override{ return _data; }
		std::optional<double> GetDoubleOpt()const{ std::optional<double> value; if( !IsNull() ) value = GetDouble();	return value; }
	private:
		uint8_t _data;
	};

	inline sp<Binding> Binding::GetBinding( SQLSMALLINT type )
	{
		sp<Binding> pBinding;
		switch( type )
		{
		case SQL_TINYINT:
			pBinding = make_shared<BindingUInt8>();
			break;
		case SQL_INTEGER:
			pBinding = make_shared<BindingInt32>( type );
			break;
		case SQL_DECIMAL:
			pBinding = make_shared<BindingDouble>( type );
		break;
		case SQL_SMALLINT:
			pBinding = make_shared<BindingInt16>();
			break;
		case SQL_FLOAT:
			pBinding = make_shared<BindingFloat>( type );
			break;
		case SQL_REAL:
			pBinding = make_shared<BindingDouble>( type );
			break;
		case SQL_DOUBLE:
			pBinding = make_shared<BindingDouble>( type );
			break;
		case SQL_DATETIME:
			pBinding = make_shared<BindingDateTime>( type);
			break;
		case SQL_BIGINT:
			pBinding = make_shared<BindingInt>( type );
			break;
		case SQL_TYPE_TIMESTAMP:
			pBinding = make_shared<BindingTimeStamp>( type );
			break;
			//SQL_C_UBIGINT
		//case SQL_UBIGINT:
		//	pBinding = make_shared<BindingType<uint64_t>>( type, SQL_C_UBIGINT );
		//	break;
		default:
			THROW( DBException("Type '{}' is not implemented.", type) );
		}
		return pBinding;
	}
	using std::get;
	inline sp<Binding> Binding::Create( DataValue parameter )
	{
		sp<Binding> pBinding;
		switch( (EDataValue)parameter.index() )
		{
		case EDataValue::Null:
			pBinding = make_shared<BindingNull>();
		break;
		case EDataValue::String:
			pBinding = make_shared<BindingString>( get<string>(parameter) );
		break;
		case EDataValue::StringView:
			pBinding = make_shared<BindingString>( get<string_view>(parameter) );
		break;
		case EDataValue::StringPtr:
			pBinding = make_shared<BindingString>( *get<sp<string>>(parameter) );
		break;
		case EDataValue::Bool:
			pBinding = make_shared<BindingBit>( get<bool>(parameter) );
		break;
		case EDataValue::Int:
			pBinding = make_shared<BindingInt32>( get<int>(parameter) );
		break;
		case EDataValue::Int64:
			pBinding = make_shared<BindingInt>( get<_int>(parameter) );
		break;
		case EDataValue::Uint:
			pBinding = make_shared<BindingUInt>( get<uint>(parameter) );
		break;
		case EDataValue::Decimal2:
			pBinding = make_shared<BindingDouble>( get<Decimal2>(parameter) );
		break;
		case EDataValue::Double:
			pBinding = make_shared<BindingDouble>( get<double>(parameter) );
		break;
		case EDataValue::DoubleOptional:
			pBinding = make_shared<BindingDouble>( get<optional<double>>(parameter) );
		break;
		case EDataValue::DateOptional:
			pBinding = make_shared<BindingDateTime>( get<optional<DBDateTime>>(parameter) );
		break;
		}
		return pBinding;
	}
	inline BindingDateTime::BindingDateTime( const optional<DBDateTime>& value ): Binding{ SQL_DATETIME, SQL_C_TYPE_TIMESTAMP }
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
			_data.fraction = date.Nanos();
		}
	}
}