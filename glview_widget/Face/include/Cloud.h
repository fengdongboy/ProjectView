#pragma once
#ifndef __CLOUD_H__
#define __CLOUD_H__
/********************************************************************
	created:	2014/03/27
	created:	27:3:2014   11:09
	filename: 	d:\test program\Face\Face\Cloud.h
	file path:	d:\test program\Face\Face
	file base:	Cloud
	file ext:	h
	author:		feng
	
	purpose:	点云数据结构
*********************************************************************/

#include "stdafx.h"
#include "Xform.h"
#include "tnt_matrix.h"
#include "utils.h"
#include "EasyBMP.h"
#include "pif_format.h"
#include "RangeGrid.h"
#include "Mesh.h"
#include "PointProperty.h"

//#define File_Bmp	".bmp"  /// 对应的图片
//#define File_Txt	".txt"  /// 对应的txt
//#define File_xf	".xf"  /// 对应的txt
//#define File_infbin "_inf.bin" /// 对应的inf.bin
//#define File_roc	""		/// 对应的roc

#define File_Bmp	".bmp"  /// 对应的图片
#define File_Txt	".txt"  /// 对应的txt
#define File_xf	".xf"  /// 对应的txt
#define File_infbin "_inf.bin" /// 对应的inf.bin
#define File_roc	""		/// 对应的roc

#define wrongLenght 0.5		/// 错误点最短距离的判断

using omesh::Xform;
using TNT::Matrix;
using omesh::Mesh;
using omesh::RangeGrid;

typedef std::vector<ESTriangle> Triangles;
typedef std::vector<PointProperty> PointPropertys;

typedef std::list<int> Hole;
typedef std::vector<Hole> Holes;

const char wrlFileHeader[] = "#VRML V2.0 utf8\n\
#\n\
# Geomagic Studio\n\
#\n\
Viewpoint\n\
{\n\
	position -22.445450 -22.256793 -147.842509\n\
		orientation 0 0 1 0\n\
		fieldOfView 0.785398\n\
}\n\
DEF T Transform\n\
{\n\
	children\n\
		[\n\
			Shape\n\
			{\n\
				appearance Appearance\n\
				{\n\
					material Material\n\
					{\n\
						diffuseColor 0.7 0.7 1\n\
							ambientIntensity 0.9\n\
							specularColor 0.8 0.8 0.8\n\
					}\n\
				}\n\
				geometry IndexedFaceSet\n\
				{\n\
					solid FALSE\n\
";
const char wrlFileTail[] = "				}\n			}\n		]\n}\n";


struct TriInt
{
	int index1;
	int index2;
	int index3;
	int triIndex;
	/// 滞后点属性，其中index 是索引为负数，index1和index2是邻边 有可能这两个边中有一个也是负数
	TriInt(int i1, int i2, int i3, int tri):index1(i1), index2(i2), index3(i3),triIndex(tri){}
};

typedef std::vector<TriInt> VecTriInt;

/// 排序相同点，删除相同点
struct OrderProducePoint
{
	bool h;
	bool r;
	int index;
	Ogre::Vector3 pos;
};
typedef std::vector<OrderProducePoint> VecOrderProducePoint;

/// 记录过程中，前后点的对应关系，为延迟关联准备
struct LinkInt
{
	int hostIndex;
	int guestIndex;
	LinkInt(int h, int g):hostIndex(h), guestIndex(g) {}
};

//typedef std::vector<LinkInt> VecLinkInt;

typedef std::vector<std::pair<int, int> > VecLinkInt;



struct ProjectPoint
{
	bool mIsOverlappedIdx;		/// 是否是重合点
	int mIndex;		/// 投影点三角形序号
	int mPointIndex;	/// 投影点序号
	double mUV[3];	/// 投影点uv
	Ogre::Vector3 mPostion;		/// 投影点坐标	
	omesh::Pnt3 mColor;		/// 投影点颜色
	ProjectPoint()
		:mIndex(0)
		,mIsOverlappedIdx(false)
		,mPointIndex(0)
	{
		mUV[0] = 0.0f;
		mUV[1] = 0.0f;
		mUV[2] = 0.0f;
	}
};

//typedef std::map<int, ProjectPoint> MapPointProject;
typedef std::vector<std::pair<int, ProjectPoint> > MapPointProject;

struct ProducePoint
{
	bool replace;	/// 这个点是新创建的还是替换，当是新创建的index为当前三角形内索引，如果是替换index为点索引
	bool order;		/// 这两个点的顺序
	int triIndex;	/// 所在三角形索引
	int srcPoint;	/// 原来的点
	int line1;		/// 所在线的索引点1
	int line2;		/// 所在线的索引点2
	int targetIndex;		/// 对应的最近点
	int third;		/// 这条边对着的点
	float lenght;	/// 当前点的长度
	Ogre::Vector3 pos;	/// 这个点的位置
	ProducePoint():replace(false),order(false),triIndex(0),srcPoint(0),line1(0),line2(0),targetIndex(0),lenght(0.0f){}

	void clear()
	{
		replace = false;
		order = false;
		triIndex = 0;
		srcPoint = 0;
		line1 = 0;
		line2 = 0;
		targetIndex = 0;
		lenght = 0.0f;
	}
};

struct HoleStatue
{
	int holeIndex;			/// 洞所在的索引
	bool statue;			/// 状态
	int mapIndex;			/// 点在映射上的位置

	bool getStatue() { return statue; }
	void setStatue( bool s) { statue = s; }
	int getHoleIndex( void ) { return holeIndex; }
	void setHoleIndex( int i ) { holeIndex = i;}
	int getIndex( void ) { return mapIndex; };

	HoleStatue():holeIndex(-1), statue(false) {}
	HoleStatue(int i):holeIndex(-1), mapIndex(i), statue(false) {}
	HoleStatue(int i, bool s):holeIndex(i), statue(s) {}	
};

typedef std::map<int, HoleStatue> PointBorderLink;

typedef std::vector<ProducePoint> VecProducePoint;
typedef std::multimap<int, ProducePoint> MulProducePoint;
typedef std::pair<MulProducePoint::iterator, MulProducePoint::iterator> MulProPointInter;


class Cloud
{
public:
	Cloud(void)
		:mMesh(NULL)
	{

	}
	~Cloud(void)
	{
		if (mMesh)
		{
			delete mMesh;
			mMesh = NULL;
		}
	}

	void setNumber(int number );
	bool readData(const std::string& path, int subSamp=1);

	bool readData( const std::string& path, const std::string RocPaht, 
		const Matrix<float> &R, const Matrix<float> &G, const Matrix<float> &B, 
		const Xform<float>& XF, const Matrix<double>& KK, const Matrix<double>& KC, 
		const Matrix<double>& RM, const Matrix<double>& TM ,int subSamp/*=1*/ );

	bool readDataPCD(const std::string& path, int subSamp=1);

	bool transPointXF(const std::string& xf, std::vector<omesh::Pnt3>& pts_xyz, std::vector<omesh::Pnt3>& pts_normal);
	bool ReadTransformResult(Xform<float>& xf, string fileName);

	//bool transPointXF(const std::string& xf, std::vector<XYZ>& pts_xyz, std::vector<NORMAL>& pts_normal);

	Matrix<double> project2oulu(const vector <omesh::Pnt3> &vtx,const Matrix<double> &r,const Matrix<double> &t,double* fc,double* cc,double* kc);
	omesh::Pnt3 BilInter(float x, float y, const Matrix<float> &R, const Matrix<float> &G, const Matrix<float> &B);
	omesh::Pnt3 GetPointRGB(int x,int y,const Matrix<float> &R, const Matrix<float> &G, const Matrix<float> &B);
	bool GetKFC( double *kc,double *fc,double *cc,Matrix<double> KK_right_l2m,Matrix<double> kc_right_l2m);
	void getPreData(const std::string& path, Matrix<float> &R, Matrix<float> &G, Matrix<float> &B, 
		Xform<float>& XF, Matrix<double>& KK, Matrix<double>& KC, Matrix<double>& RM, Matrix<double>& TM);


	int getNumber( void )
	{
		return mNumber;
	}

	Mesh* getMesh( void )
	{
		return mMesh;
	}

	const Xform<float>& getXform( void )
	{
		 return mXform;
	}

	/// 初始化数据
	void initData( void );

	/// 初始化点数据
	void initPoints( void );

	/// 初始化三角形数据
	void initTriangles( void );


	/// 预先创建区域面积
	void makeTriangleArea( void );

	const PointPropertys& getPointPropertys( void ) const
	{
		return mPointPropertys;
	}

	PointPropertys& getPointPropertys( void )
	{
		return mPointPropertys;
	}

	const Triangles& getTriangles( void ) const
	{
		return mTriangles;
	}

	Triangles& getTriangles( void )
	{
		return mTriangles;
	}

	/// 得到边界重叠点数据
	std::vector<omesh::Pnt3>& getPntBorders( void )
	{
		return mPntBorders;
	}

	VectorIndex& getPntBorderIndexs( void )
	{
		return mPntBorderIndexs;
	}

	/*VecNewPoints& getNewPoints( void )
	{
		return mNewPoints;
	}*/

	/// 保存成wrl文件
	/// fileName 要保存的文件名
	/// 返回 true 保存成功 false 保存失败
	bool saveWrl(const std::string& fileName);

	/// 读取wrl文件
	/// fileName 要保存的文件名
	/// 返回 true 读取成功 false 读取失败
	bool readWrl( const std::string& fileName );

	/// 合并另外一个点云
	bool mergeOther(Cloud* pOther);

	/// 根据得到的信息删除mesh的点和面信息
	/// 返回 true ,有删除点和面并且删除成功
	/// 返回 false 没有可删除的东西
	bool deleteTriVtxFromMesh( void );

	/// 设置特别点的特别颜色
	/// type ,类型
	void makeTestColor(const omesh::Pnt3& color);

	/// 删除重复点
	void deleteSamePoint( void );

	/// 取得某个点的另一个边界点
	/// int curIndex 当前点
	/// int lastIndex 前一个点
	int getNextBorderPoint( int curIndex, int lastIndex );

	/// 向点云中增加点
	/// pos 这个点的位置
	/// return 增加的这个点在点云中的索引
	int addPoint(const Ogre::Vector3& pos);

	/// 向点云中增加三角形
	/// index1 这个三角形的第一个点索引
	/// index2 这个三角形的第二个点索引
	/// index3 这个三角形的第三个点索引
	int addTriangle( int index1, int index2, int index3);

	/// 取得滞后三角形列表
	VecTriInt& getVecTriInt( void ) { return mVecTriInt; }
	const VecTriInt& getVecTriInt( void ) const { return mVecTriInt; }

	/// 这个重建滞后三角形的点和三角形关系
	/// size 点云1的尺寸
	/// vecTri 新增加的三角形关系
	void buildVecTriInt( int size, const VecTriInt& vecTri );

	/// 重建模型
	void reBuildData( void );
	
	/// 把点云里的一个点换成另外一个点
	/// src 被替换的点
	/// target 要替换成的点
	/// size 需要做的偏移
	void replacePoint( int src, int target, int size);

	/// 删除合并时候产生的重复点
	void deleteSameProducePoint( int size );

	VecLinkInt& getLinkInts( void )
	{
		return mLinkInts;
	}

	/// 查找共同点的三角形
	void findSamePoints( void );

	/// 把合并的三角形复制进列表里
	/// size 云图点偏移位置
	void makeMergeTri( int size );

	/// 替换合并需要替换的点
	void makeMergePoint( int size );

	void addLinePoint(int line1, int line2)
	{
		mMegerKeyPoint.push_back(line1);
		mMegerKeyPoint.push_back(line2);
	}

	void addReplacePoints(int index )
	{
		mReplacePoints.push_back(index);
	}

	/// 清理数据
	void clear();

	//VectorIndex  mHostPointIndex;	/// 主点云在客点云上最近点的索引
	VectorIndex& getHostPointIndex( void )
	{
		return mHostPointIndex;
	}
	//VectorFloat mHostPointLenght;	/// 主点云到客点云最近点的距离
	VectorFloat& getHostPointLenght( void )
	{
		return mHostPointLenght;
	}

	/// 投影和投影点信息
	//MapPointProject mMapPointProperty;
	MapPointProject& getMapPointProperty( void )
	{
		return mMapPointProperty;
	}

	VectorIndex& getBordersFromDelete( void )
	{
		return mBordersFromDelete;
	}

	//KDTree *mKdtree;


	/// 取得这个点的权重信息，在做颜色过渡的时候用到
	float getPointWeight( int index ) const;

	bool compPoint(const int& a, const int&b);

	void reserve( int size, int triSize)
	{
		mTriangles.reserve(triSize);
		mPointPropertys.reserve(size);
		if (mMesh)
		{
			mMesh->vtx.reserve(size);
			mMesh->vtxRgb.reserve(size);
			mMesh->nrm.reserve(size);
			mMesh->tris.reserve(triSize);
		}
	}

	void saveCurDeletePointStatue( int number )
	{
		char buf[32] = {0};
		sprintf(buf, "%dpointDelete.txt", number);
		FILE* f = fopen(buf, "w");
		if ( f == NULL)
		{
			return;
		}
		for (int i = 0; i < mPointPropertys.size(); i ++)
		{
			fprintf(f, "%d id:%d %d\n", i, mPointPropertys.at(i).getIndex(), mPointPropertys.at(i).getNeedDelete());
		}
		fclose(f);
	}

	void getFromOtherMesh( Cloud* pCloud );

	/// 保存mesh信息
	bool saveMesh( const std::string& fileName );

	/// 计算平均长度
	void makeAverageEdgeLenght( void );

	/// 取得这个点相邻点的中点，用来算最近点
	omesh::Pnt3 getMidBorderPos(int index);

	/// 取得这个点相邻点的中点，用来算最近点
	/// 平滑边缘
	omesh::Pnt3 getSmoothBorderPos(int index, float f = 0.5);

	/// 有一个邻居点不是投影点
	bool getAnNeighborNotPropertyPoint( int index)
	{
		if (mPointPropertys.at(index).getPropertyPoint())
		{
			return false;
		}
		const VectorIndex& vecInts = mPointPropertys.at(index).getBorderList();
		for (int i = 0; i < vecInts.size(); i ++)
		{
			if (!mPointPropertys.at(i).getNeedDelete() && !mPointPropertys.at(i).getPropertyPoint())
			{
				return true;
			}
		}
		return false;
	}

	PointBorderLink& getPointBorderLink( void )
	{
		return mPointBorderLink;
	}

	Holes& getHoles( void )
	{
		return mHoles;
	}

	VectorIndex& getmWrongPoints( void )
	{
		return mWrongPoints;
	}

	void setMesh( omesh::Mesh* p)
	{
		if (p != NULL)
		{
			if (mMesh)
			{
				delete mMesh;
			}
			mMesh = p;;
		}
	}

	/// 取得下一个顶点
	int getNextBorderIndex(int index, int last = -1);

protected:
	/// 对points里的点属性进行更新，把所有的三角形属性加triSize，所有点属性加pointSize
	/// points 点属性集合
	/// triSize 三角形偏移尺寸
	/// pointSize 点偏移尺寸
	void updatePoints(PointPropertys& points, int triSize, int pointSize) const;

	/// 对相同点的集合vec删除只剩1个
	/// vec 相同的点的集合
	/// size 点尺寸
	void replaceVec( VectorIndex& vec, int size);

	/// 对两个点云合并后的错误点进行合并
	void megerKeyPoints( int size );

	/// 通过算点的方式求得是否是边界三角形
	bool isBuildBorderPoint( int index );

	/// 对新生成的点重建边界关系
	/// 这个时候边界也许是不准确的，但是三角形关系还是准确的
	void reBuildKeyPoints( int size );

	/// 错误点修复，算法是把相邻的都是边界点的点合成一个点
	void repairWrongPoints( void );

	/// 将n个点合成一个点，然后这个最终点平均这个点的位置
	void makeVecPointCenter( const VectorIndex& vec);

	/// 重新建立这些点的关系
	void rebuildVecPoints(VectorIndex& v, int size);

	void readLineData( int state, FILE* file );

	/// 删除mesh里的相同点
	/// 需要在数据初始化前或者在数据导出时候做这步，不然出错
	void deleteMeshSamePoint( void );

	/// 删除合成两个云图点的时候产生的相同点
	void deleteSamePointMerge( void );
	public:
	/// 构造边和洞的关系
	void initHoles( void );

private:
	/// 数据的序号
	int mNumber;

	//float mAverageEdgeLenght;		/// 平均边长长度

	std::string mErrorString;		/// 错误描述

	/// 模型数据
	Mesh* mMesh;

	//int mMaxLevel;		/// 最大层数
	//int mMinLevel;		/// 最小层数

	Xform<float> mXform;

	///// 颜色
	//Matrix<float> mImageR;
	//Matrix<float> mImageG;
	//Matrix<float> mImageB;

	/// 边界重叠点数据
	std::vector<omesh::Pnt3> mPntBorders;

	/// 边界重叠点在原来的位置
	/// 这个设计有问题
	VectorIndex mPntBorderIndexs;

	PointPropertys mPointPropertys;		/// 点属性列表
	Triangles mTriangles;		/// 面列表

	/// 增加的三角形列表
	VecTriInt mVecTriInt;

	VecLinkInt mLinkInts;

	/// 合并后的边界点
	VectorIndex mMegerKeyPoint;

	VectorIndex mReplacePoints;
	
	/// 错误点集合，即修复后还是边界点的顽固点
	VectorIndex mWrongPoints;

	/// 因为删除产生的边界点
	VectorIndex mBordersFromDelete;

	PointBorderLink mPointBorderLink;
	Holes mHoles;

	///-------------------
	/// 从计算类里拿过来的数据
	VectorIndex  mHostPointIndex;	/// 主点云在客点云上最近点的索引
	VectorFloat mHostPointLenght;	/// 主点云到客点云最近点的距离

	/// 投影和投影点信息
	MapPointProject mMapPointProperty;

	//KDTree *mKdtree;

};

#endif