//
// Created by tjp56 on 2022/1/30.
//

#ifndef TAODOCKER_CONTAINER_H
#define TAODOCKER_CONTAINER_H

#include <string>

class Container
{
public:
	explicit Container(std::string command);
	int Run();

private:
	std::string command_;
};


#endif //TAODOCKER_CONTAINER_H
