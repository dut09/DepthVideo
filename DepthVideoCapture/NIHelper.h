/*
Tao Du
taodu@stanford.edu
May 27, 2014
*/

#ifndef _PSV_NIHELPER_H_
#define _PSV_NIHELPER_H_

//	this header file defines some helper function in OpenNI
#include <string>

//	OpenNI header file
#include "OpenNI.h"

//	use the openni namespace
using namespace openni;

//	print the video mode information
void printVideoMode(const VideoMode &mode);

//	report any runtime error from OpenNI
void reportError(std::string errorMessage);

#endif