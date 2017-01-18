#include "StdAfx.h"
#include "threading.h"
#include "app.h"
#include "profiling.h"
#include "getOpt.h"
#include "render.h"

using namespace render;

namespace threading
{
	static shared_ptr<boost::thread> update_thread;
	//static boost::condition_variable committed_data_available;
	static bool committed_data_available = false;
	static boost::unique_lock<boost::mutex> lock;
	static boost::mutex mutex;
	boost::mutex shared_data_mutex;

	void startUpdateThread();

	void init()
	{
		lock = boost::unique_lock<boost::mutex>(mutex);
		startUpdateThread();
	}
	
	static void commitData()
	{
		boost::lock_guard<boost::mutex> lock(shared_data_mutex);
		std::copy(bloomSurface.begin(), bloomSurface.end(), bloomSurface_committed.begin());
		std::copy(toDraw.begin(), toDraw.end(), toDraw_committed.begin());
	}
	
	void startUpdateThread()
	{
		//return;
		update_thread = make_shared<boost::thread>([&]{
			//lock.lock();
			while(true)
			{
				updateApp();
				commitData();
				//committed_data_available.notify_all();
				committed_data_available = true;
			}
		});
	}

	void cinder_update_called()
	{
		PFL(renderWaitForUpdate)
			while(!committed_data_available)
				boost::this_thread::sleep(boost::posix_time::milliseconds(1));
		//updateApp(); commitData();
		//PFL(renderWaitForUpdate)c
		//	committed_data_available.wait(lock);
		PFL(gl) {
			renderApp();
			GetOpt::render();
		}
		committed_data_available = false;
	}
}