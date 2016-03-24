/*
 * CFitsImages.cpp
 *
 *  Created on: 16 Mar 2016
 *      Author: ah14aeb
 */

#include "CFitsImages.h"
#include <iostream>
#include <string>


cv::Mat* CFitsImages::get_mat_from_fits(int image_idx)
{
/*	std::string file_path = m_fitsFilePaths[image_idx];

    int  dBitsPerPixel = 0;
    int  dFitsStatus   = 0;
    int  dNAxis        = 0;
    long dNAxes[ 2 ]   = { 0, 0 };

    fitsfile* m_fptr = NULL;
    int ret = fits_open_file( &m_fptr, file_path.c_str(), 0, &dFitsStatus );
    std::cout << "load file status: " << ret << " for file: " << file_path << std::endl;

    fits_get_img_param( m_fptr,
                        2,
                        &dBitsPerPixel,
                        &dNAxis,
                        dNAxes,
                        &dFitsStatus );
*/
	return NULL;
}

bool CFitsImages::load_fits_file(std::string file_path)
{

    //char path[] = "/run/media/ah14aeb/New Volume/Users/alexh/OneDrive/Data/Images/v1.0/macs1149/hlsp_frontier_hst_acs-30mas_macs1149_f606w_v1.0-epoch1_drz_ok.fits";
    int  dBitsPerPixel = 0;
    int  dFitsStatus   = 0;
    int  dNAxis        = 0;
    long dNAxes[ 2 ]   = { 0, 0 };

    fitsfile* m_fptr = NULL;
    int ret = fits_open_file( &m_fptr, file_path.c_str(), 0, &dFitsStatus );
    std::cout << "load file status: " << ret << " for file: " << file_path << std::endl;

    fits_get_img_param( m_fptr,
                        2,
                        &dBitsPerPixel,
                        &dNAxis,
                        dNAxes,
                        &dFitsStatus );

    int width = dNAxes[0];
    int height = dNAxes[1];
    int dataLength = dNAxes[ 0 ] * dNAxes[ 1 ];

    std::cout << "dataLength: " << dataLength << " height: " << height << " width: " << width << std::endl;
    std::cout << "dNAxes[0]: " << dNAxes[0] << "  dNAxes[1]: " << dNAxes[1] << std::endl;
    std::cout << "dNAxis: " << dNAxis << "  dBitsPerPixel: " << dBitsPerPixel << "  dFitsStatus: " << dFitsStatus << std::endl;

    float* dataBuffer = new float[ dataLength ];
    fits_read_img( m_fptr,
                   TFLOAT,
                   1,
                   dataLength,
                   NULL,
                   dataBuffer,
                   NULL,
                   &dFitsStatus );

    std::cout << "dFitsStatus: " << dFitsStatus << std::endl;
    std::cout << "copying data into Mat" << std::endl;

    cv::Mat* M2 = new cv::Mat(height, width, CV_32FC1);
    for (int y=0; y<height; y++)
    {
        for (int x=0; x<width; x++)
        {
            unsigned int offset_pos = (y * width) + x;
            M2->at<float>(y, x) = dataBuffer[offset_pos];

        }
    }
    std::cout << "finished copying data into mat" << std::endl;

    delete [] dataBuffer;
    dataBuffer = NULL;


    this->m_matImages.push_back(M2);
    this->m_fitsFilePaths.push_back(file_path);

    return ret > 0 ? false : true;
}

void CFitsImages::save_fits_image(int image_idx, std::string out_file_path, cv::Mat& outmat)
{

	std::string in_file_path = this->m_fitsFilePaths[image_idx];
	std::cout << "in file to be output: " << in_file_path << std::endl;

    fitsfile *infptr, *outfptr;   /* FITS file pointers defined in fitsio.h */
    int status = 0, ii = 1, iteration = 0, single = 0, hdupos;
    int hdutype, bitpix, bytepix, naxis = 0, nkeys, datatype = 0, anynul;
    long naxes[9] = {1, 1, 1, 1, 1, 1, 1, 1, 1};
    long first, totpix = 0, npix;
    double *array, bscale = 1.0, bzero = 0.0, nulval = 0.;
    char card[81];

    /* Open the input file and create output file */
    fits_open_file(&infptr, in_file_path.c_str(), READONLY, &status);
    fits_create_file(&outfptr, out_file_path.c_str(), &status);

    if (status != 0) {
        fits_report_error(stderr, status);
        return;
    }

    fits_get_hdu_num(infptr, &hdupos);  /* Get the current HDU position */

    /* Copy only a single HDU if a specific extension was given */
    //if (hdupos != 1 || strchr(argv[1], '[')) single = 1;
    single = 1;

    for (; !status; hdupos++)  /* Main loop through each extension */
    {

      fits_get_hdu_type(infptr, &hdutype, &status);

      if (hdutype == IMAGE_HDU) {

          /* get image dimensions and total number of pixels in image */
          for (ii = 0; ii < 9; ii++)
              naxes[ii] = 1;

          fits_get_img_param(infptr, 9, &bitpix, &naxis, naxes, &status);

          totpix = naxes[0] * naxes[1] * naxes[2] * naxes[3] * naxes[4]
             * naxes[5] * naxes[6] * naxes[7] * naxes[8];
      }

      if (hdutype != IMAGE_HDU || naxis == 0 || totpix == 0) {

          /* just copy tables and null images */
          fits_copy_hdu(infptr, outfptr, 0, &status);

      } else {

          /* Explicitly create new image, to support compression */
          fits_create_img(outfptr, bitpix, naxis, naxes, &status);

          /* copy all the user keywords (not the structural keywords) */
          fits_get_hdrspace(infptr, &nkeys, NULL, &status);

          for (ii = 1; ii <= nkeys; ii++) {
              fits_read_record(infptr, ii, card, &status);
              if (fits_get_keyclass(card) > TYP_CMPRS_KEY)
                  fits_write_record(outfptr, card, &status);
          }

          switch(bitpix) {
              case BYTE_IMG:
                  datatype = TBYTE;
                  break;
              case SHORT_IMG:
                  datatype = TSHORT;
                  break;
              case LONG_IMG:
                  datatype = TLONG;
                  break;
              case FLOAT_IMG:
                  datatype = TFLOAT;
                  break;
              case DOUBLE_IMG:
                  datatype = TDOUBLE;
                  break;
          }

          bytepix = abs(bitpix) / 8;

          npix = totpix;
          iteration = 0;

          // turn off any scaling so that we copy the raw pixel values
          fits_set_bscale(infptr,  bscale, bzero, &status);
          fits_set_bscale(outfptr, bscale, bzero, &status);

          first = 1;
          while (totpix > 0 && !status)
          {
             // convert mat to array for insert
             std::vector<float> array;
             if (outmat.isContinuous()) {
               array.assign((float*)outmat.datastart, (float*)outmat.dataend);
             } else {
               for (int i = 0; i < outmat.rows; ++i) {
                 array.insert(array.end(), (float*)outmat.ptr<uchar>(i), (float*)outmat.ptr<uchar>(i)+outmat.cols);
               }
             }
             // std::vector<double> v;
             float* outarray = &array[0];

             fits_write_img(outfptr, datatype, first, npix, outarray, &status);
             //ffppr(outfptr, datatype, first, npix, outarray, &status);
             totpix = totpix - npix;
             first  = first  + npix;
          }
      }

      if (single) break;  /* quit if only copying a single HDU */
      fits_movrel_hdu(infptr, 1, NULL, &status);  /* try to move to next HDU */
    }

    if (status == END_OF_FILE)  status = 0; /* Reset after normal error */

    fits_close_file(outfptr,  &status);
    fits_close_file(infptr, &status);

    /* if error occurred, print out error message */
    if (status)
       fits_report_error(stderr, status);
}

void CFitsImages::output_section(int x, int y, int width, int height)
{
	/*
	    for (int y=5000; y<5020; y++) {
	        for (int x = 5000; x < 5020; x++) {
	            unsigned int offset_pos = (y * width) + x;
	            cout << dataBuffer[offset_pos];
	        }
	        cout << endl;
	    }


    for (int y=5000; y<5020; y++) {
        for (int x = 5000; x < 5020; x++) {
            //unsigned int offset_pos = (y * width) + x;
            std::cout << M2.at<float>(y,x); " ";
        }
        std::cout << endl;
    }


	*/
}

