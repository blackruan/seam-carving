#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <cmath>
#include <string>
#include <cstring>

#include <pthread.h>

#include "graph.h"

#include <opencv/cv.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

pthread_t *workers;
pthread_mutex_t mutexLock;

int ver = 1;
int hor = 0;
int frameIter= 0;
int maxFrames = 0;
Mat *frames;
Mat *outFrames;

void usage();
void *reduce(void *);
Mat remove_seam(Mat image, int seam[]);
Mat remove_seam_gray(Mat GrayImage, int seam[]);
int *find_seam(Mat grayImg1, Mat grayImg2, Mat grayImg3, Mat grayImg4, Mat grayImg5);
Mat reduce_vertical(Mat &grayImg1, Mat &grayImg2,Mat &grayImg3, Mat &grayImg4, Mat &grayImg5, Mat img);
Mat reduce_horizontal(Mat &grayImg1, Mat &grayImg2,Mat &grayImg3, Mat &grayImg4, Mat &grayImg5, Mat img);
Mat reduce_frame(Mat frame1, Mat frame2, Mat frame3, Mat frame4, Mat frame5, int v, int h);

int main(int argc, char* argv[]) {
    VideoCapture cap;
    VideoWriter output;
    string inFile = "golf.mov";
    
    pthread_mutex_init(&mutexLock, NULL);
    int numWorkers = 1;

    if(argc == 5) {
        inFile = argv[1];
        ver = atoi(argv[2]);
        hor = atoi(argv[3]);
        numWorkers = atoi(argv[4]);
    }
    else {
        usage();
        return -1;
    }

    if(numWorkers == 0)
        numWorkers = 1;
    
    cout << "Using " << numWorkers << " workers." << endl;

    cap.open(inFile);
    if(!cap.isOpened()) {
        cerr << "Unable to open input file." << endl;
        return -1;
    }
    
    maxFrames = cap.get(CV_CAP_PROP_FRAME_COUNT);
    int origWid = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    int origHei = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
    
    Size S = Size((int)cap.get(CV_CAP_PROP_FRAME_WIDTH) -ver , (int)cap.get(CV_CAP_PROP_FRAME_HEIGHT)-hor);

    string::size_type pAt = inFile.find_last_of('.');   // Find extension point
    const string outFile = inFile.substr(0, pAt) + "-result.mov";
    output.open(outFile, CV_FOURCC('m','p','4','v'), cap.get(CV_CAP_PROP_FPS), S, true);

    frames = new Mat[maxFrames];
    outFrames = new Mat[maxFrames];

    for(int i = 0; i < maxFrames; ++i) {
        cap >> frames[i];
        if(frames[i].empty()) {
            cout << "Error: unable to read frame " << i << endl;
            return 1;
        }
    }

    workers = (pthread_t *)malloc(sizeof(pthread_t)*numWorkers);
    for(int i = 0; i < numWorkers; ++i) {
        if(pthread_create(&workers[i], NULL, reduce, NULL)) {
            cout << "Error creating thread\n" << endl;
            return 1;
        }
    }
    for(int i = 0; i < numWorkers; ++i) {
        if(pthread_join(workers[i], NULL)) {
            cout << "Error joining thread\n" << endl;
            return 1;
        }
    }
    
    for(int i = 0; i < maxFrames; ++i) {
        output<<outFrames[i];
    }

    return 0;
}


void usage() 
{
    cout << "Usage: heuristic5 <filename> <vertical cuts> <horizontal cuts> <# of workers>" << endl;
}


/*
 * Entrance function of the worker thread.
 */
void *reduce(void *) 
{
    Mat frame1, frame2, frame3, frame4, frame5;
    while(1) {
        int frameId;
        pthread_mutex_lock(&mutexLock);
        if(frameIter > maxFrames - 1) {
            pthread_mutex_unlock(&mutexLock);
            return NULL;
        }
        frameId = frameIter++;
        cout << "Frame " << frameIter << "/" << maxFrames << endl;
        
        pthread_mutex_unlock(&mutexLock);
        
        frame1 = frames[frameId];
        
        // check if we are close to the end of the video and
        // select frames appropriately
        if(frameId < maxFrames - 4) {
            frame2 = frames[frameId+1];
            frame3 = frames[frameId+2];
            frame4 = frames[frameId+3];
            frame5 = frames[frameId+4];
        }
        else if(frameId < maxFrames - 3) {
            frame2 = frames[frameId+1];
            frame3 = frames[frameId+2];
            frame4 = frames[frameId+3];
            frame5 = frame4;
        }
        else if(frameId < maxFrames - 2) {
            frame2 = frames[frameId+1];
            frame3 = frames[frameId+2];
            frame4 = frame3;
            frame5 = frame3;
        }
        else if(frameId < maxFrames - 1) {
            frame2 = frames[frameId+1];
            frame3 = frame2;
            frame4 = frame2;
            frame5 = frame2;
        }
        else {
            frame2 = frame1;
            frame3 = frame1;
            frame4 = frame1;
            frame5 = frame1;
        }
        outFrames[frameId] = reduce_frame(frame1, frame2, frame3, frame4, frame5, ver, hor);
    }
}


/*
 * Remove seam from the image.
 *
 * image: image to be seam-removed
 * seam: column index of each row
 */
Mat remove_seam(Mat image, int seam[])
{
    int nrows = image.rows;
    int ncols = image.cols;
    Mat reducedImage(nrows,ncols-1,CV_8UC3);
    
    for(int i=0; i<nrows; i++)
    {
        if(seam[i] != 0)
            image.row(i).colRange(Range(0,seam[i])).copyTo(reducedImage.row(i).colRange(Range(0,seam[i])));
        if(seam[i] != ncols-1)
            image.row(i).colRange(Range(seam[i]+1, ncols)).copyTo(reducedImage.row(i).colRange(Range(seam[i],ncols-1)));
    }
    return reducedImage;
}


/*
 * Remove seam from the gray image.
 *
 * GrayImage: gray image to be seam-removed
 * seam: column index of each row
 */
Mat remove_seam_gray(Mat GrayImage, int seam[])
{
    int nrows = GrayImage.rows;
    int ncols = GrayImage.cols;
    Mat reducedImage(nrows,ncols-1,CV_8UC1);
    for(int i=0; i<nrows; i++)
    {
        if(seam[i] != 0)
            GrayImage.row(i).colRange(Range(0,seam[i])).copyTo(reducedImage.row(i).colRange(Range(0,seam[i])));
        if(seam[i] != ncols-1)
            GrayImage.row(i).colRange(Range(seam[i]+1, ncols)).copyTo(reducedImage.row(i).colRange(Range(seam[i],ncols-1)));
    }
    return reducedImage;
}


/*
 * Find the seam of minimum energy according to our improved graph cut algorithm
 *
 * grayImg1: current image needed to be seam-removed
 * grayImg2, grayImg3, grayImg4, grayImg5: used to calculate the look ahead energy
 */
int *find_seam(Mat grayImg1, Mat grayImg2, Mat grayImg3, Mat grayImg4, Mat grayImg5)
{
    // define variables
    typedef Graph<int,int,int> GraphType;
    int rows = grayImg1.rows;
    int cols = grayImg1.cols;
    double inf = 100000;
    int *Seam = new int[rows];
    float a1= 0.2, a2= 0.2, a3= 0.2, a4= 0.2, a5= 0.2;
    GraphType *g = new GraphType(/*estimated # of nodes*/ rows*cols, /*estimated # of edges*/ ((rows-1)*cols + (cols-1)*rows + 2*(rows-1)*(cols-1)));
    
    int LR,LR1,LR2,LR3,LR4,LR5;
    int posLU,posLU1,posLU2,posLU3,posLU4,posLU5;
    int negLU,negLU1,negLU2,negLU3,negLU4,negLU5;
    for (int i = 1; i<=rows*cols; i++) {
        g -> add_node();
    }
    
    for(int i=0; i<rows; i++) {
        for(int j=0; j<cols; j++) {
            if(j==0) {
                g -> add_tweights( i*cols,   /* capacities */  inf,0 );
            }
            else if(j==cols-1) {
                g -> add_tweights( ((i+1)*cols) -1,   /* capacities */ 0, inf );
            }
            
            if(j==0) {
                LR1= grayImg1.at<unsigned char>(i,j+1);
                LR2= grayImg2.at<unsigned char>(i,j+1);
                LR3= grayImg3.at<unsigned char>(i,j+1);
                LR4= grayImg4.at<unsigned char>(i,j+1);
                LR5= grayImg5.at<unsigned char>(i,j+1);
                LR= a1*LR1 + a2*LR2 + a3*LR3 + a4*LR4 + a5*LR5;
                g -> add_edge( i*cols, i*cols+1,    /* capacities */ LR, inf );
            }
            else if(j!=cols-1) {
                LR1= abs(grayImg1.at<unsigned char>(i,j+1)-grayImg1.at<unsigned char>(i,j-1));
                LR2= abs(grayImg2.at<unsigned char>(i,j+1)-grayImg2.at<unsigned char>(i,j-1));
                LR3= abs(grayImg3.at<unsigned char>(i,j+1)-grayImg3.at<unsigned char>(i,j-1));
                LR4= abs(grayImg4.at<unsigned char>(i,j+1)-grayImg4.at<unsigned char>(i,j-1));
                LR5= abs(grayImg5.at<unsigned char>(i,j+1)-grayImg5.at<unsigned char>(i,j-1));
                LR= a1*LR1 + a2*LR2 + a3*LR3 + a4*LR4 + a5*LR5;
                g -> add_edge( i*cols + j, i*cols + j +1, LR, inf );
            }
            
            if(i!=rows-1) {
                if(j==0) {
                    // positive LU
                    posLU1= grayImg1.at<unsigned char>(i,j);
                    posLU2= grayImg2.at<unsigned char>(i,j);
                    posLU3= grayImg3.at<unsigned char>(i,j);
                    posLU4= grayImg4.at<unsigned char>(i,j);
                    posLU5= grayImg5.at<unsigned char>(i,j);
                    posLU= a1*posLU1 + a2*posLU2 + a3*posLU3 + a4*posLU4 + a5*posLU5;
                    // negative LU
                    negLU1= grayImg1.at<unsigned char>(i+1,j);
                    negLU2= grayImg2.at<unsigned char>(i+1,j);
                    negLU3= grayImg3.at<unsigned char>(i+1,j);
                    negLU4= grayImg4.at<unsigned char>(i+1,j);
                    negLU5= grayImg5.at<unsigned char>(i+1,j);
                    negLU= a1*negLU1 + a2*negLU2 + a3*negLU3 + a4*negLU4 + a5*negLU5;
                    g -> add_edge( i*cols + j, i*cols + j +1, negLU, posLU );
                }
                else {
                    // positive LU
                    posLU1= abs(grayImg1.at<unsigned char>(i,j)-grayImg1.at<unsigned char>(i+1,j-1));
                    posLU2= abs(grayImg2.at<unsigned char>(i,j)-grayImg2.at<unsigned char>(i+1,j-1));
                    posLU3= abs(grayImg3.at<unsigned char>(i,j)-grayImg3.at<unsigned char>(i+1,j-1));
                    posLU4= abs(grayImg4.at<unsigned char>(i,j)-grayImg4.at<unsigned char>(i+1,j-1));
                    posLU5= abs(grayImg5.at<unsigned char>(i,j)-grayImg5.at<unsigned char>(i+1,j-1));
                    posLU= a1*posLU1 + a2*posLU2 + a3*posLU3 + a4*posLU4 + a5*posLU5;
                    // negative LU
                    negLU1= abs(grayImg1.at<unsigned char>(i+1,j)-grayImg1.at<unsigned char>(i,j-1));
                    negLU2= abs(grayImg2.at<unsigned char>(i+1,j)-grayImg2.at<unsigned char>(i,j-1));
                    negLU3= abs(grayImg3.at<unsigned char>(i+1,j)-grayImg3.at<unsigned char>(i,j-1));
                    negLU4= abs(grayImg4.at<unsigned char>(i+1,j)-grayImg4.at<unsigned char>(i,j-1));
                    negLU5= abs(grayImg5.at<unsigned char>(i+1,j)-grayImg5.at<unsigned char>(i,j-1));
                    negLU= a1*negLU1 + a2*negLU2 + a3*negLU3 + a4*negLU4 + a5*negLU5;
                    g -> add_edge( i*cols + j, i*cols + j +1, negLU, posLU );
                }
            }
            if(i!=0 && j!=0)
            {
                g -> add_edge( i*cols + j, (i-1)*cols + j-1, inf, 0 );
            }
            if(i!=rows-1 && j!=0)
            {
                g -> add_edge( i*cols + j, (i+1)*cols + j-1, inf, 0 );
            }
        }
    }
    
    int flow = g -> maxflow();
    for(int i=0; i<rows; i++) {
        for(int j=0;j<cols; j++) {
            if(g->what_segment(i*cols+j) == GraphType::SINK) {
                Seam[i] = j-1;
                break;
            }
            if(j==cols-1 && g->what_segment(i*cols+j) == GraphType::SOURCE) {
                Seam[i] = cols-1;
            }
        }
    }
    delete g;
    return Seam;
}


/*
 * Remove one vertical seam from img.
 * 
 * grayImg1-grayImg5: used to calculate the seam
 * img: image needed to perform seam-cut
 */
Mat reduce_vertical(Mat &grayImg1, Mat &grayImg2,Mat &grayImg3, Mat &grayImg4, Mat &grayImg5, Mat img) {
    int rows = grayImg1.rows;
    int *seam = new int[rows];
    seam = find_seam(grayImg1, grayImg2, grayImg3, grayImg4, grayImg5);
    Mat returnImg = remove_seam(img, seam);
    grayImg1 = remove_seam_gray(grayImg1, seam);
    grayImg2 = remove_seam_gray(grayImg2, seam);
    grayImg3 = remove_seam_gray(grayImg3, seam);
    grayImg4 = remove_seam_gray(grayImg4, seam);
    grayImg5 = remove_seam_gray(grayImg5, seam);
    return returnImg;
}


/*
 * Remove one horizontal seam from img.
 *
 * grayImg1-grayImg5: used to calculate the seam
 * img: image needed to perform seam-cut
 */
Mat reduce_horizontal(Mat &grayImg1, Mat &grayImg2,Mat &grayImg3, Mat &grayImg4, Mat &grayImg5, Mat img) {
    int rows = grayImg1.rows;
    int *seam = new int[rows];
    seam = find_seam(grayImg1.t(), grayImg2.t(),grayImg3.t(),grayImg4.t(),grayImg5.t());
    Mat returnImg = remove_seam(img, seam);
    Mat grayImg1temp = remove_seam_gray(grayImg1.t(), seam);
    Mat grayImg2temp = remove_seam_gray(grayImg2.t(), seam);
    Mat grayImg3temp = remove_seam_gray(grayImg3.t(), seam);
    Mat grayImg4temp = remove_seam_gray(grayImg4.t(), seam);
    Mat grayImg5temp = remove_seam_gray(grayImg5.t(), seam);
    grayImg1 = grayImg1temp.t();
    grayImg2 = grayImg2temp.t();
    grayImg3 = grayImg3temp.t();
    grayImg4 = grayImg4temp.t();
    grayImg5 = grayImg5temp.t();
    return returnImg.t();
}

/*
 * Seam-cut to the frame1 according to the next 4 frames and itself.
 *
 * frame1: the frame to be seam-cut
 * frame2, frame3, frame4, frame5: used to calculate the look ahead energy
 * v: number of vertical seams to be removed
 * h: number of horizontal seams to be removed
 */
Mat reduce_frame(Mat frame1, Mat frame2, Mat frame3, Mat frame4, Mat frame5, int v, int h) {
    Mat grayImg1, grayImg2, grayImg3, grayImg4, grayImg5;
    cvtColor(frame1,grayImg1, CV_RGB2GRAY);
    cvtColor(frame2,grayImg2, CV_RGB2GRAY);
    cvtColor(frame3,grayImg3, CV_RGB2GRAY);
    cvtColor(frame4,grayImg4, CV_RGB2GRAY);
    cvtColor(frame5,grayImg5, CV_RGB2GRAY);

    int min = 0, diff = 0;
    Mat reducedGrayImg1,reducedGrayImg2,reducedGrayImg3,reducedGrayImg4,reducedGrayImg5,reducedImg;
    
    grayImg1.copyTo(reducedGrayImg1);
    grayImg2.copyTo(reducedGrayImg2);
    grayImg3.copyTo(reducedGrayImg3);
    grayImg4.copyTo(reducedGrayImg4);
    grayImg5.copyTo(reducedGrayImg5);
    frame1.copyTo(reducedImg);
    
    if(h > v) {
        diff = h - v;
        min = v;
    } else {
        diff = v - h;
        min = h;
    }
    
    for(int i = 0; i < min; ++i) {
        reducedImg= reduce_vertical(reducedGrayImg1,reducedGrayImg2,reducedGrayImg3,
                                   reducedGrayImg4,reducedGrayImg5,reducedImg);
        reducedImg= reduce_horizontal(reducedGrayImg1,reducedGrayImg2,reducedGrayImg3,
                                     reducedGrayImg4,reducedGrayImg5,reducedImg.t());
    }
    
    if(h > v) {
        for(int i = 0; i < diff; ++i) {
            reducedImg= reduce_horizontal(reducedGrayImg1,reducedGrayImg2,reducedGrayImg3,
                                         reducedGrayImg4,reducedGrayImg5,reducedImg.t());
        }
    } else {
        for(int i = 0; i < diff; ++i) {
            reducedImg= reduce_vertical(reducedGrayImg1,reducedGrayImg2,reducedGrayImg3,
                                       reducedGrayImg4,reducedGrayImg5,reducedImg);
        }
    }
    return reducedImg;
}

