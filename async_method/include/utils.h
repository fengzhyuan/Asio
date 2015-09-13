/* 
 * File:   utils.h
 * Author: fengzh
 *
 * Created on September 11, 2015, 6:08 PM
 */

#ifndef UTILS_H
#define	UTILS_H

#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <deque>
#include <list>
#include <string>

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace boost;
using namespace boost::asio;
using namespace boost::asio::ip;
using boost::asio::ip::tcp;

///* client side */
//typedef boost::shared_ptr<tcp::socket> socket_ptr;
//typedef boost::shared_ptr<string> string_ptr;
//typedef boost::shared_ptr< queue<string_ptr> > msgQueueClient_ptr;
//
///* server side */
//typedef map<socket_ptr, string_ptr> clientMap;
//typedef boost::shared_ptr<clientMap> clientMap_ptr;
//typedef boost::shared_ptr< list<socket_ptr> > clientList_ptr;
//typedef boost::shared_ptr< queue<clientMap_ptr> > msgQueueServer_ptr;


#endif	/* UTILS_H */

