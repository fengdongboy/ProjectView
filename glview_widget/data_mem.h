#ifndef GEOMETRYENGPT_H
#define GEOMETRYENGPT_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
//#include "ptFmt.h"

//#include "../HeadScan/Command_str.h"
#include <vector>

#include "../ProjectView/toothScanTypes.h"

enum eDrawData{
    eNullType,ePtData,eLineData,eTexture,ePtLinesData
};

class data_mem : protected QOpenGLFunctions
{
public:
    data_mem(eDrawData tp);
    virtual ~data_mem();

     void drawWithProgram(QOpenGLShaderProgram *program);
	 void drawWithProgramPoints(QOpenGLShaderProgram *program);

     void ptCloudDateUpdate(memData* ptData);
     void lineDataUpdate(memData* lineData);
     void imgAxiUpdate();

	 int getPointSize(void) const;
	 void updateData(memData* ptData);
private:

    int meshSize;
    int pointSize;
    QOpenGLBuffer arrayBuf;
    QOpenGLBuffer indexBuf;

    eDrawData drawType;

};

#endif // GEOMETRYENGPT_H
