/*
Tao Du
taodu@stanford.edu
May 27, 2014
*/

#ifndef _PSV_CVHELPER_H_
#define _PSV_CVHELPER_H_

//	this header file defines the visualization functions
//	for the depth and color images

#include <string>

#include "opencv2\core\core.hpp"
#include "opencv2\highgui\highgui.hpp"

#include "NiHelper.h"

const std::string folder = "frames/";
const std::string depthPrefix = "depth_";
const std::string depthSuffix = ".png";

//	save depth images
void saveDepthImage(const std::string name, const VideoFrameRef &depthFrame);

#endif