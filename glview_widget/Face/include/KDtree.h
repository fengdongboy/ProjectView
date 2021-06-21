#ifndef _KDTREE_H_
#define _KDTREE_H_

#include "..\\tnt\tnt.h"
#include "pnt3.h"
#include <vector>

using namespace TNT;
using namespace std;
using namespace omesh;

//// 计算用函数
//template <class Real>
//inline Real dist(const Vector<Real>& p1, const Vector<Real>& p2)
//{
//	return sqrt(pow(p1[0]-p2[0],2) + pow(p1[1]-p2[1],2) + pow(p1[2]-p2[2],2));
//};  
//
//template <class Real>
//inline Vector<Real> cross(const Vector<Real>& p1, const Vector<Real>& p2)
//{
//	Vector<Real> t(3);
//	t[0] =   p1[1]*p2[2] - p2[1]*p1[2];
//	t[1] = - p1[0]*p2[2] + p2[0]*p1[2];
//	t[2] =   p1[0]*p2[1] - p2[0]*p1[1];
//
//	return t;
//}; 
//
//template <class Real>
//inline void cross(const Vector<Real>& p1, const Vector<Real>& p2, Vector<Real>* t)
//{
//	(*t)[0] =   p1[1]*p2[2] - p2[1]*p1[2];
//	(*t)[1] = - p1[0]*p2[2] + p2[0]*p1[2];
//	(*t)[2] =   p1[0]*p2[1] - p2[0]*p1[1];
//};

/*
// 排序用结构
class VtxListItem {
public:
	VtxListItem(){i=0; value=Vector<double>(3,0.0);}
	VtxListItem(int a, Vector<double> b):i(a), value(b){}
	int i;
	Vector<double> value;
};

inline bool VtxListItem_LessSecond_X(const VtxListItem & m1, const VtxListItem & m2) {
	return m1.value[0] < m2.value[0];
};

inline bool VtxListItem_LessSecond_Y(const VtxListItem & m1, const VtxListItem & m2) {
	return m1.value[1] < m2.value[1];
};

inline bool VtxListItem_LessSecond_Z(const VtxListItem & m1, const VtxListItem & m2) {
	return m1.value[2] < m2.value[2];
};

// KD Tree Class
class KDTree
{
public:
	VtxListItem node;
	double border[3][2]; // 包裹盒边界
	KDTree* leftTree;
	KDTree* rightTree;
	int m_d;	// 分割方向 0 x , 1 y , 2 z

public:
	KDTree()
	{
		m_d = -1;
		leftTree = NULL;
		rightTree = NULL;
	};
	~KDTree()
	{
		if(leftTree != NULL)
			delete(leftTree);
		if(rightTree != NULL)
			delete(rightTree);
	};

	void createTree(vector< Vector<double> >& vtx)
	{
		int vtxNum = int(vtx.size());
		if(vtxNum == 0) return;

		border[0][0] = vtx[0][0];
		border[0][1] = vtx[0][0];
		border[1][0] = vtx[0][1];
		border[1][1] = vtx[0][1];
		border[2][0] = vtx[0][2];
		border[2][1] = vtx[0][2];
		for(int i=1 ; i<vtxNum ; i++)
		{
			border[0][0] = min(border[0][0],vtx[i][0]);
			border[0][1] = max(border[0][1],vtx[i][0]);
			border[1][0] = min(border[1][0],vtx[i][1]);
			border[1][1] = max(border[1][1],vtx[i][1]);
			border[2][0] = min(border[2][0],vtx[i][2]);
			border[2][1] = max(border[2][1],vtx[i][2]);
		}

		m_d = 0;
		if(border[1][1]-border[1][0] > border[0][1]-border[0][0]) m_d = 1;
		if(border[2][1]-border[2][0] > border[1][1]-border[1][0]) m_d = 2;

		vector<VtxListItem> vtxList;
		for(int i=0 ; i<vtxNum ; i++)
		{
			VtxListItem s(i,vtx[i]);
			vtxList.push_back(s);
		}

		if(m_d == 0)
			sort(vtxList.begin(), vtxList.end(), VtxListItem_LessSecond_X);
		if(m_d == 1)
			sort(vtxList.begin(), vtxList.end(), VtxListItem_LessSecond_Y);
		if(m_d == 2)
			sort(vtxList.begin(), vtxList.end(), VtxListItem_LessSecond_Z);

		int medIndex = vtxNum/2;
		node = vtxList[medIndex];

		if(medIndex > 0)
		{
			vector<VtxListItem> vtxListSub1;
			for(int j=0 ; j<medIndex ; j++)
				vtxListSub1.push_back(vtxList[j]);

			leftTree = new KDTree();
			createBinaryTree(vtxListSub1, leftTree);
		}
		if(medIndex < vtxNum-1)
		{
			vector<VtxListItem> vtxListSub2;
			for(int j=medIndex+1 ; j<vtxNum ; j++)
				vtxListSub2.push_back(vtxList[j]);

			rightTree = new KDTree();
			createBinaryTree(vtxListSub2, rightTree);
		}

	};

	void createBinaryTree(vector<VtxListItem>& vtxList, KDTree* tree)
	{
		int vtxNum = int(vtxList.size());
		if(vtxNum == 1)
		{
			tree->node = vtxList[0];
			tree->border[0][0] = vtxList[0].value[0];
			tree->border[0][1] = vtxList[0].value[0];
			tree->border[1][0] = vtxList[0].value[1];
			tree->border[1][1] = vtxList[0].value[1];
			tree->border[2][0] = vtxList[0].value[2];
			tree->border[2][1] = vtxList[0].value[2];
			return;
		}

		tree->border[0][0] = vtxList[0].value[0];
		tree->border[0][1] = vtxList[0].value[0];
		tree->border[1][0] = vtxList[0].value[1];
		tree->border[1][1] = vtxList[0].value[1];
		tree->border[2][0] = vtxList[0].value[2];
		tree->border[2][1] = vtxList[0].value[2];
		for(int i=1 ; i<vtxNum ; i++)
		{
			tree->border[0][0] = min(tree->border[0][0],vtxList[i].value[0]);
			tree->border[0][1] = max(tree->border[0][1],vtxList[i].value[0]);
			tree->border[1][0] = min(tree->border[1][0],vtxList[i].value[1]);
			tree->border[1][1] = max(tree->border[1][1],vtxList[i].value[1]);
			tree->border[2][0] = min(tree->border[2][0],vtxList[i].value[2]);
			tree->border[2][1] = max(tree->border[2][1],vtxList[i].value[2]);
		}

		tree->m_d = 0;
		if(tree->border[1][1] - tree->border[1][0] > tree->border[0][1] - tree->border[0][0]) tree->m_d = 1;
		if(tree->border[2][1] - tree->border[2][0] > tree->border[1][1] - tree->border[1][0]) tree->m_d = 2;

		if(tree->m_d == 0)
			sort(vtxList.begin(), vtxList.end(), VtxListItem_LessSecond_X);
		if(tree->m_d == 1)
			sort(vtxList.begin(), vtxList.end(), VtxListItem_LessSecond_Y);
		if(tree->m_d == 2)
			sort(vtxList.begin(), vtxList.end(), VtxListItem_LessSecond_Z);

		int medIndex = vtxNum/2;
		tree->node = vtxList[medIndex];

		if(medIndex > 0)
		{
			vector<VtxListItem> vtxListSub1;
			for(int j=0 ; j<medIndex ; j++)
				vtxListSub1.push_back(vtxList[j]);

			tree->leftTree = new KDTree();
			createBinaryTree(vtxListSub1, tree->leftTree);
		}
		if(medIndex < vtxNum-1)
		{
			vector<VtxListItem> vtxListSub2;
			for(int j=medIndex+1 ; j<vtxNum ; j++)
				vtxListSub2.push_back(vtxList[j]);

			tree->rightTree = new KDTree();
			createBinaryTree(vtxListSub2, tree->rightTree);
		}
	};
};


inline void kdSearch(Vector<double>& v, KDTree* tree, int* k, double* d)
{
	int tempK;
	double tempD;

	tempD = dist(tree->node.value, v);
	tempK = tree->node.i;

	if(tempD < *d){
		*d = tempD;
		*k = tempK;
	}

	int _d = tree->m_d;
	if(_d == -1) return;

	if(v[_d] <= tree->node.value[_d]){
		if(tree->leftTree != NULL){
			kdSearch(v, tree->leftTree, k, d);
		}
		if(tree->rightTree != NULL){
			if(tree->rightTree->border[_d][0] - v[_d] < *d)
				kdSearch(v, tree->rightTree, k, d);
		}
	}

	if(v[_d] >= tree->node.value[_d]){
		if(tree->rightTree != NULL){
			kdSearch(v, tree->rightTree, k, d);
		}
		if(tree->leftTree != NULL){
			if(v[_d] - tree->leftTree->border[_d][1] < *d)
				kdSearch(v, tree->leftTree, k, d);
		}
	}
};

inline void kdSearch(vector< Vector<double> >* vtxList1, vector< Vector<double> >* vtxList2, vector<int>* K, vector<double>* D)
{
	K->clear();
	D->clear();

	KDTree kdtree;
	kdtree.createTree(*vtxList2);

	for(int i=0 ; i<int(vtxList1->size()) ; i++)
	{
		double d = dist(kdtree.node.value, vtxList1->at(i));
		int k = kdtree.node.i;

		kdSearch(vtxList1->at(i), &kdtree, &k, &d);

		K->push_back(k);
		D->push_back(d);
	}
};

inline void searchClosestPoints(vector< Vector<double> >* vtxList1, vector< Vector<double> >* vtxList2, vector<int>* K, vector<double>* D)
{
	// 使用循环的方法寻找对应点对，没有使用kdtree

	K->clear();
	D->clear();

	int vtxListNum1 = int(vtxList1->size());
	int vtxListNum2 = int(vtxList2->size());

	double d = 0.0;
	int k = 0;
	for(int i=0 ; i<vtxListNum1 ; i++) {
		for(int j=0 ; j<vtxListNum2 ; j++) {
			if(j == 0) {
				d = dist(vtxList1->at(i), vtxList2->at(0));
				k = 0;
			}
			else {
				double tempD = dist(vtxList1->at(i), vtxList2->at(j));
				if(tempD < d) {
					d = tempD;
					k = j;
				}
			}
		}
		K->push_back(k);
		D->push_back(d);
	}
};
*/

static unsigned int g_useNumber = 0;
static unsigned int g_curSize = 0;
static int *pInds = NULL;

int* getIndPtr(unsigned int size)
{
	g_useNumber ++;
	if (size < g_curSize && pInds)
	{
		return pInds;
	}
	if (pInds == NULL)	/// 如果当前没有数组
	{
		pInds = new int[size*2];
		for (int i = 0; i < size*2; i ++)
		{
			pInds[i] = i;
		}
		g_curSize = size*2;
	}
	else
	{
		int* ptemp = new int[size*2];
		memcpy(ptemp, pInds, g_curSize);
		for (int i = g_curSize; i < size*2; i ++)
		{
			ptemp[i] = i;
		}
		delete[] pInds;
		pInds = ptemp;
	}
	return pInds;
}

void deleteInt( void )
{
	if (g_useNumber == 0)
	{
		delete[] pInds;
		pInds = NULL;
		g_curSize = 0;
	}
}

void unUseInt( void )
{
	g_useNumber --;
}

static int
divisionsort(vector<Pnt3> &data, int *p, int n, int dim, float med)
{
	// move values <= med to left, the rest to right
	int left = 0, right = n-1;
	while (1) {
		while (data[p[left]][dim] <= med && right > left) left++;
		while (data[p[right]][dim] > med && right > left) right--;
		int tmp = p[left];  // swap
		p[left] = p[right]; p[right] = tmp;
		if (left == right) {
			if (data[p[right]][dim] <= med) right++;
			break;
		}
		left++;
	}
	return right;
}


// KD Tree Class
class KDTree
{
private:

	int m_d;	// 分割方向 0 x , 1 y , 2 z
	float m_p;	//partition
	Pnt3 min, max;
	int Nhere;	//how many in this node
	KDTree *child[2];
	int *element;


public:
	KDTree()
	{
		m_d = -1;
		child[0] = NULL;
		child[1] = NULL;
	};

	KDTree(vector<Pnt3>& pts, int *ind, int n, int first): Nhere(0), element(NULL)
	{
		int i;
		// find the dimension of maximum range
		min = max = pts[ind[0]];
		int *end = ind+n;
		for (int *ip=ind+1; ip<end; ip++) {
			const Pnt3 &p = pts[*ip];
			min.set_min(p);
			max.set_max(p);
		}

		float dist = max[0] - min[0];
		m_d = 0;
		float tmp;
		if ((tmp = max[1]-min[1]) > dist) {
			m_d = 1; dist = tmp;
		}
		if ((tmp = max[2]-min[2]) > dist) {
			m_d = 2; dist = tmp;
		}

		if (dist == 0.0) n = 1; // a single point several times
		if (n > 16) {
			m_p = 0.5f*(max[m_d]+min[m_d]);
			int right = divisionsort(pts, ind, n, m_d, m_p);
			assert(right != 0 && right != n);
			// recurse
			child[0] = new KDTree(pts, ind, right, 0);
			child[1] = new KDTree(pts, &ind[right], n-right, 0);
		}
		else {
			// store data here
			Nhere = n;
			element = new int[n];
			for (i=0; i<n; i++) element[i] = ind[i];
			child[0] = child[1] = NULL;
		}
	};

	~KDTree()
	{
		delete[] element;
		delete child[0];
		delete child[1];

		/// 删除公用数据
		//deleteInt();
	};

	int _search(vector<Pnt3> &pts, const Pnt3 &p,  int &ind, float &d) const
	{
		assert(this);

		if (Nhere) { // terminal node
			float l, d2 = d*d;
			bool  need_sqrt = false;
			int *el  = element;
			int *end = el+Nhere;
			for (; el<end; el++) {
				l = dist2(pts[*el], p);
				if (l < d2) { 
					d2=l; ind = *el; 
					need_sqrt = true;
				}
			}
			if (need_sqrt) d = sqrtf(d2);
			return ball_within_bounds(p,d,min,max);
		}

		if (p[m_d] <= m_p) { // the point is left from partition
			if (child[0]->_search(pts,p,ind,d)) 
				return 1;
			if (bounds_overlap_ball(p,d,child[1]->min,max)) {
				if (child[1]->_search(pts,p,ind,d)) 
					return 1;
			}
		} else {             // the point is right from partition
			if (child[1]->_search(pts,p,ind,d)) 
				return 1;
			if (bounds_overlap_ball(p,d,min,child[0]->max)) {
				if (child[0]->_search(pts,p,ind,d)) 
					return 1;
			}    
		}

		return ball_within_bounds(p,d,min,max);
	};
	friend void printfTree(KDTree* tree, const std::vector<omesh::Pnt3>* vtxList);
};


///----- 这里的中间序号数组可以共用一个，先标记 /feng
inline KDTree * createKDindtree(vector<Pnt3>& pts)
{
	int nPts = int(pts.size());

	if (nPts==0)
		return NULL;

	// allocate and fill temp index array
	int *inds = new int[nPts];

// 	int *inds = getIndPtr(nPts);
// 	if (inds == NULL)
// 	{
// 		return NULL;
// 	}
	#pragma omp parallel for
	for (int i = 0; i < nPts; i++) inds[i] = i; 

	// build tree
	KDTree* kdtree = new KDTree(pts,  inds, nPts,0);

	// cleanup
	delete[] inds;

	//unUseInt();
	return kdtree;
};

inline void kdSearch(vector<Pnt3>* vtxList1, vector<Pnt3>* vtxList2, vector<int>* K, vector<float>* D)
{
	K->clear();
	D->clear();

	KDTree *kdtree;
	kdtree=createKDindtree(*vtxList2);

	float d = 1e16f;
	int k = 1;
	K->resize(vtxList1->size());
	D->resize(vtxList1->size());
	#pragma omp parallel for private(k,d)	
	for(int i=0 ; i<int(vtxList1->size()) ; i++)
	{
		d = 1e16f;
		kdtree->_search(*vtxList2,vtxList1->at(i),k,d);

		//kdSearch(vtxList1->at(i), &kdtree, &k, &d);

		//K->push_back(k);
		//D->push_back(d);
		K->at(i) = k;
		D->at(i) = d;
	}
	//printfTree(kdtree, vtxList1);
	delete kdtree;
};

// 兼容
inline void kdSearch(vector< Vector<double> >* vtxList1, vector< Vector<double> >* vtxList2, vector<int>* K, vector<double>* D)
{
	vector<Pnt3> vtx1;
	vector<Pnt3> vtx2;

	for(int i=0 ; i<int(vtxList1->size()) ; i++){
		Pnt3 p((*vtxList1)[i][0], (*vtxList1)[i][1], (*vtxList1)[i][2]);
		vtx1.push_back(p);
	}
	for(int i=0 ; i<int(vtxList2->size()) ; i++){
		Pnt3 p((*vtxList2)[i][0], (*vtxList2)[i][1], (*vtxList2)[i][2]);
		vtx2.push_back(p);
	}

	vector<float> DD;

	kdSearch(&vtx1, &vtx2, K, &DD);

	D->clear();
	for(int i=0 ; i<int(DD.size()) ; i++)
		D->push_back(DD[i]);

}

void printfTree(KDTree* tree, const std::vector<omesh::Pnt3>* vtxList)
{
	if (tree)
	{
		if (tree->child[0])
		{
			printfTree(tree->child[0], vtxList);
		}
		if (tree->child[1])
		{
			printfTree(tree->child[1], vtxList);
		}
		printf("md:%d, m_p:%f, nhere:%d, min:%f,%f,%f, max:%f,%f,%f\n",tree->m_d, tree->m_p, tree->Nhere, tree->min[0], tree->min[1], tree->min[2],tree->max[0],tree->max[1],tree->max[2]);
		for (int i = 0; i < tree->Nhere; i ++)
		{
			printf("%d	", tree->element[i]);
		}
		printf("\n");
	}
}

#endif