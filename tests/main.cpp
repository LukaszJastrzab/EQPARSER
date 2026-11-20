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
		parser.init( GetArgs(), m_sequation );

		for( T x{-2.0 }; x < 2.0; x+= 0.25 )
			for( T y{ -2.0 }; y < 2.0; y += 0.25 )
			{
				auto t1 = parser( { x, y, 0.0 } );
				auto t2 = m_fequation( x, y, 0.0 );

				EXPECT_TRUE( isfinite( t1 ) == isfinite( t2 ) );

				if( isfinite( t1 ) && isfinite( t2 ) )
					EXPECT_TRUE( abs( t1 - t2 ) < numeric_limits< T >::epsilon() );
			}
	}

	eq_parser< T > parser;
	std::function< T( T, T, T ) > m_fequation;
	wstring m_sequation;
};

using test_types = ::testing::Types< float, double >;

TYPED_TEST_SUITE( eq_parser_test, test_types );

TYPED_TEST( eq_parser_test, test_0 )
{
	using T = TypeParam;
	m_fequation = []( T x, T y, T z ) -> T { return -x / -y + y / x; };
	m_sequation = L"-x / -y + y / x";
}

TYPED_TEST( eq_parser_test, test_1 )
{
	using T = TypeParam;
	m_fequation = []( T x, T y, T z ) -> T { return -x + y * sin( x * y ) + cos( -( x + -y ) * 2 ); };
	m_sequation = L"-x + y * sin( x * y ) + cos( -( x + -y ) * 2 )";
}


