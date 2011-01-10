//
//  Copyright Mathieu Champlon 2008
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef MOCK_CONSTRAINTS_HPP_INCLUDED
#define MOCK_CONSTRAINTS_HPP_INCLUDED

#include "constraint.hpp"
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/preprocessor/stringize.hpp>

namespace mock
{
#define MOCK_CONSTRAINT(N,Expr) \
    namespace detail \
    { \
        struct N \
        { \
            template< typename Actual > \
            bool operator()( const Actual& actual ) const \
            { \
                (void)actual; \
                return Expr; \
            } \
            friend std::ostream& operator<<( std::ostream& os, const N& ) \
            { \
                return os << BOOST_STRINGIZE( N ); \
            } \
        }; \
    } \
    template<> \
    struct constraint< detail::N > \
    { \
        constraint() \
        {} \
        detail::N f_; \
    }; \
    const constraint< detail::N > N;

    MOCK_CONSTRAINT( any, true )
    MOCK_CONSTRAINT( negate, ! actual )
    MOCK_CONSTRAINT( evaluate, actual() )

#undef MOCK_CONSTRAINT

#define MOCK_CONSTRAINT(N,Expr) \
    namespace detail \
    { \
        template< typename Expected > \
        struct N \
        { \
            explicit N( const Expected& expected ) \
                : expected_( expected ) \
            {} \
            template< typename Actual > \
            bool operator()( const Actual& actual ) const \
            { \
                return Expr; \
            } \
            friend std::ostream& operator<<( std::ostream& os, const N& n ) \
            { \
                return os << BOOST_STRINGIZE( N ) << "( " << mock::format( n.expected_ ) << " )"; \
            } \
            Expected expected_; \
        }; \
    } \
    template< typename T > \
    constraint< detail::N< T > > N( T t ) \
    { \
        return detail::N< T >( t ); \
    }

    MOCK_CONSTRAINT( equal, actual == expected_ )
    MOCK_CONSTRAINT( less, actual < expected_ )
    MOCK_CONSTRAINT( greater, actual > expected_ )
    MOCK_CONSTRAINT( less_equal, actual <= expected_ )
    MOCK_CONSTRAINT( greater_equal, actual >= expected_ )
    MOCK_CONSTRAINT( contain, boost::algorithm::contains( actual, expected_ ) )

namespace detail
{
    template< typename Expected >
    struct same
    {
        explicit same( const Expected& expected )
            : expected_( &expected )
        {}
        template< typename Actual >
        bool operator()( const Actual& actual ) const
        {
            return &actual == expected_;
        }
        friend std::ostream& operator<<( std::ostream& os, const same& s )
        {
            return os << "same( " << mock::format( *s.expected_ ) << " )";
        }
        const Expected* expected_;
    };

    template< typename Expected >
    struct assign
    {
        explicit assign( const Expected& expected )
            : expected_( expected )
        {}
        template< typename Actual >
        bool operator()( Actual& actual,
            BOOST_DEDUCED_TYPENAME boost::disable_if<
                boost::is_convertible< Expected*, Actual >, Actual >::type* = 0 ) const
        {
            actual = expected_;
            return true;
        }
        template< typename Actual >
        bool operator()( Actual* actual,
            BOOST_DEDUCED_TYPENAME boost::enable_if<
                boost::is_convertible< Expected, Actual >, Actual >::type* = 0 ) const
        {
            *actual = expected_;
            return true;
        }
        friend std::ostream& operator<<( std::ostream& os, const assign& a )
        {
            return os << "assign( " << mock::format( a.expected_ ) << " )";
        }
        Expected expected_;
    };

    template< typename Expected >
    struct retrieve
    {
        explicit retrieve( Expected& expected )
            : expected_( &expected )
        {}
        template< typename Actual >
        bool operator()( const Actual& actual,
            BOOST_DEDUCED_TYPENAME boost::disable_if<
                boost::is_convertible< const Actual*, Expected >, Actual >::type* = 0 ) const
        {
            *expected_ = actual;
            return true;
        }
        template< typename Actual >
        bool operator()( Actual& actual,
            BOOST_DEDUCED_TYPENAME boost::enable_if<
                boost::is_convertible< Actual*, Expected >, Actual >::type* = 0 ) const
        {
            *expected_ = &actual;
            return true;
        }
        friend std::ostream& operator<<( std::ostream& os, const retrieve& r )
        {
            return os << "retrieve( " << mock::format( *r.expected_ ) << " )";
        }
        Expected* expected_;
    };
}

    template< typename T >
    constraint< detail::same< T > > same( T& t )
    {
        return detail::same< T >( t );
    }
    template< typename T >
    constraint< detail::retrieve< T > > retrieve( T& t )
    {
        return detail::retrieve< T >( t );
    }
    template< typename T >
    constraint< detail::assign< T > > assign( T t )
    {
        return detail::assign< T >( t );
    }
}

#endif // #ifndef MOCK_CONSTRAINTS_HPP_INCLUDED
