/*
 * CFitsImages.h
 *
 *  Created on: 16 Mar 2016
 *      Author: ah14aeb
 */

#ifndef CFITSIMAGES_H_
#define CFITSIMAGES_H_

#include <vector>
#include <string>
#include <fitsio.h>

#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/imgcodecs.hpp>

class CFitsImages
{

public:
	int get_image_count()
	{
		return m_matImages.size();
	}

	cv::Mat* get_image(unsigned int index)
	{
		if(index < m_matImages.size())
			return m_matImages[index];
		else
			return NULL;
	}
	bool load_fits_file(std::string file_path);
	void save_fits_image(int image_idx, std::string file_path, cv::Mat& in);
	cv::Mat * get_mat_from_fits(int image_idx);

private:
	std::vector<cv::Mat*> m_matImages;
	std::vector<std::string> m_fitsFilePaths;


	void output_section(int x, int y, int width, int height);

};



#endif /* CFITSIMAGES_H_ */
