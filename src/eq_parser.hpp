#pragma once

#include <vector>
#include <map>
#include <string>
#include <memory>
#include <sstream>
#include <regex>
#include <stdexcept>
#include <cmath>

template< typename T >
class functor;

template< typename T >
class eq_parser
{
public:
	eq_parser() = default;
	eq_parser( std::vector< wchar_t > args, std::wstring equation );
	void init( std::vector< wchar_t > args, std::wstring equation );

private:
	std::wstring get_new_phrase();

	template< typename F >
	bool handle_function( std::wstring& in, const std::wstring& name, const std::wstring& arg );

	std::shared_ptr< functor< T > > m_functor;

	std::vector< std::wstring > bracer( std::wstring& in );

	std::vector< wchar_t > m_args;
	std::wstring m_equation;

	int m_id_item{ 0 };
	const std::wstring m_id_prefix{ L"fpfx" };
	std::map< int, std::shared_ptr< functor< T > > > m_items;
};

template< typename T >
class functor
{
public:
	virtual T operator()() = 0;
};

template< typename T >
class f_mult : public functor< T >
{
public:
	virtual T operator()() override { return ( *arg1 )() * ( *arg2() ); };
	std::shared_ptr< functor< T > > arg1, arg2;
};

template< typename T >
class f_add : public functor< T >
{
public:
	virtual T operator()() override { return  ( *arg1 )() + ( *arg2() ); };
	std::shared_ptr< functor< T > > arg1, arg2;
};

template< typename T >
class f_brac : public functor< T >
{
public:
	virtual T operator()() override { return  ( *arg )(); };
	std::shared_ptr< functor< T > > arg;
};

template< typename T >
class f_sin : public functor< T >
{
public:
	virtual T operator()() override { return std::sin( ( *arg )() ); };
	std::shared_ptr< functor< T > > arg;
};


template< typename T >
eq_parser< T >::eq_parser( std::vector< wchar_t > args, std::wstring equation )
{
	init( args, equation );
}

template< typename T >
void eq_parser< T >::init( std::vector< wchar_t > args, std::wstring equation )
{
	m_args = args,
	m_equation = equation;

	m_equation = std::regex_replace( m_equation, std::wregex( L"\\s+" ), L"" );

	auto braces = bracer( m_equation );
}

template< typename T >
std::wstring eq_parser< T >::get_new_phrase()
{
	return m_id_prefix + std::to_wstring( m_id_item );
}

template< typename T >
template< typename F >
bool eq_parser< T >::handle_function( std::wstring& in, const std::wstring& name, const std::wstring& arg )
{
	std::wstringstream new_item;
	new_item << name << L"(" << arg << L")";

	size_t pos = in.find( new_item.str() );

	if( pos == std::wstring::npos )
		return false;

	in.replace( pos, new_item.str().length(), get_new_phrase() );

	m_items[ m_id_item ] = std::make_shared< F >();

	++m_id_item;

	return true;
}

template< typename T >
std::vector< std::wstring > eq_parser< T >::bracer( std::wstring& in )
{
	int open_counter{ 0 };
	std::vector< std::wstring > out;
	std::wstringstream brace_data;

	for( uint32_t i{ 0 }; i < in.size(); ++i )
	{
		if( in[ i ] == L')' && --open_counter == 0 )
		{
			out.push_back( brace_data.str() );
			brace_data.str( L"" );
			brace_data.clear();
		}

		if( open_counter < 0 )
			throw std::runtime_error( "eq_parser::bracer - open_counter < 0" );

		if( open_counter > 0 )
			brace_data << in[ i ];

		if( in[ i ] == L'(' )
			++open_counter;
	}

	for( auto& item : out )
	{
		if( handle_function< f_sin< T > >( in, L"sin", item ) )
			continue;

		if( !handle_function< f_brac< T > >( in, L"", item ) )
			throw std::runtime_error( "eq_parser::bracer - brac failed" );
	}

	return out;
}