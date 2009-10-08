//
//  Copyright Mathieu Champlon 2008
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef MOCK_MOCK_HPP_INCLUDED
#define MOCK_MOCK_HPP_INCLUDED

#include "error.hpp"
#include "object.hpp"
#include "expectation.hpp"
#include "type_name.hpp"
#include <boost/function.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/inc.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/seq/push_back.hpp>
#include <boost/preprocessor/seq/pop_back.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/function_types/function_type.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/erase.hpp>
#include <boost/mpl/copy.hpp>
#include <boost/mpl/back_inserter.hpp>
#define BOOST_TYPEOF_SILENT
#include <boost/typeof/typeof.hpp>
#include <stdexcept>

namespace mock
{
namespace detail
{
    template< typename T >
    T& ref( T& t )
    {
        return t;
    }
    template< typename T >
    T& ref( T* t )
    {
        if( ! t )
            throw std::invalid_argument( "derefencing null pointer" );
        return *t;
    }
    template< typename T >
    T& ref( std::auto_ptr< T >& t )
    {
        if( ! t.get() )
            throw std::invalid_argument( "derefencing null pointer" );
        return *t;
    }
    template< typename T >
    T& ref( boost::shared_ptr< T >& t )
    {
        if( ! t.get() )
            throw std::invalid_argument( "derefencing null pointer" );
        return *t;
    }

    template< typename M >
    struct signature
    {
        typedef BOOST_DEDUCED_TYPENAME
            boost::function_types::result_type< M >::type result;
        typedef BOOST_DEDUCED_TYPENAME
            boost::function_types::parameter_types< M >::type parameters;
        typedef BOOST_DEDUCED_TYPENAME
            boost::function_types::function_type<
                BOOST_DEDUCED_TYPENAME boost::mpl::push_front<
                    BOOST_DEDUCED_TYPENAME boost::mpl::pop_front<
                        BOOST_DEDUCED_TYPENAME boost::mpl::copy<
                            parameters,
                            boost::mpl::back_inserter<
                                boost::mpl::vector<>
                            >
                        >::type
                    >::type,
                    result
                >::type
            >::type type;
    };

    template< typename E >
    void set_parent( E& e, const object& o )
    {
        o.set_parent( e );
    }
    template< typename E, typename T >
    void set_parent( E&, const T&,
        BOOST_DEDUCED_TYPENAME boost::disable_if<
            BOOST_DEDUCED_TYPENAME boost::is_base_of< object, T >::type
        >::type* = 0 )
    {}
    template< typename E >
    void tag( E& e, const object& o, const std::string& type_name,
              const std::string& name )
    {
        e.tag( type_name + o.tag() + "::" + name );
    }
    template< typename E, typename T >
    void tag( E& e, const T&, const std::string& type_name,
              const std::string& name,
        BOOST_DEDUCED_TYPENAME boost::disable_if<
            BOOST_DEDUCED_TYPENAME boost::is_base_of< object, T >::type
        >::type* = 0 )
    {
        e.tag( type_name + "::" + name );
    }

    template< typename E >
    E& configure( typename E::expectation_tag, const std::string& object,
                  const std::string& name, E& e )
    {
        e.tag( name == "_" ? object : name );
        return e;
    }
    template< typename E, typename T >
    E& configure( E& e, const std::string& /*object*/,
                  const std::string& name, const T& t )
    {
        set_parent( e, t );
        tag( e, t, type_name< T >(), name );
        return e;
    }

    template< typename T >
    struct base
    {
        typedef T base_type;
    };
}
}

#define MOCK_BASE_CLASS(T, I) \
    struct T : I, mock::object, mock::detail::base< I >
#define MOCK_CLASS(T) \
    struct T : mock::object
#define MOCK_FUNCTOR(S) \
    mock::expectation< S >

#define MOCK_MOCKER(o, t) \
    mock::detail::configure( mock::detail::ref( o ).exp##t, \
        BOOST_PP_STRINGIZE(o), BOOST_PP_STRINGIZE(t), mock::detail::ref( o ) )

#define MOCK_METHOD_ARG(z, n, arg) BOOST_PP_COMMA_IF(n) \
    BOOST_PP_CAT(BOOST_PP_CAT(arg, BOOST_PP_INC(n)),_type) \
    BOOST_PP_CAT(a, BOOST_PP_INC(n))
#define MOCK_METHOD_ARGS(n, arg) \
    BOOST_PP_REPEAT_FROM_TO(0, n, MOCK_METHOD_ARG, arg)
#define MOCK_MOCKER_ARG(z, n, d) \
    BOOST_PP_COMMA_IF(n) BOOST_PP_CAT(a, BOOST_PP_INC(n))
#define MOCK_MOCKER_ARGS(n) \
    BOOST_PP_REPEAT_FROM_TO(0, n, MOCK_MOCKER_ARG, BOOST_PP_EMPTY)
#define MOCK_METHOD_EXPECTATION(S, t) \
    mutable mock::expectation< S > exp##t;

#define MOCK_METHOD_STUB(M, n, S, t, c, tpn) \
    tpn boost::function< S >::result_type M( \
        MOCK_METHOD_ARGS(n, tpn boost::function< S >::arg) ) c \
    { \
        return MOCK_MOCKER(this, t)( MOCK_MOCKER_ARGS(n) ); \
    }
#define MOCK_SIGNATURE(M) \
    mock::detail::signature< BOOST_TYPEOF(&base_type::M) >::type
#define MOCK_SIGNATURE_TPL(M) \
    BOOST_DEDUCED_TYPENAME mock::detail::signature< BOOST_TYPEOF_TPL(&base_type::M) >::type

#define MOCK_METHOD_EXT(M, n, S, t) \
    MOCK_METHOD_STUB(M, n, S, t,,) \
    MOCK_METHOD_STUB(M, n, S, t, const,) \
    MOCK_METHOD_EXPECTATION(S, t)
#define MOCK_CONST_METHOD_EXT(M, n, S, t) \
    MOCK_METHOD_STUB(M, n, S, t, const,) \
    MOCK_METHOD_EXPECTATION(S, t)
#define MOCK_NON_CONST_METHOD_EXT(M, n, S, t) \
    MOCK_METHOD_STUB(M, n, S, t,,) \
    MOCK_METHOD_EXPECTATION(S, t)
#define MOCK_METHOD(M, n) \
    MOCK_METHOD_EXT(M, n, MOCK_SIGNATURE(M), M)

#define MOCK_METHOD_EXT_TPL(M, n, S, t) \
    MOCK_METHOD_STUB(M, n, S, t,, BOOST_DEDUCED_TYPENAME) \
    MOCK_METHOD_STUB(M, n, S, t, const, BOOST_DEDUCED_TYPENAME) \
    MOCK_METHOD_EXPECTATION(S, t)
#define MOCK_CONST_METHOD_EXT_TPL(M, n, S, t) \
    MOCK_METHOD_STUB(M, n, S, t, const, BOOST_DEDUCED_TYPENAME) \
    MOCK_METHOD_EXPECTATION(S, t)
#define MOCK_NON_CONST_METHOD_EXT_TPL(M, n, S, t) \
    MOCK_METHOD_STUB(M, n, S, t,, BOOST_DEDUCED_TYPENAME) \
    MOCK_METHOD_EXPECTATION(S, t)
#define MOCK_METHOD_TPL(M, n) \
    MOCK_METHOD_EXT_TPL(M, n, MOCK_SIGNATURE_TPL(M), M)

#define MOCK_EXPECT(o,t) MOCK_MOCKER(o,t).expect( __FILE__, __LINE__ )
#define MOCK_RESET(o,t) MOCK_MOCKER(o,t).reset()
#define MOCK_VERIFY(o,t) MOCK_MOCKER(o,t).verify()

#endif // #ifndef MOCK_MOCK_HPP_INCLUDED