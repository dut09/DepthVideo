/*
Tao Du
taodu@stanford.edu
May 27, 2014
*/

#include <iostream>

#include "NiHelper.h"

//	wait time for each frame
#define WAIT_TIME	4000

//	print the video mode information
void printVideoMode(const VideoMode &mode)
{
	std::cout << "xRes = " << mode.getResolutionX() << std::endl
		<< "yRes = " << mode.getResolutionY() << std::endl;
	std::cout << "Pixel = ";
	switch (mode.getPixelFormat())
	{
	case PIXEL_FORMAT_DEPTH_1_MM:
		std::cout << "DEPTH_1_MM" << std::endl;
		break;
	case PIXEL_FORMAT_DEPTH_100_UM:
		std::cout << "DEPTH_100_UM" << std::endl;
		break;
	case PIXEL_FORMAT_SHIFT_9_2:
		std::cout << "SHIFT_9_2" << std::endl;
		break;
	case PIXEL_FORMAT_SHIFT_9_3:
		std::cout << "SHIFT_9_3" << std::endl;
		break;
	case PIXEL_FORMAT_RGB888:
		std::cout << "RGB888" << std::endl;
		break;
	case PIXEL_FORMAT_YUV422:
		std::cout << "YUV422" << std::endl;
		break;
	case PIXEL_FORMAT_GRAY8:
		std::cout << "GRAY8" << std::endl;
		break;
	case PIXEL_FORMAT_GRAY16:
		std::cout << "GRAY16" << std::endl;
		break;
	case PIXEL_FORMAT_JPEG:
		std::cout << "JPEG" << std::endl;
		break;
	case PIXEL_FORMAT_YUYV:
		std::cout << "YUYV" << std::endl;
		break;
	default:
		std::cout << "unknown pixel format" << std::endl;
		break;
	}
	std::cout << "fps = " << mode.getFps() << std::endl;
}

//	report any runtime error from OpenNI
void reportError(std::string errorMessage)
{
	std::cout << errorMessage 
		<< OpenNI::getExtendedError()
		<< std::endl;
}

/*
//	depth and ir mode
void runDepthIRMode(const std::string folder,
					   const std::string depthPrefix,
					   const std::string irPrefix,
					   const std::string depthSuffix,
					   const std::string irSuffix)
{
	//	initialize the PrimeSense
	Status rc = OpenNI::initialize();
	if (rc != STATUS_OK)
	{
		reportError("fail to initialize: ");
		return;
	}
	//	open the device
	Device device;
	rc = device.open(ANY_DEVICE);
	if (rc != STATUS_OK)
	{
		reportError("fail to open the device: ");
		return;
	}

	//	open depth and ir streams
	VideoStream depth, ir;
	if (device.getSensorInfo(SENSOR_DEPTH) != NULL)
	{
		rc = depth.create(device, SENSOR_DEPTH);
		if (rc != STATUS_OK)
		{
			reportError("fail to create depth stream: ");
			return;
		}
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
			return;
		}
	}

	//	ir stream
	if (device.getSensorInfo(SENSOR_IR) != NULL)
	{
		rc = ir.create(device, SENSOR_IR);
		if (rc != STATUS_OK)
		{
			reportError("fail to create color stream: ");
			return;
		}
		//	set color resolution, mirror
		if (ir.getMirroringEnabled())
			ir.setMirroringEnabled(false);
		//	all the modes supported by the sensor
		const SensorInfo* pSensorInfo = device.getSensorInfo(SENSOR_IR);
		const openni::Array<VideoMode> &mode = pSensorInfo->getSupportedVideoModes();
		//	by default, we use 480 x 640, GRAY16 and 30 fps
		bool settingSucceed = false;
		for (int i = 0; i < mode.getSize(); i++)
		{
			const openni::VideoMode* pSupportedMode = &mode[i];
			if (pSupportedMode->getResolutionX() == 640 && pSupportedMode->getResolutionY() == 480
				&& pSupportedMode->getPixelFormat() == PIXEL_FORMAT_GRAY16
				&& pSupportedMode->getFps() == 30)
			{
				ir.setVideoMode(*pSupportedMode);
				settingSucceed = true;
				break;
			}
		}
		if (!settingSucceed)
		{
			std::cout << "fail to set desired mode!" << std::endl;
			return;
		}

	}
	//	turn off image registration
	if (device.isImageRegistrationModeSupported(IMAGE_REGISTRATION_OFF))
	{
		rc = device.setImageRegistrationMode(IMAGE_REGISTRATION_OFF);
		if (rc != STATUS_OK)
		{
			reportError("fail to turn off image registration");
			return;
		}
	}
	else
	{
		std::cout << "fail to turn off image registration" << std::endl;
		return;
	}
	//	start depth stream
	rc = depth.start();
	if (rc != STATUS_OK)
	{
		reportError("fail to start depth stream: ");
		return;
	}
	//	start ir stream
	rc = ir.start();
	if (rc != STATUS_OK)
	{
		reportError("fail to start ir stream: ");
		return;
	}
	//	create the display window
	std::string displayWindowName = "image";
	cv::namedWindow(displayWindowName, cv::WINDOW_AUTOSIZE);
	//	show the depth and color images
	bool isRunning = true;
	VideoFrameRef depthFrame, irFrame;
	int imageId = 1;
	while (isRunning)
	{
		//	capture the depth frame
		VideoStream* pDepthStream = &depth;
		int changedStreamDummy;
		//	wait for 2000ms
		rc = OpenNI::waitForAnyStream(&pDepthStream, 1, &changedStreamDummy, WAIT_TIME);
		if (rc != STATUS_OK)
		{
			std::cout << "time out" << std::endl;
			isRunning = false;
			break;
		}
		rc = depth.readFrame(&depthFrame);
		if (rc != STATUS_OK)
		{
			reportError("fail to read depth image: ");
			isRunning = false;
			break;
		}
		//	capture the ir frame
		VideoStream* pIRStream = &ir;
		rc = OpenNI::waitForAnyStream(&pIRStream, 1, &changedStreamDummy, WAIT_TIME);
		if (rc != STATUS_OK)
		{
			std::cout << "time out" << std::endl;
			isRunning = false;
			break;
		}
		rc = ir.readFrame(&irFrame);
		if (rc != STATUS_OK)
		{
			reportError("fail to read color image: ");
			isRunning = false;
			break;
		}
		//	use opencv to show the image
		char ch = showDepthIRImages(displayWindowName, depthFrame, irFrame);
		switch (ch)
		{
		case 'e':	//	exit
			{
				std::cout << "exit" << std::endl;
				isRunning = false;
				break;
			}
		case 's':	//	save the current frame
			{
				std::cout << "save the current image ..." << std::endl;
				//	save the depth image
				std::ostringstream numberString;
				numberString.fill('0');
				numberString.width(4);
				numberString << imageId;
				const std::string number = numberString.str();
				const std::string depthImageName = folder + depthPrefix + number + depthSuffix;
				saveDepthImage(depthImageName, depthFrame);
				//	save the ir image
				const std::string irImageName = folder + irPrefix + number + irSuffix;
				saveIRImage(irImageName, irFrame);
				imageId++;
				std::cout << "done" << std::endl;
				break;
			}
		default:	//	capture a new frame
			{
				//	do nothing
				std::cout << "capture a new frame ..." << std::endl;
				break;
			}
		}
	}
	//	destroy the display window
	cv::destroyWindow(displayWindowName);
	//	stop the device
	depth.stop();
	depth.destroy();
	ir.stop();
	ir.destroy();
	//	close the device
	device.close();
	OpenNI::shutdown();
}
*/