#ifndef _TRIMESHUTILS_H_
#define _TRIMESHUTILS_H_

#include "pnt3.h"
#include "bbox.h"
#include "median.h"
#include "defines.h"
#include <vector>
#include <algorithm>

namespace omesh
{

// the index of three vertices in a triangle
class TriVtx { 
private:
	int v[3];
public:
	TriVtx(int a=0, int b=0, int c=0) { v[0] = a, v[1] = b, v[2] = c;}
	inline int& operator[](int i) { return v[i]; }
	inline const int& operator[](int i) const { return v[i]; }
};

// calculate vertex normals by averaging from triangle
// normals (obtained from cross products)
// possibly weighted with triangle areas
inline void getVertexNormals(const vector<Pnt3> &vtx,
	const vector<TriVtx> &tris,
	vector<Pnt3> &nrm,
	bool useArea )
{
	Pnt3 norm;
	vector<Pnt3>::iterator n;
	nrm = vector<Pnt3>(vtx.size(), Pnt3());
	int ntris = int(tris.size());
	if (!useArea) {
		for(int i = 0; i < ntris; i ++) {
			norm = normal(vtx[tris[i][0]], vtx[tris[i][1]], vtx[tris[i][2]]);
			nrm[tris[i][0]] += norm;
			nrm[tris[i][1]] += norm;
			nrm[tris[i][2]] += norm;
		}
	} else { // do use area norm
		for (int i = 0; i < ntris; i ++) {
			norm = cross(vtx[tris[i][0]], vtx[tris[i][1]], vtx[tris[i][2]]);
			nrm[tris[i][0]] += norm;
			nrm[tris[i][1]] += norm;
			nrm[tris[i][2]] += norm;
		}
	}
	// unitary
	for(n = nrm.begin(); n != nrm.end(); n++) {
		n->set_norm(1.0f);
	}
}

// assume bdry has the right size and has been initialized with zeroes
// for each vertex, try to find a full loop around it, if can't its boundary
inline void mark_boundary_verts(vector<int> &bdry,
								const vector<TriVtx> &tris)
{
	int nv = int(bdry.size());
	int ntri = int(tris.size());
	vector< vector<int> >nbors;
	vector<int> sub;
	for(int i=0 ; i<nv ; i++){
		nbors.push_back(sub);
	}
	for(int i=0 ; i<ntri ; i++){
		(nbors[tris[i][0]]).push_back(tris[i][1]);
		(nbors[tris[i][0]]).push_back(tris[i][2]);
		(nbors[tris[i][1]]).push_back(tris[i][0]);
		(nbors[tris[i][1]]).push_back(tris[i][2]);
		(nbors[tris[i][2]]).push_back(tris[i][0]);
		(nbors[tris[i][2]]).push_back(tris[i][1]);
	}
	for(int i=0 ; i<nv ; i++){
		sort(nbors[i].begin(), nbors[i].end());
		int n = int(nbors[i].size());
		if(n < 6){
			bdry[i] = 1;
			continue;
		}else{
			bdry[i] = 0;
			for(int j=0 ; j<n ; j+=2){
				if(nbors[i][j] != nbors[i][j+1]){
					bdry[i] = 1;
					break;
				}
			}
		}
	}
}

// find the median (or percentile) edge length
inline float median_edge_length(vector<Pnt3> &vtx,
								vector<TriVtx> &tris,
								int percentile)
{
	int i;
	int n = int(tris.size());
	if (n < 1) return 0.0f;

	Median<float> med(percentile, n);

	// edge lengths
	for(i=0; i<n; i++) {
		med += dist2(vtx[tris[i][0]], vtx[tris[i][1]]);
		med += dist2(vtx[tris[i][2]], vtx[tris[i][1]]);
		med += dist2(vtx[tris[i][0]], vtx[tris[i][2]]);
	} 

	return sqrtf(med.find());
}

// if a triangle has an edge that's longer than
// the threshold value, remove the triangle
inline void remove_stepedges(vector<Pnt3> &vtx,
							 vector<TriVtx> &tris,
							 float thr)
{
	int n = int(tris.size());
	for (int i=n-1; i>=0; i--) {
		if (dist2(vtx[tris[i][0]], vtx[tris[i][1]]) > thr ||
			dist2(vtx[tris[i][2]], vtx[tris[i][1]]) > thr ||
			dist2(vtx[tris[i][0]], vtx[tris[i][2]]) > thr) {
				// remove the triangle
				tris[i] = tris.back(); tris.pop_back();
		}
	}
}

}


#endif