#ifndef NETWORKING_H
#define NETWORKING_H

#include <iostream>
#include <time.h>
#include <set>
#include <cfloat>
#include <SFML/Network.hpp>
#include <boost/asio.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/asio.hpp>

#include "GameLogic/hlt.h"

struct InitPackage 
{
	unsigned char playerTag;
	unsigned char ageOfSentient;
	hlt::Map map;

private:
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & playerTag;
		ar & ageOfSentient;
		ar & map;
	}
};

static sf::Packet& operator<<(sf::Packet& p, const hlt::Map& m)
{
    p << m.map_width << m.map_height;
    for(auto a = m.contents.begin(); a != m.contents.end(); a++) for(auto b = a->begin(); b != a->end(); b++) p << b->owner << b->age;
    return p;
}

template<class type>
static void sendObject(boost::asio::ip::tcp::socket &s, type sendingObject)
{
    boost::asio::streambuf buf;
    std::ostream os( &buf );
    boost::archive::text_oarchive ar( os );
    ar & sendingObject;
    
    const size_t header = buf.size();
    
    // send header and buffer using scatter
    std::vector<boost::asio::const_buffer> buffers;
    buffers.push_back( boost::asio::buffer(&header, sizeof(header)) );
    buffers.push_back( buf.data() );
    const size_t rc = boost::asio::write(s, buffers);
}

template<class type>
static void getObject(boost::asio::ip::tcp::socket &s, type &receivingObject)
{
    size_t header;
    boost::asio::read(s, boost::asio::buffer( &header, sizeof(header) ));
    
    boost::asio::streambuf buf;
    const size_t rc = boost::asio::read(s, buf.prepare( header ));
    buf.commit( header );
    
    std::istream is( &buf );
    boost::archive::text_iarchive ar( is );
    ar & receivingObject;
}

static double handleInitNetworking(boost::asio::ip::tcp::socket &s, unsigned char playerTag, unsigned char ageOfSentient, std::string name, hlt::Map& m)
{
    using boost::asio::ip::tcp;
    
    InitPackage package = {playerTag, ageOfSentient, m};
    sendObject(s, package);
    
    
    std::string str = "Init Message sent to player " + name + "\n";
    std::cout << str;
    
    std::string receiveString = "";
    
    clock_t initialTime = clock();
    getObject(s, receiveString);
    str = "Init Message received from player " + name + "\n";
    std::cout << str;
    clock_t finalTime = clock() - initialTime;
    double timeElapsed = float(finalTime) / CLOCKS_PER_SEC;
    
    if(receiveString != "Done") return FLT_MAX;
    return timeElapsed;
}

static double handleFrameNetworking(boost::asio::ip::tcp::socket &s, hlt::Map& m, std::set<hlt::Move> * moves)
{
    sendObject(s, m);
    
    moves->clear();
    clock_t initialTime = clock();
    getObject(s, *moves);
    clock_t finalTime = clock() - initialTime;
    double timeElapsed = float(finalTime) / CLOCKS_PER_SEC;
    
    return timeElapsed;
}

#endif