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
//trips -> points
			void buildKdTree(KdNode *nodes, uint32_t *tmp, Point *points, uint64_t n, int depth, uint64_t thisNode, uint64_t &freeNode) {
				KdNode * nextNode = nodes + thisNode;
				if (n<2) {
					static uint64_t cnt = 0;
					cnt++;
					if (cnt%500000==0)
						fprintf(stderr, "\r%llu", cnt);
					nextNode->_childNode = 0;
					*reinterpret_cast<Point *>(&(nextNode->medianValue)) = * points;
					return;
				}
				int keyIndex = depth%3;
				size_t medianIndex = n/2-1;
				uint32_t median = getUKey(points[medianIndex], keyIndex);
				if (depth!=0) {
					for (size_t i=0; i<n; i++)
						tmp[i] = getUKey(points[i], keyIndex);
					sortArray(tmp, n);
					median = tmp[n/2-1];
					int64_t l = 0;
					int64_t r = n-1;
					while (l<r) {
						while (l<n && getUKey(points[l], keyIndex)<=median) l++;
						while (r>=0 && getUKey(points[r], keyIndex)>median) r--;
						if (l<r)
							SWAP(Point, points[l], points[r]);
					}
					medianIndex = r;
					if (medianIndex==n-1)
						medianIndex = n/2-1;
				}
				nextNode->_medianValue = median;
				nextNode->_childNode = freeNode;
				freeNode += 2 + ((uint64_t)(medianIndex+1<2))*3 + ((uint64_t)((n-medianIndex-1<2)&&(n-medianIndex-1>0)))*3;
				buildKdTree(nodes, tmp, points, medianIndex+1, depth+1, nextNode->_childNode, freeNode);
				if (medianIndex<n-1)
					buildKdTree(nodes, tmp, trips + medianIndex+1, n-medianIndex-1, depth+1,
							nextNode->_childNode+1+((uint64_t)(medianIndex+1<2))*3, freeNode);
				else
					nodes[nextNode->_childNode+1]._childNode = -1;
			}

			void createKdTree(std::string csvfile) {
				boost::iostreams::mapped_file mfile = csv2binary(csvfile);
				uint64_t n = mfile.size()/sizeof(Point);
				Point *points = (Point*)mfile.const_data();
				//KdNode *nodes = (KdNode*)malloc(sizeof(KdNode)*(4*n+n*3/2));
				KdNode *nodes = (KdNode*)malloc(sizeof(KdNode)*(6*n));
				uint32_t *tmp = (uint32_t*)malloc(sizeof(uint32_t)*n);

				uint64_t freeNode = 1;
				buildKdTree(nodes, tmp, trips, n, 0, 0, freeNode);

				// Writing new indices file

				fprintf(stderr, "\rWriting %llu nodes to %s\n", freeNode, argv[2]);
				FILE *fo = fopen(argv[2], "wb");
				fwrite(nodes, sizeof(KdNode), freeNode, fo);
				fclose(fo);

				mfile.close();
				free(nodes);
				free(tmp);
			}

			boost::iostreams::mapped_file csv2binary(std::string csvIn, std::string binOut int* indice)
			{
				boost::iostreams::mapped_file mfile;
				FILE *fo = fopen(binOut, "wb");
				std::ifstream  in(csvIn);
				std::string row;

				std::string id;
				std::string lat;
				std::string lon;
				std::string timestamp;

				int index;
				while(std::getline(in, row))
				{
					index = 0;
					
				}
	
				Point point
				return mfile;
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
