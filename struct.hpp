#ifndef STRUCT_HPP
#define STRUCT_HPP

#include <vector>
#include <stdint.h>
#include <boost/shared_ptr.hpp>


namespace QProcessor
{

#pragma pack(push, 1)
	struct KdNode
	{
		uint64_t _childNode;
		uint32_t _medianValue;
	};
#pragma pack(pop)
#pragma pack(push, 1)
	struct Point
	{

//		Point(uint32_t id, uint32_t time, float lat, float lon)
//			:_id(id), _time(time), _lat(lat), _lon(lon){}

		uint32_t _id;
		float _lon;
		float _lat;
		uint32_t _time;
	};
#pragma pack(pop)
	typedef std::vector<const Point*> PointVector;
	typedef boost::shared_ptr<PointVector> PPointVector; 


	struct Iterator 
	{
		Iterator(): _current(0), _end(0) {}
		Iterator(const KdNode *current, const KdNode *end): _current(current), _end(end) 
		{
			this->_point = reinterpret_cast<const Point*>(&(this->_current->_medianValue));
		}

		const Point &  operator *() { return *this->_point; }
		const Point *  operator->() { return this->_point; }
		bool          operator==(const Iterator &it) const { return this->_point==it._point; }
		bool          operator!=(const Iterator &it) const { return this->_point!=it._point; }
		Iterator      operator++(int) {
			this->_current += 4;
			while (this->_current<this->_end && this->_current->_childNode!=0) this->_current++;
			if (this->_current<this->_end) {
				this->_point = reinterpret_cast<const Point*>(&(this->_current->_medianValue));
			}
			else
				this->_point = reinterpret_cast<const Point*>(this->_end);
			return *this;
		}
		private:
		const KdNode * _current;
		const KdNode * _end;
		const Point * _point;
	};

	struct QueryIterator
	{
		QueryIterator(): _iter(0) {}
		QueryIterator(PointVector::const_iterator it): _iter(it)  {}
		inline const Point *  point() { return *(this->_iter); }
		inline const Point &  operator *() { return *(*this->_iter); }
		inline const Point *  operator->() { return *(this->_iter); }
		inline QueryIterator operator++() { this->_iter++; return *this; }
		inline QueryIterator operator--() { this->_iter--; return *this; }
		inline QueryIterator operator+=(int dif) { this->_iter+=dif; return *this; }
		inline QueryIterator operator-=(int dif) { this->_iter-=dif; return *this; }
		inline QueryIterator operator++(int) { this->_iter++; return *this; }
		inline QueryIterator operator--(int) { this->_iter--; return *this; }
		inline ptrdiff_t     operator-(const QueryIterator &it) { return this->_iter-it._iter;}
		inline bool          operator==(const QueryIterator &it) const { return this->_iter==it._iter; }
		inline bool          operator!=(const QueryIterator &it) const { return this->_iter!=it._iter; }
		inline bool          operator<=(const QueryIterator &it) const { return this->_iter<=it._iter; }
		inline bool          operator<(const QueryIterator &it) const { return this->_iter<it._iter; }
		inline bool          operator>=(const QueryIterator &it) const { return this->_iter>=it._iter; }
		inline bool          operator >(const QueryIterator &it) const { return this->_iter>it._iter; }
		PointVector::const_iterator _iter;
	};


	struct QueryResult
	{
		typedef QueryIterator iterator;
		inline iterator begin() { return _points.get()?QueryIterator(_points->begin()):QueryIterator(); }
		inline iterator end() { return _points.get()?QueryIterator(_points->end()):QueryIterator(); }
		inline size_t size() { return _points.get()?_points->size():0; }
		PPointVector _points;
	};


	struct Query
	{
		Query()
		{
			_minTime = 0;
			_maxTime = UINT_MAX;
			_minLat = _minLon = -FLT_MAX;
			_maxLat = _maxLon = FLT_MAX;
		}

		void setArea(float minLat, float minLon, float maxLat, float maxLon)
		{
			this->_minLat = minLat;
			this->_minLon = minLon;
			this->_maxLat = maxLat;
			this->_maxLon = maxLon;
		}

		void setTime(uint32_t minTime, uint32_t maxTime)
		{
			this->_minTime = minTime;
			this->_maxTime = maxTime;
		}

		bool isMatched(const Point * p) const
		{
			return (this->_minTime<=p->_time && p->_time<=this->_maxTime &&
					this->_minLat<=p->_lat && p->_lat<=this->_maxLat &&
					this->_minLon<=p->_lon && p->_lon<=this->_maxLon);
		}

		uint32_t _minTime;
		uint32_t _maxTime;
		float _minLat, _maxLat;
		float _minLon, _maxLon;
	};

};

#endif
