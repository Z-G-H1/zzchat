#pragma once
#include "const.h"


class RedisMgr : public Singleton<RedisMgr>, public std::enable_shared_from_this<RedisMgr>
{

private:
	RedisMgr();

	redisContext* _reply;
};

