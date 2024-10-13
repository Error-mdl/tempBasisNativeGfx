#pragma once
#include "api/ApiInterface.h"

class ApiNone : ApiInterface
{
public:
	void GfxEventInit() {}
	void GfxEventShutdown() {}
	~ApiNone() {}
};