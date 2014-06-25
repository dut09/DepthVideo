/*****************************************************************************
*                                                                            *
*  OpenNI 2.x Alpha                                                          *
*  Copyright (C) 2012 PrimeSense Ltd.                                        *
*                                                                            *
*  This file is part of OpenNI.                                              *
*                                                                            *
*  Licensed under the Apache License, Version 2.0 (the "License");           *
*  you may not use this file except in compliance with the License.          *
*  You may obtain a copy of the License at                                   *
*                                                                            *
*      http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                            *
*  Unless required by applicable law or agreed to in writing, software       *
*  distributed under the License is distributed on an "AS IS" BASIS,         *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
*  See the License for the specific language governing permissions and       *
*  limitations under the License.                                            *
*                                                                            *
*****************************************************************************/


#include <iostream>
#include <fstream>

#include "Viewer.h"
#include "glut.h"
#include "glext.h"
#include "OniSampleUtilities.h"

#include "CVHelper.h"

#define GL_WIN_SIZE_X	640
#define GL_WIN_SIZE_Y	480
#define TEXTURE_SIZE	512

#define DEFAULT_DISPLAY_MODE	DISPLAY_MODE_DEPTH

#define MIN_NUM_CHUNKS(data_size, chunk_size)	((((data_size)-1) / (chunk_size) + 1))
#define MIN_CHUNKS_SIZE(data_size, chunk_size)	(MIN_NUM_CHUNKS(data_size, chunk_size) * (chunk_size))

int imageId = 1;

SampleViewer* SampleViewer::ms_self = NULL;

void SampleViewer::glutIdle()
{
	glutPostRedisplay();
}
void SampleViewer::glutDisplay()
{
	SampleViewer::ms_self->display();
}
void SampleViewer::glutKeyboard(unsigned char key, int x, int y)
{
	SampleViewer::ms_self->onKey(key, x, y);
}

SampleViewer::SampleViewer(const char* strSampleName, openni::Device& device, openni::VideoStream& depth) :
	m_device(device), m_depthStream(depth), m_streams(NULL), m_eViewState(DEFAULT_DISPLAY_MODE), m_pTexMap(NULL)

{
	ms_self = this;
	strncpy(m_strSampleName, strSampleName, ONI_MAX_STR);
	//	clear the time stamp
	m_timeStamp.clear();
}
SampleViewer::~SampleViewer()
{
	delete[] m_pTexMap;

	ms_self = NULL;

	if (m_streams != NULL)
	{
		delete []m_streams;
	}

	//	clear the timestamp
	m_timeStamp.clear();
}

openni::Status SampleViewer::init(int argc, char **argv)
{
	openni::VideoMode depthVideoMode;
	openni::VideoMode colorVideoMode;

	if (m_depthStream.isValid())
	{
		depthVideoMode = m_depthStream.getVideoMode();
		m_width = depthVideoMode.getResolutionX();
		m_height = depthVideoMode.getResolutionY();
	}
	else
	{
		printf("Error - depth stream not valid...\n");
		return openni::STATUS_ERROR;
	}

	m_streams = new openni::VideoStream*[2];
	m_streams[0] = &m_depthStream;
	
	// Texture map init
	m_nTexMapX = MIN_CHUNKS_SIZE(m_width, TEXTURE_SIZE);
	m_nTexMapY = MIN_CHUNKS_SIZE(m_height, TEXTURE_SIZE);
	m_pTexMap = new openni::RGB888Pixel[m_nTexMapX * m_nTexMapY];

	return initOpenGL(argc, argv);

}
openni::Status SampleViewer::run()	//Does not return
{
	glutMainLoop();

	return openni::STATUS_OK;
}
void SampleViewer::display()
{
	int changedIndex;
	//	wait for 4s
	openni::Status rc = openni::OpenNI::waitForAnyStream(m_streams, 1, &changedIndex, 4000);
	if (rc != openni::STATUS_OK)
	{
		printf("Wait failed\n");
		return;
	}
	//	get the time stamp
	SYSTEMTIME t;
	GetLocalTime(&t);
	std::cout << "local time: " << t.wSecond << "\t" << t.wMilliseconds << std::endl;
	m_timeStamp.push_back(std::pair<int, SYSTEMTIME>(imageId, t));

	switch (changedIndex)
	{
	case 0:
		m_depthStream.readFrame(&m_depthFrame); break;
	default:
		printf("Error in wait\n");
	}

	//	save the frame right now!
	std::ostringstream numberString;
	numberString.fill('0');
	numberString.width(4);
	numberString << imageId;
	const std::string number = numberString.str();
	const std::string depthImageName = folder + depthPrefix + number + depthSuffix;
	saveDepthImage(depthImageName, m_depthFrame);
	imageId++;

	/*
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, GL_WIN_SIZE_X, GL_WIN_SIZE_Y, 0, -1.0, 1.0);
	
	if (m_depthFrame.isValid())
	{
		calculateHistogram(m_pDepthHist, MAX_DEPTH, m_depthFrame);
	}

	memset(m_pTexMap, 0, m_nTexMapX*m_nTexMapY*sizeof(openni::RGB888Pixel));

	// check if we need to draw depth frame to texture
	if (m_eViewState == DISPLAY_MODE_DEPTH && m_depthFrame.isValid())
	{
		const openni::DepthPixel* pDepthRow = (const openni::DepthPixel*)m_depthFrame.getData();
		openni::RGB888Pixel* pTexRow = m_pTexMap + m_depthFrame.getCropOriginY() * m_nTexMapX;
		int rowSize = m_depthFrame.getStrideInBytes() / sizeof(openni::DepthPixel);

		for (int y = 0; y < m_depthFrame.getHeight(); ++y)
		{
			const openni::DepthPixel* pDepth = pDepthRow;
			openni::RGB888Pixel* pTex = pTexRow + m_depthFrame.getCropOriginX();

			for (int x = 0; x < m_depthFrame.getWidth(); ++x, ++pDepth, ++pTex)
			{
				if (*pDepth != 0)
				{
					int nHistValue = m_pDepthHist[*pDepth];
					pTex->r = nHistValue;
					pTex->g = nHistValue;
					pTex->b = 0;
				}
			}
			pDepthRow += rowSize;
			pTexRow += m_nTexMapX;
		}
	}

	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_nTexMapX, m_nTexMapY, 0, GL_RGB, GL_UNSIGNED_BYTE, m_pTexMap);

	// Display the OpenGL texture map
	glColor4f(1,1,1,1);

	glBegin(GL_QUADS);

	int nXRes = m_width;
	int nYRes = m_height;

	// upper left
	glTexCoord2f(0, 0);
	glVertex2f(0, 0);
	// upper right
	glTexCoord2f((float)nXRes/(float)m_nTexMapX, 0);
	glVertex2f(GL_WIN_SIZE_X, 0);
	// bottom right
	glTexCoord2f((float)nXRes/(float)m_nTexMapX, (float)nYRes/(float)m_nTexMapY);
	glVertex2f(GL_WIN_SIZE_X, GL_WIN_SIZE_Y);
	// bottom left
	glTexCoord2f(0, (float)nYRes/(float)m_nTexMapY);
	glVertex2f(0, GL_WIN_SIZE_Y);
	
	glEnd();
	
	// Swap the OpenGL display buffers
	//glutSwapBuffers();
	*/
}

void SampleViewer::saveTimeStamps(const std::string filename)
{
	std::ofstream fout;
	fout.open(filename);
	int length = (int)m_timeStamp.size();
	for (int i = 0; i < length; i++)
	{
		fout << m_timeStamp[i].first << "\t";
		SYSTEMTIME t = m_timeStamp[i].second;
		//	compute the millseconds from 00:00:00
		int mills = (t.wHour * 3600 + t.wMinute * 60 + t.wSecond) * 1000 + t.wMilliseconds;
		fout << t.wHour << "\t" << t.wMinute << "\t" << t.wSecond << "\t" << t.wMilliseconds << "\t" << mills << "\n";
	}
	fout.close();
}

void SampleViewer::onKey(unsigned char key, int /*x*/, int /*y*/)
{
	switch (key)
	{
	case 27:
		m_depthStream.stop();
		m_depthStream.destroy();
		m_device.close();
		openni::OpenNI::shutdown();
		//	save the time file
		saveTimeStamps(folder + "timestamp.txt");
		exit (1);
	case '1':
		m_eViewState = DISPLAY_MODE_OVERLAY;
		m_device.setImageRegistrationMode(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR);
		break;
	case '2':
		m_eViewState = DISPLAY_MODE_DEPTH;
		m_device.setImageRegistrationMode(openni::IMAGE_REGISTRATION_OFF);
		break;
	case '3':
		m_eViewState = DISPLAY_MODE_IMAGE;
		m_device.setImageRegistrationMode(openni::IMAGE_REGISTRATION_OFF);
		break;
	case 'm':
		m_depthStream.setMirroringEnabled(!m_depthStream.getMirroringEnabled());
		break;
	}

}

openni::Status SampleViewer::initOpenGL(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(GL_WIN_SIZE_X, GL_WIN_SIZE_Y);
	glutCreateWindow (m_strSampleName);
	// 	glutFullScreen();
	glutSetCursor(GLUT_CURSOR_NONE);

	initOpenGLHooks();

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	return openni::STATUS_OK;

}
void SampleViewer::initOpenGLHooks()
{
	glutKeyboardFunc(glutKeyboard);
	glutDisplayFunc(glutDisplay);
	glutIdleFunc(glutIdle);
}
