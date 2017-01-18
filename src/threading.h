#pragma once

#include "StdAfx.h"

namespace threading
{
	void init();
	void cinder_update_called();
	extern boost::mutex shared_data_mutex;
}