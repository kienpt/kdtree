#ifndef KDTREE_HPP
#define KDTREE_HPP

#include <time.h>
#include <limits.h>
#include <float.h>
#include <fstream>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "radix.h"
#include "struct.hpp"

namespace QProcessor
{

	class KDTree
	{
		public:
			inline uint32_t createTime(std::string mytime)
			{
				/*
				std::vector<std::string> strs;
				boost::split(strs,mytime,boost::is_any_of("/"));
				int month = boost::lexical_cast<int>(strs[0]);
				int day = boost::lexical_cast<int>(strs[1]);
				int year = boost::lexical_cast<int>(strs[2]);
				
				//Default
				int hour = 12;
				int min = 12;
				int sec = 12;
				*/
				std::vector<std::string> strs;
                                boost::split(strs,mytime,boost::is_any_of("T"));			
				std::vector<std::string> date;
				boost::split(date,strs[0],boost::is_any_of("-"));
				std::vector<std::string> time;
				boost::split(time,strs[1],boost::is_any_of(":"));

				int month = boost::lexical_cast<int>(date[1]);
                                int day = boost::lexical_cast<int>(date[2]);
                                int year = boost::lexical_cast<int>(date[0]);

				int hour = boost::lexical_cast<int>(time[0]);
                                int min = boost::lexical_cast<int>(time[1]);
                                int sec = boost::lexical_cast<int>(time[2]);
				
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
			};


			inline uint32_t getUKey(const Point &point, int keyIndex)
			{
				switch (keyIndex) 
				{
					case 0:
						return point._id;
					case 1:
						return float2uint(point._lat);
					case 2:
						return float2uint(point._lon);
					case 3:
						return point._time;
					default:
						return 0;
				}
			};

			KDTree(std::string csvfile);
			QueryResult execute(const Query &q); 
			Iterator begin();
			Iterator end();
			void buildKdTree(KdNode *nodes, uint32_t *tmp, Point *points, uint64_t n, int depth, uint64_t thisNode, uint64_t &freeNode);
			bool createKdTree(std::string csvfile);
			bool csv2binary(std::string csvIn, std::string binOut);
		private:
			void searchKdTree(const KdNode *nodes, uint32_t root, uint32_t range[7][2], int depth, const Query &query, QueryResult &result);
			uint32_t float2uint(float f) ;

			boost::iostreams::mapped_file_source _fTree;
			const KdNode * _nodes;
			const KdNode * _endNode;
	};
};
#endif
