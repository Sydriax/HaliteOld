#include "Random.h"

Random::Random()
{
	srand(time(NULL));
	my_tag = getTag();
	std::cout << "Got tag\n";
	boost::interprocess::message_queue *queue = setupInitialNetworking(my_tag);
	std::cout << "Got queue\n";
	getInit(my_tag, age_of_sentient, present_map);
	std::cout << "Got initial response\n";
	sendInitResponse(queue);
	std::cout << "Sent initial response\n";
}

void Random::run()
{
	while(true)
	{
		moves.clear();
		getFrame(my_tag, present_map);
		for(unsigned short a = 0; a < present_map.map_height; a++) for(unsigned short b = 0; b < present_map.map_width; b++) if(present_map.contents[a][b].owner == my_tag && present_map.contents[a][b].age == age_of_sentient) moves.insert({ { b, a }, rand() % 5 });
		sendFrame(my_tag, moves);
	}
}