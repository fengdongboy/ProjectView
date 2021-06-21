#ifndef _PIF_FORMAT_H_
#define _PIF_FORMAT_H_

#include "xform.h"
#include "TriMeshUtils.h"
#include "tnt.h"
#include "odsutil.h"

using namespace TNT;

namespace omesh
{

//   Endian 转换(64位, 32位和16位的字节序转换)
//   中间使用了异或交换的算法
//   使用方法举例:
//   long lValue = 0xff000000;
//   ConvertEndian32( &lValue );

// double 64/8=8 byte
template <class T>
inline void ConvertEndian64( T* lpMem )
{
	char * p = (char*)lpMem;
	p[0] = p[0] ^ p[7];
	p[7] = p[0] ^ p[7];
	p[0] = p[0] ^ p[7];

	p[1] = p[1] ^ p[6];
	p[6] = p[1] ^ p[6];
	p[1] = p[1] ^ p[6];

	p[2] = p[2] ^ p[5];
	p[5] = p[2] ^ p[5];
	p[2] = p[2] ^ p[5];

	p[3] = p[3] ^ p[4];
	p[4] = p[3] ^ p[4];
	p[3] = p[3] ^ p[4];
}
// int long float 32/8=4 byte
template <class T>
inline void ConvertEndian32( T* lpMem )
{
	char * p = (char*)lpMem;
	p[0] = p[0] ^ p[3];
	p[3] = p[0] ^ p[3];
	p[0] = p[0] ^ p[3];
	p[1] = p[1] ^ p[2];
	p[2] = p[1] ^ p[2];
	p[1] = p[1] ^ p[2];
}
//
template <class T>
inline void ConvertEndian16( T* lpMem )
{
	char * p = (char*)lpMem;
	p[0] = p[0] ^ p[1];
	p[1] = p[0] ^ p[1];
	p[0] = p[0] ^ p[1];
}

// PIF Format header struct
struct PIF_header {
	char format_version[64];
	char user_comments[128];
	char dummy1[8];
	long image_param_flag;
	long image_data_type;
	float invalid_point;
	long array_width;
	long array_height;
	long data_block_length;
	long scale_flag;
	float i_scale;
	float j_scale;
	long transfo_matrix_flag;
	double transfo_matrix[16];
	long image_color_flag;
	long color_block_length;
	long camera_position_flag;
	float camera_x;
	float camera_y;
	float camera_z;
	long dummy2[30];
};

// 将 PIF_header 做大小尾之间转换
inline void PIF_header_ConvertEndian(PIF_header& header)
{
	ConvertEndian32(&(header.image_param_flag));
	ConvertEndian32(&(header.image_data_type));
	ConvertEndian32(&(header.invalid_point));
	ConvertEndian32(&(header.array_width));
	ConvertEndian32(&(header.array_height));
	ConvertEndian32(&(header.data_block_length));
	ConvertEndian32(&(header.scale_flag));
	ConvertEndian32(&(header.i_scale));
	ConvertEndian32(&(header.j_scale));
	ConvertEndian32(&(header.transfo_matrix_flag));
	for(int i=0 ; i<16 ; i++){
		ConvertEndian64(&(header.transfo_matrix[i]));
	}
	ConvertEndian32(&(header.image_color_flag));
	ConvertEndian32(&(header.color_block_length));
	ConvertEndian32(&(header.camera_position_flag));
	ConvertEndian32(&(header.camera_x));
	ConvertEndian32(&(header.camera_y));
	ConvertEndian32(&(header.camera_z));
}

// read pif format file header and show it
inline bool readPIFHeader(const char* name)
{
	ifstream ifs(name, ios::binary);
	if(!ifs){ ifs.close(); return false; }

	PIF_header header;
	ifs.read((char*)(&header), sizeof(header));

	PIF_header_ConvertEndian(header);

	cout << "format_version      " << header.format_version        << endl;
	cout << "user_comments       " << header.user_comments         << endl;
	cout << "dummy1              " << header.dummy1                << endl;
	cout << "image_param_flag    " << header.image_param_flag      << endl;
	cout << "image_data_type     " << header.image_data_type       << endl;
	cout << "invalid_point       " << header.invalid_point         << endl;
	cout << "array_width         " << header.array_width           << endl;
	cout << "array_height        " << header.array_height          << endl;
	cout << "data_block_length   " << header.data_block_length     << endl;
	cout << "scale_flag          " << header.scale_flag            << endl;
	cout << "i_scale             " << header.i_scale               << endl;
	cout << "j_scale             " << header.j_scale               << endl;
	cout << "transfo_matrix_flag " << header.transfo_matrix_flag  << endl;
	cout << "transfo_matrix      " << header.transfo_matrix[0] << "  " << header.transfo_matrix[1] << "  " << header.transfo_matrix[2] << "  " << header.transfo_matrix[3] << endl
		<< "                    " << header.transfo_matrix[4] << "  " << header.transfo_matrix[5] << "  " << header.transfo_matrix[6] << "  " << header.transfo_matrix[7] << endl
		<< "                    " << header.transfo_matrix[8] << "  " << header.transfo_matrix[9] << "  " << header.transfo_matrix[10] << "  " << header.transfo_matrix[11] << endl
		<< "                    " << header.transfo_matrix[12] << "  " << header.transfo_matrix[13] << "  " << header.transfo_matrix[14] << "  " << header.transfo_matrix[15] << endl;
	cout << "image_color_flag    " << header.image_color_flag      << endl;
	cout << "color_block_length  " << header.color_block_length    << endl;
	cout << "camera_position_flag" << header.camera_position_flag  << endl;
	cout << "camera_x            " << header.camera_x              << endl;
	cout << "camera_y            " << header.camera_y              << endl;
	cout << "camera_z            " << header.camera_z              << endl;
	cout << "dummy2              " << header.dummy2                << endl;

	ifs.close();

	return true;
}

inline bool PXYZ2SubGrid(Matrix<int>& P,
						 Matrix<float>& X,
						 Matrix<float>& Y,
						 Matrix<float>& Z,
						 float interstep,
						 Matrix<int>& NP,
						 Matrix<float>& NX,
						 Matrix<float>& NY,
						 Matrix<float>& NZ,
						 float& x0,
						 float& y0 )
{
	// compute boundary box

	float x_near = (float)INF;
	float y_near = (float)INF;
	float z_near = (float)INF;
	float x_far = (float)-INF;
	float y_far = (float)-INF;
	float z_far = (float)-INF;

	for(int i=0 ; i<P.num_rows() ; i++){
		for(int j=0 ; j<P.num_cols() ; j++){
			if(P[i][j] == 1){
				if(X[i][j] < x_near)
					x_near = X[i][j];
				if(Y[i][j] < y_near)
					y_near = Y[i][j];
				if(Z[i][j] < z_near)
					z_near = Z[i][j];
				if(X[i][j] > x_far)
					x_far = X[i][j];
				if(Y[i][j] > y_far)
					y_far = Y[i][j];
				if(Z[i][j] > z_far)
					z_far = Z[i][j];

			}
		}
	}

	printf("Boundary box : %f %f %f\n", x_far-x_near, y_far-y_near, z_far-z_near);
	printf("Interpolation Step : %f\n", interstep);

	int crow = int((x_far-x_near)/interstep) + 10;
	int ccol = int((y_far-y_near)/interstep) + 10;

	printf("Grid size : %i x %i\n", crow, ccol);

	x0 = x_near - interstep/2;
	y0 = y_near - interstep/2;

	NP = Matrix<int>(crow, ccol, 0);
	NX = Matrix<float>(crow, ccol, 0.0f);
	NY = Matrix<float>(crow, ccol, 0.0f);
	NZ = Matrix<float>(crow, ccol, 0.0f);
	
	for(int i=0 ; i<crow ; i++){
		for(int j=0 ; j<ccol ; j++){
			NX[i][j] = i*interstep+x0;
			NY[i][j] = j*interstep+y0;
		}
	}

	// interpolate Z
	Matrix<int> PP(crow, ccol, 0);
	for(int k=0 ; k<P.num_rows()-1 ; k++){
		for(int l=0 ; l<P.num_cols()-1 ; l++){
			if( P[k][l]==0 || P[k+1][l]==0 || P[k+1][l+1]==0 || P[k][l+1]==0 )
				continue;

			int a1 = int((min(min(min(X[k][l],X[k+1][l]),X[k+1][l+1]),X[k][l+1])-x0)/interstep);
			int a2 = int((max(max(max(X[k][l],X[k+1][l]),X[k+1][l+1]),X[k][l+1])-x0)/interstep)+1;
			int b1 = int((min(min(min(Y[k][l],Y[k+1][l]),Y[k+1][l+1]),Y[k][l+1])-y0)/interstep);
			int b2 = int((max(max(max(Y[k][l],Y[k+1][l]),Y[k+1][l+1]),Y[k][l+1])-y0)/interstep)+1;

			float d11,d21,d31,d41,d12,d22,d32,d42;
			d11 = (X[k  ][l  ]-x0)/interstep;   d12 = (Y[k  ][l  ]-y0)/interstep; 
			d21 = (X[k+1][l  ]-x0)/interstep;   d22 = (Y[k+1][l  ]-y0)/interstep; 
			d31 = (X[k+1][l+1]-x0)/interstep;   d32 = (Y[k+1][l+1]-y0)/interstep; 
			d41 = (X[k  ][l+1]-x0)/interstep;   d42 = (Y[k  ][l+1]-y0)/interstep; 

			float p11,p12,p21,p22,q11,q12,q21,q22;
			p11 = (d22-d12)/(d21-d11);    q11 = d12-p11*d11;
			p12 = (d32-d42)/(d31-d41);    q12 = d42-p12*d41;
			p21 = (d41-d11)/(d42-d12);    q21 = d11-p21*d12;
			p22 = (d31-d21)/(d32-d22);    q22 = d21-p22*d22;

			for(int i=a1 ; i<=a2 ; i++)
				for(int j=b1 ; j<=b2 ; j++)
					PP[i][j] = 0;

			for(int i=a1 ; i<=a2 ; i++){
				int r1 = int(p11*i+q11)+1;
				int r2 = int(p12*i+q12);
				for(int j=max(r1,b1) ; j<=min(r2,b2) ; j++)			
					PP[i][j] = PP[i][j]+1;
			}

			for(int i=b1 ; i<=b2 ; i++){
				int r1 = int(p21*i+q21)+1;
				int r2 = int(p22*i+q22);
				for(int j=max(r1,a1) ; j<=min(r2,a2) ; j++)	
					PP[j][i] = PP[j][i]+1;
			}

			for(int i=a1 ; i<=a2 ; i++){
				for(int j=b1 ; j<=b2 ; j++){
					if(PP[i][j]==2){

						//  interpolate z
						float x1,y1,z1,x2,y2,z2,nz1,nz2;
						bool va1 = true;
						bool va2 = true;
						
						if( X[k+1][l  ]-X[k][l  ]==0 || X[k+1][l+1]-X[k][l+1]==0 )
							va1 = false;
						else{
							x1 = NX[i][j];
							x2 = NX[i][j];
							y1 = (x1-X[k][l  ])/(X[k+1][l  ]-X[k][l  ])*(Y[k+1][l  ]-Y[k][l  ])+Y[k][l  ];
							y2 = (x2-X[k][l+1])/(X[k+1][l+1]-X[k][l+1])*(Y[k+1][l+1]-Y[k][l+1])+Y[k][l+1];
							z1 = (x1-X[k][l  ])/(X[k+1][l  ]-X[k][l  ])*(Z[k+1][l  ]-Z[k][l  ])+Z[k][l  ];
							z2 = (x2-X[k][l+1])/(X[k+1][l+1]-X[k][l+1])*(Z[k+1][l+1]-Z[k][l+1])+Z[k][l+1];
							if( y2-y1==0 )
								va1 = false;
							else
								nz1 = (NY[i][j]-y1)/(y2-y1)*(z2-z1)+z1;
						}

						if( Y[k  ][l+1]-Y[k  ][l]==0 || Y[k+1][l+1]-Y[k+1][l]==0 )
							va2 = false;
						else{
							y1 = NY[i][j];
							y2 = NY[i][j];
							x1 = (y1-Y[k  ][l])/(Y[k  ][l+1]-Y[k  ][l])*(X[k  ][l+1]-X[k  ][l])+X[k  ][l];
							x2 = (y2-Y[k+1][l])/(Y[k+1][l+1]-Y[k+1][l])*(X[k+1][l+1]-X[k+1][l])+X[k+1][l];
							z1 = (y1-Y[k  ][l])/(Y[k  ][l+1]-Y[k  ][l])*(Z[k  ][l+1]-Z[k  ][l])+Z[k  ][l];
							z2 = (y2-Y[k+1][l])/(Y[k+1][l+1]-Y[k+1][l])*(Z[k+1][l+1]-Z[k+1][l])+Z[k+1][l];
							if( x2-x1==0 )
								va2 = false;
							else
								nz2 = (NX[i][j]-x1)/(x2-x1)*(z2-z1)+z1;
						}

						if( va1==false && va2==false )
							continue;

						if( va1==true && va2==false )
							NZ[i][j] = nz1;

						if( va1==false && va2==true )
							NZ[i][j] = nz2;

						if( va1==true && va2==true )
							NZ[i][j] = (nz1+nz2)/2;

						NP[i][j] = 1;

					}
				}
			}

		}
	}

	return true;
}

inline bool savePIF(const char* name,
					Matrix<int>& P,
					Matrix<float>& Z,
					float interstep,
					float x0,
					float y0 )
{
	ofstream fs(name, ios::binary);
	if(!fs){
		fs.close();			
		return false;
	}

	float r[3][3], t[3];
	r[0][0]=1; r[0][1]=0; r[0][2]=0;  
	r[1][0]=0; r[1][1]=1; r[1][2]=0;  
	r[2][0]=0; r[2][1]=0; r[2][2]=1; 

	t[0]=-x0; t[1]=-y0; t[2]=0; 

	// 为了和polyworks保存格式兼容
	Matrix<int> NP = transpose(P);
	Matrix<float> NZ = transpose(Z);

	int crow = NP.num_rows();
	int ccol = NP.num_cols();

	PIF_header header;
	// PIF Format v2.0
	header.format_version[0]  = 'P';
	header.format_version[1]  = 'I';
	header.format_version[2]  = 'F';
	header.format_version[3]  = ' ';
	header.format_version[4]  = 'F';
	header.format_version[5]  = 'o';
	header.format_version[6]  = 'r';
	header.format_version[7]  = 'm';
	header.format_version[8]  = 'a';
	header.format_version[9]  = 't';
	header.format_version[10] = ' ';
	header.format_version[11] = 'v';
	header.format_version[12] = '2';
	header.format_version[13] = '.';
	header.format_version[14] = '0';
	header.format_version[15] = '\0';
	header.user_comments[0] = '\0';
	header.dummy1[0] = '\0';

	header.image_param_flag = 0;
	header.image_data_type = 0;

	header.invalid_point = 0;
	header.array_width = ccol;
	header.array_height = crow;
	header.data_block_length = crow*ccol*sizeof(float);
	header.scale_flag = 1;
	header.i_scale = interstep;
	header.j_scale = interstep;
	header.transfo_matrix_flag = 1;
	header.transfo_matrix[0]= r[0][0]; header.transfo_matrix[1]= r[0][1]; header.transfo_matrix[2]= r[0][2]; header.transfo_matrix[3]= t[0];
	header.transfo_matrix[4]= r[1][0]; header.transfo_matrix[5]= r[1][1]; header.transfo_matrix[6]= r[1][2]; header.transfo_matrix[7]= t[1];
	header.transfo_matrix[8]= r[2][0]; header.transfo_matrix[9]= r[2][1]; header.transfo_matrix[10]=r[2][2]; header.transfo_matrix[11]=t[2];
	header.transfo_matrix[12]=0; header.transfo_matrix[13]=0; header.transfo_matrix[14]=0; header.transfo_matrix[15]=1;
	header.image_color_flag = 0;
	header.color_block_length = 0;
	header.camera_position_flag = 0;
	header.camera_x = 0;
	header.camera_y = 0;
	header.camera_z = 0;

	PIF_header_ConvertEndian(header);

	fs.write((char*)(&header), sizeof(PIF_header));		

	int floatSize = sizeof(float);
	for(int i=0 ; i<crow ; i++){
		for(int j=0 ; j<ccol ; j++){
			float tz = NZ[i][j];
			if(NP[i][j]==0)
				tz = 0.0f;

			ConvertEndian32(&(tz));

			fs.write((char*)(&(tz)), floatSize);
		}
	}

	fs.close();

	return true;
}

inline bool savePIF(const char* name,
					Matrix<int>& P,
					Matrix<float>& X,
					Matrix<float>& Y,
					Matrix<float>& Z
					)
{
	ofstream fs(name, ios::binary);
	if(!fs){
		fs.close();			
		return false;
	}

	int crow = P.num_rows();
	int ccol = P.num_cols();

	float r[3][3];
	float t[3];
	r[0][0]=1.0f; r[0][1]=0.0f; r[0][2]=0.0f; 
	r[1][0]=0.0f; r[1][1]=1.0f; r[1][2]=0.0f; 
	r[2][0]=0.0f; r[2][1]=0.0f; r[2][2]=1.0f; 
	t[0]=0.0f; t[1]=0.0f; t[2]=0.0f; 

	PIF_header header;
	// PIF Format v2.0
	header.format_version[0]  = 'P';
	header.format_version[1]  = 'I';
	header.format_version[2]  = 'F';
	header.format_version[3]  = ' ';
	header.format_version[4]  = 'F';
	header.format_version[5]  = 'o';
	header.format_version[6]  = 'r';
	header.format_version[7]  = 'm';
	header.format_version[8]  = 'a';
	header.format_version[9]  = 't';
	header.format_version[10] = ' ';
	header.format_version[11] = 'v';
	header.format_version[12] = '2';
	header.format_version[13] = '.';
	header.format_version[14] = '0';
	header.format_version[15] = '\0';
	header.user_comments[0] = '\0';
	header.dummy1[0] = '\0';

	header.image_param_flag = 0;
	header.image_data_type = 1;

	header.invalid_point = 0;
	header.array_width = ccol;
	header.array_height = crow;
	header.data_block_length = crow*ccol*3*sizeof(float);
	header.scale_flag = 0;
	header.i_scale = 1;
	header.j_scale = 1;
	header.transfo_matrix_flag = 1;
	header.transfo_matrix[0]= r[0][0]; header.transfo_matrix[1]= r[0][1]; header.transfo_matrix[2]= r[0][2]; header.transfo_matrix[3]= t[0];
	header.transfo_matrix[4]= r[1][0]; header.transfo_matrix[5]= r[1][1]; header.transfo_matrix[6]= r[1][2]; header.transfo_matrix[7]= t[1];
	header.transfo_matrix[8]= r[2][0]; header.transfo_matrix[9]= r[2][1]; header.transfo_matrix[10]=r[2][2]; header.transfo_matrix[11]=t[2];
	header.transfo_matrix[12]=0; header.transfo_matrix[13]=0; header.transfo_matrix[14]=0; header.transfo_matrix[15]=1;
	header.image_color_flag = 0;
	header.color_block_length = 0;
	header.camera_position_flag = 0;
	header.camera_x = 0;
	header.camera_y = 0;
	header.camera_z = 0;

	PIF_header_ConvertEndian(header);

	fs.write((char*)(&header), sizeof(PIF_header));		

	int floatSize = sizeof(float);
	float tx,ty,tz;
	for(int i=crow-1 ; i>=0 ; i--){
		for(int j=0 ; j<ccol ; j++){
			tx = X[i][j];
			ty = Y[i][j];
			tz = Z[i][j];
			if(P[i][j]==0)
				tz = 0.0f;

			ConvertEndian32(&(tx));
			ConvertEndian32(&(ty));
			ConvertEndian32(&(tz));

			fs.write((char*)(&(tx)), floatSize);
			fs.write((char*)(&(ty)), floatSize);
			fs.write((char*)(&(tz)), floatSize);
		}
	}

	fs.close();
	return true;
}

// 保存PIF文件
inline bool savePIF(const char* name,
					Matrix<int>& P,
					Matrix<float>& X,
					Matrix<float>& Y,
					Matrix<float>& Z,
					float interstep,
					float x0,
					float y0 )
{
	if(interstep > 0)
		return savePIF(name, P, Z, interstep, x0, y0);
	else
		return savePIF(name, P, X, Y, Z);
}

// 读取PIF文件
inline bool readPIF(const char* name,
					Matrix<int>& P,
					Matrix<float>& X,
					Matrix<float>& Y,
					Matrix<float>& Z,
					float& interstep,
					float& x0,
					float& y0 )
{
	ifstream ifs(name, ios::binary);
	if(!ifs){ ifs.close(); return false; }

	PIF_header header;
	ifs.read((char*)(&header), sizeof(header));

	PIF_header_ConvertEndian(header);

	// 判断文件头
	if(string(header.format_version) != "PIF Format v2.0"){
		ifs.close();
		return false;
	}

	// 判断是否可以读入
	if(header.image_data_type != 0 && header.image_data_type != 1){
		ifs.close();
		return false;
	}

	if(header.image_data_type == 0){

		int crow = header.array_height;
		int ccol = header.array_width;

		P = Matrix<int>(crow, ccol, 1);
		Z = Matrix<float>(crow, ccol);

		int floatSize = sizeof(float);
		float tz;
		for(int i=0 ; i<crow ; i++){
			for(int j=0 ; j<ccol ; j++){
				ifs.read((char*)(&tz), floatSize);

				ConvertEndian32(&(tz));
				Z[i][j] = tz;

				if(tz == 0.0f)
					P[i][j] = 0;
			}
		}

		// 为了和polyworks保存格式兼容
		P = transpose(P);
		Z = transpose(Z);

		interstep = header.i_scale;
		x0 = float( -header.transfo_matrix[3] );
		y0 = float( -header.transfo_matrix[7] );

		crow = P.num_rows();
		ccol = P.num_cols();
		X = Matrix<float>(crow, ccol);
		Y = Matrix<float>(crow, ccol);

		for(int i=0 ; i<crow ; i++){
			for(int j=0 ; j<ccol ; j++){
				X[i][j] = i*interstep+x0;
				Y[i][j] = j*interstep+y0;
			}
		}

	}

	if(header.image_data_type == 1){

		int crow = header.array_height;
		int ccol = header.array_width;

		P = Matrix<int>(crow, ccol, 1);
		X = Matrix<float>(crow, ccol);
		Y = Matrix<float>(crow, ccol);
		Z = Matrix<float>(crow, ccol);

        int floatSize = sizeof(float);
		float tx,ty,tz;
		for(int i=crow-1 ; i>=0 ; i--){
			for(int j=0 ; j<ccol ; j++){
				ifs.read((char*)(&tx), floatSize);
				ifs.read((char*)(&ty), floatSize);
				ifs.read((char*)(&tz), floatSize);

				ConvertEndian32(&(tx));
				ConvertEndian32(&(ty));
				ConvertEndian32(&(tz));

				X[i][j] = tx;
				Y[i][j] = ty;
				Z[i][j] = tz;

				if(tz == 0.0f)
					P[i][j] = 0;

			}
		}

		interstep = 0.0f;
		x0 = 0.0f;
		y0 = 0.0f;
	}

	ifs.close();

	return true;
}

}

#endif