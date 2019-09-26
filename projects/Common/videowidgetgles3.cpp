// Copyright 2017 S2E Software, Systems and Control 
//  
// Licensed under the Apache License, Version 2.0 the "License"; 
// you may not use this file except in compliance with the License. 
// You may obtain a copy of the License at 
//  
//    http://www.apache.org/licenses/LICENSE-2.0 
//  
// Unless required by applicable law or agreed to in writing, software 
// distributed under the License is distributed on an "AS IS" BASIS, 
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
// See the License for the specific language governing permissions and 
// limitations under the License.

#include "videowidgetgles3.h"

#include <QOpenGLDebugLogger>
#include <QOpenGLShaderProgram>
#include <QtOpenGLExtensions/QOpenGLExtensions>

//#include <EGL/egl.h>
//#include <EGL/eglext.h>

VideoWidgetGLES3::VideoWidgetGLES3(QWidget* parent, Qt::WindowFlags windowFlags) :
	QOpenGLWidget(parent, windowFlags)
	,m_aspectRatio(640, 480)
{
	QSurfaceFormat newformat = format();
	newformat.setMajorVersion(3);
	newformat.setMinorVersion(0);
//	newformat.setProfile(QSurfaceFormat::CoreProfile);
	newformat.setOption(QSurfaceFormat::DebugContext);
	setFormat(newformat);

//	m_oes = new QOpenGLExtension_OES_EGL_image();

}

QSize VideoWidgetGLES3::sizeHint() const
{
	return m_aspectRatio;
}

void VideoWidgetGLES3::drawImage(const QImage& image)
{
	// Internal format of GL_RGB is not supported by
	// raspberry pi 1 & 2
	GLint const internalFormat = GL_RGBA;
	GLenum const externalType = GL_UNSIGNED_BYTE;

	// Determine external format
	GLenum externalFormat;

	switch (image.format())
	{
	case QImage::Format_RGBA8888:
	case QImage::Format_RGBA8888_Premultiplied:
	case QImage::Format_ARGB32:
	case QImage::Format_ARGB32_Premultiplied:
		externalFormat = GL_RGBA;
		break;
	case QImage::Format_RGB888:
		externalFormat = GL_RGB;
		break;
	default:
		qCritical() << "Image format" << image.format() << "not supported by VideoWidgetES";
	}

	int const width = image.width();
	int const height = image.height();

	auto pixels = static_cast<GLvoid const* >(image.constBits());
	openGLdebug("drawImage-1");
//	glTexImage2D(GL_TEXTURE_2D,
//				 0 /*level*/, internalFormat, width, height,
//				 0 /*border, "this value must be zero" [OpenGL doc] */,
//				 externalFormat, externalType, pixels);

//	EGLDisplay disp;
//	EGLContext context;
//	EGLenum target = EGL_IMAGE_BRCM_MULTIMEDIA;
//	EGLImageKHR eglImage = eglCreateImageKHR(disp, context, target);

//	openGLdebug("drawImage-tex");

//	m_oes->glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, eglImage);

	openGLdebug("drawImage-2");
}

void VideoWidgetGLES3::bindTexture()
{
//	glBindTexture(GL_TEXTURE_2D, m_textureName);
//	glBindTexture(GL_TEXTURE_EXTERNAL_OES, m_textureName);
}

void VideoWidgetGLES3::openGLdebug(const QString& msg)
{
	if (m_glLogger != nullptr)
	{
		qDebug() << "VideoWidgetGLES3::openGLdebug " << msg << ":";
		QList<QOpenGLDebugMessage> messages = m_glLogger->loggedMessages();
		if (messages.length() == 0)
		{
			qDebug() << "\t All Good.";
		}
		foreach (const QOpenGLDebugMessage &message, messages)
		{
			qDebug().noquote() << "\t" << message;
		}
	}
}

void VideoWidgetGLES3::initializeGL()
{
	QString const vertexShaderStr = QString(
		"#version 300 es						\n"
		"// Input from the application			\n"
		"in highp vec2 inTexCoord;					\n"
		"in highp vec4 inVertexLoc;					\n"
		"										\n"
		"// Output for the fragment shader		\n"
		"out highp vec2 vsTexCoords;					\n"
		"										\n"
		"void main()							\n"
		"{										\n"
		"	gl_Position = inVertexLoc;			\n"
		"	vsTexCoords = inTexCoord;			\n"
		"}										\n"
	);

	//"uniform samplerExternalOES textureSampler;			\n"
	//"#extension GL_OES_EGL_image_external : require		\n"
	QString const fragmentShaderStr = QString(
		"#version 300 es										\n"
		"//Input from vertex shader:							\n"
		"in highp vec2 vsTexCoords;								\n"
		"														\n"
		"//From application:									\n"
		"uniform highp sampler2D textureSampler;						\n"
		"														\n"
		"layout(location = 0) out highp vec4 color;				\n"
		"														\n"
		"void main()											\n"
		"{														\n"
		"	color = texture(textureSampler, vsTexCoords);		\n"
		"}														\n"
	);



	bool ret;
//	ret = m_oes->initializeOpenGLFunctions();
//	if (ret == false)
//	{
//		qDebug() << "QOpenGLExtension_OES_EGL_image not initialized";
//	}
	initializeOpenGLFunctions();

	m_glLogger = new QOpenGLDebugLogger(this);
	m_glLogger->initialize(); // initializes in the current context

	m_shader = new QOpenGLShaderProgram(this);

	ret = m_shader->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderStr);
	if (ret == false)
	{
		qCritical() << "Could not load vertex shader";
	}
	ret = m_shader->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderStr);
	if (ret == false)
	{
		qCritical() << "Could not load fragment shader";
	}

	ret = m_shader->link();
	if (ret == false)
	{
		qCritical() << "Could not link the shaders";
	}
	openGLdebug("link");

	ret = m_shader->bind();
	if (ret == false)
	{
		qCritical() << "Could not bind background shader";
	}
	openGLdebug("bind");

	int vertexLocation = m_shader->attributeLocation("inVertexLoc");
	if (vertexLocation == -1)
	{
		qCritical() << "vertexLocation could not be found.";
	}
	int texCoordLocation = m_shader->attributeLocation("inTexCoord");
	if (texCoordLocation == -1)
	{
		qCritical() << "texCoordLocation could not be found.";
	}



	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
//	glClearDepth(1.0);

	openGLdebug("glClearColor");

	// Vertex data for the GPU
	static const GLfloat bufferData[] =
	{
		//Vertex locations
		-1.0f, -1.0f,
		1.0f, -1.0f,
		1.0f, 1.0f,
		-1.0f, 1.0f,
		// Tex coords
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f
	};

	// VBO - Vertex Buffer Object
	GLuint vertexBufferObject;
	glGenBuffers(1, &vertexBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
	//Allocate space to the vertex buffer object as it is currently bound to GL_ARRAY_BUFFER
	glBufferData(GL_ARRAY_BUFFER, sizeof(bufferData), bufferData, GL_STATIC_DRAW);

	openGLdebug("VBO");

	// VAO - Vertex Array Object
	glGenVertexArrays(1, &m_vertexArrayObject);
	glBindVertexArray(m_vertexArrayObject);

	// Define the memory location that is associated with the shader locations
	// Note: this must be done after the Vertex Buffer Object is bound
	glVertexAttribPointer(vertexLocation, 2 /*nrOfComponents*/, GL_FLOAT /*type*/,
						  GL_FALSE /*normalized*/, 0 /*stride*/,
						  reinterpret_cast<const void*>(0) /*pointer*/);
	glVertexAttribPointer(texCoordLocation, 2 /*nrOfComponents*/, GL_FLOAT /*type*/,
						  GL_FALSE /*normalized*/, 0 /*stride*/,
						  reinterpret_cast<const void*>(8 * sizeof(GLfloat)) /*pointer*/);

	openGLdebug("VAO");

	// Enable shader locations
	glEnableVertexAttribArray(vertexLocation);
	glEnableVertexAttribArray(texCoordLocation);

	// Textures
//	glGenTextures(1, &m_textureName);

//	glBindTexture(GL_TEXTURE_2D, m_textureName);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


//	glBindTexture(GL_TEXTURE_EXTERNAL_OES, m_textureName);
//	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	openGLdebug("Textures");
}

void VideoWidgetGLES3::paintGL()
{
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDisable(GL_CULL_FACE);



	bindTexture();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Qt resets the viewport to rect() everytime paintGL() is called.
	// Hence set it everytime to the viewport rectangle
	glViewport(m_vpRect.left(), m_vpRect.top(), m_vpRect.width(), m_vpRect.height());
	glDrawArrays(GL_TRIANGLE_FAN, 0 /*first*/, 4 /*count*/);

//	openGLdebug("paintGL2");
}

void VideoWidgetGLES3::resizeGL(int width, int height)
{
	m_vpRect = QRect(QPoint(0,0), m_aspectRatio.scaled(width, height, Qt::KeepAspectRatio));
	m_vpRect.moveCenter(rect().center());
}

void VideoWidgetGLES3::mouseDoubleClickEvent(QMouseEvent* event)
{
	Q_UNUSED(event)

	if (isFullScreen() == true)
	{
		showNormal();
	}
	else
	{
		showFullScreen();
	}
}

QRect VideoWidgetGLES3::vpRect() const
{
	return m_vpRect;
}

QSize VideoWidgetGLES3::aspectRatio() const
{
	return m_aspectRatio;
}

void VideoWidgetGLES3::setAspectRatio(const QSize& aspectRatio)
{
	m_aspectRatio = aspectRatio;
}