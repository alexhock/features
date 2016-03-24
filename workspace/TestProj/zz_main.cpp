#include <opencv2/features2d.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>
#include <fitsio.h>
#include "CFitsImages.h"
//#include "fitsio.h"
//#include "CFitsFile.h"

using namespace std;
using namespace cv;

const float inlier_threshold = 2.5f; // Distance threshold to identify inliers
const float nn_match_ratio = 0.8f;   // Nearest neighbor matching ratio




void fits2()
{
    char path[] = "/run/media/ah14aeb/New Volume/Users/alexh/OneDrive/Data/Images/v1.0/macs1149/hlsp_frontier_hst_acs-30mas_macs1149_f606w_v1.0-epoch1_drz_ok.fits";
    int  dBitsPerPixel = 0;
    int  dFitsStatus   = 0;
    int  dNAxis        = 0;
    long dNAxes[ 2 ]   = { 0, 0 };

    //int dFitsStatus = 0;
    fitsfile* m_fptr = NULL;
    fits_open_file( &m_fptr, path, 0, &dFitsStatus );

    fits_get_img_param( m_fptr,
                        2,
                        &dBitsPerPixel,
                        &dNAxis,
                        dNAxes,
                        &dFitsStatus );

    int width = dNAxes[0];
    int height = dNAxes[1];
    int dataLength = dNAxes[ 0 ] * dNAxes[ 1 ];

    cout << "dataLength: " << dataLength << endl;

    cout << "dNAxes[0]: " << dNAxes[0] << "  dNAxes[1]: " << dNAxes[1] << endl;
    cout << "dNAxis: " << dNAxis << "  dBitsPerPixel: " << dBitsPerPixel << "  dFitsStatus: " << dFitsStatus << endl;


    float* dataBuffer = new float[ dataLength ];
    fits_read_img( m_fptr,
                   TFLOAT,
                   1,
                   dataLength,
                   NULL,
                   dataBuffer,
                   NULL,
                   &dFitsStatus );

    cout << "dFitsStatus: " << dFitsStatus << endl;
    cout << "copying data into Mat" << endl;
    Mat M2(height, width, CV_32FC1);
    for (int y=0; y<height; y++)
    {
        for (int x=0; x<width; x++)
        {
            unsigned int offset_pos = (y * width) + x;
            M2.at<float>(y, x) = dataBuffer[offset_pos];

        }
    }
    cout << "finished copying data into mat" << endl;

    delete [] dataBuffer;
    dataBuffer = NULL;


    for (int y=5000; y<5020; y++) {
        for (int x = 5000; x < 5020; x++) {
            //unsigned int offset_pos = (y * width) + x;
            cout << M2.at<float>(y,x) << " ";
        }
        cout << endl;
    }

    vector<KeyPoint> kpts1, kpts2;
    Mat desc1, desc2;

    Ptr<AKAZE> akaze = AKAZE::create();
    akaze->detectAndCompute(M2, noArray(), kpts1, desc1);

    cout << "keypoints: " << kpts1.size() << endl;
    cout << "desc1 rows:" << desc1.rows << " cols: " << desc1.cols << endl;

    /*
    CFitsFile cFitsFile(path, READONLY);
    unsigned short * data = (unsigned short*) cFitsFile.Read();
    for (int i=0;i<10;i++)
        cout << data[i] << endl;

    long naxis = 0;
    long naxis2 = 0;
    //cFitsFile.GetParameters()
*/
}


int main2(void)
{

	CFitsImages fits_images;
	std::string fits_base_path = "/run/media/ah14aeb/New Volume/Users/alexh/OneDrive/Data/Images/v1.0/";
	std::string final_path = fits_base_path + "macs1149/hlsp_frontier_hst_acs-30mas_macs1149_f606w_v1.0-epoch1_drz_ok.fits";
	fits_images.load_fits_file(final_path);


    Mat M(2,2, CV_8UC3, Scalar(0,0,255));
    cout << "M = " << endl << " " << M << endl << endl;
    Mat M2(2,2, CV_32FC1, Scalar(-0.01));
    cout << "M2 = " << endl << " " << M2 << endl << endl;

    //fits();
    fits2();

    String path = "/home/ah14aeb/Downloads/opencv-3.1.0/samples/data/";

    cout << path << endl;

    Mat img1 = imread(path + "graf1.png", IMREAD_GRAYSCALE);
    Mat img2 = imread(path + "graf3.png", IMREAD_GRAYSCALE);

    Mat homography;
    FileStorage fs(path + "H1to3p.xml", FileStorage::READ);
    fs.getFirstTopLevelNode() >> homography;

    vector<KeyPoint> kpts1, kpts2;
    Mat desc1, desc2;

    Ptr<AKAZE> akaze = AKAZE::create();
    akaze->detectAndCompute(img1, noArray(), kpts1, desc1);
    akaze->detectAndCompute(img2, noArray(), kpts2, desc2);

    BFMatcher matcher(NORM_HAMMING);
    vector< vector<DMatch> > nn_matches;
    matcher.knnMatch(desc1, desc2, nn_matches, 2);

    vector<KeyPoint> matched1, matched2, inliers1, inliers2;
    vector<DMatch> good_matches;
    for(size_t i = 0; i < nn_matches.size(); i++) {
        DMatch first = nn_matches[i][0];
        float dist1 = nn_matches[i][0].distance;
        float dist2 = nn_matches[i][1].distance;

        if(dist1 < nn_match_ratio * dist2) {
            matched1.push_back(kpts1[first.queryIdx]);
            matched2.push_back(kpts2[first.trainIdx]);
        }
    }

    for(unsigned i = 0; i < matched1.size(); i++) {
        Mat col = Mat::ones(3, 1, CV_64F);
        col.at<double>(0) = matched1[i].pt.x;
        col.at<double>(1) = matched1[i].pt.y;

        col = homography * col;
        col /= col.at<double>(2);
        double dist = sqrt( pow(col.at<double>(0) - matched2[i].pt.x, 2) +
                            pow(col.at<double>(1) - matched2[i].pt.y, 2));

        if(dist < inlier_threshold) {
            int new_i = static_cast<int>(inliers1.size());
            inliers1.push_back(matched1[i]);
            inliers2.push_back(matched2[i]);
            good_matches.push_back(DMatch(new_i, new_i, 0));
        }
    }

    Mat res;
    drawMatches(img1, inliers1, img2, inliers2, good_matches, res);
    imwrite(path + "res.png", res);

    double inlier_ratio = inliers1.size() * 1.0 / matched1.size();
    cout << "A-KAZE Matching Results" << endl;
    cout << "*******************************" << endl;
    cout << "# Keypoints 1:                        \t" << kpts1.size() << endl;
    cout << "# Keypoints 2:                        \t" << kpts2.size() << endl;
    cout << "# Matches:                            \t" << matched1.size() << endl;
    cout << "# Inliers:                            \t" << inliers1.size() << endl;
    cout << "# Inliers Ratio:                      \t" << inlier_ratio << endl;
    cout << endl;

    return 0;
}

/*
int fits()
{

    fitsfile *fptr = NULL;
    int status = 0,  nkeys = 0, ii = 0;
    char* card2 = new char[FLEN_CARD];
    //char card[FLEN_CARD];

    fits_open_file(&fptr, "/home/ah14aeb/Downloads/atlas-cdfs.fits", READONLY, &status);
    fits_get_hdrspace(fptr, &nkeys, NULL, &status);

    long fpixel[10000];
    long lpixel[10000];
    long inc[ 2 ] = { 1, 1 };

//    fits_read_subset(fptr, TFLOAT,fpixel, lpixel, inc, NULL, q, NULL, &status );

    for (ii = 1; ii <= nkeys; ii++)  {
        fits_read_record(fptr, ii, card2, &status);
        //printf("%s\n", card2);
    }
    printf("END\n\n");
    fits_close_file(fptr, &status);

    if (status)
        fits_report_error(stderr, status);

    delete [] card2;


}
*/


/*
#include "opencv2/highgui/highgui.hpp"
#include <iostream>

using namespace std;
using namespace cv;

int main()
{
    // Read image from file
    // Make sure that the image is in grayscale
    Mat img = imread("lena.JPG",0);

    Mat planes[] = {Mat_<float>(img), Mat::zeros(img.size(), CV_32F)};
    Mat complexI;    //Complex plane to contain the DFT coefficients {[0]-Real,[1]-Img}
    merge(planes, 2, complexI);
    dft(complexI, complexI);  // Applying DFT

    // Reconstructing original imae from the DFT coefficients
    Mat invDFT, invDFTcvt;
    idft(complexI, invDFT, DFT_SCALE | DFT_REAL_OUTPUT ); // Applying IDFT
    invDFT.convertTo(invDFTcvt, CV_8U);
    imshow("Output", invDFTcvt);

    //show the image
    imshow("Original Image", img);

    // Wait until user press some key
    waitKey(0);
    return 0;
}
 */

/*
    for (int y=5000; y<5020; y++) {
        for (int x = 5000; x < 5020; x++) {
            unsigned int offset_pos = (y * width) + x;
            cout << dataBuffer[offset_pos];
        }
        cout << endl;
    }
*/
