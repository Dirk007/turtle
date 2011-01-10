//
//  Copyright Mathieu Champlon 2008
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef MOCK_PLACEHOLDER_HPP_INCLUDED
#define MOCK_PLACEHOLDER_HPP_INCLUDED

#include "format.hpp"

namespace mock
{
    template< typename Constraint >
    struct constraint
    {
        constraint( const Constraint& c )
            : f_( c )
        {}
        Constraint f_;
    };
}

#endif // #ifndef MOCK_PLACEHOLDER_HPP_INCLUDED
