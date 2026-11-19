#include <filesystem>
#include <gtest/gtest.h>

#include "eq_parser.hpp"

using namespace std;

class eq_parser_test : public ::testing::Test
{
protected:

	vector< wchar_t > GetArgs()
	{
		return vector< wchar_t >({ L'x',L'y', L'z' });
	}

	wstring GetEquation()
	{
		return wstring( L"-x+ y* sin( x * y ) + cos(-(x +-y ) * 2)" );
	}

	double Equation( double x, double y, double z )
	{
		return -x + y * sin( x * y ) + cos( -( x +-y ) * 2 );
	}

	void SetUp() override
	{
	}

	void TearDown() override
	{
		parser.init( GetArgs(), GetEquation() );

		auto t1 = parser( { 1.2, 2.4 } );
		auto t2 = Equation( 1.2, 2.4, 0.0 );

		EXPECT_TRUE( true );
	}

	eq_parser< double > parser;
	vector< wchar_t > args;
	wstring equation;
};

TEST_F( eq_parser_test, test_1 )
{
}
