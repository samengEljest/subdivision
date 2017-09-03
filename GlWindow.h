#pragma once
#include <QGLShaderProgram>
#include <QGLFunctions>
#include <QtOpenGL\QGLWidget>
#include <QGLWidget>
#include <QTimer>
#include <QMouseEvent>
#include "ui_lab2.h"
#include <GL/glu.h>

#include "Carrier.h"
#include <math.h>
#include <string>

#include "objectLoader.h"
#include "lightNode.h"
#include "node.h"
#include "transformNode.h"

#include <QKeyEvent>
#include <QMouseEvent>

#include "Matrix3.h"
#include "Matrix4.h"
#include "Vector3.h"
#include "Vector4.h"

#include <cstdio>
#include <ctime>

using namespace std;

/*!
	this is the class that handels the OpenGl window
*/
class GlWindow : public QGLWidget
{
	Q_OBJECT
public:
	GlWindow(QWidget* parent = 0); ///< opens the openGL window with the chosen file, sets the size to the picture
	~GlWindow();
	int objectCount;
	int lightX;
	int lightY;
	int lightZ;
	Matrix4 cameraRotate(float theta,string axis);  ///<  rotates the camera
	double rotCamX; 
	double rotCamY;
	void doBeginning(); ///< initialize stuff
	void updateCamera(); //updates the m_camera matrix

	void newObject(QString texture, const char* newobject); ///< creates a new object

	vector<Carrier> carrier;

public slots:
	void RandObject(); ///< creates a new pyramide
	void loopSub(); ///< Do one loop of subdevision 
	void wireframe_On();
	void wireframe_Off();

private:
	Ui::Lab2Class ui;
	float rotate;
	QTimer timer;

	TransformNode* camera;
	TransformNode* ship;

	ObjectLoader *objects[500];

	double cameraXPos, cameraYPos, cameraZPos;
	QGLShaderProgram program;
	QGLFunctions funcs;

	Matrix4 m_camera;
	Matrix4 m_id;
	Matrix4 m_camerapos;
	Matrix4 m_Frustum;
	Matrix4 m_proj;

	float* matrix;
	float* matrixModView;
	float m_Movevel;

	GLuint texture;
	float tmpspeed;

	int loopCount;
	clock_t start;
	float duration;

	bool m_mouseIn;
	float m_camPitch;
	float m_camYaw;
	POINT m_mousePos;

protected:
	void initializeGL(); ///< initialize option for the gl window and sets the scene graph
	void paintGL(); ///< paints the picture on the openGL window
	void resizeGL(int width, int height);///< resize the gl window
	void keyPressEvent(QKeyEvent *); ///< if a key is pressd, do different things
	void mousePressEvent(QMouseEvent *event);///< mouse movment for the camera
	void mouseReleaseEvent(QMouseEvent *event);///< mouse movment ended for the camera

	void control(bool i_mi); ///< control the camera
	void lockCamera();

	float getFrustumBottomY(float deep); ///<used to compare id object is inside frustrum
	float getFrustumTopY(float deep); ///<used to compare id object is inside frustrum
	float getFrustumLeftX(float deep); ///<used to compare id object is inside frustrum
	float getFrustumRightX(float deep); ///<used to compare id object is inside frustrum

	GLint vertexCoordLocation;
	GLint uvLocation;
	GLint normalLocation;
	GLint projectionLocation;
	GLint viewLocation;
	GLint modelLocation;
	GLint textureLocation;
	GLint LightLocation;
	GLint isSun;
};
