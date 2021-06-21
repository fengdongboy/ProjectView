#include "PointProperty.h"

PointProperty::PointProperty() :mIndex(-1)
	//,mAlone(false)
	,mBorder(false)
	,mNeedDelete(false)
	//,mAreaValue(0.0f)
	,mLevelLayer(-1)
	,mBorderLevelLayer(-1.0f)
	,mNormalArea(0.0f)
	,mAreaNormal(0.0f, 0.0f, 0.0f)
	,mPropertyPoint(false)
	,mProjectTriIndex(-1)
{
	mBorderList.reserve(16);

	//InitializeCriticalSection( &mBelongTriCS);
	//InitializeCriticalSection( &mBorderListCS);
}

PointProperty::~PointProperty()
{
	//DeleteCriticalSection( &mBelongTriCS);
	//DeleteCriticalSection( &mBorderListCS);
	mIndex = 1000000;
}

void PointProperty::removeBorderlist( int index1, int index2 )
{
	VectorIndex::iterator it = std::find(mBorderList.begin(), mBorderList.end(), index1);
	if (it != mBorderList.end())
	{
		mBorderList.erase(it);
	}
	it = std::find(mBorderList.begin(), mBorderList.end(), index2);
	if (it != mBorderList.end())
	{
		mBorderList.erase(it);
	}
}

void PointProperty::buildBorder( void )
{
	if (mBelongTri.size() <= 2)		/// 如果这个点只属于两个三角形，一定是边界点
	{
		mBorder = true;
		return;
	}
	else
	{
		if (mBorderList.size() < 2) return;
		std::sort(mBorderList.begin(),mBorderList.end());
		//Tools::quick_sort(mBorderList, 0, mBorderList.size() - 1);
		for (int i = 0; i < mBorderList.size() -1; i += 2)
		{
			if (mBorderList[i] != mBorderList[i+1])
			{
				mBorder = true;
				break;
			}
		}
	}
}

bool PointProperty::isRingPoint( void )
{
	if (mBelongTri.size() <= 2)		/// 如果这个点只属于两个三角形，一定是边界点
	{
		mBorder = true;
		return true;
	}
	else
	{
		std::sort(mBorderList.begin(),mBorderList.end());
		for (int i = 0; i < mBorderList.size() -1; i += 2)
		{
			if (mBorderList[i] != mBorderList[i+1])
			{
				return true;
			}
		}
		return false;
	}
}

int PointProperty::BorderLine( int value ) const
{
	int number = std::count(mBorderList.begin(), mBorderList.end(), value);

	/// 有两个以上的边界点，
	/// 并且点是基数
	return ((number > 0) && (number%2));
}

void PointProperty::updateBorderList( int size )
{
	//VectorIndex::iterator it1, it2;
	//it1 = mBorderList.begin();
	//it2 = mBorderList.end();
	//for (; it1!= it2; ++it1)
	//{
	//	(*it1) += size;
	//}
	#pragma omp parallel for
	for ( int i = 0; i < mBorderList.size(); i ++)
	{
		mBorderList[i] += size;
	}
}

void PointProperty::eraseTri( int triIndex )
{
	VectorIndex::iterator it = std::find(mBelongTri.begin(), mBelongTri.end(), triIndex);
	if (it != mBelongTri.end())
	{
		mBelongTri.erase(it);
	}
}

void PointProperty::setBorderLevelLayer( float level )
{
	if ( mBorderLevelLayer == -1.0f)
	{
		mBorderLevelLayer = level;
	}
	else
	{
		mBorderLevelLayer = (std::max)(level, mBorderLevelLayer);
	}
}

void PointProperty::removeBelongTris( const VectorIndex& v )
{
	for ( int i = 0; i < v.size(); i ++)
	{
		mBelongTri.erase(std::remove(mBelongTri.begin(), mBelongTri.end(), v[i]), mBelongTri.end());
	}
}


Ogre::Vector3 ESTriangle::normal( const PointProperty &a, const PointProperty &b, const PointProperty &c )
{
	//float a0 = a.mPoints[0]-c.mPoints[0];
	//float a1 = a.mPoints[1]-c.mPoints[1];
	//float a2 = a.mPoints[2]-c.mPoints[2];
	//float b0 = b.mPoints[0]-c.mPoints[0];
	//float b1 = b.mPoints[1]-c.mPoints[1];
	//float b2 = b.mPoints[2]-c.mPoints[2];
	//float x = a1*b2 - a2*b1;
	//float y = a2*b0 - a0*b2;
	//float z = a0*b1 - a1*b0;
	//float n = x*x+y*y+z*z;
	//if (n == 0) {
	//	return Ogre::Vector3();
	//} else {
	//	n = 1.0f/sqrtf(n);
	//	return Ogre::Vector3(x*n, y*n, z*n);
	//}
	return Ogre::Vector3();
}

omesh::Pnt3 ESTriangle::normal(const omesh::Pnt3& a, const omesh::Pnt3& b, const omesh::Pnt3&c )
{
	float a0 = a[0] - c[0];
	float a1 = a[1] - c[1];
	float a2 = a[2] - c[2];
	float b0 = b[0] - c[0];
	float b1 = b[1] - c[1];
	float b2 = b[2] - b[2];
	float x = a1*b2 - a2*b1;
	float y = a2*b0 - a0*b2;
	float z = a0*b1 - a1*b0;
	float n = x*x+y*y+z*z;
	if (n == 0) {
		return omesh::Pnt3();
	} else {
		n = 1.0f/sqrtf(n);
		return omesh::Pnt3(x*n, y*n, z*n);
	}
}
