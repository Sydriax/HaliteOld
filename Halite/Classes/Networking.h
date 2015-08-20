#ifndef NETWORKING_H
#define NETWORKING_H

#include <SFML\Network.hpp>
#include <time.h>
#include <set>
#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <exception>
#include <cstdlib> 

#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/string.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/containers/set.hpp>
#include <boost/interprocess/allocators/allocator.hpp>

#include "GameLogic\hlt.h"

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

const std::string confirmation = "Done";
static unsigned short mapSize = 0;
static unsigned short moveSize = 0;

typedef boost::interprocess::allocator<hlt::Move, boost::interprocess::managed_shared_memory::segment_manager>  MoveAllocator;
typedef boost::interprocess::set<hlt::Move, std::less<hlt::Move>, MoveAllocator> MoveSet;

typedef boost::interprocess::allocator<void, boost::interprocess::managed_shared_memory::segment_manager> VoidAllocator;
typedef boost::interprocess::allocator<hlt::Site, boost::interprocess::managed_shared_memory::segment_manager>  SiteAllocator;
typedef boost::interprocess::vector<hlt::Site, SiteAllocator> SiteVector;
typedef boost::interprocess::allocator<SiteVector, boost::interprocess::managed_shared_memory::segment_manager>  SiteVectorAllocator;
typedef boost::interprocess::vector<SiteVector, SiteVectorAllocator> MapContents;

static void setupMemory(unsigned char playerTag, boost::interprocess::managed_shared_memory *&mapSegment, boost::interprocess::managed_shared_memory *&movesSegment) {
	mapSegment = new boost::interprocess::managed_shared_memory(boost::interprocess::open_or_create, "map" + (short)playerTag, 65536);
	movesSegment = new boost::interprocess::managed_shared_memory(boost::interprocess::open_or_create, "moves" + (short)playerTag, 65536);
}

template<class type>
static void sendObject(boost::interprocess::message_queue &queue, type objectToBeSent)
{
	std::ostringstream archiveStream;
	boost::archive::text_oarchive archive(archiveStream);
	archive & objectToBeSent;
	std::string serializedString(archiveStream.str());

	queue.send(serializedString.data(), serializedString.size(), 0);
}

template<class type>
static unsigned short getMaxSize(type object)
{
	std::ostringstream archiveStream;
	boost::archive::text_oarchive archive(archiveStream);
	archive & object;
	std::string serializedString(archiveStream.str());

	return serializedString.size();
}

static void sendSize(unsigned char playerTag, unsigned short size) 
{
	std::string initialQueueName = "size" + (short)playerTag;
	boost::interprocess::message_queue sizeQueue(boost::interprocess::open_or_create, initialQueueName.c_str(), 1, sizeof(unsigned short));
	sizeQueue.send(&size, sizeof(size), 0);
}

static unsigned short getSize(unsigned char playerTag)
{
	std::string initialQueueName = "playersize" + (short)playerTag;
	unsigned int priority;
	unsigned int size;
	boost::interprocess::message_queue::size_type recvd_size;
	boost::interprocess::message_queue sizeQueue(boost::interprocess::open_or_create, initialQueueName.c_str(), 1, sizeof(unsigned short));
	sizeQueue.receive(&size, sizeof(size), recvd_size, priority);
	return size;
}

static double handleInitNetworking(unsigned char playerTag, unsigned char ageOfSentient, std::string name, hlt::Map& m)
{
	hlt::Move exampleMove = { { USHRT_MAX, USHRT_MAX }, UCHAR_MAX };
	moveSize = getMaxSize(exampleMove);

	hlt::Map exampleMap(m);
	for(int a = 0; a < exampleMap.contents.size(); a++) for(int b = 0; b < exampleMap.contents[a].size(); b++) exampleMap.contents[a][b] = { UCHAR_MAX, UCHAR_MAX };
	mapSize = getMaxSize(exampleMap);

	InitPackage package = { playerTag, ageOfSentient, m };
	unsigned int packageSize = getMaxSize(package);

	sendSize(playerTag, mapSize);
	sendSize(playerTag, packageSize);

	// Send Init package
	std::string initialQueueName = "initpackage" + (short)playerTag;
	std::cout << "max: " << packageSize << "\n";
	std::cout << "m: " << m.contents.size() << "\n";
	std::cout << "pack: " << package.map.contents.size() << "\n";
	boost::interprocess::message_queue packageQueue = boost::interprocess::message_queue(boost::interprocess::open_or_create, initialQueueName.c_str(), 1, packageSize);
	sendObject(packageQueue, package);

	// Receive confirmation
	std::string stringQueueName = "initstring" + (short)playerTag;
	boost::interprocess::message_queue stringQueue(boost::interprocess::open_or_create, stringQueueName.c_str(), 1, confirmation.size());

	boost::interprocess::message_queue::size_type messageSize;
	unsigned int priority;
	std::string stringQueueString;
	stringQueueString.resize(getMaxSize(confirmation));

	clock_t initialTime = clock();
	stringQueue.receive(&stringQueueString[0], stringQueueString.size(), messageSize, priority);
	stringQueueString.resize(messageSize);
	std::cout << "Init Message received from player " + name + "\n";
	clock_t finalTime = clock() - initialTime;
	double timeElapsed = float(finalTime) / CLOCKS_PER_SEC;

	boost::interprocess::message_queue::remove("initpackage" + (short)playerTag);
	boost::interprocess::message_queue::remove("initstring" + (short)playerTag);

	if(stringQueueString != confirmation) return FLT_MAX;
	return timeElapsed;
}

static double handleFrameNetworking(unsigned char playerTag, boost::interprocess::managed_shared_memory *mapSegment, boost::interprocess::managed_shared_memory *movesSegment, hlt::Map &m, std::set<hlt::Move> * moves)
{
	// Sending Map
	VoidAllocator allocator(mapSegment->get_segment_manager());
	MapContents *mapContents = mapSegment->find<MapContents>("map").first;
	if(!mapContents) 
	{
		mapContents = mapSegment->construct<MapContents>("map")(allocator);
	}

	mapContents->clear();
	for(int a = 0; a < m.map_height; a++) {
		mapContents->push_back(SiteVector(m.contents[a].begin(), m.contents[a].end(), allocator));
	}

	sendSize(playerTag, 1);

	// Receiving moves
	moves->clear();

	clock_t initialTime = clock();
	getSize(playerTag);
	clock_t finalTime = clock() - initialTime;
	double timeElapsed = float(finalTime) / CLOCKS_PER_SEC;

	MoveSet mySet = *(movesSegment->find<MoveSet>("moves").first);
	*moves = std::set<hlt::Move>(mySet.begin(), mySet.end());
	return timeElapsed;
}

#endif