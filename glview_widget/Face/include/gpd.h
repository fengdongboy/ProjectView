// GPD ( Geomagic Point Data )

#ifndef _GPD_H_
#define _GPD_H_

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include "..\tnt\tnt.h"

using namespace TNT;
using namespace std;

// GPD Format header struct
struct GPD_header {
	char magic_id[4];
	short version;
	long points_number;
	BYTE color_flag;
	BYTE UV_flag;
	BYTE normal_flag;
	BYTE display_units;
	short targets_number;
	char reserved[16];
};

inline bool LoadGPDHeader(const string filePath, GPD_header& header)
{
	ifstream ifs(filePath.c_str(), ios::binary);
	if(!ifs) { 
		cout << "open \"" +  filePath + "\" failed" << endl;
		ifs.close(); 
		return false; 
	}

	ifs.read((char*)(&header.magic_id), sizeof(char)*4);
	ifs.read((char*)(&header.version), sizeof(short));
	ifs.read((char*)(&header.points_number), sizeof(long));
	ifs.read((char*)(&header.color_flag), sizeof(BYTE));
	ifs.read((char*)(&header.UV_flag), sizeof(BYTE));
	ifs.read((char*)(&header.normal_flag), sizeof(BYTE));
	ifs.read((char*)(&header.display_units), sizeof(BYTE));
	ifs.read((char*)(&header.targets_number), sizeof(short));
	ifs.read((char*)(&header.reserved), sizeof(char)*16);

	ifs.close();

	return true;
}

inline void ShowGPDHeader(const GPD_header& header)
{
	cout << "magic id            " << header.magic_id              << endl;
	cout << "version             " << header.version               << endl;
	cout << "points              " << header.points_number         << endl;
	cout << "color flag          " << header.color_flag            << endl;
	cout << "UV flag             " << header.UV_flag               << endl;
	cout << "normal flag         " << header.normal_flag           << endl;
	cout << "display units       " << header.display_units         << endl;
	cout << "targets             " << header.targets_number        << endl;
	cout << "reserved            " << header.reserved              << endl;
}

inline bool SaveGPD(const string filePath, const Matrix<int>& P, const Matrix<float>& X, const Matrix<float>& Y, const Matrix<float>& Z )
{
	int points_number = 0;

	for( int i=0 ; i<P.num_rows() ; i++ )
		for( int j=0 ; j<P.num_cols() ; j++ )
			if( P[i][j] == 1 ) 
				points_number ++;

	if( points_number == 0 )
		return false;

	ofstream fs(filePath.c_str(), ios::binary);
	if(!fs){
		cout << "open \"" +  filePath + "\" failed" << endl;
		fs.close();
		return false;
	}

	GPD_header header;

	header.magic_id[0]  = 'G';
	header.magic_id[1]  = 'P';
	header.magic_id[2]  = 'D';
	header.magic_id[3]  = '\0';

	header.version = short(1);
	header.points_number = long(points_number);
	header.color_flag = BYTE(0);
	header.UV_flag = BYTE(1);
	header.normal_flag = BYTE(0);
	header.display_units = BYTE(2);
	header.targets_number = short(0);

	for( int i=0 ; i<16 ; i++ )
		header.reserved[i] = '0';

	ShowGPDHeader(header);

	fs.write((char*)(&header.magic_id), sizeof(char)*4);
	fs.write((char*)(&header.version), sizeof(short));
	fs.write((char*)(&header.points_number), sizeof(long));
	fs.write((char*)(&header.color_flag), sizeof(BYTE));
	fs.write((char*)(&header.UV_flag), sizeof(BYTE));
	fs.write((char*)(&header.normal_flag), sizeof(BYTE));
	fs.write((char*)(&header.display_units), sizeof(BYTE));
	fs.write((char*)(&header.targets_number), sizeof(short));
	fs.write((char*)(&header.reserved), sizeof(char)*16);

	for( int i=0 ; i<P.num_rows() ; i++ ) {
		for( int j=0 ; j<P.num_cols() ; j++ ) {
			if( P[i][j] == 1 ) {
				float x = X[i][j]/1000.0f;
				float y = Y[i][j]/1000.0f;
				float z = Z[i][j]/1000.0f;
				fs.write((char*)(&(x)), sizeof(float));
				fs.write((char*)(&(y)), sizeof(float));
				fs.write((char*)(&(z)), sizeof(float));
				fs.write((char*)(&(i)), sizeof(short));
				fs.write((char*)(&(j)), sizeof(short));
			}
		}
	}

	return true;
}

inline bool SaveColorGPD(	const string filePath, 
							const Matrix<int>& P, 
							const Matrix<float>& X, const Matrix<float>& Y, const Matrix<float>& Z, 
							const Matrix<float>& R, const Matrix<float>& G, const Matrix<float>& B )
{
	int points_number = 0;

	for( int i=0 ; i<P.num_rows() ; i++ )
		for( int j=0 ; j<P.num_cols() ; j++ )
			if( P[i][j] == 1 ) 
				points_number ++;

	if( points_number == 0 )
		return false;

	ofstream fs(filePath.c_str(), ios::binary);
	if(!fs){
		cout << "open \"" +  filePath + "\" failed" << endl;
		fs.close();
		return false;
	}

	GPD_header header;

	header.magic_id[0]  = 'G';
	header.magic_id[1]  = 'P';
	header.magic_id[2]  = 'D';
	header.magic_id[3]  = '\0';

	header.version = short(1);
	header.points_number = long(points_number);
	header.color_flag = BYTE(1);
	header.UV_flag = BYTE(1);
	header.normal_flag = BYTE(0);
	header.display_units = BYTE(2);
	header.targets_number = short(0);

	for( int i=0 ; i<16 ; i++ )
		header.reserved[i] = '0';

	ShowGPDHeader(header);

	fs.write((char*)(&header.magic_id), sizeof(char)*4);
	fs.write((char*)(&header.version), sizeof(short));
	fs.write((char*)(&header.points_number), sizeof(long));
	fs.write((char*)(&header.color_flag), sizeof(BYTE));
	fs.write((char*)(&header.UV_flag), sizeof(BYTE));
	fs.write((char*)(&header.normal_flag), sizeof(BYTE));
	fs.write((char*)(&header.display_units), sizeof(BYTE));
	fs.write((char*)(&header.targets_number), sizeof(short));
	fs.write((char*)(&header.reserved), sizeof(char)*16);

	for( int i=0 ; i<P.num_rows() ; i++ ) {
		for( int j=0 ; j<P.num_cols() ; j++ ) {
			if( P[i][j] == 1 ) {
				float x = X[i][j]/1000.0f;
				float y = Y[i][j]/1000.0f;
				float z = Z[i][j]/1000.0f;
				fs.write((char*)(&(x)), sizeof(float));
				fs.write((char*)(&(y)), sizeof(float));
				fs.write((char*)(&(z)), sizeof(float));
				fs.write((char*)(&(R[i][j])), sizeof(float));
				fs.write((char*)(&(G[i][j])), sizeof(float));
				fs.write((char*)(&(B[i][j])), sizeof(float));
				fs.write((char*)(&(i)), sizeof(short));
				fs.write((char*)(&(j)), sizeof(short));
			}
		}
	}

	return true;
}

#endif