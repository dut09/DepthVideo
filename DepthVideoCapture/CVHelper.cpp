/*
Tao Du
taodu@stanford.edu
May 27, 2014
*/

//	warning: assert is only useful in debug mode
//	so it is recommended to run the program in debug
//	mode first, make sure there are no assertion failures
//	then move to release mode
#include <cassert>
#include "CVHelper.h"


//	save depth images
void saveDepthImage(const std::string name, const VideoFrameRef &depthFrame)
{
	int height = depthFrame.getHeight();
	int width = depthFrame.getWidth();
	cv::Mat image = cv::Mat::zeros(height, width, CV_16UC1);
	DepthPixel *pDepth = (DepthPixel *)depthFrame.getData();
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			image.at<uint16_t>(i, j) = (uint16_t)(*pDepth);
			pDepth++;
		}
	}
	cv::imwrite(name, image);
}