#ifndef _RANGE_GRID_
#define _RANGE_GRID_

#include "tnt.h"
#include "Mesh.h"
#include "utils.h"
#include "pif_format.h"

using namespace TNT;
using namespace std;

namespace omesh
{

class RangeGrid {

public:
	Matrix<int> P;
	Matrix<float> X;
	Matrix<float> Y;
	Matrix<float> Z;
	Matrix<float> R;
	Matrix<float> G;
	Matrix<float> B;

	float interstep;
	float x0;
	float y0;

public:
	RangeGrid(){ 
		ccol = 0;
		crow = 0; 
		interstep = 0.0f;
		x0 = 0.0f;
		y0 = 0.0f;
	};

	RangeGrid(Matrix<int>& _P, Matrix<float>& _X, Matrix<float>& _Y, Matrix<float>& _Z, float _interstep, float _x0, float _y0)
	{
		P = _P;
		X = _X;
		Y = _Y;
		Z = _Z;
		crow = P.num_rows(); 
		ccol = P.num_cols();

		interstep = _interstep;
		x0 = _x0;
		y0 = _y0;
	};

	RangeGrid( Matrix<float>& _R, Matrix<float>& _G, Matrix<float>& _B,Matrix<int>& _P, Matrix<float>& _X, Matrix<float>& _Y, Matrix<float>& _Z, float _interstep, float _x0, float _y0)
	{
		P = _P;
		X = _X;
		Y = _Y;
		Z = _Z;
		R = _R;
		G = _G;
		B = _B;
		crow = P.num_rows(); 
		ccol = P.num_cols();

		interstep = _interstep;
		x0 = _x0;
		y0 = _y0;
	};

	~RangeGrid(){};
	int crow; 
	int ccol;

	// Read range data from a model file.
	// type - type of model file
	// [0]	- ASCII TXT file
	// [1]	- Binary file
	// [2]	- ASCII PLY file 
	bool readRangeGrid(const char *name, int type=1)
	{
		if(type == 0)
		{
			ifstream ifs(name);
			if(!ifs){
				ifs.close();
				return false;
			}
			int lineIndex;	// line index
			string bufline;	// read line buffer
			int colidx;		// column index
			int idx, lidx;
			lineIndex = 0;
			while(ifs.good())
			{
				getline(ifs, bufline, '\n');
				if(lineIndex == 0){
					size_t tempindex = bufline.find(' ');
					crow = atoi((bufline.substr(0, tempindex)).c_str());
				}
				if(lineIndex == 1){
					size_t tempindex = bufline.find(' ');
					ccol = atoi((bufline.substr(0, tempindex)).c_str());
				}
				if(lineIndex == 2)
				{
					if(crow<=0 || crow>5000 || ccol<=0 || ccol>5000){
						ifs.close();
						crow = 0;
						ccol = 0;
						return false;
					}
					P = Matrix<int>(crow,ccol,0);
					X = Matrix<float>(crow,ccol,0.0f);
					Y = Matrix<float>(crow,ccol,0.0f);
					Z = Matrix<float>(crow,ccol,0.0f);
				}
				if(lineIndex>2 && bufline.length()!=0){
					colidx = 0;
					idx = -1;
					lidx = 0;
					while(1){
						lidx = idx;
						idx = int(bufline.find(' ', lidx+1));
						if(idx == -1)
							break;
						if(lineIndex>=3 && lineIndex<3+crow)
							P[lineIndex-3][colidx] = atoi((bufline.substr(lidx+1,idx-lidx-1)).c_str());
						if(lineIndex>=3+crow && lineIndex<3+crow*2)
							X[lineIndex-3-crow][colidx] = float(atof((bufline.substr(lidx+1,idx-lidx-1)).c_str()));
						if(lineIndex>=3+crow*2 && lineIndex<3+crow*3)
							Y[lineIndex-3-crow*2][colidx] = float(atof((bufline.substr(lidx+1,idx-lidx-1)).c_str()));
						if(lineIndex>=3+crow*3 && lineIndex<3+crow*4)
							Z[lineIndex-3-crow*3][colidx] = float(atof((bufline.substr(lidx+1,idx-lidx-1)).c_str()));
						colidx ++;
					}
				}
				lineIndex ++;
			}
			ifs.close();

			return true;
		}
		if(type == 1)
		{
			ifstream ifs(name, ios::binary);
			if(!ifs){
				ifs.close();
				return false;
			}

			int i,j;
			int intSize = sizeof(int);
			int floatSize = sizeof(float);
			char ch;
			ch = ifs.get(); if(ch != 'O') {	ifs.close(); return false; }
			ch = ifs.get(); if(ch != 'D') {	ifs.close(); return false; }
			ch = ifs.get(); if(ch != 'S') {	ifs.close(); return false; }
			ifs.ignore(32);
			ifs.read((char*)(&crow), sizeof(crow));
			ifs.read((char*)(&ccol), sizeof(ccol));
			P = Matrix<int>(crow,ccol,0);
			X = Matrix<float>(crow,ccol,0.0f);
			Y = Matrix<float>(crow,ccol,0.0f);
			Z = Matrix<float>(crow,ccol,0.0f);
			for(i=0 ; i<crow ; i++)
				for(j=0 ; j<ccol ; j++)
					ifs.read((char*)(&(P[i][j])), intSize);
			for(i=0 ; i<crow ; i++)
				for(j=0 ; j<ccol ; j++)
					if(P[i][j] == 1){
						ifs.read((char*)(&(X[i][j])), floatSize);
						ifs.read((char*)(&(Y[i][j])), floatSize);
						ifs.read((char*)(&(Z[i][j])), floatSize);
					}

			ifs.close();

			return true;
		}
		if(type == 2)
		{
			ifstream ifs(name);
			if(!ifs){
				ifs.close();
				return false;
			}
			int index = 0;
			int vertexNum = 0;
			Matrix<float> vertexList;
			int rowIndex = 0;
			int colIndex = 0;
			int headLine = 0;
			while(ifs.good()){
				string bufline;
				getline(ifs, bufline, '\n');
				size_t tempindex = bufline.find_last_of(' ');
				if(bufline.substr(0,tempindex) == "obj_info num_cols")
					crow = atoi((bufline.substr(tempindex+1)).c_str());
				if(bufline.substr(0,tempindex) == "obj_info num_rows")
					ccol = atoi((bufline.substr(tempindex+1)).c_str());
				if(bufline.substr(0,tempindex) == "element vertex")
					vertexNum = atoi((bufline.substr(tempindex+1)).c_str());
				if(bufline == "end_header"){
					if(crow!=0 && ccol!=0){
						vertexList = Matrix<float>(vertexNum, 3, 0.0f);
						P = Matrix<int>(crow, ccol, 0);
						X = Matrix<float>(crow, ccol, 0.0f);
						Y = Matrix<float>(crow, ccol, 0.0f);
						Z = Matrix<float>(crow, ccol, 0.0f);
					}else{
						crow = 0;
						ccol = 0;
						ifs.close();
						return false;
					}
					headLine = index;
				}
				if(headLine!=0 && index>headLine && bufline.length()!=0){
					int colidx = 0;
					int idx = -1;
					int lidx = 0;
					int tp = 0;
					while(1){
						lidx = idx;
						idx = int(bufline.find(' ', lidx+1));
						if(idx == -1)
							break;
						if(index<=headLine+vertexNum)
							vertexList[index-headLine-1][colidx] = float(atof((bufline.substr(lidx+1,idx-lidx-1)).c_str()));
						else{
							if(colidx == 0)
								tp = atoi((bufline.substr(lidx+1,idx-lidx-1)).c_str());
							if(colidx == 1){
								P[rowIndex][colIndex] = tp;
								if(tp == 1){
									tp = atoi((bufline.substr(lidx+1,idx-lidx-1)).c_str());									
									X[rowIndex][colIndex] = vertexList[tp][0];
									Y[rowIndex][colIndex] = vertexList[tp][1];
									Z[rowIndex][colIndex] = vertexList[tp][2];
								}
							}
						}
						colidx ++;
					}
					if(index > headLine+vertexNum){					
						rowIndex ++;
						if(rowIndex == crow){
							rowIndex = 0;
							colIndex ++;
						}
					}
				}
				index ++;
			}
			ifs.close();

			return true;
		}
		return false;
	};

	// Save range data.
	// type - type of model file
	// [0]	- ASCII TXT file
	// [1]	- Binary file
	bool saveRangeGrid(const char *name, int type=1)
	{
		if(type == 0)
		{
			int i,j;
			ofstream fs(name);
			if(!fs){
				fs.close();			
				return false;
			}
			fs  << crow << " rows" << endl
				<< ccol << " columns" << endl
				<< "#" << endl;
			for(i=0 ; i<crow ; i++){
				for(j=0 ; j<ccol ; j++)
					fs << P[i][j] << " ";
				fs << endl;
			}
			for(i=0 ; i<crow ; i++){
				for(j=0 ; j<ccol ; j++)
					fs << X[i][j] << " ";
				fs << endl;
			}
			for(i=0 ; i<crow ; i++){
				for(j=0 ; j<ccol ; j++)
					fs << Y[i][j] << " ";
				fs << endl;
			}
			for(i=0 ; i<crow ; i++){
				for(j=0 ; j<ccol ; j++)
					fs << Z[i][j] << " ";
				fs << endl;
			}
			fs.close();

			return true;
		}
		if(type == 1)
		{
			ofstream fs(name, ios::binary);
			if(!fs){
				fs.close();			
				return false;
			}

			int i,j;
			char* fileHead = "ODS";
			char fileInfo[32];
			int intSize = sizeof(int);
			int floatSize = sizeof(float);

			fs.write(fileHead, sizeof(char)*3);
			fs.write(fileInfo, sizeof(char)*32);
			fs.write((char*)(&crow), sizeof(crow));
			fs.write((char*)(&ccol), sizeof(ccol));
			for(i=0 ; i<crow ; i++)
				for(j=0 ; j<ccol ; j++)
					fs.write((char*)(&(P[i][j])), intSize);
			for(i=0 ; i<crow ; i++){
				for(j=0 ; j<ccol ; j++){
					if(P[i][j] == 1){
						fs.write((char*)(&(X[i][j])), floatSize);
						fs.write((char*)(&(Y[i][j])), floatSize);
						fs.write((char*)(&(Z[i][j])), floatSize);
					}
				}
			}

			fs.write((char*)(&(interstep)), floatSize);
			fs.write((char*)(&(x0)), floatSize);
			fs.write((char*)(&(y0)), floatSize);

			fs.close();

			return true;
		}
		return false;
	};

	// Create a triangle mesh from scan data.
	Mesh* toMesh(int subSamp=1)
	{
		int i,j,count,vin1,vin2,vin3,vin4;
		float len1,len2;
		Pnt3 coords;

		Mesh* mesh = new Mesh;

		// create a list saying whether a vertex is going to be used
		Matrix<int> vtx_index = Matrix<int>(crow, ccol, -1);

		// create the vertices
		count = 0;
		RangeGridPos rg_pos; // store the position
		rg_pos.row = 0;
		rg_pos.col = 0;
		for(i=0 ; i<crow ; i += subSamp){
			rg_pos.col = 0;
			for(j=0 ; j<ccol ; j += subSamp){
				if (P[i][j] == 1) {
					vtx_index[i][j] = count;
					coords.set(X[i][j], Y[i][j], Z[i][j]);
					mesh->vtx.push_back(coords);
					mesh->rgp.push_back(rg_pos);
					count++;
				}
				rg_pos.col ++;
			}
			rg_pos.row ++;
		}

		// create the triangles
		for(i=0 ; i<crow-subSamp ; i += subSamp){
			for(j=0 ; j<ccol-subSamp ; j += subSamp){

				// count the number of good vertices
				// 1 2
				// 4 3
				vin1 = vtx_index[i][j];
				vin2 = vtx_index[i][j+subSamp];
				vin3 = vtx_index[i+subSamp][j+subSamp];
				vin4 = vtx_index[i+subSamp][j];
				count = (vin1 >= 0) + (vin2 >= 0) + (vin3 >= 0) + (vin4 >=0);

				if (count == 4) {	// all 4 vertices okay, so make 2 tris
					// compute lengths of cross-edges
					len1 = dist(mesh->vtx[vin1], mesh->vtx[vin3]);
					len2 = dist(mesh->vtx[vin2], mesh->vtx[vin4]);

					if (len1 < len2) {
						mesh->addTri(vin2, vin1, vin3);
						mesh->addTri(vin1, vin4, vin3);
					} else {
						mesh->addTri(vin2, vin1, vin4);
						mesh->addTri(vin2, vin4, vin3);
					}
				} else if (count == 3) { // only 3 vertices okay, so make 1 tri
					if        (vin1 == -1) {
						mesh->addTri(vin2, vin4, vin3);
					} else if (vin2 == -1) {
						mesh->addTri(vin1, vin4, vin3);
					} else if (vin3 == -1) {
						mesh->addTri(vin2, vin1, vin4);
					} else { // vin4 == -1
						mesh->addTri(vin2, vin1, vin3);
					}
				}
			}
		}

		mesh->hasRangeGrid = true;
		mesh->crow = crow/subSamp;
		mesh->ccol = ccol/subSamp;
		mesh->interstep = interstep;
		mesh->x0 = x0;
		mesh->y0 = y0;

		if(crow%subSamp!=0) mesh->crow++;
		if(ccol%subSamp!=0) mesh->ccol++;

		mesh->hasVertNormals = false;
		mesh->initNormals();
		//mesh->remove_stepedges_auto(); // 在得到mesh以后再设定边长阈值，去除过长边
		mesh->computeBBox();

		return mesh;
	};

	Mesh* toMesh_RGB(int subSamp=1)
	{
		int i,j,count,vin1,vin2,vin3,vin4;
		float len1,len2;
		Pnt3 coords,coord_rgb;

		Mesh* mesh = new Mesh;

		// create a list saying whether a vertex is going to be used
		Matrix<int> vtx_index = Matrix<int>(crow, ccol, -1);

		// create the vertices
		count = 0;
		RangeGridPos rg_pos; // store the position
		rg_pos.row = 0;
		rg_pos.col = 0;
		for(i=0 ; i<crow ; i += subSamp)
		{
			rg_pos.col = 0;
			for(j=0 ; j<ccol ; j += subSamp)
			{
				if (P[i][j] == 1) {
					vtx_index[i][j] = count;
					coords.set(X[i][j], Y[i][j], Z[i][j]);
					coord_rgb.set((float)R[i][j]/255, (float)G[i][j]/255, (float)B[i][j]/255);
					mesh->vtx.push_back(coords);
					mesh->vtxRgb.push_back(coord_rgb);
					mesh->rgp.push_back(rg_pos);
					count++;
				}
				rg_pos.col ++;
			}
			rg_pos.row ++;
		}

		// create the triangles
		for(i=0 ; i<crow-subSamp ; i += subSamp)
		{
			for(j=0 ; j<ccol-subSamp ; j += subSamp)
			{

				// count the number of good vertices
				// 1 2
				// 4 3
				vin1 = vtx_index[i][j];
				vin2 = vtx_index[i][j+subSamp];
				vin3 = vtx_index[i+subSamp][j+subSamp];
				vin4 = vtx_index[i+subSamp][j];
				count = (vin1 >= 0) + (vin2 >= 0) + (vin3 >= 0) + (vin4 >=0);

				if (count == 4) 
				{	// all 4 vertices okay, so make 2 tris
					// compute lengths of cross-edges
					len1 = dist(mesh->vtx[vin1], mesh->vtx[vin3]);
					len2 = dist(mesh->vtx[vin2], mesh->vtx[vin4]);

					if (len1 < len2) 
					{
						mesh->addTri(vin2, vin1, vin3);
						mesh->addTri(vin1, vin4, vin3);
					} 
					else 
					{
						mesh->addTri(vin2, vin1, vin4);
						mesh->addTri(vin2, vin4, vin3);
					}
				} 
				else if (count == 3) 
				{ // only 3 vertices okay, so make 1 tri
					if        (vin1 == -1) 
					{
						mesh->addTri(vin2, vin4, vin3);
					} 
					else if (vin2 == -1) 
					{
						mesh->addTri(vin1, vin4, vin3);
					} 
					else if (vin3 == -1) 
					{
						mesh->addTri(vin2, vin1, vin4);
					} 
					else 
					{ // vin4 == -1
						mesh->addTri(vin2, vin1, vin3);
					}
				}
			}
		}

		mesh->hasRangeGrid = true;
		mesh->crow = crow/subSamp;
		mesh->ccol = ccol/subSamp;
		mesh->interstep = interstep;
		mesh->x0 = x0;
		mesh->y0 = y0;

		if(crow%subSamp!=0) mesh->crow++;
		if(ccol%subSamp!=0) mesh->ccol++;

		mesh->hasVertNormals = false;
		mesh->initNormals();
		//mesh->remove_stepedges_auto(); // 在得到mesh以后再设定边长阈值，去除过长边
		mesh->computeBBox();

		return mesh;
	};



	bool fromMesh(Mesh& mesh)
	{
		if(!mesh.hasRangeGrid){
			cout << "error !! \n this mesh do not contain rgp parameter that can not be retrieved" << endl;
			return false;
		}

		crow = mesh.crow;
		ccol = mesh.ccol;
		interstep = mesh.interstep;
		x0 = mesh.x0;
		y0 = mesh.y0;

		P = Matrix<int>(crow, ccol, 0);
		X = Matrix<float>(crow, ccol, 0.0f);
		Y = Matrix<float>(crow, ccol, 0.0f);
		Z = Matrix<float>(crow, ccol, 0.0f);

		//stringstream ss;
		//100717：解相位改为并行后，此处出现错误，表现为mesh.rgp.size() > mesh.vtx.size()
		//		  因此暂时做相应修改
		Pnt3 coords;
		for(int i=0 ; i<int(min(mesh.rgp.size(), mesh.vtx.size())) ; i++){
		//for(int i=0 ; i<int(mesh.rgp.size()) ; i++){
			//ss.str("");
			//ss<<i<<" "<<mesh.rgp[i].row<<" "<<mesh.rgp[i].col<<" "<<coords[0]<<" "<<coords[1]<<" "<<coords[2]<<endl;
			//OutputDebugString(ss.str().c_str());
			coords =  mesh.vtx[i];
			P[mesh.rgp[i].row][mesh.rgp[i].col] = 1;
			X[mesh.rgp[i].row][mesh.rgp[i].col] = coords[0];
			Y[mesh.rgp[i].row][mesh.rgp[i].col] = coords[1];
			Z[mesh.rgp[i].row][mesh.rgp[i].col] = coords[2];			
		}

		return true;
	};

	// convert RangeGridPos to p 
	// convert this rg to mesh for display in opengl and retrieve it back
	bool RangeGridPos2P(Mesh& mesh)
	{
		if(!mesh.hasRangeGrid){
			cout << "error !! \n this mesh do not contain rgp parameter that can not be retrieved" << endl;
			return false;
		}
		
		if(crow != mesh.crow || ccol != mesh.ccol){
			cout << "error !! \n the mesh do not has same row and column count with this rangegrid" << endl;
			return false;
		}

		P = Matrix<int>(crow, ccol, 0);
		for(int i=0 ; i<int(mesh.rgp.size()) ; i++){
			P[mesh.rgp[i].row][mesh.rgp[i].col] = 1;
		}

		return true;
	};

	// 得到模型点个数
	int num_verts()
	{
		int i,j;
		int total = 0;
		for(i=0 ; i<crow ; i ++)
			for(j=0 ; j<ccol ; j ++)
				if(P[i][j] == 1)
					total ++;
		return total;
	};

};

}

#endif
