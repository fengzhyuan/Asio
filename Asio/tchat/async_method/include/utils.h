/* 
 * File:   utils.h
 * Author: fengzh
 *
 * Created on September 11, 2015, 6:08 PM
 */

#ifndef ASIO_ASYNC_METHOD_INCLUDE_UTILS_H
#define	ASIO_ASYNC_METHOD_INCLUDE_UTILS_H

#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <deque>
#include <list>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>



using namespace std;
using namespace boost;
using namespace boost::asio;
using namespace boost::asio::ip;

namespace Color {
    enum Code {
        FG_RED      = 31,
        FG_GREEN    = 32,
        FG_BLUE     = 34,
        FG_DEFAULT  = 39,
        BG_RED      = 41,
        BG_GREEN    = 42,
        BG_BLUE     = 44,
        BG_DEFAULT  = 49
    };
    class Modifier {
        Code code;
    public:
        Modifier() : code(FG_DEFAULT) {}
        Modifier(Code pCode) : code(pCode) {}
        friend std::ostream&
        operator<<(std::ostream& os, const Modifier& mod) {
            return os << "\033[" << mod.code << "m";
        }
    };
}

#endif	/* ASIO_ASYNC_METHOD_INCLUDE_UTILS_H */

