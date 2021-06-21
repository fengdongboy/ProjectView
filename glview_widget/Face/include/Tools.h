#pragma once
#ifndef __TOOLS_H__
#define __TOOLS_H__
/********************************************************************
	created:	2014/04/02
	created:	2:4:2014   9:29
	filename: 	D:\test program\Face\Face\include\Tools.h
	file path:	D:\test program\Face\Face\include
	file base:	Tools
	file ext:	h
	author:		feng
	
	purpose:	实现一些工具的集合
*********************************************************************/
#include "Pnt3.h"
#include "OgreVector3.h"
#include "LogMessage.h"


#define float_diff 1e-6
#define PI 3.1415926535898

namespace Tools
{
	static bool equalf(float v1, float v2, float Differ = float_diff)
	{
		return (((v1 - v2) < Differ) && ((v2 - v1) < Differ));
	}
	static bool isZero(float value)
	{
		return (equalf(value, 0.0f));
	}
	static bool isZero(const Ogre::Vector3& v)
	{
		return( isZero(v.x) && isZero(v.y) && isZero(v.z));
	}
	static bool isZero(const omesh::Pnt3& v)
	{
		return ( isZero(v[0]) && isZero(v[1]) && isZero(v[2]));
	}
	/// 计算法向量的反折度
	/// 算法直接从源复制
	static bool getTwoNorLineAngle(const Ogre::Vector3& nor1,const Ogre::Vector3& nor2)
	{
		//cos
		//if ((nor1[0]*nor1[0]+nor1[1]*nor1[1]+nor1[2]*nor1[2])*(nor2[0]*nor2[0]+nor2[1]*nor2[1]+nor2[2]*nor2[2]) == 0) 
		//{
		//	return -1;
		//}
		/// 一个点是原点
		if (isZero(nor1) || isZero(nor2))
		{
			return true;
		}
		//float TwoLineNorCos =(nor1[0]*nor2[0] + nor1[1]*nor2[1] + nor1[2]*nor2[2])/sqrt((nor1[0]*nor1[0]+nor1[1]*nor1[1]+nor1[2]*nor1[2])*(nor2[0]*nor2[0]+nor2[1]*nor2[1]+nor2[2]*nor2[2]));

		////1、4象限返回0
		//if (TwoLineNorCos>0)
		//{
		//	return 0;
		//}
		//else
		//{
		//	return 1;
		//}

		float f = (nor1.x*nor2.x + nor1.y*nor2.y + nor1.z*nor2.z);
		return (f<0);
	}

	static bool getTwoNorLineAngle(const omesh::Pnt3& nor1,const omesh::Pnt3& nor2)
	{
		if (isZero(nor1) || isZero(nor2))
		{
			return true;
		}
		float f = (nor1[0]*nor2[0] + nor1[1]*nor2[1] + nor1[2]*nor2[2]);
		return (f<0);
	}

	// calculate barycentric coordinates of the point p
	// on triangle t1 t2 t3
	inline void bary(const omesh::Pnt3& p, 
		const omesh::Pnt3& t1, const omesh::Pnt3& t2, const omesh::Pnt3& t3,
		double &b1, double &b2, double &b3)
	{
		// figure out the plane onto which to project the vertices
		// by calculating a cross product and finding its largest dimension
		// then use Cramer's rule to calculate two of the
		// barycentric coordinates
		// e.g., if the z coordinate is ignored, and v1 = t1-t3, v2 = t2-t3
		// b1 = det[x[0] v2[0]; x[1] v2[1]] / det[v1[0] v2[0]; v1[1] v2[1]]
		// b2 = det[v1[0] x[0]; v1[1] x[1]] / det[v1[0] v2[0]; v1[1] v2[1]]
		float v10 = t1[0]-t3[0];
		float v11 = t1[1]-t3[1];
		float v12 = t1[2]-t3[2];
		float v20 = t2[0]-t3[0];
		float v21 = t2[1]-t3[1];
		float v22 = t2[2]-t3[2];
		float c[2];
		c[0] = fabs(v11*v22 - v12*v21);
		c[1] = fabs(v12*v20 - v10*v22);
		int i = 0;
		if (c[1] > c[0]) i = 1;
		if (fabs(v10*v21 - v11*v20) > c[i]) 
		{
			// ignore z
			float d = 1.0f / (v10*v21-v11*v20);
			float x0 = (p[0]-t3[0]);
			float x1 = (p[1]-t3[1]);
			b1 = (x0*v21 - x1*v20) * d;
			b2 = (v10*x1 - v11*x0) * d;
		} 
		else if (i==0) 
		{
			// ignore x
			float d = 1.0f / (v11*v22-v12*v21);
			float x0 = (p[1]-t3[1]);
			float x1 = (p[2]-t3[2]);
			b1 = (x0*v22 - x1*v21) * d;
			b2 = (v11*x1 - v12*x0) * d;
		} 
		else 
		{
			// ignore y
			float d = 1.0f / (v12*v20-v10*v22);
			float x0 = (p[2]-t3[2]);
			float x1 = (p[0]-t3[0]);
			b1 = (x0*v20 - x1*v22) * d;
			b2 = (v12*x1 - v10*x0) * d;
		}
		b3 = 1.0f - b1 - b2;
	}

	/*// distance from x to linesegment from a to b
	inline float  dist2_lineseg(const omesh::Pnt3 &x, const omesh::Pnt3 &a, const omesh::Pnt3 &b)
	{
		Pnt3 ba = b; ba -= a;
		Pnt3 xa = x; xa -= a;

		float xa_ba = dot(xa,ba);
		// if the dot product is negative, the point is closest to a
		if (xa_ba < 0.0)   return dist2(x,a);

		// if the dot product is greater than squared segment length,
		// the point is closest to b
		float nba2 = ba.norm2();
		if (xa_ba >= nba2) return dist2(x,b);

		// take the squared dist x-a, squared dot of x-a to unit b-a,
		// use Pythagoras' rule
		return xa.norm2() - xa_ba*xa_ba/nba2;
	}*/


	static float pointProject2Line_wzk(const omesh::Pnt3& P1,const omesh::Pnt3& P2, const omesh::Pnt3& P_Get, omesh::Pnt3& PP)
	{
		float t=0.0f;
		omesh::Pnt3 BA((P1[0]-P2[0]),(P1[1]-P2[1]),(P1[2]-P2[2])),AC((P1[0]-P_Get[0]),(P1[1]-P_Get[1]),(P1[2]-P_Get[2]));
		if(((BA[0])*(BA[0])+(BA[1])*(BA[1])+(BA[2])*(BA[2]))!=0)
			t=(BA[0]*AC[0]+BA[1]*AC[1]+BA[2]*AC[2])/((BA[0])*(BA[0])+(BA[1])*(BA[1])+(BA[2])*(BA[2]));

		PP[0]=(-BA[0]*t+P1[0]);
		PP[1]=(-BA[1]*t+P1[1]);
		PP[2]=(-BA[2]*t+P1[2]);
		return t;
	}

	static bool PointinTriangle(const omesh::Pnt3& A, const omesh::Pnt3& B, const omesh::Pnt3 C, const omesh::Pnt3& P, double* uvw)
	{
		omesh::Pnt3 v0 = C - A ;
		omesh::Pnt3 v1 = B - A ;
		omesh::Pnt3 v2 = P - A ;

		float dot00 = dot(v0, v0);//v0.Dot(v0) ;
		float dot01 = dot(v0, v1);//v0.Dot(v1) ;
		float dot02 = dot(v0, v2);//v0.Dot(v2) ;
		float dot11 = dot(v1, v1);//v1.Dot(v1) ;
		float dot12 = dot(v1, v2);//v1.Dot(v2) ;

		float inverDeno = 1 / (dot00 * dot11 - dot01 * dot01) ;

		double u = (dot11 * dot02 - dot01 * dot12) * inverDeno ;
		double v = (dot00 * dot12 - dot01 * dot02) * inverDeno ;
		uvw[0] = u;
		uvw[1] = v;
		uvw[2] = 1 - u - v;
		if (u < 0 || u > 1) // if u out of range, return directly
		{
			return false ;
		}

		if (v < 0 || v > 1) // if v out of range, return directly
		{
			return false ;
		}

		return u + v <= 1 ;
	}

	static void nearpoint(const omesh::Pnt3& PP,const omesh::Pnt3& PA,const omesh::Pnt3& PB, const omesh::Pnt3& PC,double& lll, double* uvw ,omesh::Pnt3& PN,double* uvw_old)
	{
		omesh::Pnt3 P02 = PA - PC;
		omesh::Pnt3 P10 = PB - PA;
		omesh::Pnt3 P12 = PB - PC;
		omesh::Pnt3 P2v = PC - PP;
		omesh::Pnt3 P20 = PC - PA;
		float ax = dot(P02,P10);
		float bx = dot(P12,P10);
		float cx = dot(P2v,P10);
		float ay = dot(P02,P20);
		float by = dot(P12,P20);
		float cy = dot(P2v,P20);
		float u = -(bx*cy-by*cx)/(ay*bx-ax*by);
		float v = (ax*cy-ay*cx)/(ay*bx-ax*by);
		float w = 1-u-v;
		omesh::Pnt3 Pv = u*PA + v*PB + w*PC;//这是投影点

		uvw[0] = uvw_old[0] = (double)u; 
		uvw[1] = uvw_old[1] = (double)v; 
		uvw[2] = uvw_old[2] = (double)w; 
		//uvw_old = uvw;

		if(u>=0 && v>=0&&w>=0) 
		{
			lll = dist(Pv,PP);
			PN = Pv;
			uvw[0] = u;uvw[1] = v;uvw[2] = w;

		}
		if(u>=0 && v<0 && w<0)
		{
			lll = dist(PA,PP);
			PN = PA;
			uvw[0] = 1;uvw[1] = 0;uvw[2] = 0;
		}
		if(u<0 && v>=0 && w<0)
		{
			lll = dist(PB,PP);
			PN = PB;
			uvw[0] = 0;uvw[1] = 1;uvw[2] = 0;
		}
		if(u<0 && v<0 && w>=0) 
		{
			lll = dist(PC,PP);
			PN = PC;
			uvw[0] = 0;uvw[1] = 0;uvw[2] = 1;
		}
		if(u>=0 && v>=0 && w<0) 
		{
			lll = omesh::dist2_lineseg(PP,PA,PB);
			pointProject2Line_wzk(PA,PB,PP,PN);
			bary(PN,PA,PB,PC,uvw[0],uvw[1],uvw[2]);
		}
		if(u<0 && v>=0 && w>=0)
		{
			lll = omesh::dist2_lineseg(PP,PB,PC);
			pointProject2Line_wzk(PB,PC,PP,PN);
			bary(PN,PA,PB,PC,uvw[0],uvw[1],uvw[2]);
		}
		if(u>=0 && v<0 && w>=0)
		{
			lll = dist2_lineseg(PP,PC,PA);
			pointProject2Line_wzk(PC,PA,PP,PN);
			bary(PN,PA,PB,PC,uvw[0],uvw[1],uvw[2]);
		}
	}

	static bool nearpoint(const omesh::Pnt3& PP,const omesh::Pnt3& Pnor,const omesh::Pnt3& PA,const omesh::Pnt3& PB, const omesh::Pnt3& PC,double& lll, double* uvw ,omesh::Pnt3& PN)
	{
		omesh::Pnt3 tempNor1 = PC - PA;
		omesh::Pnt3 tempNor2 = PB - PA;
		omesh::Pnt3 nor = cross(tempNor1, tempNor2);
		nor = nor.normalize();		/// 这个平面的法线
		float d = -dot(nor, PA);		/// 这个平面的距离

		///float denom = plane.normal.dotProduct(ray.getDirection());
		float denom = dot(Pnor, nor);	/// 线跟平面夹角
		if (std::abs(denom) < std::numeric_limits<Real>::epsilon())
		{
			// Parallel
			// return std::pair<bool, Real>(false, 0);
			/// 平行
		}
		else
		{
			//Real nom = plane.normal.dotProduct(ray.getOrigin()) + plane.d;
			float nom = dot(nor, PP) + d;
			double t = (nom/denom);
			PN = PP - t* Pnor;	/// 投影点
			lll = std::abs(t);
			return PointinTriangle(PA, PB, PC, PN, uvw);
			//return std::pair<bool, Real>(t >= 0, t);
		}
		return false;
	}

	static void copyPntToVector3( Ogre::Vector3& vec3, const omesh::Pnt3& pnt)
	{
		vec3[0] = pnt[0];
		vec3[1] = pnt[1];
		vec3[2] = pnt[2];
	}

	static void copyVector3ToPnt( omesh::Pnt3& pnt, const Ogre::Vector3& vec3)
	{
		pnt[0] = vec3[0];
		pnt[1] = vec3[1];
		pnt[2] = vec3[2];
	}

	template<class T>
	void copyEveryTypePtr(T* to, const T* src)
	{
		to[0] = src[0];
		to[1] = src[1];
		to[2] = src[2];
	}

	template<class T>
	void copyEveryType(T& to, const T& src)
	{
		to[0] = src[0];
		to[1] = src[1];
		to[2] = src[2];
	}

	#define ANGLE_ERR 10.0
	static double retTwoNorLineAngle(const omesh::Pnt3& nor1,const omesh::Pnt3& nor2)
	{
		//cos
		if (isZero(nor1) && isZero(nor2)) 
		{
			return ANGLE_ERR;
		}
		//double TwoLineNorCos =nor1*nor2/sqrt((nor1[0]*nor1[0]+nor1[1]*nor1[1]+nor1[2]*nor1[2])*(nor2[0]*nor2[0]+nor2[1]*nor2[1]+nor2[2]*nor2[2]));
		double TwoLineNorCos = dot(nor1, nor2)/(nor1.norm()*nor2.norm());

		return TwoLineNorCos;
	}

	/*//点C(vtx2)到直线AB(vtx1)的距离t 表示从A到B的位置
	float PointProject2Line(int A,int B,int C,vector<Pnt3>& Vtx1,vector<Pnt3> & Vtx2,Pnt3 & PP){
		float t=0.0f;
		Pnt3 BA((Vtx1[A][0]-Vtx1[B][0]),(Vtx1[A][1]-Vtx1[B][1]),(Vtx1[A][2]-Vtx1[B][2])),AC((Vtx1[A][0]-Vtx2[C][0]),(Vtx1[A][1]-Vtx2[C][1]),(Vtx1[A][2]-Vtx2[C][2]));
		if(((BA[0])*(BA[0])+(BA[1])*(BA[1])+(BA[2])*(BA[2]))!=0)
			t=(BA[0]*AC[0]+BA[1]*AC[1]+BA[2]*AC[2])/((BA[0])*(BA[0])+(BA[1])*(BA[1])+(BA[2])*(BA[2]));
		if(t<1 && t>0){
			PP[0]=(-BA[0]*t+Vtx1[A][0]);
			PP[1]=(-BA[1]*t+Vtx1[A][1]);
			PP[2]=(-BA[2]*t+Vtx1[A][2]);
		}
		return t;
	}*/

	//点T到直线AB 的距离t 表示从A到B的位置
	//static float PointProject2Line(const omesh::Pnt3& pointA, const omesh::Pnt3& pointB, const omesh::Pnt3& pointT, omesh::Pnt3& outPoint)
	//{
	//	omesh::Pnt3 ba = pointA - pointB;
	//	omesh::Pnt3 at = pointA - pointT;
	//	float t = 0.0f;
	//	if (!isZero(ba))
	//	{
	//		t = dot(ba, at)/ba.norm2();
	//		/*k = |P0-P1|/|P2-P1| = d/|P2-P1| = (P3 - P1) * (P2   - P1)   / abs (P2   - P1)
	//			= (P3 - P1) * (P2   - P1)   / (P2   - P1) *(P2   - P1)*/
	//	}
	//	if ( t > 0 && t < 1)	/// 限制在0到1中间的
	//	{
	//		outPoint = -ba*t + pointA;
	//	}
	//	return t;

	//	float t=0.0f;
	//	omesh::Pnt3 BA(pointA-pointB),AC(pointA-pointT);
	//	if(((BA[0])*(BA[0])+(BA[1])*(BA[1])+(BA[2])*(BA[2]))!=0)
	//		t=(BA[0]*AC[0]+BA[1]*AC[1]+BA[2]*AC[2])/((BA[0])*(BA[0])+(BA[1])*(BA[1])+(BA[2])*(BA[2]));
	//	if(t<1 && t>0){
	//		outPoint[0]=(-BA[0]*t+pointA[0]);
	//		outPoint[1]=(-BA[1]*t+pointA[1]);
	//		outPoint[2]=(-BA[2]*t+pointA[2]);
	//	}
	//	return t;
	//}

	static float PointProject2Line(const omesh::Pnt3& p1, const omesh::Pnt3& p2, const omesh::Pnt3& p, omesh::Pnt3& outPoint)
	{
		omesh::Pnt3 diff = p-p1;
		omesh::Pnt3 dir = p2-p1;
		float dot1 = dot(diff,dir);
		if (dot1 <= 0.0f) 
		{
			outPoint = p1;
			return dot1;
		}    
		float dot2 = dot(dir,dir);
		if (dot2 <= dot1) 
		{
			outPoint = p2;
			return dot2;
		}
		float t=dot1/dot2;
		outPoint = p1 + t * dir;
		return t;
	}

	static int getPointsUnin(int know1, int know2, const unsigned int* pos)
	{
		for (int i = 0; i < 3; i ++)
		{
			if (pos[i] != know1 && pos[i] != know2)
			{
				return pos[i];
			}			
		}
		return -1;
	}

	/// 查找这个三个点是否共点
	static bool findSamePoint(const unsigned int const* p)
	{
		return (p[0] == p[1] || p[1] == p[2] || p[0] == p[2]);
	}

	/// 删除一个vector里相同的元素，只保留单一的元素
	static void deleteSameInt( VectorIndex& v)
	{
		std::sort(v.begin(), v.end());
		//int number = 0;
		//VectorIndex temp;
		//temp.reserve(v.size());
		//VectorIndex::iterator it1, it2;
		//it1 = v.begin();
		//it2 = v.end();
		//for (; it1 != it2; ++it1)
		//{
		//	if (*it1 != number)
		//	{
		//		temp.push_back(*it1);
		//		number = (*it1);
		//	}
		//}
		v.erase( unique( v.begin(), v.end() ), v.end() );
		//v.swap(temp);
	}


	static std::string getCurTime( void)
	{
		time_t tt = time(NULL);//这句返回的只是一个时间cuo
		tm* t= localtime(&tt);
		char buf[32] = {0};
		sprintf(buf, "%d-%02d-%02d %02d:%02d:%02d", 
			t->tm_year + 1900,
			t->tm_mon + 1,
			t->tm_mday,
			t->tm_hour,
			t->tm_min,
			t->tm_sec);
		return buf;
	}

	/// 得到这两个点在三角形中是顺时针还是逆时针
	static bool getDirTriPoint(const unsigned int* p, int index1, int index2)
	{
		int i1 = -1, i2 = -1;
		for (int i = 0; i < 3; i ++)
		{
			if (p[i] == index1)
			{
				i1 = i;
			}
			if (p[i] == index2)
			{
				i2 = i;
			}
		}
		assert(i1 != -1 && i2 != -1);
		/// 当i1在1 i2 在2 或者i1 在2 i2在3 或者 i1 在3 i2在1 都是顺时针
		int ret = i2 - i1;
		return (ret == 1 || ret == -2);
	}

	template<typename _Ret, typename _Tp, typename _Arg>
	class mem_fun2_t : public binary_function<_Arg , _Arg , _Ret>
	{
	public:
		explicit
			mem_fun2_t(_Ret (_Tp::*__pf)(const _Arg &, const _Arg & ), _Tp *__ptr)
			: _M_f(__pf), __p(__ptr){ }

		_Ret
			operator()(const _Arg & __x, const _Arg & __y) const
		{ return (__p->*_M_f)(__x,__y); }

	private:
		_Ret (_Tp::*_M_f)(_Arg & ,_Arg & );
		_Tp * __p;
	};

	template<typename _Ret, typename _Tp, typename _Arg>
	inline mem_fun2_t<_Ret, _Tp, _Arg>
		mem_fun2(_Ret (_Tp::*__f)(_Arg &,_Arg &), _Tp *__p)
	{ return mem_fun2_t<_Ret, _Tp, _Arg>(__f, __p); }


	/// 把序号排序
	static void sortPoints(VectorIndex& v, const std::vector<omesh::Pnt3>& points )
	{
		/// 先用最简单的排序，不追求效率
		if (v.size() < 2) return;
		for ( int i = 0; i < v.size() - 1; i ++)
		{
			for (int j = 0; j < v.size() - i -1; j ++)
			{
				const omesh::Pnt3& a = points.at(v[j]);
				const omesh::Pnt3& b = points.at(v[j+1]);
				if ( a > b)
				{
					std::swap(v[j], v[j+1]);
				}
			}
		}
	}

	/// 从一个vector里减去另外一个vector里所有的元素
	static void vecRemoveVec( VectorIndex& src, const VectorIndex& vec)
	{
		for ( int i = 0; i < vec.size(); i ++)
		{
			src.erase(std::remove(src.begin(), src.end(), vec[i]), src.end());
		}
	}

	static bool getRocFileNumber(VectorString& Vecstr)
	{
		/// 调用系统的获取当前文件夹内data里的所有roc的文件
		system("dir \Project\ /S /B >faceroc.txt");

		Vecstr.clear();
		FILE* f = fopen("faceroc.txt", "r");
		if (f == NULL) return false;
		
		char buf[1024] = {0};
		std::string str, str1;
		while ( fgets(buf, 1024, f) )
		{
			str = buf;
			str1 = str.substr(str.size() - 4) ;
			if (str1.substr(0, 3) == "roc")
			{
				str =str.substr(0, str.size() - 5);
				Vecstr.push_back(str);
			}
		}

		return true;
	}

	static bool getRocFileNumber(const std::string& path, VectorString& Vecstr)
	{
		std::string file = path + "\\_odsproject_";
		FILE* f = fopen(file.c_str(), "r");
		if (f == NULL) return false;
		char buf[1024] = {0};
		fgets(buf,1024, f);		/// 跳一行
		std::string str;
		Vecstr.clear();
		while (fgets(buf, 1024, f))
		{
			str = buf;
			str = str.substr(0, str.size() - 1);
			if (str.find(".roc") != std::string::npos || str.find(".pcd") != std::string::npos)
			{
				Vecstr.push_back(path + "\\"+str);
			}
		}
		return true;
	}

	/// 重写排序算法
	/// stl 的排序比这个排序还快？
	static void quick_sort (VectorIndex& data, size_t left, size_t right) 
	{
			size_t p = (left + right) / 2;
			int pivot = data[p];
			size_t i = left,j = right;
			for ( ; i < j;) 
			{
				while (! (i>= p || pivot < data[i]))
					++i;
				if (i < p) 
				{
					data[p] = data[i];
					p = i;
				}
				while (! (j <= p || data[j] < pivot))
					--j;
				if (j > p) 
				{
					data[p] = data[j];
					p = j;
				}
			}
			data[p] = pivot;
			//#pragma omp parallel sections
			{
				//#pragma omp section
				{
				if (p - left > 1)
					quick_sort(data, left, p - 1);
				}
				//#pragma omp section
				{
				if (right - p > 1)
					quick_sort(data, p + 1, right);
				}
			}
	}

		//// https://svn.code.sf.net/p/snice/code/trunk/trunk
	static void RGB2HSL(float r, float g, float b, float * h, float * s, float * l)
	{
		float maximum = max( r, max(g, b));
		float minimum = min( r, min(g, b));
		*l = (maximum + minimum)/2;

		if (minimum == maximum)
		{
			*s = 0;
			*h = 0;
		}
		else
		{
			if ((*l) < 0.5)
			{
				*s = (maximum - minimum) / (maximum + minimum);
			}
			else
			{
				*s = (maximum - minimum) / (2 - maximum - minimum);
			}

			if (r == maximum)
			{
				*h = (g - b)/(maximum-minimum);
			}
			if (g == maximum)
			{
				*h = (float)2.0 + (b - r)/(maximum-minimum);
			}
			if (b == maximum)
			{
				*h = (float)4.0 + (r - g)/(maximum-minimum);
			}
			*h = (*h) * 60;
			if (*h<0){*h = (*h) + 360;}
		}
	}

	static void RGB2HSL(const omesh::Pnt3& color, float& h, float& s, float& l)
	{
		RGB2HSL(color[0]*255, color[1]*255, color[2]*255, &h, &s, &l);
	}

	static float Hue_2rGB( float v1, float v2, float vH )             //Function Hue_2rGB
	{
		if ( vH < 0.0f ) {vH += 1.0f;}
		if ( vH > 1.0f ) {vH -= 1.0f;}
		if ( ( 6.0f * vH ) < 1.0f ) {return ( v1 + ( v2 - v1 ) * 6.0f * vH );}
		if ( ( 2.0f * vH ) < 1.0f ) {return ( v2 );}
		if ( ( 3.0f * vH ) < 2.0f ) {return ( v1 + ( v2 - v1 ) * ( ( 2.0f / 3.0f ) - vH ) * 6.0f );}
		return ( v1 );
	}

	static void HSL2RGB(float h, float s, float l, float * r, float * g, float * b)
	{

		h = h / 360.0f;
		if ( s == 0 )                       //HSL values = From 0 to 1
		{
			*r = l;                  //RGB results = From 0 to 255
			*g = l; 
			*b = l; 
		}
		else
		{
			float var_2, var_1;
			if ( l < 0.5f ) 
			{
				var_2 = l * ( 1.0f + s );
			}
			else
			{
				var_2 = ( l + s ) - ( s * l );
			}

			var_1 = 2.0f * l - var_2;

			*r =  Hue_2rGB( var_1, var_2, h + ( 1.0f / 3.0f ) ); 
			*g =  Hue_2rGB( var_1, var_2, h );
			*b =  Hue_2rGB( var_1, var_2, h - ( 1.0f / 3.0f ) );
		}
	}

	static void HSL2RGB(float h, float s, float l, omesh::Pnt3& color)
	{
		HSL2RGB(h, s, l, &color[0], &color[1], &color[2]);
		color[0] /= 255;
		color[1] /= 255;
		color[2] /= 255;
	}

	/// git://github.com/ragnraok/android-image-filter.gitmaster
	static void RGB2HSI(double R, double G, double B, double& h, double& s, double& i) 
	{ // 0 ~ 255
		double r = R;// / COLOR_UPPER_BOUND;
		double g = G;// / COLOR_UPPER_BOUND;
		double b = B;// / COLOR_UPPER_BOUND;

		double theta = acos(
			((r - g) + (r - b)) * 0.5
			/ pow((pow((r - g), 2) + (r - b) * (g - b)), 0.5));
		theta = theta * 180 / PI;
		h;
		if (b <= g) {
			h = theta;
		} else {
			h = 360 - theta;
		}

		i = (r + g + b) / 3.0;

		double minColor = 0;
		minColor = r < g ? r : g;
		minColor = minColor < b ? minColor : b;
		s = 1 - i / (r + g + b);
		//return HSI(h, s, i);
	}
	static void RGB2HSI(const omesh::Pnt3& color, double& h, double& s, double& i)
	{
		RGB2HSI(color[0], color[1], color[2], h, s, i);
	}

	static void HSI2RGB(double h, double s, double i, double& r, double& g, double& b) 
	{
		//double r, g, b;
		if (h >= 0 && h < 120.0) {
			b = i * (1 - s);
			r = i * (1 + (s * cos(h * PI / 180.0)) / (cos((60 - h) * PI / 180.0)));
			g = 3 * i - (r + b);
			//return RGB(int(r * COLOR_UPPER_BOUND), int(g * COLOR_UPPER_BOUND), int(b * COLOR_UPPER_BOUND));
		} 
		else if (h >= 120.0 && h < 240.0) 
		{
			h = h - 120;
			r = i * (1 - s);
			g = i * (1 + (s * cos(h * PI / 180.0)) / (cos((60 - h) * PI / 180.0)));
			b = 3 * i - (r + g);
			//return RGB(int(r * COLOR_UPPER_BOUND), int(g * COLOR_UPPER_BOUND), int(b * COLOR_UPPER_BOUND));
		} 
		else if (h >= 240.0 && h <= 360.0) 
		{
			h = h - 240;
			g = i * (1 - s);
			b = i * (1 + (s * cos(h * PI / 180.0)) / (cos((60 - h) * PI / 180.0)));
			r = 3 * i - (g + b);
			//return RGB(int(r * COLOR_UPPER_BOUND), int(g * COLOR_UPPER_BOUND), int(b * COLOR_UPPER_BOUND));
		}
		//LOGE("HSI2RGB, rgb is -1");
		//return RGB(-1, -1, -1);
	}
	static void HSI2RGB(double h, double s, double i, omesh::Pnt3& color)
	{
		double r, g, b;
		HSI2RGB(h, s, i, r, g, b);
		color[0] = r;
		color[1] = g;
		color[2] = b;
	}

	/// 叉积公式：u x v = { u2v3-v2u3 , u3v1-v3u1 , u1v2-u2v1 }
	/// 点积公式：u * v = u1v1+u2v2+u3v33=lul*lvl*COS(U,V)
	/// 如果点乘的结果为0，那么这两个向量互相垂直；如果结果大于0，那么这两个向量的夹角小于90度；如果结果小于0，那么这两个向量的夹角大于90度
	inline float getAngleTwoPnt( const omesh::Pnt3&a, const omesh::Pnt3&b)
	{
		if (a.norm()*b.norm() == 0)
		{
			return -1.0;
		}
		return omesh::dot(a, b)/(a.norm()*b.norm());
	}
}

#endif