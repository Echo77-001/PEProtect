#pragma once
#include<string>

#include"BasicPE.h"

class PEBuilder : public BasicPE {

public:

	PEBuilder();

	void createHeaders();

	void destroyHeaders();
	
	void write(const std::string& fileName);

};