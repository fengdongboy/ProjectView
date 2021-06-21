#include "ModelManager.h"
#include <QFile>
#include <QTextStream>


ModelManager::ModelManager()
{
}


ModelManager::~ModelManager()
{
}

ModelManager* ModelManager::getModelManager(void)
{
	static ModelManager* p = new ModelManager;
	return p;
}

const Clouds& ModelManager::getClouds(void) const
{
	return mClouds;
}

Clouds& ModelManager::getClouds(void)
{
	return mClouds;
}

bool ModelManager::loadXyz(const QString & name, PtCloud & cloud)
{
	QFile f(name);
	if (!f.open(QFile::ReadOnly))
		return false;
	QTextStream text(&f);
	cloud.clear();
	QString str;
	QStringList list;
	cloud.reserve(2048);
	Vertex3Normal point;
	while (!text.atEnd())
	{
		str = text.readLine();
		list = str.split(" ");
		for (int i = 0; i < 6; i++)
			point[i] = list[i].toFloat();
		cloud.push_back(point);
	}
	f.close();
	return true;
	return false;
}

bool ModelManager::loadMatrix(const QString & name, QMatrix4x4 & matrix)
{
	QFile f(name);
	if (f.open(QFile::WriteOnly | QFile::Text))
	{
		QTextStream stream(&f);
		for (int j = 0; j < 4; j++)
		{
			for (int i = 0; i < 4; i++)
			{
				stream >> matrix(j, i);
			}
		}
		return true;
	}
	return false;
}

bool ModelManager::writeMatrix(const QString& str, const QMatrix4x4& matrix)
{
	QFile f(str);
	if (f.open(QFile::WriteOnly | QFile::Text))
	{
		QTextStream stream(&f);
		for (int j = 0; j < 4; j++)
		{
			for (int i = 0; i < 4; i++)
			{
				stream << matrix(j, i) << " ";
			}
			stream << endl;
		}
		return true;
	}
	return false;
}

bool ModelManager::loadPly(const QString& name, PtCloud& cloud)
{
	QFile f(name);
	if (!f.open(QFile::ReadOnly))
		return false;
	QTextStream text(&f);
	cloud.clear();
	QString str;
	QStringList list;
	bool findPoint = false;
	bool end = false;
	Vertex3Normal point;
	while (!text.atEnd())
	{
		str = text.readLine();
		if (str.indexOf("element vertex") != -1)
		{
			list = str.split(" ");
			cloud.reserve(list[2].toInt());
			continue;
		}
		if (str.indexOf("end_header") != -1)
		{
			findPoint = true;
			continue;
		}
		if (findPoint)
		{
			list = str.split(" ");
			if (list.size() != 6)
				break;
			for (int i = 0; i < 6; i++)
				point[i] = list[i].toFloat();
			cloud.push_back(point);
		}
	}
	f.close();
	return true;
}

void ModelManager::tranModel(const QMatrix4x4& m, PtCloud& cloud)
{
	auto t = [](Vertex3Normal& v, const QMatrix4x4& m)
	{
		float d[3] = { 0 };
		std::copy(&v.x, &v.z, d);
		v.x = d[0] * m(0, 0) + d[1] * m(0, 1) + d[2] * m(0, 2) + m(0, 3);
		v.y = d[0] * m(1, 0) + d[1] * m(1, 1) + d[2] * m(1, 2) + m(1, 3);
		v.z = d[0] * m(2, 0) + d[1] * m(2, 1) + d[2] * m(2, 2) + m(2, 3);

		std::copy(&v.nx, &v.nz, d);
		v.nx = d[0] * m(0, 0) + d[1] * m(0, 1) + d[2] * m(0, 2);
		v.ny = d[0] * m(1, 0) + d[1] * m(1, 1) + d[2] * m(1, 2);
		v.nz = d[0] * m(2, 0) + d[1] * m(2, 1) + d[2] * m(2, 2);
	};

	for (auto p:cloud)
	{
		t(p, m);
	}
}
