#include <iostream>
#include "boost/filesystem.hpp"
#include "kdtree.hpp"

namespace QProcessor
{

	//Init
	KDTree::KDTree(std::string csvfile)
	{
		//build kdtree here	
		bool res = true;
		std::string kdtreefile = csvfile + ".kdtree";
		if (!boost::filesystem::exists(kdtreefile))
			res = this->createKdTree(csvfile);
		if (res)
		{
			this->_fTree.open(kdtreefile);
			this->_nodes = reinterpret_cast<const KdNode*>(_fTree.data());
			size_t nodeCount = this->_fTree.size()/sizeof(KdNode);
			this->_endNode = this->_nodes+nodeCount;
		}
	}

	Iterator KDTree::begin()
	{
		const KdNode *nodes = this->_nodes;
		while (nodes<this->_endNode && nodes->_childNode!=0) nodes++;
		return Iterator(nodes, this->_endNode);
	}

	Iterator KDTree::end()
	{
		return Iterator(this->_endNode, this->_endNode);
	}

	QueryResult KDTree::execute(const Query &q) 
	{
		uint32_t range[3][2] = {
			{q._minTime, q._maxTime},
			{float2uint(q._minLat), float2uint(q._maxLat)},
			{float2uint(q._minLon), float2uint(q._maxLon)},
		};
		QueryResult result;
		result._points = PPointVector(new PointVector());
		searchKdTree(_nodes, 0, range, 0, q, result);
		return result;
	}

	void KDTree::buildKdTree(KdNode *nodes, uint32_t *tmp, Point *points, uint64_t n, int depth, uint64_t thisNode, uint64_t &freeNode) {
		KdNode * nextNode = nodes + thisNode;
		if (n<2) {
			static uint64_t cnt = 0;
			cnt++;
			if (cnt%500000==0)
				fprintf(stderr, "\r%lu", cnt);
			nextNode->_childNode = 0;
			*reinterpret_cast<Point *>(&(nextNode->_medianValue)) = * points;
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
			buildKdTree(nodes, tmp, points + medianIndex+1, n-medianIndex-1, depth+1,
					nextNode->_childNode+1+((uint64_t)(medianIndex+1<2))*3, freeNode);
		else
			nodes[nextNode->_childNode+1]._childNode = -1;
	}

	bool KDTree::createKdTree(std::string csvfile) {
		//Conver csv to binary first
		std::string binaryfile = csvfile + ".bin";
		std::string kdtreefile = csvfile + ".kdtree";
		if (this->csv2binary(csvfile, binaryfile) == false)
			return false;
		boost::iostreams::mapped_file mfile(binaryfile, boost::iostreams::mapped_file::priv);
		uint64_t n = mfile.size()/sizeof(Point);
		Point *points = (Point*)mfile.const_data();
		//KdNode *nodes = (KdNode*)malloc(sizeof(KdNode)*(4*n+n*3/2));
		KdNode *nodes = (KdNode*)malloc(sizeof(KdNode)*(6*n));//the number 6 is upper-bound estimated
		uint32_t *tmp = (uint32_t*)malloc(sizeof(uint32_t)*n);

		uint64_t freeNode = 1;
		buildKdTree(nodes, tmp, points, n, 0, 0, freeNode);

		// Writing new indices file

		fprintf(stderr, "\rWriting %lu nodes to %s\n", freeNode, kdtreefile.c_str());
		FILE *fo = fopen(kdtreefile.c_str(), "wb");
		fwrite(nodes, sizeof(KdNode), freeNode, fo);
		fclose(fo);

		mfile.close();
		free(nodes);
		free(tmp);
		return true;
	}

	bool KDTree::csv2binary(std::string csvIn, std::string binOut)
	{
		int indices[] = {-1 ,-1 , -1, -1}; //array of indices to the attributes with the order: id, lattitude, longitude, time.
		int attNum; //number of attributes
		FILE *fo = fopen(binOut.c_str(), "wb");
		std::ifstream  in(csvIn.c_str());
		std::string line;
		Point point;

		//Get the first line (attribute names) of the csv file
		std::getline(in, line);
		std::vector<std::string> attNames;
		boost::split(attNames, line, boost::is_any_of(","));
		attNum = attNames.size();
		for(int i=0; i<attNum; i++)
		{
			std::cout<<attNames[i]<<std::endl;
			if 	(attNames[i].compare("ID") ==0) 	indices[0] = i;
			else if (attNames[i].compare("Lattitude") == 0) indices[1] = i;
			else if (attNames[i].compare("Longitude") == 0) indices[2] = i;
			else if (attNames[i].compare("Time") == 0)	indices[3] = i;
		}

		//Check indices
		for(int i=0; i<4; i++)
		{
			if (indices[i] == -1)
				return false;
		}

		std::string temp;
		while(std::getline(in, line))
		{
			int index = 0;
			std::stringstream lineStream(line);
			while (index < attNum)
			{
				std::getline(lineStream, temp, ',');
				if (index == indices[0])
					point._id = boost::lexical_cast<uint32_t>(temp);
				else if (index == indices[1])
					point._lat = boost::lexical_cast<float>(temp);
				else if (index == indices[2])
					point._lon = boost::lexical_cast<float>(temp);
				else if (index == indices[3])
					point._time = createTime(temp);
				index ++;
			}
			fwrite(&point, 1, sizeof(point), fo);

		}
		return true;
	}

	void KDTree::searchKdTree(const KdNode *nodes, uint32_t root, uint32_t range[3][2], int depth, const Query &query, QueryResult &result) 
	{
		std::cout<<"searchKdTree"<<std::endl;
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

	uint32_t KDTree::float2uint(float f) 
	{
		register uint32_t t(*((uint32_t*)&f));
		return t ^ ((-(t >> 31)) | 0x80000000);
	}

};
