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
	eq_parser();
	eq_parser( std::vector< wchar_t > args, std::wstring equation );
	void init( std::vector< wchar_t > args, std::wstring equation );

	T operator()( const std::vector< T >& vars );

private:
	std::wstring get_new_phrase();

	template< typename F >
	bool handle_function( std::wstring& in, const std::wstring& name, std::wstring& arg );
	std::shared_ptr< functor< T > > handle_item( const std::wstring& in );

	void bracer( std::wstring& in );
	bool algebrer( std::wstring& in );

private:
	std::shared_ptr< functor< T > > m_functor;

	std::vector< wchar_t > m_args;
	std::wstring m_equation;

	int m_id_item{ 0 };
	const std::wstring m_id_prefix{ L"fpfx" };
	const std::wregex m_id_prefix_rgx;
	std::map< int, std::shared_ptr< functor< T > > > m_items;
};

template< typename T >
class functor
{
public:
	virtual T operator()( const std::vector< T >& vars ) = 0;
	std::shared_ptr< functor< T > > arg1;
};

template< typename T >
class functor2 : public functor< T >
{
public:
	std::shared_ptr< functor< T > > arg2;
};

template< typename T >
class f_const final : public functor< T >
{
public:
	f_const( T _value ) : value( _value ) {}
	T operator()( const std::vector< T >& vars ) override { return value; };
	T value{};
};

template< typename T >
class f_var final : public functor< T >
{
public:
	f_var( uint8_t _var_idx ) : var_idx( _var_idx ) {}
	T operator()( const std::vector< T >& vars ) override { return vars[ var_idx ]; };
	uint8_t var_idx{};
};

template< typename T >
class f_mult final : public functor2< T >
{
public:
	T operator()( const std::vector< T >& vars ) override { return ( *arg1 )( vars ) * ( *arg2 )( vars ); };
};

template< typename T >
class f_div final : public functor2< T >
{
public:
	T operator()( const std::vector< T >& vars ) override { return ( *arg1 )( vars ) / ( *arg2 )( vars ); };
};

template< typename T >
class f_add final : public functor2< T >
{
public:
	T operator()( const std::vector< T >& vars ) override { return  ( *arg1 )( vars ) + ( *arg2 )( vars ); };
};

template< typename T >
class f_sub final : public functor2< T >
{
public:
	T operator()( const std::vector< T >& vars ) override { return  ( *arg1 )( vars ) - ( *arg2 )( vars ); };
};

template< typename T >
class f_brac final : public functor< T >
{
public:
	T operator()( const std::vector< T >& vars ) override { return  ( *arg1 )( vars ); };
};

template< typename T >
class f_sin final : public functor< T >
{
public:
	T operator()( const std::vector< T >& vars ) override { return std::sin( ( *arg1 )( vars ) ); };
};

template< typename T >
eq_parser< T >::eq_parser( )
	: m_id_prefix_rgx( m_id_prefix + LR"(([0-9]+))" )
{
}

template< typename T >
eq_parser< T >::eq_parser( std::vector< wchar_t > args, std::wstring equation )
	: eq_parser()
{
	init( args, equation );
}

template< typename T >
void eq_parser< T >::init( std::vector< wchar_t > args, std::wstring equation )
{
	m_args = args,

	equation = std::regex_replace( equation, std::wregex( L"\\s+" ), L"" );

	m_equation = equation;

	bracer( equation );
	while( algebrer( equation ) ){};
	m_functor = handle_item( equation );
}

template< typename T >
T eq_parser< T >::operator()( const std::vector< T >& vars )
{
	return ( *m_functor )( vars );
}

template< typename T >
std::wstring eq_parser< T >::get_new_phrase()
{
	return m_id_prefix + std::to_wstring( m_id_item );
}

template< typename T >
template< typename F >
bool eq_parser< T >::handle_function( std::wstring& in, const std::wstring& name, std::wstring& arg )
{
	std::wstringstream new_item;
	new_item << name << L"(" << arg << L")";

	size_t pos = in.find( new_item.str() );

	if( pos == std::wstring::npos )
		return false;

	in.replace( pos, new_item.str().length(), get_new_phrase() );

	m_items[ m_id_item ] = std::make_shared< F >();

	auto& f_functor = m_items[ m_id_item ];

	++m_id_item;

	bracer( arg );
	while( algebrer( arg ) ) {};
	f_functor->arg1 = handle_item( arg );

	return true;
}

template< typename T >
void eq_parser< T >::bracer( std::wstring& in )
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
}

static const std::wregex number_rgx( LR"(([0-9]+(?:\.[0-9]+)?))" );

template< typename T >
std::shared_ptr< functor< T > > eq_parser< T >::handle_item( const std::wstring& in )
{
	std::wsmatch match;

	if( std::regex_search( in, match, m_id_prefix_rgx ) )
	{
		auto functorId = std::stoi( match[ 1 ].str() );
		return m_items.find( functorId )->second;
	}

	if( std::regex_search( in, match, number_rgx ) )
	{
		auto const_val = static_cast< T >( std::stod( match[ 1 ].str() ) );
		return std::make_shared< f_const< T > >( const_val );
	}

	for( uint8_t idx{ 0 }; idx < m_args.size(); ++idx )
		if( in.size() == 1 && in[ 0 ] == m_args[ idx ] )
			return std::make_shared< f_var< T > >( idx );

	throw std::runtime_error( "eq_parser< T >::handle_item - no match" );
}

static const std::wregex multdiv_rgx(
	LR"(([a-zA-Z0-9_]+(?:\.[0-9]+)?)([*/])([a-zA-Z0-9_]+(?:\.[0-9]+)?))"
);
static const std::wregex addsub_rgx(
	LR"(([a-zA-Z0-9_]+(?:\.[0-9]+)?)([+-])([a-zA-Z0-9_]+(?:\.[0-9]+)?))"
);

template< typename T >
bool eq_parser< T >::algebrer( std::wstring& in )
{
	bool operation_found{ false };
	std::wsmatch match;
	std::shared_ptr< functor2< T > > alg_op;

	if( std::regex_search( in, match, multdiv_rgx ) )
	{
		if( match[ 2 ].str() == L"*" )
			alg_op = std::make_shared< f_mult< T > >();
		else
			alg_op = std::make_shared< f_div< T > >();

		alg_op->arg1 = handle_item( match[ 1 ].str() );
		alg_op->arg2 = handle_item( match[ 3 ].str() );

		operation_found = true;
	}
	else if ( std::regex_search( in, match, addsub_rgx ) )
	{
		if( match[ 2 ].str() == L"+" )
			alg_op = std::make_shared< f_add< T > >();
		else
			alg_op = std::make_shared< f_sub< T > >();

		alg_op->arg1 = handle_item( match[ 1 ].str() );
		alg_op->arg2 = handle_item( match[ 3 ].str() );

		operation_found = true;
	}

	if( operation_found )
	{
		size_t pos = in.find( match[ 0 ].str() );

		if( pos == std::wstring::npos )
			throw std::runtime_error( "eq_parser< T >::algebrer - smth went wrong" );

		in.replace( pos, match[ 0 ].str().length(), get_new_phrase() );

		m_items[ m_id_item ] = alg_op;

		++m_id_item;

		return true;
	}

	return false;
}