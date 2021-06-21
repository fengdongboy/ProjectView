#pragma once
#ifndef __MODELMANAGER_H__
#define __MODELMANAGER_H__
/*!
 * \class ModelManager
 *
 * \brief 模型管理
 *
 * \author fdl
 * \date 2021/06/21
 */

#include "toothScanTypes.h"
#include <QMatrix4x4>

typedef std::vector<QMatrix4x4> Matrix4x4s;

class ModelManager
{
public:
	ModelManager();
	~ModelManager();

	static ModelManager* getModelManager(void);

	const Clouds& getClouds(void) const;

	 Clouds& getClouds(void);

	 bool loadXyz(const QString& name, PtCloud& cloud);
	 bool loadMatrix(const QString& name, QMatrix4x4& matrix);
	 bool writeMatrix(const QString& str, const QMatrix4x4& matrix);

	 bool loadPly( const QString& name, PtCloud& cloud);

	 void tranModel(const QMatrix4x4& m, PtCloud& cloud);

private:
	Clouds mClouds;
	Matrix4x4s mMatrix4x4s;
};

#endif