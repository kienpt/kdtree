#ifndef KDTREE_HPP
#define KDTREE_HPP

#include <string>
#include <Point.hpp>

namespace QProcessor
{

class KDTree{
	public:
		struct Query
		{
			Query()
			{
				_minID = 0;
				_maxID = UINT_MAX;
				_minTime = 0;
				_maxTime = UINT_MAX;
				_minLat = _minLon = -FLT_MAX;
				_maxLat = _maxLon = FLT_MAX;				
			}
			//SET GET here

			void setArea(float minLat, float minLon, float maxLat, float maxLon) 
			{
				this->_minLat = minLat
				this->_minLon = minLon
				this->_maxLat = maxLat
				this->_maxLon = maxLon
			}

			bool isMatched(const PPoint & p) const
			{
				return (this->_minTime<=p->_time && p->_time<=this->_maxTime &&
					this->_minLat<=p->_lat && p->_lat<=this->_maxLat &&
					this->_minLon<=p->_lat && p->_lon<=this->_maxLon && 
					this->_minID<=p->_id && p->_id<=this->_maxID &&)
			}
		}

		//Init
		KDTree(std::string csvfile)




	private:

}

#endif
