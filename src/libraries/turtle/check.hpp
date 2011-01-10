//
//  Copyright Mathieu Champlon 2008
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef MOCK_CHECK_HPP_INCLUDED
#define MOCK_CHECK_HPP_INCLUDED

#include "is_functor.hpp"
#include "constraints.hpp"
#include "operators.hpp"
#include "format.hpp"
#include <boost/function.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/concept/assert.hpp>
#include <boost/concept_check.hpp>
#include <stdexcept>
#include <ostream>

namespace mock
{
namespace detail
{
    template< typename Functor, typename Actual >
    struct FunctorCompatible : private boost::noncopyable
    {
    public:
        BOOST_CONCEPT_USAGE( FunctorCompatible )
        {
            boost::require_boolean_expr(
                // if an error is generated by the line below it means an argument
                // passed to 'with' was of the wrong type.
                functor_accepts( actual_argument_type )
            );
        }
    private:
        FunctorCompatible( int ) {}
        Functor functor_accepts;
        Actual actual_argument_type;
    };

    template< typename Expected, typename Actual >
    struct EqualityComparable : private boost::noncopyable
    {
    public:
        BOOST_CONCEPT_USAGE( EqualityComparable )
        {
            boost::require_boolean_expr(
                // if an error is generated by the line below it means an argument
                // passed to 'with' was of the wrong type.
                actual_argument_type == expected_argument_type
            );
        }
    private:
        EqualityComparable( int ) {}
        Expected expected_argument_type;
        Actual actual_argument_type;
    };

    template< typename Actual >
    class check
    {
    public:
        template< typename Functor >
        explicit check( const Functor& f,
            BOOST_DEDUCED_TYPENAME boost::enable_if<
                    BOOST_DEDUCED_TYPENAME detail::is_functor< Functor >
            >::type* = 0 )
            : desc_( mock::format( f ) )
        {
            BOOST_CONCEPT_ASSERT(( FunctorCompatible< Functor, Actual > ));
            f_ = f;
            if( ! f_ )
                std::invalid_argument( "invalid constraint" );
        }
        template< typename Expected >
        explicit check( const Expected& expected,
            BOOST_DEDUCED_TYPENAME boost::disable_if<
                    BOOST_DEDUCED_TYPENAME detail::is_functor< Expected >
            >::type* = 0 )
            : desc_( mock::format( expected ) )
        {
            BOOST_CONCEPT_ASSERT(( EqualityComparable< Expected, Actual > ));
            f_ = mock::equal( expected ).f_;
            if( ! f_ )
                std::invalid_argument( "invalid constraint" );
        }
        template< typename Functor >
        explicit check( const constraint< Functor >& ph )
            : desc_( mock::format( ph.f_ ) )
        {
            BOOST_CONCEPT_ASSERT(( FunctorCompatible< Functor, Actual > ));
            f_ = ph.f_;
            if( ! f_ )
                std::invalid_argument( "invalid constraint" );
        }

        bool operator()( Actual actual ) const
        {
            return f_( actual );
        }

        friend std::ostream& operator<<( std::ostream& s, const check& c )
        {
            return s << c.desc_;
        }

    private:
        boost::function< bool( Actual ) > f_;
        std::string desc_;
    };
}
}

#endif // #ifndef MOCK_CHECK_HPP_INCLUDED
