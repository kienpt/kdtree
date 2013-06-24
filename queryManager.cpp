#include "queryManager.hpp"

namespace QProcessor
{
	QueryManager::QueryManager(std::string csvfile)
	{
		_kdtree = new KDTree(csvfile);
	}

	QueryManager::~QueryManager()
	{
	}

	void QueryManager::queryData()
	{
	}
}
