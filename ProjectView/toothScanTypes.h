#pragma once
#ifndef __TOOTHSCANTYPE_H__
#define __TOOTHSCANTYPE_H__
/*!
 * \class classname
 *
 * \brief 实现一些类型的预定义
 *
 * \author fdl
 * \date 2/23 2021 
 */

//#include <QVector3D>
#include <vector>
#include <algorithm>

enum DeviceStatus
{
	/// 直播模式
	enDeviceVideo,

	/// 扫描模式
	enDeviceScan,

	/// 空闲模式
	enDeviceFree
};

struct DevicePulse
{
	int cycle;		/// 周期
	int width;		/// 宽度
	int num;		/// 个数

	DevicePulse():cycle(3300),width(1),num(1){}
	DevicePulse& operator=(const DevicePulse& other)
	{
		cycle = other.cycle;
		width = other.width;
		num = other.num;
		return (*this);
	}
};

struct cameraConfig
{
	int expose;
	int gain;
	cameraConfig():expose(300),gain(10){}
};

struct FrameRt
{
	float value[16];
};

struct ProjectionICPParam
{
	float fx;
	float fy;
	float cx;
	float cy;
	FrameRt rt;
	ProjectionICPParam(float fx_, float fy_, float cx_, float cy_) :fx(fx_), fy(fy_), cx(cx_), cy(cy_) {}
	ProjectionICPParam():fx(0),fy(0),cx(0),cy(0){}
};

struct DeviceConfig
{
	cameraConfig camera;
	ProjectionICPParam ICPParam;
	DevicePulse pulseA;
	DevicePulse pulseB;

};

#define MAX_IMAGE_BUFFER 12
//typedef std::vector<QVector3D> QCloud;

struct Vertex3Normal
{
	float x;
	float y;
	float z;
	float nx;
	float ny;
	float nz;

	float& operator[](int index)
	{
		switch (index)
		{
		case 0:
			return x;
		case 1:
			return y;
		case 2:
			return z;
		case 3:
			return nx;
		case 4:
			return ny;
		case 5:
			return nz;
		}
		static float v;
		return v;
	}
	float operator[](int index) const
	{
		switch (index)
		{
		case 0:
			return x;
		case 1:
			return y;
		case 2:
			return z;
		case 3:
			return nx;
		case 4:
			return ny;
		case 5:
			return nz;
		}
	}
	Vertex3Normal() :x(0), y(0), z(0), nx(0), ny(0), nz(0) {}
	Vertex3Normal(float x_, float y_, float z_, float nx_, float ny_, float nz_) :x(x_), y(y_), z(z_), nx(nx_), ny(ny_), nz(nz_) {}
};

typedef std::vector< Vertex3Normal > memData;
typedef std::vector< Vertex3Normal > PtCloud;
typedef std::vector<PtCloud> Clouds;

struct ToothGrid
{
	int width;
	int height;
	std::vector<int> grid;
	int index;
	ToothGrid& operator=(const ToothGrid& other)
	{
		this->width = other.width;
		this->height = other.height;
		this->grid = other.grid;
		this->index = other.index;
		return *this;
	}
	ToothGrid():width(0),height(0),index(0){}
};

enum EnScanType
{
	eScanUp,	/// 扫描上
	eScanDown,	/// 扫描下
	eScanTotal,	/// 扫描咬合
	eScanRet,	/// 匹配后的结果
	eScanTypeAll	/// 所有扫描类型
};

struct GridCloud
{
	PtCloud cloud;
	ToothGrid grid;

	GridCloud(){}
	GridCloud(const PtCloud& cloud, const ToothGrid& grid)
	{
		this->cloud = cloud;
		this->grid = grid;
	}
	GridCloud& operator=(const GridCloud& other)
	{
		this->cloud = other.cloud;
		this->grid = other.grid;
		return *this;
	}
};

typedef std::vector<GridCloud> GridClouds;
typedef std::vector<FrameRt> FrameRts;

static void getCloudBox(const PtCloud& cloud, float& minx, float& miny, float& minz, float& maxx, float& maxy, float& maxz)
{
	auto xExtremes = std::minmax_element(cloud.begin(), cloud.end(),[](const Vertex3Normal & lhs, const Vertex3Normal & rhs) {
		return lhs.x < rhs.x;
	});
	minx = xExtremes.first->x;
	maxx = xExtremes.second->x;

	auto yExtremes = std::minmax_element(cloud.begin(), cloud.end(), [](const Vertex3Normal & lhs, const Vertex3Normal & rhs) {
		return lhs.y < rhs.y;
	});
	miny = yExtremes.first->y;
	maxy = yExtremes.second->y;

	auto zExtremes = std::minmax_element(cloud.begin(), cloud.end(), [](const Vertex3Normal & lhs, const Vertex3Normal & rhs) {
		return lhs.z < rhs.z;
	});
	minz = yExtremes.first->z;
	maxz = yExtremes.second->z;
}

#endif