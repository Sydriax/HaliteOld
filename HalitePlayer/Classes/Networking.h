#ifndef NETWORKING_H
#define NETWORKING_H

#include <time.h>
#include <set>
#include <iostream>
#include <cstdlib>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>

#include "hlt.h"

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

template<class type>
static void sendObject(boost::asio::ip::tcp::socket *s, type sendingObject)
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
    s->write_some(buffers);
}

template<class type>
static void getObject(boost::asio::ip::tcp::socket *s, type &receivingObject)
{
    size_t header;
    s->read_some(boost::asio::buffer( &header, sizeof(header)));
    
    boost::asio::streambuf buf;
    s->read_some(buf.prepare( header ));
    buf.commit( header );
    
    std::istream is( &buf );
    boost::archive::text_iarchive ar( is );
    ar & receivingObject;
}

static boost::asio::ip::tcp::socket * connectToGame()
{
    using boost::asio::ip::tcp;
    
    while(true)
    {
        std::string in;
        unsigned short portNumber;
        std::cout << "What port would you like to connect to? Please enter a valid port number: ";
        while(true)
        {
            std::getline(std::cin, in);
            std::transform(in.begin(), in.end(), in.begin(), ::tolower);
            try
            {
                portNumber = std::stoi(in);
                break;
            }
            catch(std::exception e)
            {
                std::cout << "That isn't a valid input. Please enter a valid port number: ";
            }
        }
        boost::asio::io_service io_service;
        
        tcp::resolver resolver(io_service);
        tcp::resolver::query query(tcp::v4(), "localhost", "");
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
        
        tcp::socket *socket = new tcp::socket(io_service);
        boost::system::error_code error = boost::asio::error::host_not_found;
        boost::asio::connect(*socket, endpoint_iterator);
        
        if (error) {
            std::cout << "There was a problem connecting. Let's try again: \n";
        } else {
            std::cout << "Successfully established contact with " << socket->remote_endpoint().address().to_string() << ".\n";
            return socket;
        }
        
    }
}


static void getInit(boost::asio::ip::tcp::socket *s, unsigned char& playerTag, unsigned char& ageOfSentient, hlt::Map& m)
{
    InitPackage package;
    getObject(s, package);
    
    playerTag = package.playerTag;
    ageOfSentient = package.ageOfSentient;
    m = package.map;
    
    std::cout << "Received init message.\n";
}

static void sendInitResponse(boost::asio::ip::tcp::socket *s)
{
    std::string response = "Done";
    sendObject(s, response);
    std::cout << "Sent init response.\n";
}

static void getFrame(boost::asio::ip::tcp::socket *s, hlt::Map& m)
{
    getObject(s, m);
}

static void sendFrame(boost::asio::ip::tcp::socket *s, const std::set<hlt::Move>& moves)
{
    sendObject(s, moves);
}

#endif