#include <filesystem>
#include <gtest/gtest.h>

#include "eq_parser.hpp"

using namespace std;

class eq_parser_test : public ::testing::Test
{
protected:

	void SetUp() override
	{
	}

	void TearDown() override
	{
		parser.init( args, equation );
		EXPECT_TRUE( true );
	}

	eq_parser< double > parser;
	vector< wchar_t > args;
	wstring equation;
};

TEST_F( eq_parser_test, test_1 )
{
	args = { L'x',L'y' };
	equation = L"x+y* sin( x * y ) + ((x +y ) * 2)";
}
