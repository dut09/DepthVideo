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
#include "NiHelper.h"
#include "Viewer.h"



int main(int argc, char** argv)
{
	Status rc = openni::STATUS_OK;

	Device device;
	VideoStream depth;
	const char* deviceURI = openni::ANY_DEVICE;
	if (argc > 1)
	{
		deviceURI = argv[1];
	}

	rc = openni::OpenNI::initialize();

	reportError("After initialization: ");

	rc = device.open(deviceURI);
	if (rc != openni::STATUS_OK)
	{
		reportError("SimpleViewer: Device open failed: ");
		openni::OpenNI::shutdown();
		return 1;
	}

	rc = depth.create(device, openni::SENSOR_DEPTH);
	if (rc == openni::STATUS_OK)
	{
		//	set the depth sensor here
				//	set depth resolution, mirror, registration
		if (depth.getMirroringEnabled())
			depth.setMirroringEnabled(false);
		//	all the modes supported by the sensor
		const SensorInfo* pSensorInfo = device.getSensorInfo(SENSOR_DEPTH);
		const openni::Array<VideoMode> &mode = pSensorInfo->getSupportedVideoModes();
		//	by default, we use 480 x 640, 1mm and 30 fps
		bool settingSucceed = false;
		for (int i = 0; i < mode.getSize(); i++)
		{
			const openni::VideoMode* pSupportedMode = &mode[i];
			if (pSupportedMode->getResolutionX() == 640 && pSupportedMode->getResolutionY() == 480
				&& pSupportedMode->getPixelFormat() == PIXEL_FORMAT_DEPTH_1_MM
				&& pSupportedMode->getFps() == 30)
			{
				depth.setVideoMode(*pSupportedMode);
				settingSucceed = true;
				break;
			}
		}
		if (!settingSucceed)
		{
			std::cout << "fail to set desired mode!" << std::endl;
			return 0;
		}
		//	turn off image registration
		if (device.isImageRegistrationModeSupported(IMAGE_REGISTRATION_OFF))
		{
			rc = device.setImageRegistrationMode(IMAGE_REGISTRATION_OFF);
			if (rc != STATUS_OK)
			{
				reportError("fail to turn off image registration");
				return 0;
			}
		}
		else
		{
			std::cout << "fail to turn off image registration" << std::endl;
			return 0;
		}
		rc = depth.start();
		if (rc != openni::STATUS_OK)
		{
			reportError("SimpleViewer: Couldn't start depth stream: ");
			depth.destroy();
		}
	}
	else
	{
		reportError("SimpleViewer: Couldn't find depth stream: ");
	}

	if (!depth.isValid())
	{
		reportError("SimpleViewer: No valid streams. Exiting.");
		openni::OpenNI::shutdown();
		return 2;
	}

	SampleViewer sampleViewer("Simple Viewer", device, depth);

	//	start the program!
	std::cout << "the program is ready, press any key to start..." << std::endl;
	char ch;
	std::cin >> ch;

	rc = sampleViewer.init(argc, argv);
	if (rc != openni::STATUS_OK)
	{
		openni::OpenNI::shutdown();
		return 3;
	}
	sampleViewer.run();
}