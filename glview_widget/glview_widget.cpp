#include "glview_widget.h"
//#include "newMesh/include/TriMesh.h"
#include "data_mem.h"

#include <fstream>
#include <QPushButton>
#include "QMouseEvent"
using namespace std;
//using namespace trimesh;
using namespace omesh;


glview_widget::glview_widget(QWidget *parent) :
     QOpenGLWidget(parent),mCoodinateVisible(false), mScanType(eScanUp)
	,mMeshCloud(NULL)
	,mShowCurFrame(false)
{
        //光照设置
        uLightPosition = QVector3D(0, 0, 100);
        uLightAmbient  = QVector3D(0.1, 0.1, 0.1);
        uLightDiffuse  = QVector3D(0.8, 0.8, 0.8);
        uLightSpecular = QVector3D(0.2, 0.2, 0.2);

        //模型材质参数 绘制点云//254 224 203
        uMaterialAmbient  = QVector3D(0.2, 0.2, 0.2);
        uMaterialDiffuse  = QVector3D(0.282*3.0, 0.224*3.0, 0.173*3.0);
        uMaterialSpecular = QVector3D(0.6,0.6, 0.6);
        uShininess = 68;

        //模型材质参数 绘制线条for line//254 224 203
        uMaterialAmbient_line  = QVector3D(0.7, 0.2, 0.2);
        uMaterialDiffuse_line  = QVector3D(0.282*3.0, 0.224*1.0, 0.173*1.0);
        uMaterialSpecular_line = QVector3D(0.9,0.4, 0.6);

        mPressedTag=1;

        scal_g=5.3;
        BOX_SCALL=1.2;
        PTVIEW_SCALL= 2;

		QSurfaceFormat surfaceFormat;
		surfaceFormat.setSamples(4);//多重采样
		setFormat(surfaceFormat);
}

glview_widget::~glview_widget()
{
	if (mCoordinateMem)
	{
		delete mCoordinateMem[0];
		delete mCoordinateMem[1];
		delete mCoordinateMem[2];
	}
	if (mMeshCloud)
	{
		delete mMeshCloud;
	}
}



void glview_widget::addCloud(PtCloud* cloud, QMatrix4x4 matrix, EnScanType type /*= eScanUp*/)
{
	mBox.clear();
	for (auto it = cloud->begin(); it != cloud->end(); it++)
	{
		mBox.add(it->x, it->y, it->z);
	}
	mCurFrameBox = mBox;
	undateCurBox();
	setBox(mBox);

	data_mem *l = new data_mem(ePtData);
	makeCurrent();
	l->ptCloudDateUpdate(cloud);
	mClouds[type].push_back(l);
	mModelMatrix = matrix;
	//mModelMatrix.translate(0, 0, 0);
	update();
	mCoodinateVisible = true;

}


void glview_widget::addMeshCloud(PtCloud* cloud)
{
	if (mMeshCloud!=NULL)
		delete mMeshCloud;
	mMeshCloud = new data_mem(ePtData);

	for (auto it = cloud->begin(); it != cloud->end(); it++)
	{
		mBox.add(it->x, it->y, it->z);
	}

	//setBox(mBox);
	makeCurrent();
	mMeshCloud->ptCloudDateUpdate(cloud);
	mScanType = eScanTypeAll;
	update();
}

void glview_widget::setBox(omesh::Bbox& box)
{
	auto cent = box.center();
	auto dis = box.diag();
	pt3_g = cent;

	scal_g = 1.0 / dis;
	setProperty("scal_g", scal_g);
	float sc = PTVIEW_SCALL / dis;// (15 / dis);
	box_forlinePts = box;
}

void glview_widget::clear(void)
{
	for (int j = 0; j < eScanTypeAll; j++)
	{
		for (int i = 0; i < mClouds[j].size(); i++)
		{
			delete mClouds[j][i];
		}
		mClouds[j].clear();
	}

	delete mMeshCloud;
	mMeshCloud = NULL;
	mCoodinateVisible = false;
	mModelMatrix.setToIdentity();
	mRoteView.setToIdentity();
	viewReset();
	
	update();
}

void glview_widget::clear(EnScanType type )
{
	if (type < eScanTypeAll)
	{
		for (int i = 0; i < mClouds[type].size(); i++)
		{
			delete mClouds[type][i];
		}
		mClouds[type].clear();
	}
}

void glview_widget::setColor(const QVector3D& MaterialAmbient, const QVector3D& MaterialDiffuse, const QVector3D& MaterialSpecular)
{
	uMaterialAmbient = MaterialAmbient;
	uMaterialDiffuse = MaterialDiffuse;
	uMaterialSpecular = uLightSpecular;
}


void glview_widget::addCoordinateData(void)
{
	PtCloud lx, lx1, lx2;
	PtCloud* lines = &lx;

	int size = 100;
	float scal_arrow = 1.2f;

	/// x
	lx.push_back(Vertex3Normal(-4.0*size, 0.0f, 0.0f,0, 0, 1));
	lx.push_back(Vertex3Normal(4.0*size, 0.0f, 0.0f, 0, 0, 1));

	// arrow
	lx.push_back(Vertex3Normal(4.0*size, 0.0f, 0.0f, 0, 0, 1));
	lx.push_back(Vertex3Normal(3.0*size*scal_arrow, 1.0f*size/ scal_arrow/2, 0.0f, 0, 0, 1));

	lx.push_back(Vertex3Normal(4.0*size, 0.0f, 0.0f, 0, 0, 1));
	lx.push_back(Vertex3Normal(3.0*size*scal_arrow, -1.0f*size / scal_arrow/2, 0.0f, 0, 0, 1));


	// y 
	lx1.push_back(Vertex3Normal(0.0, -4.0f*size, 0.0f, 0, 0, 1));
	lx1.push_back(Vertex3Normal(0.0, 4.0f*size, 0.0f, 0, 0, 1));

	// arrow
	lx1.push_back(Vertex3Normal(0.0, 4.0f*size, 0.0f, 0, 0, 1));
	lx1.push_back(Vertex3Normal(1.0*size / scal_arrow/2, 3.0f*size*scal_arrow, 0.0f, 0, 0, 1));

	lx1.push_back(Vertex3Normal(0.0, 4.0f*size, 0.0f, 0, 0, 1));
	lx1.push_back(Vertex3Normal(-1.0*size / scal_arrow/2, 3.0f*size*scal_arrow, 0.0f, 0, 0, 1));

	// z 
	//lx.push_back(yxCoda::Vertex3Normal(0.0, 0.0f, -4.0f*size, 0, 0, 1));
	lx2.push_back(Vertex3Normal(0.0, 0.0f, 0, 0, 0, 1));
	lx2.push_back(Vertex3Normal(0.0, 0.0f, 4.0f*size, 0, 0, 1));

	// arrow
	lx2.push_back(Vertex3Normal(0.0, 0.0f, 4.0f*size, 0, 0, 1));
	lx2.push_back(Vertex3Normal(0.0, 1.0f*size / scal_arrow/2, 3.0f*size*scal_arrow, 0, 0, 1));

	lx2.push_back(Vertex3Normal(0.0, 0.0f, 4.0f*size, 0, 0, 1));
	lx2.push_back(Vertex3Normal(0.0, -1.0f*size / scal_arrow/2, 3.0f*size*scal_arrow, 0, 0, 1));

	mCoordinateMem[0] = new data_mem(ePtLinesData);
	mCoordinateMem[0]->ptCloudDateUpdate(lines);

	mCoordinateMem[1] = new data_mem(ePtLinesData);
	mCoordinateMem[1]->ptCloudDateUpdate(&lx1);

	mCoordinateMem[2] = new data_mem(ePtLinesData);
	mCoordinateMem[2]->ptCloudDateUpdate(&lx2);
	update();
}

void glview_widget::showScanType(EnScanType type)
{
	mScanType = type;
	update();
}

void glview_widget::clearMouseState(void)
{
	rotation = QQuaternion();
	viewShift = QVector2D(0, 0);
	diffAll = QVector2D(0, 0);
	scal_g = property("scal_g").toFloat();
	mRoteView.setToIdentity();
	update();
}

void glview_widget::viewFront() {

    rotation=QQuaternion();
	diffAll = QVector2D(0, 0);
    update();
}

void glview_widget::viewLeft()
{
	rotation = QQuaternion::fromAxisAndAngle(0.0, 1.0, 0.0, 90);
	diffAll = QVector2D(0, 0);
	update();
}

void glview_widget::viewLeft60() {

   rotation=QQuaternion::fromAxisAndAngle(0.0,1.0,0.0, -40);
}

void glview_widget::viewRight60(){

   rotation=QQuaternion::fromAxisAndAngle(0.0,1.0,0.0, 40);
   update();
}

void glview_widget::viewUp60(){

    rotation=QQuaternion::fromAxisAndAngle(1.0,0.0,0.0, 40);
    update();
}

void glview_widget::viewDown60(){

     rotation=QQuaternion::fromAxisAndAngle(1.0,0.0,0.0,-40);
     update();
}


void glview_widget::viewReset()
{
	rotation = QQuaternion();
	viewShift = QVector2D(0, 0);
	diffAll = QVector2D(0, 0);
	mBox.clear();
	scal_g = property("scal_g").toFloat();
	mRoteView.setToIdentity();
	update();
}

void glview_widget::viewByQQuaternion(const QQuaternion& qqua)
{
	rotation = qqua;
	diffAll = QVector2D(0, 0);
	update();
}

void glview_widget::viewByVector(const QVector3D & axis, float angle)
{
	rotation = QQuaternion::fromAxisAndAngle(axis, angle);
	diffAll = QVector2D(0, 0);
	update();
}

void glview_widget::setCoordinateVisible(bool visible /*= true*/)
{
	mCoodinateVisible = visible;
	update();
}

void glview_widget::mousePressEvent(QMouseEvent *e)
{
    QOpenGLWidget::mousePressEvent(e);

    if(e->button()==Qt::LeftButton)
    {
        mPressedTag=1;

    }else if(e->button()==Qt::RightButton)
    {
        mPressedTag=0;

    }else if(e->button()==Qt::MidButton){
        mPressedTag=-1;
    }
    mousePressPosition=QVector2D(e->localPos());
	mousePressPosition = QVector2D(e->pos());
	mShowCurFrame = false;
    return;

}

void glview_widget::mouseReleaseEvent(QMouseEvent *e)
{
    QOpenGLWidget::mouseReleaseEvent(e);
    return;

}

void glview_widget::mouseMoveEvent(QMouseEvent *e)
{

    if(mPressedTag==1)
    {

		auto ps = e->pos();
        QVector2D NowMousePos(ps);

        QVector2D diff =(NowMousePos - mousePressPosition);
        float xall=diffAll.x()+diff.x();
        float yall=diffAll.y()+diff.y();

        //QVector2D diff =NowMousePos - mousePressPosition;
        //float xall=diffAll.x()+diff.x();
        //float yall=diffAll.y()+diff.y();

        mousePressPosition=NowMousePos;

        //if(diff.x()>100) return;
        //if(diff.y()>100) return;
        diffAll.setX(xall);
        diffAll.setY(yall);
        auto rotationTemp0 = QQuaternion::fromAxisAndAngle(0.0,1.0,0.0,xall);
        auto rotationTemp1 = QQuaternion::fromAxisAndAngle(1.0,0.0,0.0,-yall);
        //rotation=rotationTemp0*rotationTemp1;

		//mMatriViewCoodinate.rotate(rotation);

		const static QVector3D LocalForward(0.0f, 0.0f, 1.0f);
		const static QVector3D LocalUp(0.0f, 1.0f, 0.0f);
		const static QVector3D LocalRight(1.0f, 0.0f, 0.0f);

		rotation *= QQuaternion::fromAxisAndAngle((LocalUp), diff.x());
		rotation *= QQuaternion::fromAxisAndAngle((LocalRight), -diff.y());

		QVector3D lu = mRoteView.inverted().map(LocalUp);
		QVector3D lr = mRoteView.inverted().map(LocalRight);
		if (diff.x() != 0)
		mRoteView.rotate(diff.x(), lu);
		if (diff.y() != 0)
		mRoteView.rotate(-diff.y(), lr);
        update();


    }
    else if(mPressedTag==0)
    {
        auto NowMousePos=QVector2D(e->localPos());

        QVector2D diff = NowMousePos - mousePressPosition;

        viewShift+=diff;
        // Request an update
        update();

        mousePressPosition=NowMousePos;
    }

}

 void glview_widget::wheelEvent(QWheelEvent *event)
 {
    int ang=event->delta()/120;
   // ang++;
    scal_g*=(1+ang*0.05);
    update();
 }

 void glview_widget::mouseDoubleClickEvent(QMouseEvent *e){

     viewFront();
	 mShowCurFrame = false;
 }



 void glview_widget::initializeGL()
 { 
     initializeOpenGLFunctions();

     GLenum err = glGetError();

	 /// 浅蓝背景
	 glClearColor(0.749f, 0.835f, 0.96, 1);
     err = glGetError();

     initShaders();
     err = glGetError();

     // Enable depth buffer
     glEnable(GL_DEPTH_TEST);
     glFrontFace(GL_CCW);
     glCullFace(GL_BACK);

     // Enable back face culling
     glEnable(GL_CULL_FACE);

	addCoordinateData();

	mCurFrameBoxMem = new data_mem(eLineData);
	PtCloud cloud(24);
	mCurFrameBoxMem->ptCloudDateUpdate(&cloud);
 }

//! [5]
void glview_widget::resizeGL(int w, int h)
{
    // Calculate aspect ratio
    qreal aspect = qreal(w) / qreal(h ? h : 1);

    // Set near plane to 3.0, far plane to 7.0, field of view 45 degrees
    const qreal zNear = 3.0, zFar = 170.0, fov = 45.0;

    // Reset projection
    projection.setToIdentity();

    // Set perspective projection
    //projection.perspective(fov, aspect, zNear, zFar);
    projection.ortho(-aspect,aspect,-1.0,1.0,-100,100.0);

    viewShiftMutPara[0]=2.0*((float)w/(float)h)/((float)(w?w:1));
    viewShiftMutPara[1]=2.0*(1.0)/((float)(h?h:1));
}
//! [5]

void glview_widget::paintGL()
{
    makeCurrent();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_MULTISAMPLE_ARB);

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

    //! [6]
    // Calculate model view transformation
    QMatrix4x4 matrix;

    matrix.scale( scal_g );

    //matrix.rotate( rotation );//pt3_g

	matrix.translate(-pt3_g[0], -pt3_g[1], -pt3_g[2]);
	matrix *= mRoteView;
	matrix.translate(pt3_g[0], pt3_g[1], -pt3_g[2]);

	QMatrix4x4 matrixInit;

    matrixInit.translate( -pt3_g[0], -pt3_g[1], -pt3_g[2]);

    QMatrix4x4 view;
    view.lookAt(QVector3D(0, 0, -10), QVector3D(/*viewShift[0]*0.01*/0,/*viewShift[1]*0.01*/0,0), QVector3D(0,1,0));

    QMatrix4x4 shitfMat;

    shitfMat.translate( (-viewShift[0]-10)*viewShiftMutPara[0], -viewShift[1]*viewShiftMutPara[1], 0);

    //QMatrix4x4 modelview = view * shitfMat*matrix * matrixInit;
	QMatrix4x4 modelview = view * shitfMat*matrix* mModelMatrix.inverted();

    QMatrix3x3 normalRot = modelview.normalMatrix();


	if (mCoordinateMem && mCoodinateVisible && 0)
	{
		programAxes.bind();

		programAxes.setUniformValue("uModelView", modelview);
		programAxes.setUniformValue("uNormalRotation", normalRot);
		programAxes.setUniformValue("uModelViewProjection", projection*modelview /*projection * modelview*/);

		QMatrix4x4 matrix;
		matrix.rotate(90, 1.0, 0.0, 0.0);
		matrix.rotate(180.0, 0.0, 0.0, 1.0);
		matrix.rotate(rotation);

		matrix.scale(100);
		//trant.translate(-0.9, -0.9, 0);
		//programPtcloud.setUniformValue("uModelViewProjection", trant* matrix /*projection * modelview*/);

		programAxes.setUniformValue("renderType", 3);
		programAxes.setUniformValue("inColor", QVector4D(0, 0, 1, 1));
		glLineWidth(2.5);
		mCoordinateMem[0]->drawWithProgram(&programAxes);

		programAxes.setUniformValue("renderType", 3);
		programAxes.setUniformValue("inColor", QVector4D(1, 0, 0, 1));
		mCoordinateMem[1]->drawWithProgram(&programAxes);

		programAxes.setUniformValue("renderType", 3);
		programAxes.setUniformValue("inColor", QVector4D(1, 1, 0, 1));
		mCoordinateMem[2]->drawWithProgram(&programAxes);
	}

    programPtcloud.bind();

    // Set modelview-projection matrix
    programPtcloud.setUniformValue("uModelView", modelview);
    programPtcloud.setUniformValue("uNormalRotation", normalRot);
    programPtcloud.setUniformValue("uModelViewProjection",projection*modelview /*projection * modelview*/);

    programPtcloud.setUniformValue("uLightPosition", uLightPosition);
    programPtcloud.setUniformValue("uLightAmbient", uLightAmbient);
    programPtcloud.setUniformValue("uLightDiffuse", uLightDiffuse);
    programPtcloud.setUniformValue("uLightSpecular", uLightSpecular);

    programPtcloud.setUniformValue("uMaterialAmbient", uMaterialAmbient);
    programPtcloud.setUniformValue("uMaterialDiffuse", uMaterialDiffuse);
    programPtcloud.setUniformValue("uMaterialSpecular", uMaterialSpecular);
    programPtcloud.setUniformValue("uShininess", uShininess);
	programPtcloud.setUniformValue("aPointSize", 10.5f);
    //! [6]

	if (mScanType == eScanTypeAll)
	{
		/// 画面
		programPtcloud.setUniformValue("renderType", 0);
		
		if (mMeshCloud)
		mMeshCloud->drawWithProgram(&programPtcloud);
		
	}
   else
   {
	   if (!mClouds[mScanType].empty())
		   for (int i = 0; i < mClouds[mScanType].size(); i++)
		   {/// 画扫描点
			   int type = 1 ? 0 : 3;
			   programPtcloud.setUniformValue("aPointSize", 2.5f);
			   programPtcloud.setUniformValue("renderType", type);
			   if (type == 3)
				   mClouds[mScanType][i]->drawWithProgram(&programPtcloud);
			   else
				   mClouds[mScanType][i]->drawWithProgramPoints(&programPtcloud);
		   }

	   if (!mClouds[mScanType].empty() && mShowCurFrame)
	   {
		   /// 画当前点
		   programPtcloud.setUniformValue("aPointSize", 3.5f);
		   programPtcloud.setUniformValue("renderType", 1);
		   mClouds[mScanType].back()->drawWithProgramPoints(&programPtcloud);
		   programPtcloud.setUniformValue("aPointSize", 2.5f);
		   mCurFrameBoxMem->drawWithProgram(&programPtcloud);
	   }
   }
}


//! [3]
void glview_widget::initShaders()
{
    // Compile vertex shader



    //img texture
    {

            if (!programTexture.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shader/vshader.glsl"))
            //if (!program.addShaderFromSourceCode(QOpenGLShader::Vertex, shaderVP))
            {
                QString errs = programTexture.log();

                GLenum err = glGetError();
                printf(" vp.glsl err code:%d,%s\n",err,errs.toStdString().c_str());
                exit(-1);
            };

            if (!programTexture.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shader/fshader.glsl"))
            //if (!program.addShaderFromSourceCode(QOpenGLShader::Vertex, shaderVP))
            {
                QString errs = programTexture.log();

                GLenum err = glGetError();
                printf(" vp.glsl err code:%d,%s\n",err,errs.toStdString().c_str());
                exit(-1);
            };

            // Link shader pipeline
            if (!programTexture.link())
            {
                QString errs = programPtcloud.log();
                GLenum err = glGetError();
                printf(" glsl link err code:%d ,%s\n",err,errs.toStdString().c_str());
                  exit(-1);
            };

             //errs = programPtcloud.log();
    }
    //point cloud
    {
        if (!programPtcloud.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shader/vp_pt.glsl"))
        //if (!program.addShaderFromSourceCode(QOpenGLShader::Vertex, shaderVP))
        {
            QString errs = programPtcloud.log();

            GLenum err = glGetError();
            printf(" vp.glsl err code:%d,%s\n",err,errs.toStdString().c_str());
            exit(-1);
        };

        QString errs = programPtcloud.log();
        // Compile fragment shader
        if (!programPtcloud.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shader/vf_pt.glsl"))
        //if (!program.addShaderFromSourceCode(QOpenGLShader::Fragment, shaderFP))
        {
            QString errs = programPtcloud.log();
            GLenum err = glGetError();
            printf(" vf.glsl err code:%d ,%s\n",err,errs.toStdString().c_str());
            exit(-1);
        };

         errs = programPtcloud.log();


        // Link shader pipeline
        if (!programPtcloud.link())
        {
            QString errs = programPtcloud.log();
            GLenum err = glGetError();
            printf(" glsl link err code:%d ,%s\n",err,errs.toStdString().c_str());
              exit(-1);
        };

         errs = programPtcloud.log();

        // Bind shader pipeline for use
        if (!programPtcloud.bind())
        {
            QString errs = programPtcloud.log();
            GLenum err = glGetError();
            printf(" glsl bind err code:%d ,%s\n",err,errs.toStdString().c_str());
              exit(-1);
        };

         errs = programPtcloud.log();
    }

	/// axes
	{
		if (!programAxes.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shader/vp_pt_axes.glsl"))
		{
			QString errs = programAxes.log();

			GLenum err = glGetError();
			printf(" vp.glsl err code:%d,%s\n", err, errs.toStdString().c_str());
			exit(-1);
		};

		QString errs = programAxes.log();
		// Compile fragment shader
		if (!programAxes.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shader/vf_pt_axes.glsl"))
		{
			QString errs = programAxes.log();
			GLenum err = glGetError();
			printf(" vf.glsl err code:%d ,%s\n", err, errs.toStdString().c_str());
			exit(-1);
		};
		errs = programAxes.log();

		// Link shader pipeline
		if (!programAxes.link())
		{
			QString errs = programAxes.log();
			GLenum err = glGetError();
			printf(" glsl link err code:%d ,%s\n", err, errs.toStdString().c_str());
			exit(-1);
		};

		errs = programAxes.log();

		// Bind shader pipeline for use
		if (!programAxes.bind())
		{
			QString errs = programAxes.log();
			GLenum err = glGetError();
			printf(" glsl bind err code:%d ,%s\n", err, errs.toStdString().c_str());
			exit(-1);
		};
		errs = programAxes.log();
	}


     //line shader
     {
         if (!programLine.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shader/vp_line.glsl"))
         //if (!program.addShaderFromSourceCode(QOpenGLShader::Vertex, shaderVP))
         {
             QString errs = programLine.log();

             GLenum err = glGetError();
             printf(" vp.glsl err code:%d,%s\n",err,errs.toStdString().c_str());
             exit(-1);
         };

         QString errs = programLine.log();

         // Compile fragment shader
         if (!programLine.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shader/vf_line.glsl"))
         //if (!program.addShaderFromSourceCode(QOpenGLShader::Fragment, shaderFP))
         {
             QString errs = programLine.log();
             GLenum err = glGetError();
             printf(" vf.glsl err code:%d ,%s\n",err,errs.toStdString().c_str());
             exit(-1);
         };

          errs = programLine.log();


         // Link shader pipeline
         if (!programLine.link())
         {
             QString errs = programLine.log();
             GLenum err = glGetError();
             printf(" glsl link err code:%d ,%s\n",err,errs.toStdString().c_str());
               exit(-1);
         };

          errs = programLine.log();
     }
}
void glview_widget::undateCurBox(void)
{
	static PtCloud cloud;
	cloud.resize(24);
	std::vector<omesh::Pnt3> edgeLists;
	mCurFrameBox.edgeList(edgeLists);
	cloud.resize(edgeLists.size());
	QVector3D point, out;
	QMatrix4x4 normalMatrix = mModelMatrix;
	normalMatrix.translate(0, 0, 0);
	for (int i = 0; i < edgeLists.size(); i++)
	{
		//cloud[i] = Vertex3Normal(edgeLists[i][0], edgeLists[i][1], edgeLists[i][2], -1, 0, 0);
		point = QVector3D(edgeLists[i][0], edgeLists[i][1], edgeLists[i][2]);
		out = normalMatrix.mapVector(point);
		cloud[i] = Vertex3Normal(out.x(),out.y(),out.z(), -1, 0, 0);
	}
	//mCurFrameBoxMem->updateData(&cloud);
	//mCurFrameBoxMem->ptCloudDateUpdate(&cloud);
	mShowCurFrame = true;
}


