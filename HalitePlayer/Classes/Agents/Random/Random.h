#ifndef RANDOM_H
#define RANDOM_H

#include <time.h>
#include <set>

#include "Networking.h"
#include "hlt.h"

class Random
{
private:
	unsigned char my_tag;
	unsigned char age_of_sentient;
	sf::TcpSocket * connection;
	hlt::Map present_map;
	std::set<hlt::Move> moves;
public:
	Random();
	void run();
};

#endif