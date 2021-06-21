#include "Cloud.h"
#include "LogMessage.h"


//#include "MeshType.h"
// 
// Cloud::Cloud(void)
// {
// }


// Cloud::~Cloud(void)
// {
// }

/// 是否使用补洞
//#define FILL_HOLE

//#define DontReadBmp 0

//#ifdef FILL_HOLE
//#include "RepairHoleDll.h"
//#pragma comment(lib, "RepairHoleDll.lib")
//#endif

//#define USE_PCD
/// pcd 模块
#ifdef USE_PCD
#include "simple_cloud.h"
#pragma comment(lib, "SimpleCloud.lib")
#endif

bool Cloud::readData( const std::string& path, int subSamp/*=1*/ )
{
	//Matrix<float> R; Matrix<float> G; Matrix<float> B; 
	//Xform<float> XF; Matrix<double> KK; Matrix<double> KC; 
	//Matrix<double> RM; Matrix<double> TM;
	//getPreData(path, R, G, B, XF, KK, KC, RM, TM);


	//readData(path, path + ".roc", R, G, B, XF, KK, KC, RM, TM, subSamp);
	//return true; 

	std::string str;
	printf("read data %s\n", path.c_str());
	logv("read data %s", path.c_str());

	/// 颜色
	Matrix<float> mImageR;
	Matrix<float> mImageG;
	Matrix<float> mImageB;

	/// 读取数据矩阵
	str = path + File_Txt;
	if(	!readTxtfrom(mXform, str.c_str()) )
	{	
		/// 多重读取标准
		str = path + File_xf;
		if (!readXform(mXform, str.c_str()) )
		{
			mErrorString += str;
			mErrorString += " 读取失败 ;";
			log1(mErrorString.c_str());
			return false;
		}
	}

	/// 读取图片信息
#ifndef DontReadBmp
	str = path + File_Bmp;
	if( !readFromBmp(&mImageR, &mImageG, &mImageB, str.c_str()) )
	{
		mErrorString += str;
		mErrorString += " 读取失败 ;";
		log1(mErrorString.c_str());
		return false;
	}
#endif

	/// 读取模型
	Matrix<int> _P;
	Matrix<float> _X;
	Matrix<float> _Y;
	Matrix<float> _Z;
	float _interstep;
	float _x0;
	float _y0;
	str = path + File_roc;
	if(!omesh::readPIF(str.c_str(), _P, _X, _Y, _Z, _interstep, _x0, _y0))
	{
		mErrorString += str;
		mErrorString += " 读取失败 ;";
		log1(mErrorString.c_str());
		return false;
	}

	/// 使用补洞
#ifdef FILL_HOLE
	PRO(_P, _X, _Y, _Z);
#endif

	RangeGrid rg(mImageR, mImageG, mImageB,_P, _X, _Y, _Z, _interstep, _x0, _y0);
	if (mMesh)
	{
		delete mMesh;
		mMesh = NULL;
	}

#ifdef DontReadBmp
	mMesh = rg.toMesh(subSamp); if (mMesh == NULL) return false;
#else
	mMesh = rg.toMesh_RGB(subSamp); if (mMesh == NULL) return false;
#endif

	mMesh->remove_stepedges_auto();
	mMesh->remove_unused_vtxs();	

	float in[3],out[3];
	int vtxSize = mMesh->vtx.size();

	Xform<float>& xf = mXform;
	//#pragma omp parallel for   不知道为什么，这里不能多核
	for(int i=0 ; i<vtxSize ; i++)
	{

		in[0] = mMesh->vtx[i][0]; in[1] = mMesh->vtx[i][1]; in[2] = mMesh->vtx[i][2];
		xf.apply(in, out);
		mMesh->vtx[i][0] = out[0]; mMesh->vtx[i][1] = out[1]; mMesh->vtx[i][2] = out[2];

		in[0] = mMesh->nrm[i][0]; in[1] = mMesh->nrm[i][1]; in[2] = mMesh->nrm[i][2];
		xf.apply_nrm(in, out);
		mMesh->nrm[i][0] = out[0]; mMesh->nrm[i][1] = out[1]; mMesh->nrm[i][2] = out[2];
	}

	////转置向量(0,0,1) 用于做法向权重的时候计算夹角用
	in[0] = 0; in[1] = 0; in[2] = 1;
	xf.apply_nrm(in, out);
	mMesh->transZ[0] = out[0]; mMesh->transZ[1] = out[1]; mMesh->transZ[2] = out[2];
	mNumber = path.at(path.size()-1) - '0';
#ifdef DontReadBmp
	mMesh->vtxRgb.resize(mMesh->vtx.size());
#endif
	return true;
}

#ifdef USE_PCD
bool transPointXF(const std::string& xf, std::vector<XYZ>& pts_xyz, std::vector<NORMAL>& pts_normal);

bool Cloud::readDataPCD( const std::string& path, int subSamp/*=1*/ )
{
	printf("read pcd %s\n", path.c_str());
	std::vector<XYZ> pts_xyz; std::vector<RGB> pts_rgb; std::vector<NORMAL> pts_normal;
	read_cloud(pts_xyz, pts_rgb, pts_normal,		path);
	printf("points=%d normal=%d rbg=%d\n", pts_xyz.size(), pts_normal.size(), pts_normal.size());

	//transPointXF(path+".xf", pts_xyz, pts_normal);

	if (mMesh) delete mMesh;
	mMesh = new Mesh;
	mMesh->vtx.resize(pts_xyz.size());
//#pragma omp parallel for
	for (int i = 0; i < pts_xyz.size(); i ++)
	{
		mMesh->vtx[i][0] = pts_xyz[i].x;
		mMesh->vtx[i][1] = pts_xyz[i].y;
		mMesh->vtx[i][2] = pts_xyz[i].z;
	}

	mMesh->nrm.resize(pts_normal.size());
	//#pragma omp parallel for
	for (int i = 0; i < pts_normal.size(); i ++)
	{
		mMesh->nrm[i][0] = pts_normal[i].nx;
		mMesh->nrm[i][1] = pts_normal[i].ny;
		mMesh->nrm[i][2] = pts_normal[i].nz;
	}

	mMesh->vtxRgb.resize(pts_rgb.size());
	//#pragma omp parallel for
	for (int i = 0; i < pts_rgb.size(); i ++)
	{
		mMesh->vtxRgb[i][0] = pts_rgb[i].r;
		mMesh->vtxRgb[i][1] = pts_rgb[i].g;
		mMesh->vtxRgb[i][2] = pts_rgb[i].b;
	}

	transPointXF(path+".xf", mMesh->vtx, mMesh->nrm);
	return true;
}

bool Cloud::ReadTransformResult( Xform<float>& xf, string fileName )
{
	ifstream ifs(fileName);
	if(!ifs){
		ifs.close();
		return false;
	}

	float r[3][3], t[3];
	string bufline;	// read line buffer
	int lineIndex = 0;
	while(ifs.good()){
		getline(ifs, bufline, '\n');
		if(lineIndex<3 && bufline.length()!=0){
			int idx = -1;
			int lidx = 0;
			lidx = idx;	idx = int(bufline.find(' ', lidx+1));
			r[lineIndex][0] = float(atof((bufline.substr(lidx+1,idx-lidx-1)).c_str()));
			lidx = idx;	idx = int(bufline.find(' ', lidx+1));
			r[lineIndex][1] = float(atof((bufline.substr(lidx+1,idx-lidx-1)).c_str()));
			lidx = idx;	idx = int(bufline.find(' ', lidx+1));
			r[lineIndex][2] = float(atof((bufline.substr(lidx+1,idx-lidx-1)).c_str()));
			lidx = idx;	idx = int(bufline.find(' ', lidx+1));
			t[lineIndex] = float(atof((bufline.substr(lidx+1,idx-lidx-1)).c_str()));
		}
		lineIndex ++;
	}
	ifs.close();

	xf = Xform<float>(r, t);

	return true;
}

void transXF(Xform<float>& xf)
{
	std::swap(xf.m[1][0], xf.m[0][1]);
	std::swap(xf.m[2][0], xf.m[0][2]);
	std::swap(xf.m[2][1], xf.m[1][2]);
}

bool transPointXF( const std::string& xf, std::vector<XYZ>& pts_xyz, std::vector<NORMAL>& pts_normal )
{
	Xform<float> tform;
	if(	!readTxtfrom(tform, xf.c_str()) )
	{
		printf(" can not load xf %s\n", xf.c_str());
		return false;
	}
	transXF(tform);
	float in[3],out[3];
	#pragma omp parallel for
	for (int i = 0; i < pts_xyz.size(); i ++)
	{
		in[0] = pts_xyz[i].x;
		in[1] = pts_xyz[i].y;
		in[2] = pts_xyz[i].z;
		tform.apply(in, out);
		pts_xyz[i].x = out[0];
		pts_xyz[i].y = out[1];
		pts_xyz[i].z = out[2];
	}

#pragma omp parallel for
	for (int i = 0; i < pts_normal.size(); i ++)
	{
		in[0] = pts_normal[i].nx;
		in[1] = pts_normal[i].ny;
		in[2] = pts_normal[i].nz;
		tform.apply_nrm(in, out);
		pts_normal[i].nx = out[0];
		pts_normal[i].ny = out[1];
		pts_normal[i].nz = out[2];
	}
	return true;
}

bool Cloud::transPointXF(const std::string& xf, std::vector<omesh::Pnt3>& pts_xyz, std::vector<omesh::Pnt3>& pts_normal)
{
	Xform<float> tform;
	//if(	!readTxtfrom(tform, xf.c_str()) )
	//{
	//	printf(" can not load xf %s\n", xf.c_str());
	//	return false;
	//}
	//transXF(tform);
	ReadTransformResult(tform, xf);
	float in[3],out[3];
//#pragma omp parallel for
	for (int i = 0; i < pts_xyz.size(); i ++)
	{
		in[0] = pts_xyz[i][0];
		in[1] = pts_xyz[i][1];
		in[2] = pts_xyz[i][2];
		tform.apply(in, out);
		pts_xyz[i][0] = out[0];
		pts_xyz[i][1] = out[1];
		pts_xyz[i][2] = out[2];
	}

//#pragma omp parallel for
	for (int i = 0; i < pts_normal.size(); i ++)
	{
		in[0] = pts_normal[i][0];
		in[1] = pts_normal[i][1];
		in[2] = pts_normal[i][2];
		tform.apply_nrm(in, out);
		pts_normal[i][0] = out[0];
		pts_normal[i][1] = out[1];
		pts_normal[i][2] = out[2];
	}
	return true;
}
#endif
void Cloud::initData( void )
{
	initPoints();   //不存点信息
	initTriangles();
	makeTriangleArea();
}


void Cloud::reBuildData( void )
{
	clear();
	initData();
}


void Cloud::initPoints( void )
{
	if ( mMesh == NULL)
		return;
	int size = mMesh->vtx.size();
	mPointPropertys.resize(size);

	assert(mMesh->vtx.size() == mMesh->nrm.size());
	
	#pragma omp parallel for
	for (int i = 0; i < size; i ++)
	{
		PointProperty& point = mPointPropertys[i];
		point.mIndex = i;
	}
}

void Cloud::initTriangles( void )
{
	if ( mMesh == NULL) return;
	int size = mMesh->tris.size();
	mTriangles.resize(size);

	unsigned int pointSize = mPointPropertys.size();
	#pragma omp parallel for
	for ( int i = 0; i < size; i++)
	{
		ESTriangle& t = mTriangles[i];
		t.mIndex = i;
		t.mPoints[0] = mMesh->tris.at(i)[0];
		t.mPoints[1] = mMesh->tris.at(i)[1];
		t.mPoints[2] = mMesh->tris.at(i)[2];
		//mTriangles.push_back(t);
	}

	/// 记录点所属面
	for (int i = 0; i < size; i ++)
	{
		ESTriangle& t = mTriangles[i];
		assert(t.mPoints[0] < pointSize && t.mPoints[1] < pointSize && t.mPoints[2] < pointSize);
		mPointPropertys.at(t.mPoints[0]).makeBelongTri(i);
		mPointPropertys.at(t.mPoints[1]).makeBelongTri(i);
		mPointPropertys.at(t.mPoints[2]).makeBelongTri(i);

		/// 记录一环点
		mPointPropertys.at(t.mPoints[0]).addBorderList(t.mPoints[1], t.mPoints[2]);
		mPointPropertys.at(t.mPoints[1]).addBorderList(t.mPoints[0], t.mPoints[2]);
		mPointPropertys.at(t.mPoints[2]).addBorderList(t.mPoints[0], t.mPoints[1]);
	}

	/// 建立点的边界信息

	#pragma omp parallel for
	for (int i = 0; i < mPointPropertys.size(); i ++)
	{
		//it1->buildBorder();
		mPointPropertys.at(i).buildBorder();
	}
}

/// 相当于原函数的GetVertexNormals_Area
void Cloud::makeTriangleArea( void )
{
	#pragma omp parallel for
	for (int i = 0; i < mTriangles.size(); i ++)
	{
		//it1->makeArea(mPointPropertys);
		//mTriangles.at(i).makeArea(mPointPropertys);
		mTriangles.at(i).makeArea(mMesh->vtx);
	}

	/// 根据面积算权重关系
	#pragma omp parallel for
	for (int i = 0; i < mTriangles.size(); i ++)
	{
		//it1->processArea(mPointPropertys);
		mTriangles.at(i).processArea(mPointPropertys);
	}

	/// 最后进行标准化
	#pragma omp parallel for
	for (int i = 0; i < mPointPropertys.size(); i ++)
	{
		//itt1->endAreaNormal();
		mPointPropertys.at(i).endAreaNormal();
	}
}

bool Cloud::saveWrl( const std::string& fileName )
{
	FILE* f = fopen(fileName.c_str(), "w");
	if (f == NULL || mMesh == NULL || mMesh->vtx.empty() || mMesh->vtxRgb.empty())
	{
		return false;
	}

	/// 写头数据
	fprintf(f, wrlFileHeader);
	/// 写点数据
	fprintf( f, "				coord DEF coord0 Coordinate\n				{\n					point\n						[\n");
	for ( int i = 0; i < mMesh->vtx.size(); i ++)
	{
		omesh::Pnt3& pnt = mMesh->vtx.at(i);
		fprintf(f, "							%f %f %f,\n", pnt[0], pnt[1], pnt[2]);
	}
	fprintf(f, "						]\n				}\n");

	/// 写颜色列表
	fprintf(f, "			color DEF color0 Color {\n					color\n						[\n");
	for ( int i = 0; i < mMesh->vtxRgb.size(); i ++)
	{
		omesh::Pnt3& pnt = mMesh->vtxRgb.at(i);
		fprintf(f, "							%f %f %f,\n", pnt[0], pnt[1], pnt[2]);
	}
	fprintf(f, "						]\n				}\n");

	/// 写三角形信息
	fprintf(f, "				coordIndex\n						[\n");
	for ( int i = 0; i < mMesh->tris.size(); i ++)
	{
		omesh::TriVtx& trs = mMesh->tris.at(i);
		fprintf(f, "							%d, %d, %d, %d,\n", trs[0], trs[1], trs[2], -1);
	}
	fprintf(f, "						]\n");

	/// 写文件尾
	fprintf(f, wrlFileTail);
	fclose(f);
	//fileName.
	
	return true;
}

bool Cloud::mergeOther( Cloud* pOther )
{
	if ( pOther == NULL)
	{
		return false;
	}
	Mesh* pOtherMesh = pOther->getMesh();

	int size = mMesh->vtx.size();
	int triSize = mMesh->tris.size();


	/// 点拷贝
	//mMesh->vtx.reserve(mMesh->vtx.size() + pOtherMesh->vtx.size());
	//mMesh->vtx.insert(mMesh->vtx.end(), pOtherMesh->vtx.begin(), pOtherMesh->vtx.end());
	mMesh->vtx.resize(mMesh->vtx.size() + pOtherMesh->vtx.size());
	#pragma omp parallel for
	for (int i = 0; i < pOtherMesh->vtx.size(); i ++)
	{
		mMesh->vtx.at(size + i) = pOtherMesh->vtx.at(i);
	}

	/// 法线拷贝
	//mMesh->nrm.reserve(mMesh->nrm.size() + pOtherMesh->nrm.size());
	//mMesh->nrm.insert(mMesh->nrm.end(), pOtherMesh->nrm.begin(), pOtherMesh->nrm.end());
	mMesh->nrm.resize(mMesh->nrm.size() + pOtherMesh->nrm.size());
	#pragma omp parallel for
	for (int i = 0; i < pOtherMesh->nrm.size(); i ++)
	{
		mMesh->nrm.at(size + i) = pOtherMesh->nrm.at(i);
	}

	/// 颜色拷贝
	//mMesh->vtxRgb.reserve(mMesh->vtxRgb.size() + pOtherMesh->vtxRgb.size());
	//mMesh->vtxRgb.insert(mMesh->vtxRgb.end(), pOtherMesh->vtxRgb.begin(), pOtherMesh->vtxRgb.end());
	mMesh->vtxRgb.resize(mMesh->vtxRgb.size() + pOtherMesh->vtxRgb.size());
	#pragma omp parallel for
	for (int i = 0; i < pOtherMesh->vtxRgb.size(); i ++)
	{
		mMesh->vtxRgb.at(size + i) = pOtherMesh->vtxRgb.at(i);
	}

	/// 三角形拷贝
	mMesh->tris.reserve(mMesh->tris.size()+ pOtherMesh->tris.size());

	/// 点属性增加
	PointPropertys pointPros;
	pointPros.assign(pOther->getPointPropertys().begin(), pOther->getPointPropertys().end());
	//std::for_each(pointPros.begin(), pointPros.end(), std::bind2nd(std::mem_fun_ref(&PointProperty::addIndexBy), size));
	updatePoints(pointPros, triSize, size);
	//mPointPropertys.insert(mPointPropertys.end(), pointPros.begin(), pointPros.end());
	mPointPropertys.resize(mPointPropertys.size() + pointPros.size());
	#pragma omp parallel for
	for (int i = 0; i < pointPros.size(); i ++)
	{
		mPointPropertys.at(size + i) = pointPros.at(i);
	}

	//omesh::TriVtx tri;
	//Triangles otherTriangles;
	//otherTriangles.reserve(pOther->getTriangles().size());
	
	mMesh->tris.resize(triSize + pOtherMesh->tris.size());
	mTriangles.resize(mTriangles.size() + pOtherMesh->tris.size());
	#pragma omp parallel for  
	for ( int i = 0; i < pOtherMesh->tris.size(); i ++)
	{
		const ESTriangle& triangle = pOther->getTriangles().at(i);
		omesh::TriVtx& tri = mMesh->tris[triSize + i];
		tri[0] = triangle.mPoints[0] + size;
		tri[1] = triangle.mPoints[1] + size;
		tri[2] = triangle.mPoints[2] + size;
		//mMesh->tris.push_back(tri);
		//mMesh->tris[triSize + i] = tri;
	}
	//Triangles& trias = pOther->getTriangles();
	//#pragma omp parallel for //这里居然不能多核
	for (int i = 0; i < pOther->getTriangles().size(); i ++)
	{
		ESTriangle& triangle = mTriangles[triSize + i];
		triangle = pOther->getTriangles().at(i);
		triangle.mIndex = i + triSize;
		triangle.mPoints[0] += size;
		triangle.mPoints[1] += size;
		triangle.mPoints[2] += size;

		//mTriangles[triSize + i] = triangle;
	}

	makeMergeTri(size);
	makeMergePoint(size);

	/// 边界点信息重构
	/// 边界重构在模型重新初始化完成
	//reBuildKeyPoints(size);
	
	//megerKeyPoints(size);
	//deleteSamePointMerge();
	//repairWrongPoints();
	return true;
}

bool Cloud::deleteTriVtxFromMesh( void )
{
	std::vector<omesh::Pnt3> vtx;		/// 过渡点信息
	std::vector<omesh::Pnt3> vtxRgb;		/// 过渡颜色信息
	std::vector<omesh::Pnt3> nrm;
	std::vector<omesh::TriVtx> tris;
	PointPropertys pointPros;				/// 过渡属性信息
	//Triangles triangles;
	PointProperty tPointPro;
	int curSum = 0;
	int vtxSize = mMesh->vtx.size();
	int* pNumber = new int[mMesh->vtx.size()];

	//std::fill(pNumber, pNumber+mMesh->vtx.size(), -1);
	#pragma omp parallel for
	for (int i = 0; i < mMesh->vtx.size(); i ++)
	{
		pNumber[i] = -1;
	}

	vtx.reserve(mMesh->vtx.size());
	nrm.reserve(mMesh->nrm.size());
	int max = pointPros.max_size();
	pointPros.reserve(mMesh->vtx.size());

	for ( int i = 0; i < mMesh->vtx.size(); i ++)
	{
		if (mPointPropertys.at(i).getIndex() == i)
		{
			if (!mPointPropertys.at(i).getNeedDelete())		/// 非删除对象
			{
				vtx.push_back(mMesh->vtx.at(i));	/// 把新对象入栈
				vtxRgb.push_back(mMesh->vtxRgb.at(i));	/// 新颜色入栈
				nrm.push_back(mMesh->nrm.at(i));
				tPointPro = mPointPropertys.at(i);
				tPointPro.mIndex = i;
				pointPros.push_back(tPointPro);
				pNumber[i] = curSum++;

			}
			else	/// 是删除对象
			{
				//logv("old %d need delete", i);
			}
		}
		else
		{
			//logv("the id:%d != i:%d", mPointPropertys.at(i).getIndex(), i);
		}
	}
	mMesh->vtx.swap(vtx);
	mMesh->vtxRgb.swap(vtxRgb);
	mMesh->nrm.swap(nrm);
	mPointPropertys.swap(pointPros);

	//omesh::TriVtx tri;
	tris.reserve(mMesh->tris.size());
	//triangles.resize(mMesh->tris.size());
	ESTriangle ttriangle;
	//#pragma omp parallel for
	for ( int i = 0; i < mMesh->tris.size(); i ++)
	{
		if (mTriangles.at(i).getIndex() == i)
		{
			if (!mTriangles.at(i).getNeedDelete())	/// 非删除对象
			{
				/// 以记录的为标准，在移点的时候只移了记录的
				omesh::TriVtx& tempTri = mMesh->tris.at(i);
				omesh::TriVtx tri;
				ESTriangle& tempTriangle = mTriangles.at(i);
				tri[0] = pNumber[ mTriangles.at(i).mPoints[0] ];
				tri[1] = pNumber[ mTriangles.at(i).mPoints[1] ];
				tri[2] = pNumber[ mTriangles.at(i).mPoints[2] ];				

				if (tri[0] == -1 || tri[1] == -1 || tri[2] == -1 || (tri[0]==0 && tri[1]==0&&tri[2]==0))
				{
					logv("%d:%d %d %d -> %d %d %d", i, tempTriangle.mPoints[0], tempTriangle.mPoints[1], tempTriangle.mPoints[2], tri[0], tri[1], tri[2]);
					continue;
				}
				tris.push_back(tri);
			}
			else
			{
				//logv("the %d tri need to delete", i);
			}
		}
		else
		{
			//logv("the tri id:%d != i:%d", mTriangles.at(i).getIndex(), i);
		}
	}
	///logv("end vtx number %d, tris number %d", mMesh->vtx.size(), mMesh->tris.size());
	mMesh->tris.swap(tris);
	//mTriangles.swap(triangles);

	delete[] pNumber;
	return true;
}

void Cloud::makeTestColor( const omesh::Pnt3& color )
{
	PointPropertys::iterator it1, it2;
	it1 = mPointPropertys.begin();
	it2 = mPointPropertys.end();
	for ( ; it1 != it2; ++it1)
	{
		//if (it1->isBorderPoint() && it1->getNeedDelete()) /// 既是边，也需要删除的
		//{
		//	mMesh->vtxRgb[ it1->getIndex() ] = omesh::Pnt3(1.0f, 1.0f, 1.0f);
		//}
		//if (it1->isBorderPoint())
		//{
		//	mMesh->vtxRgb[ it1->getIndex() ] = omesh::Pnt3(1.0f, 0.0f, 0.0f);
		//}
		if (it1->isBorderPoint())
		{
			mMesh->vtxRgb[ it1->getIndex() ] = color;
		}
		/*if (it1->is)
		{
		}*/
		/// 按层数上颜色
		//if (it1->getLevelLayer() > 0)
		//mMesh->vtx[ it1->getIndex() ] = omesh::Pnt3(0.0f, 0.0f, 1.0f)*float(it1->getLevelLayer()/30);
	}
}

bool cmp( const omesh::Pnt3& m1, const omesh::Pnt3& m2 ) 
{
	bool b = false;

	if(m1[0] > m2[0])
		b = true;
	else if(m1[0] == m2[0])
		if(m1[1] > m2[1])
			b = true;
		else if(m1[1] == m2[1])
			if(m1[2] > m2[2])
				b = true;
			else
				b = false;

	return b;
} 

struct T
{
	int index;
	int count;
	T():index(0),count(){}
};
void Cloud::deleteSamePoint( void )
{
	std::vector<omesh::Pnt3> pnts;
	pnts.assign(mMesh->vtx.begin(), mMesh->vtx.end());
	std::sort(pnts.begin(), pnts.end(), cmp);
	int lastIndex = 0;

	T t;
	t.index = -1;
	t.count = 0;
	std::vector<T> intpnt;
	omesh::Pnt3 pnt;
	for (int i = 0; i < pnts.size(); i ++)
	{
		if (pnt != pnts[i])
		{
			if (t.count > 1)
			{
				intpnt.push_back(t);
			}			
			t.index = i;
			t.count = 0;
			pnt = pnts[i];
		}
		else
			t.count ++;
	}
	
	char buf[32] = {0};
	sprintf(buf, "%d_point.txt", mNumber);
	FILE* f = fopen(buf, "w");
	for (int i = 0; i < pnts.size(); i ++)
	{
		fprintf(f, "%f	%f	%f\n", pnts[i][0], pnts[i][1], pnts[i][2]);
	}
	fclose(f);
}

int Cloud::getNextBorderPoint( int curIndex, int lastIndex )
{
	assert(curIndex > 0 && curIndex < mPointPropertys.size());
	//assert(lastIndex > 0 && lastIndex < mPointPropertys.size());
	const VectorIndex& vec = mPointPropertys.at(curIndex).getBorderList();
	
	//VectorIndex::const_iterator it1, it2;
	//it1 = vec.begin();
	//it2 = vec.end();

	//for (; it1 != it2; it1++)
	//{
	//	if (*it1 == lastIndex)
	//	{
	//		/// 每个点有两个单独的点
	//		continue;
	//	}
	//	if (mPointPropertys.at(*it1).isBorderPoint() && !mPointPropertys.at(*it1).getNeedDelete())
	//	{
	//		return (*it1);
	//	}
	//}
	for (int i = 0; i < vec.size(); i ++)
	{
		if (lastIndex == vec[i])
		{
			continue;
		}
		const PointProperty& pp = mPointPropertys.at(vec[i]);

		if (pp.isBorderPoint() && !pp.getNeedDelete())
		{
			return vec[i];
		}
	}

	return -1;
}

int Cloud::addPoint( const Ogre::Vector3& pos )
{
	return 0;		/// 好像暂时不需要增加点
					/// 都是替换点
	omesh::Pnt3 pnt;
	Tools::copyVector3ToPnt(pnt, pos);
	mMesh->vtx.push_back(pnt);
}

int Cloud::addTriangle( int index1, int index2, int index3 )
{
	mVecTriInt.push_back(TriInt(index1, index2, index3, 0));
	return 0;
}

void Cloud::buildVecTriInt( int size, const VecTriInt& vecTri )
{
	/// 废弃函数
	VecTriInt::const_iterator it1, it2;
	it1 = vecTri.begin();
	it2 = vecTri.end();
	int nIndex = 0;
	int index2 = 0;
	int index3 = 0;
	for (; it1 != it2; ++it1)
	{
		const TriInt& tri = (*it1);
		nIndex = tri.index1 + size;
		index2 = tri.index2>0?tri.index2:(tri.index2+size);
		index3 = tri.index3>0?tri.index3:(tri.index3+size);
		mPointPropertys.at(nIndex).makeBelongTri(tri.triIndex);
		mPointPropertys.at(nIndex).addBorderList(index2, index3);
	}
}

void Cloud::replacePoint( int src, int target, int size )
{
	PointPropertys& pointPros = mPointPropertys;
	assert(src < pointPros.size() && src != target);
	if(pointPros.at(target).getNeedDelete()) return;
	if(pointPros.at(src).getNeedDelete()) return;
	const VectorIndex& vecTris = pointPros.at(src).getBelongTri();
	VectorIndex::const_iterator it1, it2;
	it1 = vecTris.begin();
	it2 = vecTris.end();
	VectorIndex v;
	for (; it1 != it2; ++it1)
	{
		ESTriangle& triangle = mTriangles.at( *it1 );
		if ( !triangle.getNeedDelete() )
		{
			for ( int i = 0; i < 3; i ++)
			{
				/// 属于被替换的点
				if (triangle.mPoints[i] == src)
				{
					/// 因为这个点是主点云的，但是在合并的时候强制+size了
					/// 这个方案已经抛弃，所以传进来的size=0
					triangle.mPoints[i] = (target - size);
					/// 这个点需要删除
					//std::sort(triangle.mPoints, triangle.mPoints+3);
					if (Tools::findSamePoint(triangle.mPoints))
					{
						v.push_back(*it1);
						triangle.setNeedDelete(true);
					}
					
					break;
				}
			}
		}
		else
			v.push_back(*it1);
	}
	/// 测试发现这段代码影响不大，为了速度先注释
	//VectorIndex::iterator it = v.begin();
	//for (; it != v.end(); ++it)
	//{
	//	mPointPropertys.at(src).eraseTri(*it);
	//}
	pointPros.at(target).merge(pointPros.at(src));
	pointPros.at(src).setNeedDelete(true);
}


bool cmpOrderProducePoint(const OrderProducePoint& a, const OrderProducePoint& b)
{
	//bool ret = false;
	if (a.pos.x < b.pos.x)
	{
		return true;
	}
	else if (a.pos.x == b.pos.x)
	{
		if (a.pos.y < b.pos.y)
		{
			return true;
		}
		else if (a.pos.y == b.pos.y)
		{
			if (a.pos.z < b.pos.z)
			{
				return true;
			}
		}
	}

	return false;
}

void Cloud::deleteSameProducePoint(  int size  )
{/// 废弃函数
	/*
	VecOrderProducePoint& vec = mOrderProducePoint;
	std::sort(vec.begin(), vec.end(), cmpOrderProducePoint);

	VectorIndex vInts;
	VecOrderProducePoint::const_iterator it1, it2;
	it1 = vec.begin();
	it2 = vec.end();
	Ogre::Vector3 curPos;

	int src = 0; int key = 0;
	for ( ; it1 != it2; ++it1)
	{
		if (curPos != it1->pos)
		{
			curPos = it1->pos;
			replaceVec(vInts, size);
			vInts.clear();
			vInts.push_back(it1->index);
			mMegerKeyPoint.push_back(it1->index>0?it1->index:(it1->index*(-1)+size));
		}
		else
		{
			vInts.push_back(it1->index);
		}
	}*/
}

void Cloud::makeMergeTri(  int size  )
{
	VecTriInt::iterator it1, it2;
	it1 = mVecTriInt.begin();
	it2 = mVecTriInt.end();
	int index1 = 0;
	int index2 = 0;
	int index3 = 0;
	ESTriangle t;
	mTriangles.reserve(mTriangles.size() + mVecTriInt.size());
	//#pragma omp parallel for
	for (/*; it1 != it2; ++it1*/int i = 0; i < mVecTriInt.size(); i ++)
	{
		//index1 = it1->index1>0?it1->index1:(it1->index1*(-1) + size);
		//index2 = it1->index2>0?it1->index2:(it1->index2*(-1) + size);
		//index3 = it1->index3>0?it1->index3:(it1->index3*(-1) + size);
		index1 = mVecTriInt[i].index1 > 0? mVecTriInt[i].index1:(mVecTriInt[i].index1*(-1) + size);
		index2 = mVecTriInt[i].index2 > 0? mVecTriInt[i].index2:(mVecTriInt[i].index2*(-1) + size);
		index3 = mVecTriInt[i].index3 > 0? mVecTriInt[i].index3:(mVecTriInt[i].index3*(-1) + size);
		//#pragma omp critical
		{

		mMesh->tris.push_back(omesh::TriVtx(index1, index2, index3));

		t.mPoints[0] = index1;
		t.mPoints[1] = index2;
		t.mPoints[2] = index3;
		//std::sort(t.mPoints, t.mPoints + 3);
		t.mIndex = mTriangles.size();
		mTriangles.push_back(t);

		mPointPropertys.at(index1).addBorderList(index2, index3);
		mPointPropertys.at(index2).addBorderList(index1, index3);
		mPointPropertys.at(index3).addBorderList(index1, index2);

		mPointPropertys.at(index1).makeBelongTri(t.getIndex());
		mPointPropertys.at(index2).makeBelongTri(t.getIndex());
		mPointPropertys.at(index3).makeBelongTri(t.getIndex());
		}
	}
}

bool compareLink(const std::pair<int, int>& f, const std::pair<int, int>& s)
{
	return f.first > s.first;
}

void Cloud::makeMergePoint( int size )
{
	VecLinkInt::iterator it1, it2;
	it1 = mLinkInts.begin();
	it2 = mLinkInts.end();
	for (; it1 != it2; ++it1)
	{
		replacePoint(it1->first + size, it1->second, 0);
	}

	/// 处理a点云多点对b点云情况
	std::sort(mLinkInts.begin(), mLinkInts.end(), compareLink);
	int lastSecond = 0, lastFirst = 0;
	for (int i = 0; i < mLinkInts.size(); i ++)
	{
		if (mLinkInts[i].first != lastFirst)
		{
			lastFirst = mLinkInts[i].first;
			lastSecond = mLinkInts[i].second;
		}
		/// 有可能是两个点相互都是最近点，所以会出现成对，所以判断是否是自身
		else  if( mLinkInts[i].first == lastFirst && mLinkInts[i].second != lastSecond)
		{
			replacePoint( mLinkInts[i].second, lastSecond, 0 );
		}
	}
}

void Cloud::updatePoints( PointPropertys& points, int triSize, int pointSize ) const
{
	#pragma omp parallel for
	for (int i = 0; i < points.size(); i ++)
	{
		points.at(i).addIndexBy(pointSize);
		points.at(i).updateBelongTri(triSize);
		points.at(i).updateBorderList(pointSize);
	}
}

void Cloud::replaceVec( VectorIndex& vec, int size )
{
	if (vec.size() <= 1)
		return;
	
	VectorIndex::const_iterator it1, it2;
	it1 = vec.begin();
	it2 = vec.end();
	int target = 0;
	int index = 0; int last = 0;
	//std::sort(vec.begin(), vec.end());
	for (; it1 != it2; ++it1)
	{
		index = *it1;
		index = index>0?index:((-1)*index + size);
		PointProperty& p = mPointPropertys.at(index);

		
		if ( target == 0 && !p.getNeedDelete())
		{
			target = index;
		}
		else
		{
			if (index == last || index == target)
			{
				continue;
			}
			else
			{
				last = index;
				replacePoint(index, target, 0);
			}
		}
	}
}

void Cloud::megerKeyPoints( int size )
{
	VectorIndex::iterator it1, it2;
	it1 = mMegerKeyPoint.begin();
	it2 = mMegerKeyPoint.end();
	int index = 0;
	for (; it1!= it2; ++it1)
	{
		index = *it1;
		if ( index < 0)
			index = index*(-1) + size;
		assert(index < mPointPropertys.size());
		PointProperty& p = mPointPropertys.at(index);
		if (p.isBorderPoint() && !p.getNeedDelete())
		{
			mWrongPoints.push_back(index);
		}
	}

	it1 = mReplacePoints.begin();
	it2 = mReplacePoints.end();

	for (; it1!= it2; ++it1)
	{
		index = *it1;
		if ( index < 0)
			index = index*(-1) + size;
		//mWrongPoints.push_back(index);
		PointProperty& p = mPointPropertys.at(index);
		if (p.isBorderPoint() && !p.getNeedDelete())
		{
			mWrongPoints.push_back(index);
		}
	}
	return;	
}

bool Cloud::isBuildBorderPoint( int index )
{
	assert(index < mPointPropertys.size());
	VectorIndex points = mPointPropertys.at(index).getBorderList();
	std::sort(points.begin(), points.end());
	for ( int i = 0; i < points.size(); i ++)
	{
		if (mPointPropertys.at(points[i]).getNeedDelete())
		{
			continue;
		}
		if ( points[i] != points[i + 1])
		{
			return true;
		}
	}
	return false;
}

void Cloud::reBuildKeyPoints( int size )
{
	rebuildVecPoints(mMegerKeyPoint, size);
	rebuildVecPoints(mReplacePoints, size);
	return; 

	/// 等确认没问题了，删除以下代码
	VectorIndex::iterator it1, it2;
	it1 = mMegerKeyPoint.begin();
	it2 = mMegerKeyPoint.end();
	int index = 0;
	for (; it1 != it2; ++it1)
	{
		index = *it1>0?(*it1):(-1*(*it1) + size);
		assert(index < mPointPropertys.size() && index >= 0);
		PointProperty& p = mPointPropertys.at(index);
		if (p.getNeedDelete())	/// 已经删除的点就不在运算
			continue;
		p.clearBorderList();	/// 点清空
		const VectorIndex& vec = p.getBelongTri();
		for (int i = 0; i < vec.size(); i ++)
		{
			ESTriangle& t = mTriangles.at( vec[i] );
			if ( !t.getNeedDelete() )	/// 还是有效的三角形
			{
				if (t.mPoints[0] == index)
				{
					p.addBorderList(t.mPoints[1], t.mPoints[2]);
				}
				else if (t.mPoints[1] == index)
				{
					p.addBorderList(t.mPoints[0], t.mPoints[2]);
				}
				else if (t.mPoints[2] == index)
				{
					p.addBorderList(t.mPoints[0], t.mPoints[1]);
				}
				else logv("error index %d", *it1);
			}
		}
		p.buildBorder();
	}

	it1 = mReplacePoints.begin();
	it2 = mReplacePoints.end();
	index = 0;
	for (; it1 != it2; ++it1)
	{
		index = *it1>0?(*it1):(-1*(*it1) + size);
		assert(index < mPointPropertys.size() && index >= 0);
		PointProperty& p = mPointPropertys.at(index);
		if (p.getNeedDelete())	/// 已经删除的点就不在运算
			continue;
		p.clearBorderList();	/// 点清空
		const VectorIndex& vec = p.getBelongTri();
		for (int i = 0; i < vec.size(); i ++)
		{
			ESTriangle& t = mTriangles.at( vec[i] );
			if ( !t.getNeedDelete() )	/// 还是有效的三角形
			{
				if (t.mPoints[0] == index)
				{
					p.addBorderList(t.mPoints[1], t.mPoints[2]);
				}
				else if (t.mPoints[1] == index)
				{
					p.addBorderList(t.mPoints[0], t.mPoints[2]);
				}
				else if (t.mPoints[2] == index)
				{
					p.addBorderList(t.mPoints[0], t.mPoints[1]);
				}
				else logv("error index %d", *it1);
			}
		}
		p.buildBorder();
	}
}

void Cloud::repairWrongPoints( void )
{
	VectorIndex::iterator it1, it2;

	/// 把重复的点去掉
	Tools::deleteSameInt(mWrongPoints);

	//wcMesh* p = wcGetMesh( this );

	//saveSTLFile("test.stl", p);
	//savewcWrl("test.wrl", p);

	//delete p;
	//return;

	it1 = mWrongPoints.begin();
	it2 = mWrongPoints.end();

	VectorIndex vec;
	int last = -1, cur = 0, ret = 0;
	int size = mPointPropertys.size();
	omesh::Pnt3 pos;
	float lenght = 0.0f;
	for (; it1 != it2; ++it1)
	{
		assert(*it1 < size);

		PointProperty& p = mPointPropertys.at(*it1);
		//mMesh->vtxRgb[*it1] = omesh::Pnt3(1.0f, 1.0f, 0.0f);continue;
		if (!p.getNeedDelete() && p.isBorderPoint() )	/// 边界点，不删除的点
		{
			cur = *it1;
			pos = mMesh->vtx.at(cur);	/// 记录第一个的位置
			vec.push_back(cur);			
			for (int i = 0; i < 5; i ++)	/// 错误点最多久10几个，给50的期限
			{
				ret = getNextBorderPoint(cur, last);
				if (ret != -1) /// 回来的点有效，进行距离判断
					lenght = dist2(pos, mMesh->vtx.at(ret));
				if (ret == -1 || ret == *it1 || lenght > wrongLenght)	/// 已经到结尾了
				{
					break;
				}
				else
				{
					/// 点存储，位置更新
					vec.push_back(ret);
					last = cur;
					cur = ret;
				}
			}

			if (vec.size() > 1)
			{
				/// 把所有的点位置平均到第一个上面
				makeVecPointCenter(vec);
				/// 把这些点都合成一个
				/// 可能要给个好位置
				replaceVec(vec, 0);
			}
			/// 寻找下一个错误点的点集
			vec.clear();
		}
	}
}

void Cloud::makeVecPointCenter( const VectorIndex& vec )
{

	VectorIndex::const_iterator it1, it2;
	it1 = vec.begin();
	it2 = vec.end();
	int n = 0; int first = -1;
	omesh::Pnt3 pos;
	for (; it1 != it2; ++it1)
	{
		const PointProperty& p = mPointPropertys.at(*it1);
		if (!p.getNeedDelete())	/// 严格来说，应该不需要再判断它是否删除
		{
			pos += mMesh->vtx.at(*it1);
			n++;

			if (first == -1)
			{
				first = *it1;
			}
		}
	}

	assert(first > 0 && first < mMesh->vtx.size());
	assert( n != 0);
	mMesh->vtx.at(first) = pos/n;
}

void Cloud::clear()
{
	/// 边界重叠点数据
	mPntBorders.clear();

	/// 边界重叠点在原来的位置
	/// 这个设计有问题
	mPntBorderIndexs.clear();

	mPointPropertys.clear();		/// 点属性列表
	mTriangles.clear();		/// 面列表

	/// 增加的三角形列表
	mVecTriInt.clear();


	mLinkInts.clear();

	/// 合并后的边界点
	mMegerKeyPoint.clear();

	mReplacePoints.clear();

	/// 错误点集合，即修复后还是边界点的顽固点
	mWrongPoints.clear();

	/// 从计算类过来的数据
	mHostPointIndex.clear();	/// 主点云在客点云上最近点的索引
	mHostPointLenght.clear();	/// 主点云到客点云最近点的距离

	/// 投影和投影点信息
	mMapPointProperty.clear();

	/// 删除产生的边界
	mBordersFromDelete.clear();

	mHoles.clear();
	//KDTree *mKdtree;
}

float Cloud::getPointWeight( int index ) const
{
	assert( index < mPointPropertys.size() );
	const VectorIndex& v = mPointPropertys.at(index).getBorderList();

	float f = 0.0f;
	for ( int i = 0; i < v.size(); i ++)
	{
		const PointProperty& p = mPointPropertys.at( v[i] );
		if ( !p.getNeedDelete() )
		{
		}
	}
	return f;
}

bool Cloud::compPoint( const int& a, const int&b )
{
	assert(mMesh && a < mMesh->vtx.size() && b < mMesh->vtx.size());
	const omesh::Pnt3& pa = mMesh->vtx.at(a);
	const omesh::Pnt3& pb = mMesh->vtx.at(b);
	return a > b;
}

void Cloud::rebuildVecPoints( VectorIndex& v, int size )
{
	VectorIndex::iterator it1, it2;
	it1 = v.begin();
	it2 = v.end();
	int index = 0;
	VectorIndex temp;
	for (; it1 != it2; ++it1)
	{
		index = *it1>0?(*it1):(-1*(*it1) + size);
		assert(index < mPointPropertys.size() && index >= 0);
		PointProperty& p = mPointPropertys.at(index);
		if (p.getNeedDelete())	/// 已经删除的点就不在运算
			continue;
		p.clearBorderList();	/// 点清空
		temp.clear();
		const VectorIndex& vec = p.getBelongTri();
		for (int i = 0; i < vec.size(); i ++)
		{
			ESTriangle& t = mTriangles.at( vec[i] );
			if ( !t.getNeedDelete() )	/// 还是有效的三角形
			{
				if (t.mPoints[0] == index)
				{
					p.addBorderList(t.mPoints[1], t.mPoints[2]);
				}
				else if (t.mPoints[1] == index)
				{
					p.addBorderList(t.mPoints[0], t.mPoints[2]);
				}
				else if (t.mPoints[2] == index)
				{
					p.addBorderList(t.mPoints[0], t.mPoints[1]);
				}
				else temp.push_back( vec[i] );
			}
			else temp.push_back( vec[i] );
		}
		p.buildBorder();
		p.removeBelongTris(temp);
	}
}

void Cloud::getFromOtherMesh( Cloud* pCloud )
{
	Mesh* pMesh = pCloud->getMesh();
	if (pMesh == NULL)
	{
		return;
	}
	if (mMesh == NULL)
	{
		mMesh = new Mesh;
	}
	mMesh->vtx.clear();
	mMesh->tris.clear();
	mMesh->nrm.clear();
	mMesh->vtxRgb.clear();

	mMesh->vtx.assign(pMesh->vtx.begin(), pMesh->vtx.end());
	mMesh->tris.assign(pMesh->tris.begin(), pMesh->tris.end());
	mMesh->nrm.assign(pMesh->nrm.begin(), pMesh->nrm.end());
	mMesh->vtxRgb.assign(pMesh->vtxRgb.begin(), pMesh->vtxRgb.end());
}

#define MAX_BUF 1024
bool Cloud::readWrl( const std::string& fileName )
{

	FILE* f = fopen(fileName.c_str(), "r");
	if (f == NULL)
	{
		return false;
	}

	if (mMesh == NULL)
	{
		mMesh = new Mesh;
	}
	std::vector<omesh::Pnt3>& vertex = mMesh->vtx;
	std::vector<omesh::Pnt3>& colors = mMesh->vtxRgb;
	std::vector<omesh::Pnt3>& normals = mMesh->nrm;
	std::vector<omesh::TriVtx>& tris = mMesh->tris;
	vertex.clear();
	colors.clear();
	normals.clear();
	tris.clear();


	char buf[MAX_BUF] = {0};

	int state = -1;	/// 数据读取

	while (fgets(buf, MAX_BUF, f))
	{
		if (strstr(buf, "point"))		/// 点
		{
			state = 0;
		}
		else if (strstr(buf, "color"))	/// 颜色
		{
			state = 1;
		}
		else if (strstr(buf, "coordIndex"))	/// 三角形
		{
			state = 2;
		}
		else if(!strstr(buf, "["))
		{
			state = -1;
		}

		if (strstr(buf, "[") && state != -1)	/// 数据开始
		{
			readLineData(state, f);
		}		
	}

	fclose(f);
	return true;
}

void replaceChar(char*p, int size, const char& s, const char& t)
{
	while( size --)
	{
		if (p[size] == s)
		{
			p[size] = t;
		}
	}
}

void Cloud::readLineData( int state, FILE* file )
{
	std::vector<omesh::Pnt3>& vertex = mMesh->vtx;
	std::vector<omesh::Pnt3>& colors = mMesh->vtxRgb;
	std::vector<omesh::Pnt3>& normals = mMesh->nrm;
	std::vector<omesh::TriVtx>& tris = mMesh->tris;

	char buf[MAX_BUF] = {0};
	float number[3] = {0};	/// 三个数据
	int index[4] = {0};		/// 4个int
	omesh::TriVtx tri;
	omesh::Pnt3 pnt;
	//char* p = NULL;

	while (fgets(buf, MAX_BUF, file))
	{
		//replaceChar(buf, MAX_BUF, ",", " ");
		//p = buf+strlen(buf)+1;
		//std::replace(buf, p, ",", " ");	/// 把逗号去掉。以免影响结果
		if (strstr(buf, "]"))	/// 数据结束
		{
			break;
		}
		if (strstr(buf, "#"))	/// 注释符
		{
			continue;
		}

		/// 开始读数据
		if (state == 2)	/// 读4个int
		{
			//sscanf(buf, "%d,%d,%d,%d,", &index[0], &index[1], &index[2], &index[3]);
			sscanf(buf, "%d,%d,%d,%d,", &index[0], &index[1], &index[2], &index[3]);
			tri[0] = index[0];
			tri[1] = index[1];
			tri[2] = index[2];
			tris.push_back(tri);
		} 
		else
		{
			/// 当3个float 来读
			sscanf(buf, "%f %f %f", &number[0], &number[1], &number[2]);
			pnt[0] = number[0];
			pnt[1] = number[1];
			pnt[2] = number[2];
			if (state == 0)	/// 点数据
			{
				vertex.push_back(pnt);
			}
			else
				colors.push_back(pnt);
		}
	}
}

void Cloud::deleteMeshSamePoint( void )
{
	std::vector<omesh::Pnt3>& vtx = mMesh->vtx;
	std::vector<omesh::Pnt3>& vtxRgb = mMesh->vtxRgb;
	std::vector<omesh::Pnt3>& nrm = mMesh->nrm;
	std::vector<omesh::TriVtx>& tris = mMesh->tris;

	int size = vtx.size();
	VectorIndex vIndex; vIndex.reserve(size);
	for (int i = 0; i < size; i ++)
		vIndex.push_back(i);

	Tools::sortPoints(vIndex, vtx);

	omesh::Pnt3 pnt;
	VectorIndex vec;
	VectorIndex vDeleteVec;
	for (int i = 0; i < size; i ++)
	{
		omesh::Pnt3& temp = vtx[ vIndex[i] ];
		if (pnt != temp)
		{
			pnt = temp;
			
			vec.clear();
		}
		else
		{
			vec.push_back( vIndex[i] );
		}
	}
}

#define SAVE_ON		1
#define SAVE_OFF	0
#define SAVE_POINT SAVE_ON
#define SAVE_COLOR SAVE_ON
bool Cloud::saveMesh( const std::string& fileName )
{
	typedef std::vector<omesh::Pnt3> VecPnts;
	const VecPnts& vtxs = mMesh->vtx;
	const VecPnts& vtxRgb = mMesh->vtxRgb;
	const VecPnts& nors = mMesh->nrm;
	const std::vector<omesh::TriVtx>& tris = mMesh->tris;

	FILE* f = fopen(fileName.c_str(), "w");
	if (f == NULL)
	{
		return false;
	}
	if (SAVE_POINT)
	{
		
	}
	return true;
}

void Cloud::makeAverageEdgeLenght( void )
{
	if (mMesh == NULL)
	{
		return;
	}
	std::vector<omesh::Pnt3>& vtx = mMesh->vtx;
	std::vector<omesh::TriVtx>& tris = mMesh->tris;

	float sum = 0.0f;

	#pragma omp parallel for reduction(+:sum)
	for (int i = 0; i < tris.size(); i ++)
	{
		omesh::TriVtx& t = tris[i];
		sum += omesh::dist2(vtx[t[0] ], vtx[t[1]]);
		sum += omesh::dist2(vtx[t[1] ], vtx[t[2]]);
		sum += omesh::dist2(vtx[t[0] ], vtx[t[2]]);
	}
	int number = tris.size();
	//mAverageEdgeLenght = 2000*std::sqrt(sum)/number;		/// 得到一条边的平均长度
	//logv("the average edge lenght %f", mAverageEdgeLenght);
}

omesh::Pnt3 Cloud::getMidBorderPos( int index )
{
	
	VectorIndex v;
	const VectorIndex& vecs = mPointPropertys.at(index).getBorderList();

	for (int i = 0; i < vecs.size(); i ++)
	{
		PointProperty& point = mPointPropertys.at(vecs[i]);
		if (!point.getNeedDelete() && point.isBorderPoint() && vecs[i]!= index )
		{
			v.push_back(vecs[i]);
		}
	}
	Tools::deleteSameInt(v);
	if (v.size() != 2)
	{
		return mMesh->vtx[index];
	}
	else
	{
		omesh::Pnt3 temp = (mMesh->vtx[v[0]] + mMesh->vtx[v[1]])*0.5;
		if (omesh::dist2(mMesh->vtx[index], temp) > 2.5)
		{
			return mMesh->vtx[index];
		}
		return (temp + mMesh->vtx[index])*0.5;
	}
}

omesh::Pnt3 Cloud::getSmoothBorderPos(int index, float f /*= 0.5*/)
{
	VectorIndex v;
	const VectorIndex& vecs = mPointPropertys.at(index).getBorderList();

	for (int i = 0; i < vecs.size(); i ++)
	{
		PointProperty& point = mPointPropertys.at(vecs[i]);
		if (!point.getNeedDelete() && point.isBorderPoint() && vecs[i]!= index )
		{
			v.push_back(vecs[i]);
		}
	}
	Tools::deleteSameInt(v);
	if (v.size() != 2)
	{
		return mMesh->vtx[index];
	}
	else
	{
		omesh::Pnt3 temp = (mMesh->vtx[v[0]] + mMesh->vtx[v[1]])*0.5;
		if (omesh::dist2(mMesh->vtx[index], temp) > 2.5)
		{
			return mMesh->vtx[index];
		}
		return temp;
	}
}

void Cloud::deleteSamePointMerge( void )
{
	if (mWrongPoints.size() < 2)
	{
		return;
	}
	Tools::sortPoints(mWrongPoints, mMesh->vtx);
	omesh::Pnt3 last = mMesh->vtx[ mWrongPoints[0] ];
	int lastIndex = 0;
	logv("wrong point size %d", mWrongPoints.size());
	for ( int i = 1; i < mWrongPoints.size(); i ++)
	{
		//mMesh->vtxRgb[mWrongPoints[i]] = omesh::Pnt3(1.0, 1.0, 0.0);
		//continue;
		omesh::Pnt3& cur = mMesh->vtx[ mWrongPoints[i] ];
		if (dist2(last, cur) < 1)
		{
			replacePoint(i, lastIndex, 0);
		}
		else
		{
			last = cur;
			lastIndex = i;
		}
	}
}

/// 找时间写的优雅点
void Cloud::initHoles( void )
{
	int index = 0, last = 0, right = 0, temp = 0;;
	PointBorderLink::iterator it;
	int holeIndex = 0;

	for (int i = 0; i < mPntBorderIndexs.size(); i ++)
	{
		//mMesh->vtxRgb[ mPntBorderIndexs[i] ] = omesh::Pnt3(1.0, 1.0, 0.0);continue;
		if (mPointBorderLink[ mPntBorderIndexs[i] ].getStatue() == true)	/// 已经使用了就pass
		{
			continue;
		}		
		mHoles.push_back(Hole());
		Hole& h = mHoles[mHoles.size() - 1];
		holeIndex = mHoles.size() - 1;
		h.push_back( mPntBorderIndexs[i] );
		mPointBorderLink[ mPntBorderIndexs[i] ].setStatue(true);
		mPointBorderLink[ mPntBorderIndexs[i] ].setHoleIndex(holeIndex);
		
		index = getNextBorderPoint(mPntBorderIndexs[i],-1);
		last = mPntBorderIndexs[i];
		it = mPointBorderLink.find(index);
		right = index;

		/// 向左找
		while (it != mPointBorderLink.end() && it->second.getStatue() == false)  /// 循环寻找边界
		{
			/// 存边界
			h.push_back(index);
			temp = index;
			index = getNextBorderPoint(index, last);
			last = temp;
			it->second.setStatue(true);	/// 标志已经计算
			it->second.setHoleIndex(holeIndex);
			it = mPointBorderLink.find(index);
		}
		/// 向右找
		index = getNextBorderPoint(mPntBorderIndexs[i], right);
		last = mPntBorderIndexs[i];
		it = mPointBorderLink.find(index);

		while (it != mPointBorderLink.end() && it->second.getStatue() == false)  /// 循环寻找边界
		{
			/// 存边界
			h.push_front(index);
			temp = index;
			index = getNextBorderPoint(index, last);
			last = temp;
			it->second.setStatue( true);	/// 标志已经计算
			it->second.setHoleIndex(holeIndex);
			it = mPointBorderLink.find(index);
		}
	}
	return;
	logv("mHoes size %d", mHoles.size());
	//return;
	omesh::Pnt3 color;
	Holes::iterator itt1, itt2;
	itt1 = mHoles.begin();
	itt2 = mHoles.end();
	for ( ;itt1!= itt2;++itt1)
	{
		Hole& h = *itt1;
		Hole::iterator ittt = h.begin();
		logv("line size %d first %d end %d", h.size(), h.front(), h.back());
		if (h.size() < 3)
		{
			//continue;
			mMesh->vtxRgb[*(ittt++)] = omesh::Pnt3(1.0f, 1.0f, 0.0);continue;
		}
		continue;
		color[0] = float(rand()%255)/255;
		color[1] = float(rand()%255)/255;
		color[2] = float(rand()%255)/255;
		for (int i = 0; i < h.size(); i ++)
		{
			mMesh->vtxRgb[*(ittt++)] = color;
		}
	}
	FILE* f = fopen("border.txt", "w");
	VectorIndex v; int sum;
	for (int i = 0; i < mPntBorderIndexs.size(); i ++)
	{
		PointProperty& p = mPointPropertys.at( mPntBorderIndexs[i]);
		const VectorIndex& vec = p.getBorderList();
		v.clear();
		for (int j = 0; j < vec.size(); j ++)
		{
			PointProperty& pp = mPointPropertys.at( vec[j] );
			if (pp.isBorderPoint() && !pp.getNeedDelete())
			{
				sum ++;
				v.push_back( vec[j] );
			}
		}
		fprintf(f, "%d:", mPntBorderIndexs[i]);
		for (int j = 0; j < v.size(); j ++)
		{
			fprintf(f, "%d ", v[j]);
		}
		fprintf(f,"\n");
	}
	fclose(f);
}

int Cloud::getNextBorderIndex( int index, int last /*= -1*/ )
{
	int next = getNextBorderPoint(index, last);
	PointBorderLink::iterator it = mPointBorderLink.find(next);
	if (it != mPointBorderLink.end() && it->second.getStatue() == false)
	{
		it->second.setStatue(true);
		return next;
	}
	return -1;
}


//bool Cloud::readData( const std::string& path, const std::string RocPaht, 
//					const Matrix<float> &R, const Matrix<float> &G, const Matrix<float> &B, 
//						const Xform<float>& XF, const Matrix<double>& KK, const Matrix<double>& KC, 
//							const Matrix<double>& RM, const Matrix<double>& TM ,int subSamp/*=1*/ )
//{
//	std::string str;
//	printf("read data %s\n", path.c_str());
//	logv("read data %s", path.c_str());
//
//	/// 颜色
//	//Matrix<float> mImageR;
//	//Matrix<float> mImageG;
//	//Matrix<float> mImageB;
//
//	/// 读取数据矩阵
//	str = path + File_Txt;
//
//	if(	!readTxtfrom(mXform, str.c_str()) )
//	{	
//		/// 多重读取标准
//		str = path + File_xf;
//		if (!readXform(mXform, str.c_str()) )
//		{
//			mErrorString += str;
//			mErrorString += " 读取失败! ;";
//			log1(mErrorString.c_str());
//
//			cout<<"读取失败"<<endl;
//			return false;
//		}
//	}
//
//	/// 读取图片信息
//	//str = path + File_Bmp;
//	//if( !readFromBmp(&mImageR, &mImageG, &mImageB, str.c_str()) )
//	//{
//	//	mErrorString += str;
//	//	mErrorString += " 读取失败 ;";
//	//	log1(mErrorString.c_str());
//	//	return false;
//	//}
//
//	/// 读取模型
//	Matrix<int> _P;
//	Matrix<float> _X;
//	Matrix<float> _Y;
//	Matrix<float> _Z;
//	float _interstep;
//	float _x0;
//	float _y0;
//	//str = path + File_roc;
//	/*if(!omesh::readPIF(str.c_str(), _P, _X, _Y, _Z, _interstep, _x0, _y0))
//	{
//		mErrorString += str;
//		mErrorString += " 读取失败 ;";
//		log1(mErrorString.c_str());
//		return false;
//	}*/
//
//	if(!omesh::readPIF(RocPaht.c_str(), _P, _X, _Y, _Z, _interstep, _x0, _y0))
//	{
//		mErrorString += RocPaht;
//		mErrorString += " 读取失败;";
//		log1(mErrorString.c_str());
//		return false;
//	}
//
//	/// 使用补洞
//#ifdef FILL_HOLE
//	PRO(_P, _X, _Y, _Z);
//#endif
//
//	RangeGrid rg(_P, _X, _Y, _Z, _interstep, _x0, _y0);
//	if (mMesh)
//	{
//		delete mMesh;
//		mMesh = NULL;
//	}
//	mMesh = rg.toMesh(subSamp); if (mMesh == NULL) return false;
//	mMesh->remove_stepedges_auto();
//	mMesh->remove_unused_vtxs();
//
//	float in[3],out[3];
//	int vtxSize = mMesh->vtx.size();
//
//	Xform<float>& xf = mXform;
//
//	/**********************************************/
//	/*for(int i=0 ; i<int(vtx_private.size()) ; i++)
//	{
//		in[0] = vtx_private[i][0]; in[1] = vtx_private[i][1]; in[2] = vtx_private[i][2];
//
//		xf_Vector[itVec[BigGroud]].apply_inv(in, out);
//		vtx_private[i][0] = out[0]; vtx_private[i][1] = out[1]; vtx_private[i][2] = out[2];
//	}
//
//	getVertexNormals(vtx_private,tris,nrm_private);
//
//	//点数组修正
//	Pnt3 vtx_begin_temp;
//
//	for (int i = 0; i<vtx_private.size(); i++)
//	{
//		vtx_begin_temp[1] = vtx_private[i][0];
//		vtx_begin_temp[0] = vtx_private[i][1];
//		vtx_begin_temp[2] = -vtx_private[i][2];
//		vtx_private[i] = vtx_begin_temp;
//	}
//
//	//去背面,多幅图片的uv，则使用法向量较为正对的图的uv
//	DelBack(vtx_private,tris,nrm_private,BigGroud);
//
//	if (!GetKFC(kc,fc,cc,KK_right_l2m[itVec[BigGroud]]
//		,kc_right_l2m[itVec[BigGroud]]))
//	{
//		return false;
//	}
//
//	//反投影
//	RetUV = project2oulu(vtx_private,RVec[itVec[BigGroud]],TVec[itVec[BigGroud]],fc,cc,kc);*/
//	/**********************************************/
//	vector<omesh::Pnt3> vtxSave(mMesh->vtx.size());
//	float vtxTemp = 0.0;
//
//	//#pragma omp parallel for   不知道为什么，这里不能多核
//	for(int i=0 ; i<vtxSize ; i++)
//	{
//		in[0] = mMesh->vtx[i][0]; in[1] = mMesh->vtx[i][1]; in[2] = mMesh->vtx[i][2];
//		xf.apply(in, out);
//		mMesh->vtx[i][0] = out[0]; mMesh->vtx[i][1] = out[1]; mMesh->vtx[i][2] = out[2];
//
//		in[0] = mMesh->nrm[i][0]; in[1] = mMesh->nrm[i][1]; in[2] = mMesh->nrm[i][2];
//		xf.apply_nrm(in, out);
//		mMesh->nrm[i][0] = out[0]; mMesh->nrm[i][1] = out[1]; mMesh->nrm[i][2] = out[2];
//
//		/*in[0] = mMesh->vtx[i][0]; in[1] = mMesh->vtx[i][1]; in[2] = mMesh->vtx[i][2];
//		XF.apply_inv(in, out);
//		mMesh->vtx[i][0] = out[0]; mMesh->vtx[i][1] = out[1]; mMesh->vtx[i][2] = out[2];
//
//		in[0] = mMesh->nrm[i][0]; in[1] = mMesh->nrm[i][1]; in[2] = mMesh->nrm[i][2];
//		XF.apply_nrm(in, out);
//		mMesh->nrm[i][0] = out[0]; mMesh->nrm[i][1] = out[1]; mMesh->nrm[i][2] = out[2];
//
//		vtxTemp = mMesh->vtx[i][0];
//
//		mMesh->vtx[i][0] = mMesh->vtx[i][1];
//		mMesh->vtx[i][1] = vtxTemp;
//		mMesh->vtx[i][2] = -mMesh->vtx[i][2];*/
//
//		vtxSave[i] = mMesh->vtx[i];
//	}
//
//	for(int i=0 ; i<vtxSize ; i++)
//	{
//		in[0] = vtxSave[i][0]; in[1] = vtxSave[i][1]; in[2] = vtxSave[i][2];
//		XF.apply_inv(in, out);
//		vtxSave[i][0] = out[0]; vtxSave[i][1] = out[1]; vtxSave[i][2] = out[2];
//
//		vtxTemp = vtxSave[i][0];
//
//		vtxSave[i][0] = vtxSave[i][1];
//		vtxSave[i][1] = vtxTemp;
//		vtxSave[i][2] = -vtxSave[i][2];
//	}
//
//	double kc[5];
//	double fc[2];
//	double cc[2];
//
//	GetKFC( kc, fc, cc, KK, KC);
//	Matrix<double> UV =  project2oulu(vtxSave,RM,TM,fc,cc,kc);
//
//	mMesh->vtxRgb.resize(vtxSize);
//	for (int i = 0 ; i < vtxSize ; i++)
//	{
//		mMesh->vtxRgb[i] = BilInter(UV[i][0], UV[i][1], R, G, B);
//		mMesh->vtxRgb[i][0] /= 255.0f; 
//		mMesh->vtxRgb[i][1] /= 255.0f; 
//		mMesh->vtxRgb[i][2] /= 255.0f; 
//	}
//	//logv("here6");
//	//for(int i=0 ; i<vtxSize ; i++)
//	//{
//	//	in[0] = mMesh->vtx[i][0]; in[1] = mMesh->vtx[i][1]; in[2] = mMesh->vtx[i][2];
//
//	//	XF.apply_inv(in, out);
//	//	mMesh->vtx[i][0] = out[0]; mMesh->vtx[i][1] = out[1]; mMesh->vtx[i][2] = out[2];
//	//}
//
//	////转置向量(0,0,1) 用于做法向权重的时候计算夹角用
//	in[0] = 0; in[1] = 0; in[2] = 1;
//	xf.apply_nrm(in, out);
//	mNumber = path.at(path.size()-1) - '0';
//
//	return true;
//}


bool Cloud::readData( const std::string& path, const std::string RocPaht, 
					const Matrix<float> &R, const Matrix<float> &G, const Matrix<float> &B, 
						const Xform<float>& XF, const Matrix<double>& KK, const Matrix<double>& KC, 
							const Matrix<double>& RM, const Matrix<double>& TM ,int subSamp/*=1*/ )
{


	std::string str;

	/// 读取数据矩阵
	str = path + File_Txt;

	if(	!readTxtfrom(mXform, str.c_str()) )
	{	
		/// 多重读取标准
		str = path + File_xf;

		//logv("aaadddd");
		//log1(str.c_str());
		if (!readXform(mXform, str.c_str()) )
		{
			mErrorString += str;
			mErrorString += " 读取失败! ;";
			log1(mErrorString.c_str());

			cout<<"读取失败"<<endl;
			return false;
		}
	}

	/// 读取模型
	Matrix<int> _P;
	Matrix<float> _X;
	Matrix<float> _Y;
	Matrix<float> _Z;
	float _interstep;
	float _x0;
	float _y0;

	if(!omesh::readPIF(RocPaht.c_str(), _P, _X, _Y, _Z, _interstep, _x0, _y0))
	{
		mErrorString += RocPaht;
		mErrorString += " 读取失败;";
		log1(mErrorString.c_str());
		return false;
	}

	/// 使用补洞
#ifdef FILL_HOLE
	PRO(_P, _X, _Y, _Z);
#endif

	RangeGrid rg(_P, _X, _Y, _Z, _interstep, _x0, _y0);
	if (mMesh)
	{
		delete mMesh;
		mMesh = NULL;
	}
	mMesh = rg.toMesh(subSamp); if (mMesh == NULL) return false;

	mMesh->remove_stepedges_auto();
	mMesh->remove_unused_vtxs();

	/*
	RangeGrid rg(_P, _X, _Y, _Z, _interstep, _x0, _y0);
	if (mMesh)
	{
		delete mMesh;
		mMesh = NULL;
	}


	mMesh = rg.toMesh(subSamp); if (mMesh == NULL) return false;
	mMesh->remove_stepedges_auto();
	mMesh->remove_unused_vtxs();

	rg.fromMesh(*mMesh);

	PRO(rg.P, rg.X, rg.Y, rg.Z);

	RangeGrid rg1(rg.P, rg.X, rg.Y, rg.Z,_interstep, _x0, _y0);

	mMesh = rg1.toMesh(subSamp); if (mMesh == NULL) return false;
	*/

	float in[3],out[3];
	int vtxSize = mMesh->vtx.size();

	Xform<float>& xf = mXform;

	vector<omesh::Pnt3> vtxSave(mMesh->vtx.size());
	float vtxTemp = 0.0;

	//#pragma omp parallel for   不知道为什么，这里不能多核
	for(int i=0 ; i<vtxSize ; i++)
	{
		vtxSave[i] = mMesh->vtx[i];
	}

	for(int i=0 ; i<vtxSize ; i++)
	{
		vtxTemp = vtxSave[i][0];

		vtxSave[i][0] = vtxSave[i][1];
		vtxSave[i][1] = vtxTemp;
		vtxSave[i][2] = -vtxSave[i][2];
	}

	double kc[5];
	double fc[2];
	double cc[2];

	GetKFC( kc, fc, cc, KK, KC);
	Matrix<double> UV =  project2oulu(vtxSave,RM,TM,fc,cc,kc);

	mMesh->vtxRgb.resize(vtxSize);

	for(int i=0 ; i<vtxSize ; i++)
	{
		mMesh->vtxRgb[i] = BilInter(UV[i][0], UV[i][1], R, G, B);
		mMesh->vtxRgb[i][0] /= 255.0f; 
		mMesh->vtxRgb[i][1] /= 255.0f; 
		mMesh->vtxRgb[i][2] /= 255.0f; 

		in[0] = mMesh->vtx[i][0]; in[1] = mMesh->vtx[i][1]; in[2] = mMesh->vtx[i][2];
		xf.apply(in, out);
		mMesh->vtx[i][0] = out[0]; mMesh->vtx[i][1] = out[1]; mMesh->vtx[i][2] = out[2];

		in[0] = mMesh->nrm[i][0]; in[1] = mMesh->nrm[i][1]; in[2] = mMesh->nrm[i][2];
		xf.apply_nrm(in, out);
		mMesh->nrm[i][0] = out[0]; mMesh->nrm[i][1] = out[1]; mMesh->nrm[i][2] = out[2];
	}

	////转置向量(0,0,1) 用于做法向权重的时候计算夹角用
	in[0] = 0; in[1] = 0; in[2] = 1;
	xf.apply_nrm(in, out);
	mNumber = path.at(path.size()-1) - '0';

	return true;
}

Matrix<double> Cloud::project2oulu(const vector <omesh::Pnt3> &vtx,const Matrix<double> &r,const Matrix<double> &t,double* fc,double* cc,double* kc)
{
	int total_num = vtx.size();
	Matrix<double> vtx_out(total_num,3,0.0f);
	Matrix<double> x(total_num,2,0.0f);
	Matrix<double> m(4,4,0.0f);

	//RT转换
	m[0][0] = r[0][0]; m[0][1]=r[1][0]; m[0][2]=r[2][0]; m[0][3]=0.0;
	m[1][0] = r[0][1]; m[1][1]=r[1][1]; m[1][2]=r[2][1]; m[1][3]=0.0;
	m[2][0] = r[0][2]; m[2][1]=r[1][2]; m[2][2]=r[2][2]; m[2][3]=0.0;
	m[3][0] = t[0][0]; m[3][1] = t[1][0]; m[3][2] = t[2][0]; m[3][3] = 1.0;

	for(int i = 0;i<total_num;i++)
	{
		double invw = 
			1.0 / (m[0][3]*vtx[i][0]+m[1][3]*vtx[i][1]+m[2][3]*vtx[i][2]+m[3][3]);
		vtx_out[i][0] = (m[0][0]*vtx[i][0]+m[1][0]*vtx[i][1]+m[2][0]*vtx[i][2]+m[3][0])*invw;
		vtx_out[i][1] = (m[0][1]*vtx[i][0]+m[1][1]*vtx[i][1]+m[2][1]*vtx[i][2]+m[3][1])*invw;
		vtx_out[i][2] = (m[0][2]*vtx[i][0]+m[1][2]*vtx[i][1]+m[2][2]*vtx[i][2]+m[3][2])*invw;
	}

	x[0][0] = vtx_out[0][0]/vtx_out[0][2];
	x[0][1] = vtx_out[0][1]/vtx_out[0][2];
	double radius_2 = pow(x[0][0],2)+pow(x[0][1],2);
	double radial_distortion = 1+kc[0]*radius_2+kc[1]*pow(radius_2,2);
	double delta_x = 2*kc[2]*x[0][0]*x[0][1]+kc[3]*(radius_2+2*pow(x[0][0],2));
	double delta_y = kc[2]*(radius_2+2*pow(x[0][1],2))+2*kc[3]*x[0][0]*x[0][1];
	x[0][0] = (x[0][0]*radial_distortion+delta_x)*fc[0]+cc[0];
	x[0][1] = (x[0][1]*radial_distortion+delta_y)*fc[1]+cc[1];

	for(int i = 1;i<total_num;i++)
	{
		x[i][0] = vtx_out[i][0]/vtx_out[i][2];
		x[i][1] = vtx_out[i][1]/vtx_out[i][2];
		double radius_2 = pow(x[i][0],2)+pow(x[i][1],2);
		double radial_distortion = 1+kc[0]*radius_2+kc[1]*pow(radius_2,2);
		double delta_x = 2*kc[2]*x[i][0]*x[i][1]+kc[3]*(radius_2+2*pow(x[i][0],2));
		double delta_y = kc[2]*(radius_2+2*pow(x[i][1],2))+2*kc[3]*x[i][0]*x[i][1];
		x[i][0] = (x[i][0]*radial_distortion+delta_x)*fc[0]+cc[0];
		x[i][1] = (x[i][1]*radial_distortion+delta_y)*fc[1]+cc[1];
	}
	return x;
}

omesh::Pnt3 Cloud::BilInter(float x, float y, 
				const Matrix<float> &R, const Matrix<float> &G, const Matrix<float> &B)
{
	omesh::Pnt3 RGB_new;
	RGB_new[0] = 0.0;
	RGB_new[1] = 0.0;
	RGB_new[2] = 0.0;
	//修改名字 mark
	float UVX1 = (int)x;
	float UVX2 = (int)x+1;

	float UVY1 = (int)y;
	float UVY2 = (int)y+1;

	omesh::Pnt3 RGB11;
	omesh::Pnt3 RGB12;
	omesh::Pnt3 RGB21;
	omesh::Pnt3 RGB22;

	//logv("UVX1 = %f , UVY1 = %f, rnum_rows = %d, rnum_cols = %d,gnum_rows = %d, gnum_cols = %d,bnum_rows = %d, bnum_cols = %d\n",UVX1,UVY1,R.num_rows(),R.num_cols(),G.num_rows(),G.num_cols(),B.num_rows(),B.num_cols());

	if (UVX1<0||UVY1<0||UVX2>=R.num_cols()||UVY2>=R.num_rows())
	{
	//	logv("error");
		return GetPointRGB(0, 0, R, G, B);
	}
	//logv("here5.1");
	RGB11 = GetPointRGB(UVX1, UVY1, R, G, B);
	RGB12 = GetPointRGB(UVX1, UVY2, R, G, B);
	RGB21 = GetPointRGB(UVX2 ,UVY1, R, G, B);
	RGB22 = GetPointRGB(UVX2, UVY2, R, G, B);

	double R1 = -1;
	double R2 = -1;
	//logv("here5.2");
	for (int i=0;i<3;i++)
	{
		R1 = (x - UVX1)*RGB21[i] + (UVX2 - x)*RGB11[i];

		R2 = (x - UVX1)*RGB22[i] + (UVX2 - x)*RGB12[i];

		RGB_new[i] = (y - UVY1)*(R2)+(UVY2 - y)*R1;
	}

	return RGB_new;
}

omesh::Pnt3 Cloud::GetPointRGB(int x,int y,const Matrix<float> &R, const Matrix<float> &G, const Matrix<float> &B)
{
	omesh::Pnt3 RGB_new;

//	logv("UVX1 = %d , UVY1 = %d, rnum_rows = %d, rnum_cols = %d,gnum_rows = %d, gnum_cols = %d,bnum_rows = %d, bnum_cols = %d\n",x,y,R.num_rows(),R.num_cols(),G.num_rows(),G.num_cols(),B.num_rows(),B.num_cols());


	RGB_new[0] = R[y][x];
	RGB_new[1] = G[y][x];
	RGB_new[2] = B[y][x];

	//RGB_new[0] = 1;
	//RGB_new[1] = 1;
	//RGB_new[2] = 1;

	return RGB_new;
}

bool Cloud::GetKFC( double *kc,double *fc,double *cc,Matrix<double> KK_right_l2m,Matrix<double> kc_right_l2m )
{
	if (KK_right_l2m.num_rows() != 3 || KK_right_l2m.num_cols() != 3 || kc_right_l2m.num_rows() != 5 || kc_right_l2m.num_cols() != 1)
	{
		return false;
	}

	fc[0] = KK_right_l2m[0][0];
	fc[1] = KK_right_l2m[1][1];
	cc[0] = KK_right_l2m[0][2];
	cc[1] = KK_right_l2m[1][2];
	kc[0] = kc_right_l2m[0][0];
	kc[1] = kc_right_l2m[1][0];
	kc[2] = kc_right_l2m[2][0];
	kc[3] = kc_right_l2m[3][0];
	kc[4] = kc_right_l2m[4][0];

	return true;
}

void Cloud::getPreData( const std::string& path, Matrix<float> &R, Matrix<float> &G, Matrix<float> &B, Xform<float>& XF, Matrix<double>& KK, Matrix<double>& KC, Matrix<double>& RM, Matrix<double>& TM )
{
	bool bRead = false;

	std::string bmpstr = path + ".rocD.bmp";
	bRead = readFromBmp(&R, &G, &B, bmpstr.c_str());

	std::string xfstr = path + ".roc.xf";
	bRead = readXform(XF, xfstr.c_str());

	std::string KKfile = path + "_D_KK_right";
	bRead = readFromBin(&KK, KKfile.c_str());

	std::string KCfile = path + "_D_kc_right";
	bRead = readFromBin(&KC, KCfile.c_str());

	std::string Rfile = path + "_D_R";
	bRead = readFromBin(&RM, Rfile.c_str());

	std::string Tfile = path + "_D_T";
	bRead = readFromBin(&TM, Tfile.c_str());
}
