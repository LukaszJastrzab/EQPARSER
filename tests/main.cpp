#include <filesystem>
#include <gtest/gtest.h>

#include "eq_parser.hpp"

using namespace std;

template< typename T >
class eq_parser_test : public ::testing::Test
{
protected:

	vector< wchar_t > GetArgs()
	{
		return vector< wchar_t >({ L'x',L'y', L'z' });
	}

	void SetUp() override
	{
	}

	void TearDown() override
	{
		m_parser.init( GetArgs(), m_s_equation );

		for( T x{-5.0 }; x < 5.0; x += 0.25 )
			for( T y{ -5.0 }; y < 5.0; y += 0.25 )
			{
				auto t1 = m_parser( { x, y, 0.0 } );
				auto t2 = m_f_equation( x, y, 0.0 );

				EXPECT_TRUE( isfinite( t1 ) == isfinite( t2 ) );

				if( isfinite( t1 ) && isfinite( t2 ) )
					EXPECT_TRUE( abs( t1 - t2 ) < numeric_limits< T >::epsilon() );
			}
	}

	eq_parser< T > m_parser;
	std::function< T( T, T, T ) > m_f_equation;
	wstring m_s_equation;
};

using test_types = ::testing::Types< float, double >;

TYPED_TEST_SUITE( eq_parser_test, test_types );

TYPED_TEST( eq_parser_test, test_0 )
{
	using T = TypeParam;
	m_f_equation = []( T x, T y, T z ) -> T { return -x / -y + y / x; };
	m_s_equation = L"-x / -y + y / x";
}

TYPED_TEST( eq_parser_test, test_1 )
{
	using T = TypeParam;
	m_f_equation = []( T x, T y, T z ) -> T { return -x + y * sin( x * y ) + cos( -( x + -y ) * 2 ); };
	m_s_equation = L"-x + y * sin( x * y ) + cos( -( x + -y ) * 2 )";
}


