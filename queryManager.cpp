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
	//-73.75120067917915,40.71467824434192,2011-05-09T00:00:00
		Query q;
		q.setArea(40, -73.76, 50, -73.74);
		uint32_t t1 = KDTree::createTime("2010-04-09T00:00:00");
		uint32_t t2 = KDTree::createTime("2012-06-19T00:00:00");
		std::cout<<t1<<std::endl;
		std::cout<<t2<<std::endl;
		q.setTime(0, UINT_MAX);
		QueryResult qr = _kdtree->execute(q);
		std::cout<<"Done"<<std::endl;
		std::cout<<qr._points->size()<<std::endl;
	}
}
