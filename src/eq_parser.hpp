#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <regex>
#include <stdexcept>

class eq_parser
{
public:
	eq_parser() = default;
	eq_parser( std::vector< wchar_t > args, std::wstring equation );
	void init( std::vector< wchar_t > args, std::wstring equation );

private:

	std::vector< std::wstring > bracer( std::wstring& in );

	std::vector< wchar_t > m_args;
	std::wstring m_equation;
};


eq_parser::eq_parser( std::vector< wchar_t > args, std::wstring equation )
{
	init( args, equation );
}

void eq_parser::init( std::vector< wchar_t > args, std::wstring equation )
{
	m_args = args,
	m_equation = equation;

	m_equation = std::regex_replace( m_equation, std::wregex( L"\\s+" ), L"" );

	auto braces = bracer( m_equation );
}


std::vector< std::wstring > eq_parser::bracer( std::wstring& in )
{
	int open_counter{ 0 };
	std::vector< std::wstring > out;
	std::wstringstream brace_data;

	for( uint32_t i{ 0 }; i < in.size(); ++i )
	{
		if( in[ i ] == L')' )
		{
			--open_counter;

			if( open_counter == 0 )
			{
				out.push_back( brace_data.str() );
				brace_data.str( L"" );
				brace_data.clear();
			}
		}

		if( open_counter < 0 )
			throw std::runtime_error( "eq_parser::bracer - open_counter < 0" );

		if( open_counter > 0 )
			brace_data << in[ i ];

		if( in[ i ] == L'(' )
			++open_counter;
	}

	return out;
}