#ifndef _ODS_UTIL_H_
#define _ODS_UTIL_H_

#include <complex>
#include <math.h>
#include "EasyBMP.h"
#include "tnt.h"
#include "tnt_linalg.h"
#include "utils.h"

#if _MSC_VER >= 1500
#include <omp.h>
#endif

using namespace TNT;
using namespace std;

#ifndef PI
#define PI 3.14159265358979323846264
#endif

#ifndef EPS
#define EPS 2.220446049250313e-016
#endif

#ifndef INF
#define INF 2.220446049250313e016
#endif
// imwrite
template <class Real>
	inline void imWrite_scale(Matrix<Real>& m, const char* filename)
{  
	int ccol = m.num_cols();
	int crow = m.num_rows();
	Real max = m[0][0];
	Real min = m[0][0];
	for(int i=0 ; i<crow ; i++){
		for(int j=0 ; j<ccol ; j++){
			if(m[i][j] > max)
				max = m[i][j];
			if(m[i][j] < min)
				min = m[i][j];
		}
	}

	BMP bmp;
	bmp.SetSize(ccol, crow);
	for(int i=0 ; i<crow ; i++){
		for(int j=0 ; j<ccol ; j++){
			int value;
			if(max-min != 0)
				value = int( ( m[i][j]-min )/(max-min)*255.0 );
			else
				if(double(min) > 0)  // 将min强制转换为double是为了避免输入矩阵为bool类型时， bool>0的警告
					value = 255;
				else
					value = 0;
			bmp(j,i)->Red = value;
			bmp(j,i)->Green = value;
			bmp(j,i)->Blue = value;
		}
	}
	bmp.WriteToFile(filename);
};

// imwrite
template <class Real>
	inline void im_scale(Matrix<Real>& m)
{
	int ccol = m.num_cols();
	int crow = m.num_rows();
	Real max = m[0][0];
	Real min = m[0][0];
	for(int i=0 ; i<crow ; i++){
		for(int j=0 ; j<ccol ; j++){
			if(m[i][j] > max)
				max = m[i][j];
			if(m[i][j] < min)
				min = m[i][j];
		}
	}

	for(int i=0 ; i<crow ; i++){
		for(int j=0 ; j<ccol ; j++){
			if(max-min != 0)
				m[i][j] = Real( ( m[i][j]-min )/(max-min)*255.0 );
			else
				if(min > 0)
					m[i][j] = Real(255);
				else
					m[i][j] = Real(0);
		}
	}
};

//
template <class Real>
	inline void imWrite(Matrix<Real>& m, const char* filename)
{  
	int ccol = m.num_cols();
	int crow = m.num_rows();
	BMP bmp;
	bmp.SetSize(ccol, crow);
	for(int i=0 ; i<crow ; i++){
		for(int j=0 ; j<ccol ; j++){
			bmp(j,i)->Red = ebmpBYTE(m[i][j]);
			bmp(j,i)->Green = ebmpBYTE(m[i][j]);
			bmp(j,i)->Blue = ebmpBYTE(m[i][j]);      		
		}
	}
	bmp.WriteToFile(filename);
};

// 写彩色图像
template <class Real>
inline void imWrite(Matrix<Real>& m_r, Matrix<Real>& m_g, Matrix<Real>& m_b, const char* filename)
{  
	// rgb三个矩阵大小必须相同
	if (!( (m_r.num_cols() == m_g.num_cols()) && (m_r.num_cols() == m_b.num_cols()) ))
		return;
	if (!( (m_r.num_rows() == m_g.num_rows()) && (m_r.num_rows() == m_b.num_rows()) ))
		return;

	int ccol = m_r.num_cols();
	int crow = m_r.num_rows();
	BMP bmp;
	bmp.SetSize(ccol, crow);
	for(int i=0 ; i<crow ; i++){
		for(int j=0 ; j<ccol ; j++){
			bmp(j,i)->Red = ebmpBYTE(m_r[i][j]);
			bmp(j,i)->Green = ebmpBYTE(m_g[i][j]);
			bmp(j,i)->Blue = ebmpBYTE(m_b[i][j]);      		
		}
	}
	bmp.WriteToFile(filename);
};

// read binary file
template <class Real>
inline bool readFromBin(Matrix<Real>* mt, const char* filename)
{
	ifstream ifs(filename, ios::binary);
	if(!ifs){
		ifs.close();
		return false;
	}

	int crow, ccol;
	int i,j;
	int RealSize = sizeof(Real);
	ifs.read((char*)(&crow), sizeof(crow));
	ifs.read((char*)(&ccol), sizeof(ccol));

	*mt = Matrix<Real>(crow,ccol);
	for(i=0 ; i<crow ; i++)
		for(j=0 ; j<ccol ; j++)
			ifs.read((char*)(&((*mt)[i][j])), RealSize);

	ifs.close();
	return true;
}

// readFromTxt
template <class Real>
	inline bool readFromTxt(Matrix<Real>* mt, const char* filename)
{
	cout << "load " << filename << endl;
	ifstream ifs(filename);
	if(!ifs)
		return false;
	int height,width;
	int index = 0;
	while(ifs.good()){
		string bufline;
		getline(ifs, bufline, '\n');
		if(index == 0){
			size_t tempindex = bufline.find(' ');
			height = atoi((bufline.substr(0, tempindex)).c_str());
		}
		if(index == 1){
			size_t tempindex = bufline.find(' ');
			width = atoi((bufline.substr(0, tempindex)).c_str());
		}
		if(index == 2)
			if(height!=0 && width!=0)
				*mt = Matrix<Real>(height,width);
			else
				return false;
		if(index>2 && bufline.length()!=0){
			int colidx = 0;
			size_t idx = -1;
			size_t lidx = 0;
			while(1){
				lidx = idx;
				idx = bufline.find(' ', lidx+1);
				if(idx == -1)
					break;
				(*mt)[index-3][colidx] = Real(atof((bufline.substr(lidx+1,idx-lidx-1)).c_str()));
				colidx ++;
			}
		}
		index ++;
	}
	ifs.close();

	return true;
};

// read from bmp
template <class Real>
	inline bool readFromBmp(Matrix<Real>* mt, const char* filename)
{
	int i,j;
	BMP bmp;
	if(!bmp.ReadFromFile(filename)) return false;
	int crow = bmp.TellHeight();
	int ccol = bmp.TellWidth();
	*mt = Matrix<Real>(crow,ccol);
	for(i=0 ; i<crow ; i++){
		for(j=0 ; j<ccol ; j++){
			(*mt)[i][j] = Real(bmp(j,i)->Red * 0.299 + bmp(j,i)->Green * 0.587 + bmp(j,i)->Blue * 0.114);
		}
	}
	return true;
};

// read from bmp
template <class Real>
	inline bool readFromBmp(Matrix<Real>* r, Matrix<Real>* g, Matrix<Real>* b, const char* filename)
{
	int i,j;
	BMP bmp;
	if(!bmp.ReadFromFile(filename)) return false;
	int crow = bmp.TellHeight();
	int ccol = bmp.TellWidth();
	*r = Matrix<Real>(crow,ccol);
	*g = Matrix<Real>(crow,ccol);
	*b = Matrix<Real>(crow,ccol);
	for(i=0 ; i<crow ; i++){
		for(j=0 ; j<ccol ; j++){
			(*r)[i][j] = Real(bmp(j,i)->Red);
			(*g)[i][j] = Real(bmp(j,i)->Green);
			(*b)[i][j] = Real(bmp(j,i)->Blue);
		}
	}
	return true;
};

// 均值铝波
template <class Real>
	inline void meanfilt(Matrix<Real>* mt, int window, Matrix<int> p)
{
	Matrix<Real> mt2;
	mt2 = *mt;
	int totalNum;
	int height = mt->num_rows();
	int width = mt->num_cols();
	for(int i=window/2 ; i<height-window/2 ; i++){
		for(int j=window/2 ; j<width-window/2 ; j++){
			Real total = Real(0);
			totalNum = 0;
			for(int k=-window/2 ; k<=window/2 ; k++)
				for(int l=-window/2 ; l<=window/2 ; l++)
					if(p[i+k][j+l] == 1){
						total += mt2[i+k][j+l];
						totalNum ++;
					}
			if(totalNum > 0)
				(*mt)[i][j] = total/Real(totalNum);
		}
	}
};

template <class Real>
	inline void meanfilt(Matrix<Real>* mt, int window)
{
	Matrix<Real> mt2;
	mt2 = *mt;
	int totalNum;
	int height = mt->num_rows();
	int width = mt->num_cols();
	for(int i=window/2 ; i<height-window/2 ; i++){
		for(int j=window/2 ; j<width-window/2 ; j++){
			Real total = Real(0);
			totalNum = 0;
			for(int k=-window/2 ; k<=window/2 ; k++)
				for(int l=-window/2 ; l<=window/2 ; l++){
					total += mt2[i+k][j+l];
					totalNum ++;
				}
			if(totalNum > 0)
				(*mt)[i][j] = total/Real(totalNum);
		}
	}
};

// 中值滤波
template <class Real>
	inline void medfilt(Matrix<Real>* mt, int window, Matrix<int> p)
{
	Matrix<Real> mt2;
	mt2 = *mt;
	vector<Real> array;
	int height = mt->num_rows();
	int width = mt->num_cols();

#if _MSC_VER >= 1500
	//#pragma omp parallel for private(array)
#endif

	for(int i=window/2 ; i<height-window/2 ; i++){
		for(int j=window/2 ; j<width-window/2 ; j++){
			array.clear();
			for(int k=-window/2 ; k<=window/2 ; k++)
				for(int l=-window/2 ; l<=window/2 ; l++)
					if(p[i+k][j+l] == 1)
						array.push_back(mt2[i+k][j+l]);
			if(!array.empty()){
				sort(array.begin(), array.end());
				(*mt)[i][j] = array[array.size()/2];
			}
		}
	}
};

//template <class Real>
//	inline void medfilt(Matrix<Real>* mt, int window)
//{
//	Matrix<Real> mt2;
//	mt2 = *mt;
//	vector<Real> array;
//	int height = mt->num_rows();
//	int width = mt->num_cols();
//	for(int i=window/2 ; i<height-window/2 ; i++){
//		for(int j=window/2 ; j<width-window/2 ; j++){
//			array.clear();
//			for(int k=-window/2 ; k<=window/2 ; k++)
//				for(int l=-window/2 ; l<=window/2 ; l++)
//					array.push_back(mt2[i+k][j+l]);
//			if(!array.empty()){
//				sort(array.begin(), array.end());
//				(*mt)[i][j] = array[array.size()/2];
//			}
//		}
//	}
//};

// 高斯滤波
template <class Real>
	inline void guassfilt(Matrix<Real>* mt, int window, double std, Matrix<int> p)
{
	Matrix<Real> mt2;
	mt2 = *mt;
    
	Matrix<double> h(window,window,0.0);
	double total_h=0.0;
	for(int u=0; u<window ; u++){
		for(int v=0; v<window; v++){
			double x = double(u-(window-1)/2);
			double y = double(v-(window-1)/2);
			h[u][v] = exp(-(x*x+y*y)/(2.0*std*std)); 
			total_h = total_h+h[u][v];
		}
	}

	for(int u=0; u<window ; u++){
		for(int v=0; v<window; v++){
			h[u][v] = h[u][v]/total_h;
			
		}
		//cout<<h[u][0]<<" "<<h[u][1]<<" "<<h[u][2]<<" "<<h[u][3]<<" "<<h[u][4]<<endl;
	}

	int height = mt->num_rows();
	int width = mt->num_cols();

#if _MSC_VER >= 1500
	//#pragma omp parallel for firstprivate(h) 
#endif

	for(int i=window/2 ; i<height-window/2 ; i++){
		for(int j=window/2 ; j<width-window/2 ; j++){
			Real total = Real(0);
			for(int k=-window/2 ; k<=window/2 ; k++)
				for(int l=-window/2 ; l<=window/2 ; l++)
					if(p[i+k][j+l] == 1){
						total += mt2[i+k][j+l]*Real(h[k+window/2][l+window/2]);
					}
			(*mt)[i][j] = total;
		}
	}
};

// bilateral滤波
template <class Real>
inline void bilateralfilt(Matrix<Real>* mt, int window, double std_d,double std_r, Matrix<int> p)
{
	Matrix<Real> mt2;
	mt2 = *mt;

	Matrix<double> h(window,window,0.0);
	double total_h=0.0;
	for(int u=0; u<window ; u++){
		for(int v=0; v<window; v++){
			int x = u-(window-1)/2;
			int y = v-(window-1)/2;
			h[u][v] = exp(-(x*x+y*y)/(2.0*std_d*std_d)); 
			total_h = total_h+h[u][v];
		}
	}

	for(int u=0; u<window ; u++){
		for(int v=0; v<window; v++){
			h[u][v] = h[u][v]/total_h;
		}
	}

	double totalNum;
	int height = mt->num_rows();
	int width = mt->num_cols();
	for(int i=window/2 ; i<height-window/2 ; i++){
		for(int j=window/2 ; j<width-window/2 ; j++){
			double total = 0.0;
			totalNum = 0.0;
			for(int k=-window/2 ; k<=window/2 ; k++)
				for(int l=-window/2 ; l<=window/2 ; l++)
					if(p[i+k][j+l] == 1){
						double p = h[k+window/2][l+window/2] * exp(- pow((mt2[i][j]-mt2[i+k][j+l]),2) /(2.0*std_r*std_r));
						total += mt2[i+k][j+l]*p;
						totalNum += p;
					}
					if(totalNum > 0)
						(*mt)[i][j] = Real(total/totalNum);
		}
	}
};

// 去除边界点
inline void removeEdge(Matrix<int>* P, int c)
{
	int tempWindow = 1+c*2;
	Matrix<int> tempP = *P;
	int cRows = P->num_rows();
	int cCols = P->num_cols();
	for(int i=0 ; i<cRows ; i++){
		for(int j=0 ; j<cCols ; j++){
			if(tempP[i][j] == 1){
				for(int m=-tempWindow/2 ; m<=tempWindow/2 ; m++){
					for(int n=-tempWindow/2 ; n<=tempWindow/2 ; n++){
						if(i+m<0 || i+m>=cRows || j+n<0 || j+n>=cCols) //这个判断完全可以通过合理设定i，j的循环范围避免！？
							(*P)[i][j] = 0;
						else{
							if( tempP[i+m][j+n] == 0 )
								(*P)[i][j] = 0;
							//100717：好像与上面完全一样
							//else if(i+m<0 || i+m>=cRows || j+n<0 || j+n>=cCols)
							//	(*P)[i][j] = 0;
						}
					}
				}
			}
		}
	}
}

// save matrix
template <class Real>
	inline bool saveMatrix(Matrix<Real>& mt, const char* filename)
{
	ofstream fs(filename, ios::binary);
	if(!fs){
		fs.close();			
		return false;
	}

	int crow = mt.num_rows();
	int ccol = mt.num_cols();
	fs.write((char*)(&crow), sizeof(crow));
	fs.write((char*)(&ccol), sizeof(ccol));

	int RealSize = sizeof(Real);
	int i,j;
	for(i=0 ; i<crow ; i++)
		for(j=0 ; j<ccol ; j++)
			fs.write((char*)(&(mt[i][j])), RealSize);

	fs.close();

	return true;
};

// save matrix
template <class Real>
inline bool saveVector(Vector<Real>& mt, const char* filename)
{
	ofstream fs(filename, ios::binary);
	if(!fs){
		fs.close();			
		return false;
	}

	int crow = mt.dim();
	int ccol = 1;
	fs.write((char*)(&crow), sizeof(crow));
	fs.write((char*)(&ccol), sizeof(ccol));

	int RealSize = sizeof(Real);
	int i;
	for(i=0 ; i<crow ; i++)
		fs.write((char*)(&(mt[i])), RealSize);

	fs.close();

	return true;
};

// save matrix as txt
template <class Real>
inline bool saveMatrix_txt(Matrix<Real> &mt, const char* filename)
{
	cout<<"save matrix to "<<filename<<endl;
	ofstream sf(filename);
	if(!sf){
		sf.close();			
		return false;
	}

	sf<<(int) mt.num_rows()<<" rows"<<endl;
	sf<<(int) mt.num_cols()<<" columns"<<endl;
	sf<<"# "<<endl;
	for(int i=0; i<mt.num_rows(); i++)
	{
		for(int j=0; j<mt.num_cols(); j++)
		{
			sf<<setprecision(16)<<mt[i][j]<<" ";
		}
		sf<<endl;
	}
	sf.close();
	return true;
};

// matrix inverse
template <class Real>
inline Matrix<Real> matrix_inv(Matrix<Real> &M)
{
	Linear_Algebra::LU<Real> lu(M);
	Matrix<Real> I(M.num_rows(), M.num_cols(), 0.0);
	for(int i=0; i< M.num_cols(); i++)
	{
		I[i][i] = 1;
	}

	return lu.solve(I);	
};

template <class Real>
inline Real vector_norm(Vector<Real> &V)
{
	return sqrt(dot_prod(V, V));
}

template <class Real>
inline Real dist(const Vector<Real>& p1, const Vector<Real>& p2)
{
	return sqrt(pow(p1[0]-p2[0],2) + pow(p1[1]-p2[1],2) + pow(p1[2]-p2[2],2));
};  

template <class Real>
inline Vector<Real> cross(const Vector<Real>& p1, const Vector<Real>& p2)
{
	Vector<Real> t(3);
	t[0] =   p1[1]*p2[2] - p2[1]*p1[2];
	t[1] = - p1[0]*p2[2] + p2[0]*p1[2];
	t[2] =   p1[0]*p2[1] - p2[0]*p1[1];

	return t;
}; 

template <class Real>
inline void cross(const Vector<Real>& p1, const Vector<Real>& p2, Vector<Real>* t)
{
	(*t)[0] =   p1[1]*p2[2] - p2[1]*p1[2];
	(*t)[1] = - p1[0]*p2[2] + p2[0]*p1[2];
	(*t)[2] =   p1[0]*p2[1] - p2[0]*p1[1];
};

template <class Real>
inline void hsv2rgb(Real h, Real s, Real v, Real& r, Real& g, Real& b)
{
	h = Real(6.0) * h;
	Real k = Real(int(h-6.0*EPS));
	Real f = h - k;
	Real t = Real(1.0) - s;
	Real n = Real(1.0) - s * f;
	Real p = Real(1.0) - (s*(Real(1.0)-f));
	Real e = Real(1.0);
	
	r = (k==0)*e + (k==1)*n + (k==2)*t + (k==3)*t + (k==4)*p + (k==5)*e;
	g = (k==0)*p + (k==1)*e + (k==2)*e + (k==3)*n + (k==4)*t + (k==5)*t;
	b = (k==0)*t + (k==1)*t + (k==2)*p + (k==3)*1 + (k==4)*1 + (k==5)*n;
	f = v / max(max(r,g),b);

	r = f * r;
	g = f * g;
	b = f * b;
};

// 解密

//Data为输入数据指针，Log2N=log2(length),flag=-1时为正变换，flag=1时为反变换，变换结果同样由指针Data指向的空间返回
inline void fft(complex<double>*Data,int Log2N,int flag)
{
	double pi = 3.1415926535;
	int i,j,length;
	complex<double> wn;
	length=1<<Log2N;
	complex<double> *temp = new complex<double>[length];

	for(i=0;i<length;i++)
	{
		temp[i]=0;
		for(j=0;j<length;j++)
		{
			wn=complex<double>(cos(2.0*pi/length*i*j),sin(flag*2.0*pi/length*i*j));
			temp[i]+=Data[j]*wn;    
		}           
	}

	if(flag==1)
		for(i=0;i<length;i++)
			Data[i]=temp[i]/double(length);

	if(flag==-1)
		for(i=0;i<length;i++)
			Data[i]=temp[i];

	delete(temp);
}  

inline bool decrypt(Matrix<double>& data, Matrix<double>& lut )
{
	int newsize = data.num_cols()/2;
	int logn = 0;
	int t = newsize;
	while(t>1){
		t = t / 2;
		logn ++;
	}

	if(t != 1){
		cout << "t != 1" << endl;
		return false;
	}

	complex<double> *T;
	T = new complex<double>[newsize];

	for(int i=0 ; i<newsize ; i++)
		T[i] = complex<double>(data[0][i*2],data[0][i*2+1]);

	fft(T, logn, 1);

	lut = Matrix<double>(3, newsize/3);
	int idx = 0;
	for(int i=0 ; i<newsize/3 ; i++){
		for(int j=0 ; j<3 ; j++){
			lut[j][i] = abs(T[idx]) - 10000.0;
			idx ++;
		}
	}

	delete(T);
	return true;
}

inline bool decrypt(string filename, Matrix<double>& lut )
{
	Matrix<double> data;
	if(!readFromBin(&data, filename.c_str()))
		return false;

	return decrypt(data, lut);
}

inline void savelog(string logstr, string logpath="loglog.txt")
{
	return;

	static clock_t start;

	ofstream sf(logpath.c_str(),ios::app);
	if(!sf){
		sf.close();
		return;
	}
	sf << "[ " << clock()-start << " ] - " << logstr << endl;
	start = clock();

	sf.close();
}

#endif

