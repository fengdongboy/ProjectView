#ifndef _MESH_
#define _MESH_

#include "pnt3.h"
#include "bbox.h"
#include "defines.h"
#include <vector>
#include <fstream>
#include "TriMeshUtils.h"
#include "utils.h"

namespace omesh
{

// record the position in rangegrid
struct RangeGridPos {
	int row;
	int col;
};

class Mesh {

friend class RangeGrid;

public:

	vector<Pnt3> vtx;
	vector<Pnt3> vtxRgb;
	vector<Pnt3> nrm;
	vector<TriVtx> tris;
	vector<int> bdry; // 1 for boundary vtx, used for registration
	bool hasVertNormals;

	// parameters for store rangegrid format
	bool hasRangeGrid;
	int crow; 
	int ccol;
	vector<RangeGridPos> rgp;
	float interstep;
	float x0;
	float y0;

	Bbox bbox;
	Pnt3 center;
	float radius;

	//wzk 
	Pnt3 transZ;	
	//点原始属于哪个面，用于计算法向量
	vector<int> PointB2Face;

	// initialize parameters
	void init()
	{
		hasVertNormals = false;
		hasRangeGrid = false;

		vtx.clear();
		tris.clear();
		nrm.clear();
		bdry.clear();
		rgp.clear();

	};

	inline void copyVertFrom (Mesh *meshSrc, int src, int dst)
	{
		if (dst < int(this->vtx.size()))
			this->vtx[dst] = meshSrc->vtx[src];
		else
			this->vtx.push_back(meshSrc->vtx[src]);

		if (meshSrc->vtxRgb.size()>0) 
		{
			if (dst < int(this->vtx.size()))
				this->vtxRgb[dst] = meshSrc->vtxRgb[src];
			else
				this->vtxRgb.push_back(meshSrc->vtxRgb[src]);
		}

		if (this->nrm.capacity() && meshSrc->nrm.size()) 
		{
			if (dst < int(this->nrm.size()))
				this->nrm[dst] = meshSrc->nrm[src];
			else
				this->nrm.push_back(meshSrc->nrm[src]);
		}

		if (this->hasRangeGrid && meshSrc->hasRangeGrid && this->rgp.capacity() && meshSrc->rgp.size()) 
		{
			if (dst < int(this->rgp.size()))
				this->rgp[dst] = meshSrc->rgp[src];
			else
				this->rgp.push_back(meshSrc->rgp[src]);
		}
	};

	inline void copyTriFrom (Mesh *meshSrc, int src, int dst)
	{
		if (dst < int(this->tris.size()))
			this->tris[dst] = meshSrc->tris[src];
		else
			this->tris.push_back(meshSrc->tris[src]);
	};

public:
	Mesh()
	{
		init();
	};

	Mesh::Mesh (const vector<Pnt3>& _vtx, const vector<TriVtx>& _tris)
	{
		init();
		vtx = _vtx;
		tris = _tris;

		hasVertNormals = false;
		initNormals();	// 计算法向
		computeBBox();
	}

	~Mesh(){};

	// compute the nrm of vtx
	void initNormals(bool useArea = false)
	{
		if (!hasVertNormals) 
		{
			getVertexNormals(vtx, tris, nrm, useArea);
			hasVertNormals = true;
		}
	};

	// compute 3d bounding box
	void computeBBox()
	{
		bbox.clear();

		for(int i = 0; i < int(vtx.size()); i++) 
		{
			bbox.add(vtx[i]);
		}

		if (vtx.size() == 0) 
		{
			center = Pnt3();
			radius = 0;
		} else 
		{
			center = bbox.center();
			radius = bbox.diag()*0.5f;
		}
	};

	// get border list
	// can only fit manifoldness
	void mark_boundary_verts(void)
	{
		if (bdry.size()) bdry.clear();
		// initialize the boundary flagss
		bdry.insert(bdry.end(), vtx.size(), 0);
		omesh::mark_boundary_verts(bdry, tris);
	};

	void remove_unused_vtxs(void)
	{
		// prepare vertex index map
		vector<int> vtx_ind(vtx.size(), -1);

		// march through the triangles
		// and mark the vertices that are actually used
		int n = int(tris.size());
		for(int i=0; i<n; i++) 
		{
			vtx_ind[tris[i][0]] = tris[i][0];
			vtx_ind[tris[i][1]] = tris[i][1];
			vtx_ind[tris[i][2]] = tris[i][2];
		}

		// remove the vertices that were not marked,
		// also keep tab on how the indices change
		int cnt = 0;
		n = int(vtx.size());
		for(int i=0; i<n; i++) 
		{
			if (vtx_ind[i] != -1) 
			{
				vtx_ind[i] = cnt;
				copyVertFrom (this, i, cnt);
				cnt++;
			}
		}
		vtx.erase(vtx.begin()+cnt, vtx.end());
		if (vtxRgb.size()>0)
			vtxRgb.erase(vtxRgb.begin()+cnt, vtxRgb.end());
		//if(vtxRgb.size()>0)
		//	vtxRgb.erase(vtx.begin()+cnt, vtx.end());
		if(nrm.size()) nrm.erase(nrm.begin()+cnt, nrm.end());
		// march through triangles and correct the indices
		n = int(tris.size());
		for(int i=0; i<n; i++) 
		{
			tris[i][0] = vtx_ind[tris[i][0]];
			tris[i][1] = vtx_ind[tris[i][1]];
			tris[i][2] = vtx_ind[tris[i][2]];
		}
	};

	// 得到临近点列表
	void getNeighborVtxM(vector<Pnt3>& vtx, vector<TriVtx>& tris, vector< vector<int> >& outVec)
	{
		vector< vector<int> >& neighborVtx = outVec;

		neighborVtx.resize(vtx.size());

		for(int i=0 ; i<int(tris.size()) ; i++)
		{
			neighborVtx[tris[i][0]].push_back(tris[i][1]);
			neighborVtx[tris[i][0]].push_back(tris[i][2]);
			neighborVtx[tris[i][1]].push_back(tris[i][0]);
			neighborVtx[tris[i][1]].push_back(tris[i][2]);
			neighborVtx[tris[i][2]].push_back(tris[i][0]);
			neighborVtx[tris[i][2]].push_back(tris[i][1]);
		}

		// 去除冗余点
		for(int i=0 ; i<int(neighborVtx.size()) ; i++)
		{
			vector<int> t = neighborVtx[i];
			neighborVtx[i].clear();
			for(int j=0 ; j<int(t.size()) ; j++)
			{
				bool a = true;
				for(int k=0 ; k<int(neighborVtx[i].size()) ; k++)
				{
					if(t[j]==neighborVtx[i][k])
					{
						a = false;
						break;
					}
				}
				if(a==true)
					neighborVtx[i].push_back(t[j]);
			}		
		}

		//return neighborVtx;
	};

	void divideIntoPieceM(vector<Pnt3>& vtx, vector<TriVtx>& tris, vector< vector<int> >& outVec)
	{
		vector< vector<int> > neighborVtx;
		getNeighborVtxM(vtx, tris, neighborVtx);

		// 点所在面片的序号
		vector<int> vtxPieceIndex(vtx.size(),-1);

		int pieceIndex = 0;
		outVec.clear();
		vector< vector<int> >& Piece = outVec;
		while(1)
		{
			Piece.push_back(vector<int>());

			// 寻找连同区域起始点
			vector<int> cadidateList;
			for(int i=0 ; i<int(vtx.size()) ; i++){
				if(vtxPieceIndex[i] == -1){
					cadidateList.push_back(i);
					vtxPieceIndex[i] = pieceIndex;
					Piece[pieceIndex].push_back(i);
					break;
				}
			}
			if(cadidateList.empty())
				break;

			while(1){
				vector<int> neighbor = neighborVtx[cadidateList[0]];
				cadidateList.erase(cadidateList.begin());

				for(int j=0 ; j<int(neighbor.size()) ; j++){
					if(vtxPieceIndex[neighbor[j]] == -1){
						cadidateList.push_back(neighbor[j]);
						vtxPieceIndex[neighbor[j]] = pieceIndex;
						Piece[pieceIndex].push_back(neighbor[j]);
					}
				}

				if(cadidateList.empty())
					break;
			}

			pieceIndex ++;
		}

		//return Piece;
	}

	void remove_small_piece(int numThreshold = 10)
	{
		vector< vector<int> > Piece;
		divideIntoPieceM(vtx, tris, Piece);

		// prepare vertex index map
		vector<int> vtx_ind(vtx.size(), -1);

		for(int i=0 ; i<int(Piece.size()) ; i++){
			if(int(Piece[i].size()) <= numThreshold){
				for(int j=0 ; j<int(Piece[i].size()) ; j++){
					vtx_ind[Piece[i][j]] = 1;
				}
			}
		}

		// remove the vertices that were not marked,
		// also keep tab on how the indices change
		int cnt = 0;
		int n = int(vtx.size());
		for(int i=0; i<n; i++) {
			if (vtx_ind[i] != -1) {
				vtx_ind[i] = cnt;
				copyVertFrom (this, i, cnt);
				cnt++;
			}
		}
		vtx.erase(vtx.begin()+cnt, vtx.end());
		if (vtxRgb.size()>0)
			vtxRgb.erase(vtxRgb.begin()+cnt, vtxRgb.end());

		//if(vtxRgb.size()>0)
		//	vtxRgb.erase(vtx.begin()+cnt, vtx.end());
		if(nrm.size()) nrm.erase(nrm.begin()+cnt, nrm.end());
		// march through triangles and correct the indices
		n = int(tris.size());
		for(int i=0; i<n; i++) {
			tris[i][0] = vtx_ind[tris[i][0]];
			tris[i][1] = vtx_ind[tris[i][1]];
			tris[i][2] = vtx_ind[tris[i][2]];
		}
	};

	// if a triangle has an edge that's longer than
	// the threshold value, remove the triangle
	void remove_stepedges(float thr)
	{
		omesh::remove_stepedges(vtx, tris, thr);
		//remove_unused_vtxs();
	};

	// find the median edge length
	// if a triangle has an edge that's longer than
	// factor times the median (or percentile), remove the triangle
	void remove_stepedges_auto()
	{
		// calculate the threshold
		int percentile = 50;
		int factor = 4;
		float thr = median_edge_length(vtx, tris, percentile);
		thr *= thr * float(factor * factor);
		// remove stepedges
		omesh::remove_stepedges(vtx, tris, thr);
		//remove_unused_vtxs();
	};

	// compute tris nrm by vtx nrm
	// for opengl display and stl format
	void remove_angle(float Th)
	{	
		int n = int(tris.size());
		for (int i=n-1; i>=0; i--) {
			if(angle_plant_vecter(vtx[tris[i][0]],vtx[tris[i][1]],vtx[tris[i][2]]) < Th){
				tris[i] = tris.back();
				tris.pop_back();
			}
		
		}
	};
	void simulateFaceNormals (vector<Pnt3>& facen)
	{
		facen.clear();
		facen.reserve(tris.size());

		// note: Mesh doesn't store per tri normals any more
		for(int i=0; i<int(tris.size()) ; i++) {
			Pnt3 n  = nrm[tris[i][0]];
			n += nrm[tris[i][1]];
			n += nrm[tris[i][2]];
			n /= 3.0f;
			facen[i] = n;
		}
	};

	void addTri(int v1, int v2, int v3)
	{
		int vs = int(vtx.size());
		if (v1 < 0 || v1 >= vs || v2 < 0 || v2 >= vs || v3 < 0 || v3 >= vs) {
			cerr << "\n\nRed alert: invalid triangle "
				<< v1 << " " << v2 << " "  << v3
				<< " (tri " << int(tris.size())
				<< ", valid range=0.." << vs-1 << ")\n" << endl;
		} else {
			TriVtx tv(v1,v2,v3);
			tris.push_back(tv);
		}
	};

	int num_tris(void) { return int(tris.size()); };
	int num_verts(void) { return int(vtx.size()); };

	// 读取模型
	// 模型文件为二进制格式，包括点列表，面列表
	bool readMesh(const char *name)
	{
		ifstream ifs(name, ios::binary);
		if(!ifs){ ifs.close(); return false; }

		init();

		int i;
		int intSize = sizeof(int);
		int floatSize = sizeof(float);
		char ch;
		ch = ifs.get(); if(ch != 'O') {	ifs.close(); return false; }
		ch = ifs.get(); if(ch != 'D') {	ifs.close(); return false; }
		ch = ifs.get(); if(ch != 'S') {	ifs.close(); return false; }
		ch = ifs.get(); if(ch != 'P') {	ifs.close(); return false; }
		ch = ifs.get(); if(ch != 'F') {	ifs.close(); return false; }
		ifs.ignore(32);
		// 点和面的个数
		int num_verts;
		int num_tris;
		ifs.read((char*)(&num_verts), intSize);
		ifs.read((char*)(&num_tris), intSize);
		// 点列表，面列表
		float p0,p1,p2;
		int t0,t1,t2;
		for(i=0 ; i<num_verts ; i++){
			ifs.read((char*)(&(p0)), floatSize);
			ifs.read((char*)(&(p1)), floatSize);
			ifs.read((char*)(&(p2)), floatSize);
			vtx.push_back(Pnt3(p0,p1,p2));
			//cout << p0 << " " << p1 << " " << p2 << endl;
		}
		for(i=0 ; i<num_tris ; i++){
			ifs.read((char*)(&(t0)), intSize);
			ifs.read((char*)(&(t1)), intSize);
			ifs.read((char*)(&(t2)), intSize);
			addTri(t0, t1, t2);
			//cout << t0 << " " << t1 << " " << t2 << endl;
		}
		ifs.close();

		hasVertNormals = false;
		initNormals();	// 计算法向
		computeBBox();

		// read rgp
		hasRangeGrid = false;
		readRangeGridPos((string(name)+".rgp").c_str());

		return true;
	}

	// 保存模型
	bool saveMesh(const char *name)
	{
		ofstream fs(name, ios::binary);
		if(!fs){ fs.close(); return false; }

		int i;
		int intSize = sizeof(int);
		int floatSize = sizeof(float);
		// 文件头
		char* fileHead = "ODSPF";
		fs.write(fileHead, sizeof(char)*5);
		// 附加信息
		char fileInfo[32];
		fs.write(fileInfo, sizeof(char)*32);
		// 点和面的个数
		int num_verts = int(vtx.size());
		int num_tris = int(tris.size());
		fs.write((char*)(&num_verts), intSize);
		fs.write((char*)(&num_tris), intSize);
		// 点列表，面列表
		for(i=0 ; i<num_verts ; i++){
			fs.write((char*)(&(vtx[i][0])), floatSize);
			fs.write((char*)(&(vtx[i][1])), floatSize);
			fs.write((char*)(&(vtx[i][2])), floatSize);
		}
		for(i=0 ; i<num_tris ; i++){
			fs.write((char*)(&(tris[i][0])), intSize);
			fs.write((char*)(&(tris[i][1])), intSize);
			fs.write((char*)(&(tris[i][2])), intSize);
		}
		fs.close();

		// save rgp
		if(hasRangeGrid)
			saveRangeGridPos((string(name)+".rgp").c_str());

		return true;
	};

	// read rgp info
	// the default rgp file name is meshname + '.rgp'
	bool readRangeGridPos(const char *name)
	{
		ifstream ifs(name, ios::binary);
		if(!ifs){ ifs.close(); return false; }

		hasRangeGrid = true;
		rgp.clear();

		int intSize = sizeof(int);
		int floatSize = sizeof(float);
		char ch;
		ch = ifs.get(); if(ch != 'O') {	ifs.close(); return false; }
		ch = ifs.get(); if(ch != 'D') {	ifs.close(); return false; }
		ch = ifs.get(); if(ch != 'S') {	ifs.close(); return false; }
		ch = ifs.get(); if(ch != 'R') {	ifs.close(); return false; }
		ch = ifs.get(); if(ch != 'G') {	ifs.close(); return false; }
		ch = ifs.get(); if(ch != 'P') {	ifs.close(); return false; }
		ifs.ignore(32);
		// 点的个数
		int num_verts;
		ifs.read((char*)(&num_verts), intSize);
		ifs.read((char*)(&crow), intSize);
		ifs.read((char*)(&ccol), intSize);
		// 点列表，面列表
		RangeGridPos r;
		int RangeGridPosSize = sizeof(RangeGridPos);
		for(int i=0 ; i<num_verts ; i++){
			ifs.read((char*)(&(r)), RangeGridPosSize);
			rgp.push_back(r);
		}
		ifs.close();

		return true;
	};

	bool saveRangeGridPos(const char *name)
	{
		if(!hasRangeGrid) return false;

		ofstream fs(name, ios::binary);
		if(!fs){ fs.close(); return false; }

		int intSize = sizeof(int);
		int floatSize = sizeof(float);
		// 文件头
		char* fileHead = "ODSRGP";
		fs.write(fileHead, sizeof(char)*6);
		// 附加信息
		char fileInfo[32];
		fs.write(fileInfo, sizeof(char)*32);
		// 点个数
		int num_verts = int(vtx.size());
		fs.write((char*)(&num_verts), intSize);
		// 行列
		fs.write((char*)(&crow), intSize);
		fs.write((char*)(&ccol), intSize);
		// 点列表，面列表
		int RangeGridPosSize = sizeof(RangeGridPos);
		for(int i=0 ; i<num_verts ; i++){
			fs.write((char*)(&(rgp[i])), RangeGridPosSize);
		}
		fs.close();

		return true;
	};

};

}


#endif
