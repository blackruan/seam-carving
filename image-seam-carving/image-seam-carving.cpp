#include <iostream>
#include <fstream>
#include <cstring>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

// Maximum rows and cols of the image that can be dealt with
#define MAXR 1500
#define MAXC 1500

// Gray image and energy map of the image
Mat gray,energy;
// dp matrix for seam calculation
int dp[MAXR][MAXC];
// Store the direction of each cell
int dir[MAXR][MAXC];

void usage();
int get(Mat I, int x, int y);
Vec3b average(Vec3b x, Vec3b y);
Mat calculate_energy(Mat I);
void add_vertical(Mat &I, int Xd);
void add_horizontal(Mat &I, int Yd);
void remove_vertical(Mat &I, int Xd);
void remove_horizontal(Mat &I, int Yd);

int main(int argc, char* argv[])
{
    if(argc != 4)
    {
      usage();
      return -1;
    }

    string file = argv[1];
    Mat_<Vec3b> img = imread(file);

    if(!img.data)
    {
        cout << "Unable to open image file." << endl;
        return -1;
    }

    int orig_h = img.rows, orig_w = img.cols;
    int desired_h = atoi(argv[2]);
    int desired_w = atoi(argv[3]);

    Mat_<Vec3b> dupImg = img.clone();
    if(desired_h <= orig_h)
        remove_horizontal(dupImg, desired_h);
    else
        add_horizontal(dupImg, desired_h);

    if(desired_w <= orig_w)
        remove_vertical(dupImg, desired_w);
    else
        add_vertical(dupImg, desired_w);

    imwrite("carved-" + file, dupImg);

    return 0;
}


void usage()
{
    cout << "usage: image-seam-carving <image> <desired height> <desired width> " << endl;
}


int get(Mat I, int x, int y) 
{
    return (int)I.at<uchar>(y, x);
}


Vec3b average(Vec3b x, Vec3b y)
{
    Vec3b ret;
    
    for(int i = 0; i < 3; ++i)
        ret.val[i] = (x.val[i] + y.val[i]) / 2;
    
    return ret;
}

/*
 * Calculate the energy map of input I.
 */
Mat calculate_energy(Mat I){
    int Y = I.rows,X = I.cols;
    Mat energy = Mat(Y, X, CV_32S);
    
    for(int x = 0;x < X;++x){
        for(int y = 0;y < Y;++y){
            int val = 0;
            
            if(x > 0 && x + 1 < X)
                val += abs((int)I.at<uchar>(y,x + 1) - (int)I.at<uchar>(y,x - 1));
            else if(x > 0)
                val += 2 * abs((int)I.at<uchar>(y,x) - (int)I.at<uchar>(y,x - 1));
            else
                val += 2 * abs((int)I.at<uchar>(y,x + 1) - (int)I.at<uchar>(y,x));
            
            if(y > 0 && y + 1 < Y)
                val += abs((int)I.at<uchar>(y + 1,x) - (int)I.at<uchar>(y - 1,x));
            else if(y > 0)
                val += 2 * abs((int)I.at<uchar>(y,x) - (int)I.at<uchar>(y - 1,x));
            else
                val += 2 * abs((int)I.at<uchar>(y + 1,x) - (int)I.at<uchar>(y,x));
            
            energy.at<int>(y,x) = val;
        }
    }
    
    return energy;
}

/*
 * Remove X0 - Xd vertical seams from the input I, where X0 is the original size and 
 * Xd is the size desired.
 */
void remove_vertical(Mat &I, int Xd){
    int X0 = I.cols;
    int X = X0, Y = I.rows;
    
    for(int k = 0; k < X0 - Xd; ++k){
        cvtColor(I, gray, CV_BGR2GRAY);
        energy = calculate_energy(gray);
        
        for(int x = 0; x < X; ++x)
            dp[x][0] = energy.at<int>(0, x);
        
        for(int y = 1; y < Y; ++y){
            for(int x = 0; x < X; ++x){
                int val = energy.at<int>(y,x);
                dp[x][y] = -1;
                
                if(x > 0 && (dp[x][y] == -1 || val + dp[x - 1][y - 1] < dp[x][y])){
                    dp[x][y] = val + dp[x - 1][y - 1];
                    dir[x][y] = -1;
                }
                
                if(dp[x][y] == -1 || val + dp[x][y - 1] < dp[x][y]){
                    dp[x][y] = val + dp[x][y - 1];
                    dir[x][y] = 0;
                }
                
                if(x + 1 < X && (dp[x][y] == -1 || val + dp[x + 1][y - 1] < dp[x][y])){
                    dp[x][y] = val + dp[x + 1][y - 1];
                    dir[x][y] = 1;
                }
            }
        }
        
        int best = dp[0][Y - 1];
        int cur = 0;
        
        for(int x = 0; x < X; ++x){
            if(dp[x][Y - 1] < best){
                best = dp[x][Y - 1];
                cur = x;
            }
        }
        
        Mat_<Vec3b> tmp(Y,X - 1);
        
        for(int y = Y - 1; y >= 0; --y){
            for(int i = 0; i < X; ++i){
                if(i < cur){
                    tmp.at<Vec3b>(y,i) = I.at<Vec3b>(y,i);
                } else if(i > cur){
                    tmp.at<Vec3b>(y,i - 1) = I.at<Vec3b>(y,i);
                }
            }
            
            if(y > 0)
                cur = cur + dir[cur][y];
        }
        
        I = tmp;
        --X;
    }
}


/*
 * Remove horizontal seams can be achieved by that first transpose the matrix, 
 * then remove the vertical seams.
 */
void remove_horizontal(Mat &I, int Yd){
    I = I.t();
    remove_vertical(I, Yd);
    I = I.t();
}


/*
 * Add Xd - X0 vertical seams to the input I, where X0 is the original size and 
 * Xd is the size desired.
 */
void add_vertical(Mat &I, int Xd){
    Mat I0 = I;
    int X0 = I.cols;
    int X = X0,Y = I.rows;
    bool mark[Y][X];
    int pos[X][Y];
    
    memset(mark, false, sizeof mark);
    
    for(int i = 0; i < X; ++i)
        for(int j = 0; j < Y; ++j)
            pos[i][j] = i;
    
    for(int k = 0; k < Xd - X0; ++k){
        cvtColor(I, gray, CV_BGR2GRAY);
        energy = calculate_energy(gray);
        
        for(int x = 0; x < X; ++x)
            dp[x][0] = energy.at<int>(0,x);
        
        for(int y = 1; y < Y; ++y){
            for(int x = 0; x < X; ++x){
                int val = energy.at<int>(y,x);
                dp[x][y] = -1;
                
                if(x > 0 && (dp[x][y] == -1 || val + dp[x - 1][y - 1] < dp[x][y])){
                    dp[x][y] = val + dp[x - 1][y - 1];
                    dir[x][y] = -1;
                }
                
                if(dp[x][y] == -1 || val + dp[x][y - 1] < dp[x][y]){
                    dp[x][y] = val + dp[x][y - 1];
                    dir[x][y] = 0;
                }
                
                if(x + 1 < X && (dp[x][y] == -1 || val + dp[x + 1][y - 1] < dp[x][y])){
                    dp[x][y] = val + dp[x + 1][y - 1];
                    dir[x][y] = 1;
                }
            }
        }
        
        int best = dp[0][Y - 1];
        int cur = 0;
        
        for(int x = 0; x < X; ++x){
            if(dp[x][Y - 1] < best){
                best = dp[x][Y - 1];
                cur = x;
            }
        }
        
        Mat_<Vec3b> tmp(Y,X - 1);
        
        for(int y = Y - 1; y >= 0; --y){
            for(int i = 0; i < X; ++i){
                if(i < cur){
                    tmp.at<Vec3b>(y,i) = I.at<Vec3b>(y,i);
                }else if(i > cur){
                    tmp.at<Vec3b>(y,i - 1) = I.at<Vec3b>(y,i);
                    pos[i - 1][y] = pos[i][y];
                }else{
                    mark[y][ pos[i][y] ] = true;
                }
            }
            
            if(y > 0)
                cur = cur + dir[cur][y];
        }
        I = tmp;
        --X;
    }
    
    Mat_<Vec3b> tmp(Y,Xd);
    
    for(int i = 0; i < Y; ++i){
        int cont = 0;
        
        for(int j = 0; j < X0; ++j){
            if(mark[i][j]){
                Vec3b aux;
                
                if(j == 0) aux = average(I0.at<Vec3b>(i,j),I0.at<Vec3b>(i,j + 1));
                else if(j == X0 - 1) aux = average(I0.at<Vec3b>(i,j),I0.at<Vec3b>(i,j - 1));
                else aux = average(I0.at<Vec3b>(i,j - 1),I0.at<Vec3b>(i,j + 1));
                
                tmp.at<Vec3b>(i,cont) = aux; cont++;
                tmp.at<Vec3b>(i,cont) = aux; cont++;
            }else{
                tmp.at<Vec3b>(i,cont) = I0.at<Vec3b>(i,j);
                cont++;
            }
        }
    }
    
    I = tmp;
}


/*
 * Remove horizontal seams can be achieved by that first transpose the matrix, 
 * then remove the vertical seams.
 */
void add_horizontal(Mat &I, int YF){
    I = I.t();
    add_vertical(I, YF);
    I = I.t();
}
