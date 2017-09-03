#pragma once
#include "GlWindow.h"



std::string vs = 
"#version 330 core\n"
"uniform mat4 projection;\n"
"uniform mat4 view;\n"
"uniform mat4 model;\n"
"uniform vec3 LightPosition_worldspace;\n"

"in vec3 vertex;\n"
"in vec2 uv;\n"
"in vec3 normal;\n"

"out vec2 UV;\n"
"out vec3 Position_worldspace;\n"
"out vec3 Normal_cameraspace;\n"
"out vec3 EyeDirection_cameraspace;\n"
"out vec3 LightDirection_cameraspace;\n"

"void main()\n"
"{\n"
"	gl_Position = projection * view * model * vec4(vertex, 1.0);\n"

"	Position_worldspace = (model * vec4(vertex,1)).xyz;\n"

"	vec3 vertexPosition_cameraspace = ( view * model * vec4(vertex,1)).xyz;\n"
"	EyeDirection_cameraspace = vec3(0,0,0) - vertexPosition_cameraspace;\n"

"	vec3 LightPosition_cameraspace = (view * vec4(LightPosition_worldspace,1)).xyz;\n"
"	LightDirection_cameraspace = LightPosition_cameraspace + EyeDirection_cameraspace;\n"

	// Normal of the the vertex, in camera space
"	Normal_cameraspace = (view * model * vec4(normal,0)).xyz;\n" 
	
	// UV of the vertex. No special space for this one.
"	UV = uv;\n"
"}\n";

// pixel shader which quite simply just draws the texture to the surface.
std::string ps =
"#version 330 core\n"
"uniform sampler2D tex;\n"
"uniform vec3 LightPosition_worldspace;\n"
"uniform float isSun;\n"

"in vec2 UV;\n"
"in vec3 Position_worldspace;\n"
"in vec3 Normal_cameraspace;\n"
"in vec3 EyeDirection_cameraspace;\n"
"in vec3 LightDirection_cameraspace;\n"

"out vec3 color;\n"


"void main()\n"
"{\n"
"	if(isSun == 1){\n"
"		color = texture2D( tex, UV ).rgb;}\n"
"	else{\n"
		// Light emission properties
"		vec3 LightColor = vec3(1,0.9,0.9);\n"
"		float LightPower = 1500.0f;\n"
	
		// Material properties
"		vec3 MaterialDiffuseColor = texture2D( tex, UV ).rgb;\n"
"		vec3 MaterialAmbientColor = vec3(0.1,0.1,0.1) * MaterialDiffuseColor;\n"
"		vec3 MaterialSpecularColor = vec3(0.3,0.3,0.3);\n"

		// Distance to the light
"		float distance = length( LightPosition_worldspace - Position_worldspace );\n"

		// Normal of the computed fragment, in camera space
"		vec3 n = normalize( Normal_cameraspace );\n"
		// Direction of the light (from the fragment to the light)
"		vec3 l = normalize( LightDirection_cameraspace );\n"
		// Cosine of the angle between the normal and the light direction, 
		// clamped above 0
		//  - light is at the vertical of the triangle -> 1
		//  - light is perpendicular to the triangle -> 0
		//  - light is behind the triangle -> 0
"		float cosTheta = clamp( dot( n,l ), 0,1 );\n"

		// Eye vector (towards the camera)
"		vec3 E = normalize(EyeDirection_cameraspace);\n"
		// Direction in which the triangle reflects the light

"		vec3 R = reflect(-l,n);\n"
		// Cosine of the angle between the Eye vector and the Reflect vector,
		// clamped to 0
		//  - Looking into the reflection -> 1
		//  - Looking elsewhere -> < 1
"		float cosAlpha = clamp( dot( E,R ), 0,1 );\n"
	
"		color = MaterialAmbientColor + MaterialDiffuseColor * LightColor * LightPower * cosTheta / (distance*distance) + MaterialSpecularColor * LightColor * LightPower * pow(cosAlpha,5) / (distance*distance);\n"
"	}\n"
"}\n";


GlWindow::GlWindow(QWidget* parent) : QGLWidget(parent)
{
	doBeginning();
	this->timer.setInterval(1);
	connect(&this->timer, SIGNAL(timeout()), this, SLOT(updateGL()));
	this->timer.start();
}

void GlWindow::doBeginning()
{
	cameraXPos = 0;
	cameraYPos = 0;
	cameraZPos = 0;
	m_camera = Matrix4(4,4);
	m_id = Matrix4(4,4);
	m_camerapos = Matrix4(4,4);
	matrix = new float[16];
	matrixModView = new float[16];
	carrier=vector<Carrier>();
	objectCount = 0;
	loopCount = 0;
	m_proj = Matrix4(4,4);

	m_mouseIn = false;
	m_camPitch = 0.0f;
	m_camYaw = 0.0f;

	//camera = new TransformNode(0,0,0,0,0);

	//TransformNode* sun = new TransformNode(0,0,0,0.0,0.);
	//ObjectLoader* sunObj = new ObjectLoader(11);
	//objects[objectCount] = sunObj;
	//objectCount++;
	//sunObj->setSun();
	//sunObj->loadTexture("sun.png");
	//sunObj->loadOBJ("cube2.obj", sunObj->ver, sunObj->edge,sunObj->face, funcs);
	//camera->addChild(sun);
	//sun->addChild(sunObj);
}

GlWindow::~GlWindow()
{}

void GlWindow::wireframe_On()
{
	glPolygonMode(GL_FRONT, GL_LINE);
	glPolygonMode(GL_BACK, GL_LINE);
	glEnable(GL_CULL_FACE);
}

void GlWindow::wireframe_Off()
{
	glPolygonMode(GL_FRONT, GL_FILL);
	glPolygonMode(GL_BACK, GL_FILL);
	glDisable(GL_CULL_FACE);
}

void GlWindow::initializeGL()
{
	glClearColor(0., 0., 0., 1);
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_TEXTURE_2D);

	this->funcs.initializeGLFunctions();

	bool compiled, linked, bound;
	compiled = this->program.addShaderFromSourceCode(QGLShader::Vertex,vs.c_str());
	if (!compiled)
	{
		QString error = this->program.log();
		qErrnoWarning((const char*)error.constData());
	}

	compiled = this->program.addShaderFromSourceCode(QGLShader::Fragment, ps.c_str());
	if (!compiled)
	{
		QString error = this->program.log();
		qErrnoWarning((const char*)error.constData());
	}

	linked = this->program.link();

	if (!linked)
	{
		QString error = this->program.log();
		qErrnoWarning((const char*)error.constData());
	}
	else
	{
		this->modelLocation = this->program.uniformLocation("model");
		if (this->modelLocation == -1) qErrnoWarning("Model matrix has been removed because of optimization");
		this->viewLocation = this->program.uniformLocation("view");
		if (this->viewLocation == -1) qErrnoWarning("View matrix has been removed because of optimization");
		this->projectionLocation = this->program.uniformLocation("projection");
		if (this->projectionLocation == -1) qErrnoWarning("Projection matrix has been removed because of optimization");
		this->textureLocation = this->program.uniformLocation("tex");
		if (this->textureLocation == -1) qErrnoWarning("Texture variable has been removed because of optimization");
		this->vertexCoordLocation = this->program.attributeLocation("vertex");
		if (this->vertexCoordLocation == -1) qErrnoWarning("Vertex coordinate has been removed because of optimization");
		this->uvLocation = this->program.attributeLocation("uv");
		if (this->uvLocation == -1) qErrnoWarning("UV coordinate has been removed because of optimization");
		this->normalLocation = this->program.attributeLocation("normal");
		if (this->normalLocation == -1) qErrnoWarning("normal coordinate has been removed because of optimization");
		this->LightLocation = this->program.uniformLocation("LightPosition_worldspace");
		if (this->LightLocation == -1) qErrnoWarning("Light coordinate has been removed because of optimization");
		this->isSun = this->program.uniformLocation("isSun");
		if (this->isSun == -1) qErrnoWarning("sun boolean has been removed because of optimization");
	}

	this->program.enableAttributeArray(this->vertexCoordLocation);
	this->program.enableAttributeArray(this->uvLocation);

	funcs.glEnableVertexAttribArray(vertexCoordLocation);
	funcs.glEnableVertexAttribArray(uvLocation);
	funcs.glEnableVertexAttribArray(normalLocation);

	bound = this->program.bind();
	Q_ASSERT(bound);



	camera = new TransformNode(0,0,0,0,0);

	TransformNode* sun = new TransformNode(0,0,0,0,0);
	ObjectLoader* sunObj = new ObjectLoader(100);
	objects[objectCount] = sunObj;
	objectCount++;
	sunObj->setSun();
	sunObj->loadTexture("blue.png");
	sunObj->loadOBJ("d20.obj", sunObj->ver, sunObj->edge,sunObj->face, funcs);
	camera->addChild(sun);
	sun->addChild(sunObj);


}
void GlWindow::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	

	//camera
	control( m_mouseIn);
	updateCamera();

	Matrix4 M(4,4);
	Matrix4 trans(4,4);

	M[3][0] = -cameraXPos;
	M[3][1] = -cameraYPos;
	M[3][2] = -cameraZPos;
	
	M = M.rotateAxis(-rotCamX, "X") * M;
	M = M.rotateAxis(-rotCamY, "Y") * M;
	M = ~M;


	funcs.glUniform3f(LightLocation, 0,0,0);
	Matrix4 id(4,4);
	camera->update(id, carrier);


	for(int i = 0; i < carrier.size(); i++)
	{
		Matrix4 modelView(4,4);
		modelView = modelView  * carrier[i].trans * m_camera;

		float x = modelView[3][0];
		float y = modelView[3][1];
		float z = modelView[3][2];
		bool culled = false;

		if(x + 2.1 + carrier[i].boundingRightX < getFrustumLeftX(z) || 
			x - 2.1 - carrier[i].boundingLeftX > getFrustumRightX(z) || 
			y + 2.1 + carrier[i].boundingTopY < getFrustumBottomY(z) ||
			y - 2.1 - carrier[i].boundingBottomY > getFrustumTopY(z))
		{
			culled = true;
		}

		program.setUniformValue(textureLocation, 0);
		funcs.glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, carrier[i].texture);

		funcs.glUniformMatrix4fv(modelLocation, 1, false, carrier[i].trans.getMatrix16());
		
		if(carrier[i].sun)
			funcs.glUniform1f(isSun, 1);
		else
			funcs.glUniform1f(isSun, 0);

		//vertex
		funcs.glBindBuffer(GL_ARRAY_BUFFER, carrier[i].vertexID);
		funcs.glVertexAttribPointer(vertexCoordLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
		//uv
		funcs.glBindBuffer(GL_ARRAY_BUFFER, carrier[i].uvID);
		funcs.glVertexAttribPointer(uvLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
		//normal
		funcs.glBindBuffer(GL_ARRAY_BUFFER, carrier[i].normalID);
		funcs.glVertexAttribPointer(normalLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
		//draw
		if (!culled)
		{
			glDrawArrays(GL_TRIANGLES,0,carrier[i].verSize); 
		}
		else
		{
			cout << "Culled:" << i << " ";
		}
		
	}
	carrier.clear();
}


void GlWindow::resizeGL( int width, int height )
{
	m_proj = m_proj.persProj(60.0, 16/10, 0.001, 1000);
	funcs.glUniformMatrix4fv(projectionLocation, 1, false, m_proj.getMatrix16());
	glViewport(0, 0, width, height);

}

void GlWindow::updateCamera()
{

	Matrix4 traTmp(4,4);
	traTmp[3][0] = cameraXPos; 
	traTmp[3][1] = cameraYPos;
	traTmp[3][2] = cameraZPos;

	traTmp = traTmp * traTmp.rotateAxis(rotCamY, "Y");
	traTmp = traTmp * traTmp.rotateAxis(rotCamX, "X");

	m_camera = traTmp;

	funcs.glUniformMatrix4fv(viewLocation, 1, false, m_camera.getMatrix16());
}

void GlWindow::lockCamera()
{
	if (m_camPitch > 90)
	{
		m_camPitch = 90;
	}
	if (m_camPitch < -90)
	{
		m_camPitch = -90;
	}
	if (m_camYaw < 0.0)
	{
		m_camYaw += 360;
	}
	if (m_camYaw > 360)
	{
		m_camYaw -= 360;
	}
}

void GlWindow::control(bool i_mi)
{

	if(i_mi)
	{
		QPoint t_screenRes = mapToGlobal(QPoint(width()/2,height()/2));

		POINT t_mousePos;
		GetCursorPos(&t_mousePos);

		m_camYaw += 0.1f * (t_screenRes.rx()-t_mousePos.x);	
		m_camPitch += 0.1f * (t_screenRes.ry()-t_mousePos.y);

		lockCamera();
		SetCursorPos(t_screenRes.rx(), t_screenRes.ry());
	}
	rotCamX = m_camPitch;
	rotCamY = m_camYaw;
}

float GlWindow::getFrustumRightX(float deep)
{
	float pi = 3.14159265359;
	float tmp = pi - ((pi/3)*(16/10));
	return -(1.0f / 2.0f) * deep / sin(tmp/2);
}

float GlWindow::getFrustumLeftX(float deep)
{
	float pi = 3.14159265359;
	float tmp = pi - ((pi/3)*(16/10));
	return (1.0f / 2.0f) * deep / sin(tmp/2);
}

float GlWindow::getFrustumTopY(float deep)
{
	float pi = 3.14159265359;
	return -(1.0f / 2.0f) * deep / sin(pi / 3);
}

float GlWindow::getFrustumBottomY(float deep)
{
	float pi = 3.14159265359;
	return (1.0f / 2.0f) * deep / sin(pi / 3);
}



void GlWindow::keyPressEvent(QKeyEvent *event)
{
	float pi = 3.14159265359;
    if(event->key() == Qt::Key_W)
	{
		cameraZPos += cos(rotCamY*pi/180);
		cameraXPos += sin(rotCamY*pi/180);
		cameraYPos -= sin(rotCamX*pi/180);
	}
	else if(event->key() == Qt::Key_A)
	{
		cameraXPos += cos(rotCamY*pi/180);
		cameraZPos -= sin(rotCamY*pi/180);
	}
	else if(event->key() == Qt::Key_S)
	{
		cameraZPos -= cos(rotCamY*pi/180);
		cameraXPos -= sin(rotCamY*pi/180);
		cameraYPos += sin(rotCamX*pi/180);
	}
	else if(event->key() == Qt::Key_D)
	{
		cameraXPos -= cos(rotCamY*pi/180);
		cameraZPos += sin(rotCamY*pi/180);
	}
	else if(event->key() == Qt::Key_Up)
    {
        rotCamX++;
    }
	else if(event->key() == Qt::Key_Down)
    {
        rotCamX--;
    }
	else if(event->key() == Qt::Key_Left)
    {
        rotCamY++;
    }
	else if(event->key() == Qt::Key_Right)
    {
        rotCamY--;
    }
	else if(event->key() == Qt::Key_Space)
    {
        cameraYPos--;
    }
	else if(event->key() == Qt::Key_Control)
    {
        cameraYPos++;
    }
	else if(event->key() == Qt::Key_1)
    {
        RandObject();
    }
}

void GlWindow::mousePressEvent(QMouseEvent *event)
 {
	QPoint t_screenRes = mapToGlobal(QPoint(width()/2,height()/2));
	SetCursorPos(t_screenRes.rx(), t_screenRes.ry());

	GetCursorPos(&m_mousePos);
	m_mouseIn = true;
	ShowCursor(false);
 }

 void GlWindow::mouseReleaseEvent(QMouseEvent *event)
 {
	 m_mouseIn = false;
	 ShowCursor(true);
 }

void GlWindow::RandObject()
{
	float distance = rand() % 100;
	float orbSpeed = rand() % 20;
	orbSpeed -= 10;
	orbSpeed /= 100;
	float selfSpeed = rand() % 20;
	selfSpeed -= 10;
	selfSpeed /= 100;

	float scale = rand() % 10;
	scale /= 10;

	TransformNode* p2 = new TransformNode(0,0,distance,orbSpeed,selfSpeed);
	ObjectLoader* p2Obj = new ObjectLoader(scale);
	objects[objectCount] = p2Obj;
	objectCount++;
	p2Obj->loadTexture("eatrh.png");
	p2Obj->loadOBJ("sphere.obj", p2Obj->ver, p2Obj->edge, p2Obj->face, funcs);
	camera->addChild(p2);
	p2->addChild(p2Obj);
}


void GlWindow::loopSub()
{

	start = clock();
	objects[0]->subdivide(funcs);
	duration = (clock() - start) / ((float)CLOCKS_PER_SEC / 1000);

	loopCount++;

	cout << "\nSubdivision: " << loopCount << ". time: " << duration << "ms." << endl << endl;
}