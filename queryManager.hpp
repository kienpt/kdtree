#ifndef QUERYMANAGER_HPP
#define QUERYMANAGER_HPP

#include "kdtree.hpp"

namespace QProcessor
{
	class QueryManager
	{
	private:
		KDTree *_kdtree;
	public:
		QueryManager(std::string csvfile);
		~QueryManager();
		void queryData();

	};

};

#endif // QUERYMANAGER_HPP
	
