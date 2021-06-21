#ifndef GLVIEW_WIDGET_H
#define GLVIEW_WIDGET_H

#include "glview_widget_global.h"


#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QQuaternion>
#include <QVector2D>
#include <QBasicTimer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QMutex>


#include "data_mem.h"
#include "./Face/include/Bbox.h"

class data_mem;


//本模块最好能承担显示二维图，三维图，线条，面，可重入释放，可以多个界面并存的特点。
//class PtCloud;


//class PtCloud;
class GLVIEW_WIDGETSHARED_EXPORT glview_widget:public QOpenGLWidget, protected QOpenGLFunctions
{

    Q_OBJECT
public:
    explicit glview_widget(QWidget *parent = 0);

    ~glview_widget();

	void addCloud(PtCloud* cloud, QMatrix4x4 matrix, EnScanType type = eScanUp);
	void addMeshCloud(PtCloud* cloud);
	void clear(void);
	void clear(EnScanType type);
	void setBox(omesh::Bbox& box);

	void setColor(const QVector3D& MaterialAmbient, const QVector3D& MaterialDiffuse, const QVector3D& MaterialSpecular);

	void addCoordinateData(void);

	void showScanType(EnScanType type);

	void clearMouseState(void);

public slots:
    //below no scal work
    void viewFront();
	void viewLeft();
    void viewLeft60();
    void viewRight60();
    void viewUp60();
    void viewDown60();
	void viewReset();
	void viewByQQuaternion(const QQuaternion& qqua);
	void viewByVector(const QVector3D & axis, float angle);
	void setCoordinateVisible(bool visible = true);

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    //void timerEvent(QTimerEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    //init gl eviroment,load shader
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void initShaders();

	void undateCurBox(void);

private:
    QBasicTimer timer;
    QOpenGLShaderProgram programPtcloud;
    QOpenGLShaderProgram programLine;
    QOpenGLShaderProgram programTexture;
	QOpenGLShaderProgram programAxes;

	data_mem *mCoordinateMem[3];		/// 坐标轴的点
	omesh::Bbox mBox;

    QMatrix4x4 projection;

    QVector2D mousePressPosition;
    QVector3D rotationAxis;
    qreal angularSpeed;
    QQuaternion rotation;

	EnScanType mScanType;
	std::vector<data_mem *> mClouds[eScanTypeAll];
	data_mem* mMeshCloud;

	omesh::Bbox mCurFrameBox;
	data_mem* mCurFrameBoxMem;

	bool mShowCurFrame;

private:
    //光照设置
    QVector3D uLightPosition;
    QVector3D uLightAmbient;
    QVector3D uLightDiffuse;
    QVector3D uLightSpecular;
    //模型材质参数
    QVector3D uMaterialAmbient;
    QVector3D uMaterialDiffuse;
    QVector3D uMaterialSpecular;
    //模型材质参数
    QVector3D uMaterialAmbient_line;
    QVector3D uMaterialDiffuse_line;
    QVector3D uMaterialSpecular_line;
    float uShininess;

    int mPressedTag;//0:right ,1:left,-1:midle
    //may be useless
    QVector2D viewShift;
    QVector2D viewShiftMutPara;

    omesh::Bbox box_forlinePts;
    float BOX_SCALL;//=1.2
    float PTVIEW_SCALL;// 2

    float scal_g;
    omesh::Pnt3 pt3_g;
    QVector2D diffAll;


	/// 坐标轴是否显示
	bool mCoodinateVisible;

	QMatrix4x4 mRoteView;
	QMatrix4x4 mModelMatrix;
};

#endif // GLVIEW_WIDGET_H
