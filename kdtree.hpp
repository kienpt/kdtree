#ifndef KDTREE_HPP
#define KDTREE_HPP

#include <time.h>
#include <limits.h>
#include <float.h>
#include <fstream>
#include <boost/iostreams/device/mapped_file.hpp>

#include "struct.hpp"

namespace QProcessor
{

	class KDTree
	{
		public:

			//Init
			KDTree(std::string csvfile)
			{
				//build kdtree here
			}

			inline static uint64_t createTime(int year, int month, int day, int hour, int min, int sec)
			{
				struct tm timeinfo;
				memset(&timeinfo, 0, sizeof(timeinfo));
				timeinfo.tm_year = year-1900;
				timeinfo.tm_mon  = month-1;
				timeinfo.tm_mday = day;
				timeinfo.tm_hour = hour;
				timeinfo.tm_min  = min;
				timeinfo.tm_sec  = sec;
				timeinfo.tm_isdst = -1;
				return mktime(&timeinfo);
			}


			Iterator begin()
			{
				const KdNode *nodes = this->_nodes;
				while (nodes<this->_endNode && nodes->_childNode!=0) nodes++;
				return Iterator(nodes, this->_endNode);
			}

			Iterator end()
			{
				return Iterator(this->_endNode, this->_endNode);
			}



			QueryResult execute(const Query &q) 
			{
				uint32_t range[3][2] = {
					{q._minTime, q._maxTime},
					{float2uint(q._minLon), float2uint(q._maxLon)},
					{float2uint(q._minLat), float2uint(q._maxLat)},
				};
				QueryResult result;
				result._points = PPointVector(new PointVector());
				searchKdTree(_nodes, 0, range, 0, q, result);
				return result;
			}


		private:
			boost::iostreams::mapped_file_source _fTree;
			const KdNode * _nodes;
			const KdNode * _endNode;


			void searchKdTree(const KdNode *nodes, uint32_t root, uint32_t range[7][2], int depth, const Query &query, QueryResult &result) 
			{
				const KdNode * pNode = nodes + root;
				if (pNode->_childNode==-1) return;
				if (pNode->_childNode==0) {
					const Point *candidate = reinterpret_cast<const Point*>(&(pNode->_medianValue));
					if (query.isMatched(candidate))
						result._points->push_back(candidate);
					return;
				}
				int rangeIndex = depth%3;
				uint32_t median = pNode->_medianValue;
				if (range[rangeIndex][0]<=median)
					searchKdTree(nodes, pNode->_childNode, range, depth+1, query, result);
				if (range[rangeIndex][1]>median) {
					int nextNode = pNode->_childNode+1;
					if (nodes[pNode->_childNode]._childNode==0)
						nextNode+=3;
					searchKdTree(nodes, nextNode, range, depth+1, query, result);
				}
			}

			uint32_t float2uint(float f) 
			{
				register uint32_t t(*((uint32_t*)&f));
				return t ^ ((-(t >> 31)) | 0x80000000);
			}


	};
};
#endif
