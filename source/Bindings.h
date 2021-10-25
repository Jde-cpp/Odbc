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
		Ω Create( DataValue parameter )->up<Binding>;
		
		β Data()noexcept->void* = 0;
		β GetDataValue()const->DataValue=0;
		[[noreturn]] virtual bool GetBit()const{ THROWX( DBException("{} not implemented for DBType={} CodeType={}", "bit", DBType, CodeType) ); }
		[[noreturn]] virtual string to_string()const{ THROWX( DBException("to_string not implemented for DBType='{}' CodeType='{}' {}", DBType, CodeType, GetTypeName<decltype(this)>()) ); }
		[[noreturn]] virtual int64_t GetInt()const{ THROWX( DBException("{} not implemented for DBType={} CodeType={}", "GetInt", DBType, CodeType) ); }
		[[noreturn]] virtual int32_t GetInt32()const{ THROWX( DBException("{} not implemented for DBType={} CodeType={}", "GetInt32", DBType, CodeType) ); }
		[[noreturn]] virtual std::optional<_int> GetIntOpt()const{ THROWX( DBException("{} not implemented for DBType={} CodeType={} {}", "GetIntOpt", DBType, CodeType, GetTypeName<decltype(this)>()) ); }
		[[noreturn]] virtual double GetDouble()const{ THROWX( DBException("{} not implemented for DBType={} CodeType={}", "GetDouble", DBType, CodeType) ); }
		virtual float GetFloat()const{ return static_cast<float>( GetDouble() ); }
		[[noreturn]] β GetDoubleOpt()const->std::optional<double>{ THROWX( DBException("{} not implemented for DBType={} CodeType={}", "GetDoubleOpt", DBType, CodeType) ); }
		[[noreturn]] β GetDateTime()const->DBDateTime{ THROWX( DBException("{} not implemented for DBType={} CodeType={}", "GetDateTime", DBType, CodeType) ); }
		[[noreturn]] β GetDateTimeOpt()const->std::optional<DBDateTime>{ THROWX( DBException("{} not implemented for DBType={} CodeType={}", "GetDateTimeOpt", DBType, CodeType) ); }
		[[noreturn]] virtual uint GetUInt()const{ THROWX( DBException("{} not implemented for DBType={} CodeType={}", "GetUInt", DBType, CodeType) ); }
		virtual uint32_t GetUInt32(uint position )const{ return static_cast<uint32_t>(GetUInt()); }
		[[noreturn]] virtual std::optional<uint> GetUIntOpt()const{ THROWX( DBException("{} not implemented for DBType={} CodeType={} - {}", "GetUIntOpt", DBType, CodeType, GetTypeName<decltype(this)>() ) ); };
		bool IsNull()const{ return Output==SQL_NULL_DATA; }
		virtual SQLULEN Size()const noexcept{return 0;}
		virtual SQLSMALLINT DecimalDigits()const noexcept{return 0;}
		virtual SQLLEN BufferLength()const noexcept{return 0;}
		SQLSMALLINT DBType{0};
		SQLSMALLINT CodeType{0};
		SQLLEN Output;
	};
#pragma warning( pop )
	template <class T, SQLSMALLINT TSql, SQLSMALLINT TC>
	struct TBinding : Binding
	{
		TBinding()noexcept:Binding{TSql,TC}{}
		void* Data()noexcept override{ return &_data; }
	protected:
		SQL_NUMERIC_STRUCT _data;
	};


	struct BindingNull : public Binding
	{
		BindingNull( SQLSMALLINT type=SQL_VARCHAR ):
			Binding{ type, SQL_C_CHAR, SQL_NULL_DATA }
		{}
		void* Data()noexcept override{ return nullptr; }
		DB::DataValue GetDataValue()const override{ return DataValue{nullptr}; }
	};

	struct BindingString final: public Binding
	{
		BindingString( SQLSMALLINT type, SQLLEN size ):Binding{ type, SQL_C_CHAR, size }{ _buffer.reserve( size ); }
		BindingString( str value ):Binding{ SQL_VARCHAR, SQL_C_CHAR, value.size() },_buffer( value.begin(), value.end() ){}
		BindingString( sv value ):Binding{ SQL_VARCHAR, SQL_C_CHAR, value.size() },_buffer( value.begin(), value.end() ){}
		α Data()noexcept->void* override{ return _buffer.data(); }
		α GetDataValue()const->DB::DataValue override{ return Output==-1 ? DB::DataValue{} : DB::DataValue{ to_string() }; }
		α to_string()const->string override{ return Output==-1 ? string{} : string{ _buffer.data(), _buffer.data()+Output }; }

		α BufferLength()const noexcept->SQLLEN override{return _buffer.size();}
		α Size()const noexcept->SQLULEN override{ return _buffer.size(); }
	private:
		vector<char> _buffer;
	};

	struct BindingBit : public Binding
	{
		BindingBit():BindingBit{ (SQLSMALLINT)SQL_BIT }{}//-7
		BindingBit( SQLSMALLINT type ):Binding{ type, SQL_C_BIT }{}
		BindingBit( bool value ): Binding{ SQL_CHAR, SQL_C_BIT },_data{value ? '\1' : '\0'}{}
		void* Data()noexcept override{ return &_data; }
		DataValue GetDataValue()const override{ return DataValue{_data!=0}; }
		bool GetBit()const override{ return _data!=0; }
		int64_t GetInt()const override{ return static_cast<int64_t>(_data); }
		string to_string()const override{ return _data ? "true" : "false"; }
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
		int _data;
	};
	
	struct BindingDecimal : public Binding
	{};

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

	struct BindingTimeStamp final: public Binding
	{
		BindingTimeStamp( SQLSMALLINT type=SQL_C_TYPE_TIMESTAMP ): Binding{ type, SQL_C_TYPE_TIMESTAMP }{}
		BindingTimeStamp( SQL_TIMESTAMP_STRUCT value ): Binding{ SQL_TYPE_TIMESTAMP, SQL_C_TYPE_TIMESTAMP },_data{value}{}
		void* Data()noexcept override{ return &_data; }
		DataValue GetDataValue()const override{ return IsNull() ? DataValue(nullptr) : DataValue{GetDateTime()}; };
		DBDateTime GetDateTime()const{ return IsNull() ? DBDateTime{} : Jde::DateTime( _data.year, (uint8)_data.month, (uint8)_data.day, (uint8)_data.hour, (uint8)_data.minute, (uint8)_data.second, Duration(_data.fraction) ).GetTimePoint(); }
		std::optional<DBDateTime> GetDateTimeOpt()const override{ return IsNull() ? std::nullopt : std::make_optional(GetDateTime()); }
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
	{//{ SQL_DATETIME, SQL_C_TYPE_TIMESTAMP }
		BindingDateTime( SQLSMALLINT type=SQL_TYPE_TIMESTAMP ):Binding{ type, SQL_C_TIMESTAMP, sizeof(SQL_TIMESTAMP_STRUCT) }{}
		
		BindingDateTime( const optional<DBDateTime>& value );

		void* Data()noexcept override{ return &_data; }
		DataValue GetDataValue()const override{ return IsNull() ? DataValue{nullptr} : DataValue{GetDateTimeOpt()}; }
		SQLLEN BufferLength()const noexcept override{ return sizeof(_data); }
		SQLULEN Size()const noexcept override{ return 27; }//https://wezfurlong.org/blog/2005/Nov/calling-sqlbindparameter-to-bind-sql-timestamp-struct-as-sql-c-type-timestamp-avoiding-a-datetime-overflow/
		SQLSMALLINT DecimalDigits()const noexcept{return 7;}
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
	//private:
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
	struct BindingNumeric : public TBinding<SQL_NUMERIC_STRUCT,SQL_NUMERIC,SQL_C_NUMERIC>
	{
		α GetDataValue()const->DataValue override{ return DataValue{GetDouble()}; }
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

		α Data()noexcept->void* override{ return &_data; }
		α GetDataValue()const->DataValue override{ return Output==-1 ? DataValue{nullptr} : DataValue{GetInt()}; }
		α GetUInt()const noexcept->uint override{ return static_cast<uint>(_data); }
		α GetInt()const noexcept->_int override{ return static_cast<_int>(_data); }
		α GetIntOpt()const noexcept->optional<_int> override{ std::optional<_int> value; if( !IsNull() ) value = GetInt(); return value; }
		α GetDouble()const->double override{ return _data; }
		α GetDoubleOpt()const->std::optional<double>{ std::optional<double> value; if( !IsNull() ) value = GetDouble(); return value; }
	private:
		int16_t _data;
	};

	struct BindingUInt8 : public Binding
	{
		BindingUInt8():Binding{ SQL_TINYINT, SQL_C_TINYINT }{}
		BindingUInt8( uint8_t value ): Binding{ SQL_TINYINT, SQL_C_TINYINT },_data{value}{}

		void* Data()noexcept override{ return &_data; }
		DataValue GetDataValue()const override{ return DataValue{_data}; }

		uint GetUInt()const noexcept override{ return static_cast<uint>(_data); }
		std::optional<uint> GetUIntOpt()const{ std::optional<_int> value; if( !IsNull() ) value = GetUInt(); return value; }
		std::optional<_int> GetIntOpt()const override{ std::optional<_int> value; if( !IsNull() ) value = GetInt(); return value; }
		_int GetInt()const override{ return static_cast<uint>(_data); }
		double GetDouble()const override{ return _data; }
		std::optional<double> GetDoubleOpt()const{ std::optional<double> value; if( !IsNull() ) value = GetDouble();	return value; }
	private:
		uint8_t _data;
	};

	inline up<Binding> Binding::GetBinding( SQLSMALLINT type )noexcept(false)
	{
		up<Binding> pBinding;
		if( type==SQL_BIT )
			pBinding = make_unique<BindingBit>();
		else if( type==SQL_TINYINT )
			pBinding = make_unique<BindingUInt8>();
		else if( type==SQL_INTEGER )
			pBinding = make_unique<BindingInt32>( type );
		else if( type==SQL_DECIMAL )
			pBinding = make_unique<BindingDouble>( type );
		else if( type==SQL_SMALLINT )
			pBinding = make_unique<BindingInt16>();
		else if( type==SQL_FLOAT )
			pBinding = make_unique<BindingFloat>( type );
		else if( type==SQL_REAL )
			pBinding = make_unique<BindingDouble>( type );
		else if( type==SQL_DOUBLE )
			pBinding = make_unique<BindingDouble>( type );
		else if( type==SQL_DATETIME )
			pBinding = make_unique<BindingDateTime>( type );
		else if( type==SQL_BIGINT )
			pBinding = make_unique<BindingInt>( type );
		else if( type==SQL_TYPE_TIMESTAMP )
			pBinding = make_unique<BindingTimeStamp>( type );
		else if( type==SQL_BIT )
			pBinding = make_unique<BindingBit>();
		else if( type==SQL_NUMERIC )
			pBinding = make_unique<BindingNumeric>();
		else
			THROWX( DBException("Binding type '{}' is not implemented.", type) );
		return pBinding;
	}
	using std::get;
	inline up<Binding> Binding::Create( DataValue parameter )
	{
		up<Binding> pBinding;
		switch( (EDataValue)parameter.index() )
		{
		case EDataValue::Null:
			pBinding = make_unique<BindingNull>();
		break;
		case EDataValue::String:
			pBinding = make_unique<BindingString>( get<string>(parameter) );
		break;
		case EDataValue::StringView:
			pBinding = make_unique<BindingString>( get<sv>(parameter) );
		break;
		case EDataValue::StringPtr:
			pBinding = make_unique<BindingString>( *get<sp<string>>(parameter) );
		break;
		case EDataValue::Bool:
			pBinding = make_unique<BindingBit>( get<bool>(parameter) );
		break;
		case EDataValue::Int:
			pBinding = make_unique<BindingInt32>( get<int>(parameter) );
		break;
		case EDataValue::Int64:
			pBinding = make_unique<BindingInt>( get<_int>(parameter) );
		break;
		case EDataValue::Uint:
			pBinding = make_unique<BindingInt>( (_int)get<uint>(parameter) );
		break;
		case EDataValue::Decimal2:
			pBinding = make_unique<BindingDouble>( get<Decimal2>(parameter) );
		break;
		case EDataValue::Double:
			pBinding = make_unique<BindingDouble>( get<double>(parameter) );
		break;
		case EDataValue::DoubleOptional:
			pBinding = make_unique<BindingDouble>( get<optional<double>>(parameter) );
		break;
		case EDataValue::DateOptional:
			pBinding = make_unique<BindingDateTime>( get<optional<DBDateTime>>(parameter) );
		break;
		}
		return pBinding;
	}
	inline BindingDateTime::BindingDateTime( const optional<DBDateTime>& value ): 
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
			//_data.fraction = date.Nanos();
			//_data.fraction = (date.Nanos()+500)/1000*1000;//7 digits - [22008] [Microsoft][ODBC Driver 17 for SQL Server]Datetime field overflow. Fractional second precision exceeds the scale specified in the parameter binding. 
			_data.fraction = 0;
		}
	}
}