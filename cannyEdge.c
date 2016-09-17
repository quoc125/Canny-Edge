#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define  MAXMASK 100
#define MAX 256

void getPic();
void smooth();
void computePeak();
void threshold();
int Histogram();

void main(int argc,char** argv)
{
  //set up array
  int pic[MAX][MAX];
  double ival[MAX][MAX];
  double outpicx[MAX][MAX];
  double outpicy[MAX][MAX];
  double angle[MAX][MAX];
  double can[MAX][MAX];
  //define variable
  double sigma, percent;
  double high;
  int i, j;
  //open file
  FILE *fo1, *fopen();
  fo1 = fopen("canny.pgm", "wb");
  //get the argument from the user
  char *foobar;
  argc--; argv++;
  foobar = *argv;
  getPic(foobar, pic);
  argc--; argv++;
  foobar = *argv;
  sigma = atof(foobar);
  smooth(pic, sigma, ival, angle, outpicx, outpicy);
  computePeak(ival,outpicx, outpicy, angle,can);
  argc--; argv++;
  foobar = *argv;
  percent = atof(foobar);
  high  = Histogram(ival, percent);
  //print out the file
  fprintf(fo1, "P5\n256 256\n255\n");
  for (i=0;i<256;i++)
    {
     for (j=0;j<256;j++)
        {
        if (can[i][j] == 255)
        {
          if (high < ival[i][j])
            ival[i][j] = 255;
          else if (high*.35 > ival[i][j])
            ival[i][j] = 0;
           else
              threshold(i,j, -1, -1, high, ival);
        }
        else
          ival[i][j] = 0;

         fprintf(fo1,"%c",(char)((int)(ival[i][j])));
        }
      }
}
//get the picture
void getPic(char *file, int pic[MAX][MAX])
{
  int i, j;
  char throwaway[80];
  FILE *fp1,*fopen();
  fp1 = fopen(file,"rb");


  fgets(throwaway, 80, fp1);
  fgets(throwaway, 80, fp1);
  fgets(throwaway, 80, fp1);
  if ( !( (throwaway[0]=='2') && (throwaway[1]=='5') && (throwaway[2]=='5')))
      fgets(throwaway, 80, fp1);
  for (i=0;i<MAX;i++)
      {
        for (j=0;j<MAX;j++)
          {
            pic[i][j]  =  getc (fp1);
            pic[i][j]  &= 0377;
          }
      }
}
//filter out the noise and get the gradient  
void smooth(int pic[MAX][MAX],double sigma, double ival[MAX][MAX], double angle[MAX][MAX],
            double outpicx[MAX][MAX], double outpicy[MAX][MAX])
{
  int i, j, p, q;
  double sum1, sum2;
  int sig = sigma *3;
  FILE *fo1,*fopen();
  fo1 = fopen("cannyMag.pgm","wb");
  int center = MAXMASK/2;
  double max =0;
  double maskx[MAXMASK][MAXMASK];
  double masky[MAXMASK][MAXMASK];
  //create a x and y mask
  for (i = -sig; i <= sig; i++)
    {
      for (j = -sig; j <=sig; j++)
      {
        maskx[center+i][center+j]=j* exp(-(i*i+j*j)/(2*sigma*sigma));
        masky[center+i][center+j]=i* exp(-(i*i+j*j)/(2*sigma*sigma));
        
      }
    }
  //convolution
  for(i = sig; i< MAX-sig; i++)
    {
      for (j= sig; j< MAX-sig; j++)
        {
          sum1 = 0;
          sum2 = 0;
          for(p =-sig; p <= sig; p++)
            {
              for(q=-sig; q <=sig; q++)
                {
                  sum1 += pic[i+p][j+q] * maskx[p+center][q+center];
                  sum2 += pic[i+p][j+q] * masky[p+center][q+center];
                }
            }
            outpicx[i][j] = sum1;
            outpicy[i][j] = sum2;
        }
    }
  //find the magitude 
  for(i = 0; i< MAX; i++)
    {
      for (j= 0; j< MAX; j++)
        {
          ival[i][j] = sqrt(outpicx[i][j] * outpicx[i][j] + outpicy[i][j] * outpicy[i][j]);
          if (outpicx[i][j] == 0)
            outpicx[i][j] = 0.00000001;
          angle[i][j] = atan(outpicy[i][j]/outpicx[i][j]) *180/3.14159265;
          if (ival[i][j] > max)
            max = ival[i][j];
        }
    }
    //print out the picture
       fprintf(fo1, "P5\n256 256\n255\n");
      for (i=0;i<MAX;i++)
        { for (j=0;j<MAX;j++)
          {
          ival[i][j] = ival[i][j]/max *255;
          fprintf(fo1,"%c",(char)((int)(ival[i][j])));      
          }
        }
}
//find protential candidate for edge 
void computePeak(double ival[MAX][MAX], double outpicx[MAX][MAX], double outpicy[MAX][MAX], 
                double angle[MAX][MAX], double can[MAX][MAX])
{
  int i, j;
  double sum1, sum2;
  FILE *fo1,*fopen();
  fo1 = fopen("cannyPeak.pgm","wb");
  for(i = 1; i <MAX-1; i++)
   {
     for(j = 1; j < MAX-1; j++)
      {
        if((angle[i][j] > -22.5 && angle[i][j] <= 22.5) )
          {
              if((ival[i][j] > ival[i][j-1])&& (ival[i][j] > ival[i][j+1]))
              {
                    can[i][j] = 255;
              }
            
          }
        else if((angle[i][j] > 22.5 && angle[i][j] <= 67.5) )
        {
              if ((ival[i][j] > ival[i+1][j+1])&&(ival[i][j] > ival[i-1][j-1]))
                {
                  can[i][j] = 255;
                }
        }
      else if ((angle[i][j] > -67.5 && angle[i][j] <= -22.5))
        {
          if ((ival[i][j] > ival[i-1][j+1])&&(ival[i][j] > ival[i+1][j-1]))
             {
                can[i][j] = 255;
             }
        }
      else
      {
        if ((ival[i][j] > ival[i-1][j])&& (ival[i][j] > ival[i+1][j]))
           {
            can[i][j] = 255;
          }
       }
      }
   }
   fprintf(fo1, "P5\n256 256\n255\n");
   for (i=0;i<MAX;i++)
     { 
       for (j=0;j<MAX;j++)
        {
         fprintf(fo1,"%c",(char)((int)(can[i][j])));      
        }
     }
}
//find value that near that reach the threshold 
void threshold (int x, int y,int i, int j, double high, double ival[MAX][MAX])
{
  if (x+i == MAX || x+i >1)
    {
      ival[x][y] = 0;
      return;
    }
  if (y+j == MAX||x+i <0 || y+j >1)
    threshold(x,y,i+1, -1, high, ival);
  if (y+j <0)
    threshold(x,y,i, j+1, high, ival);
  if (ival[x+i][y+j] >=high)
    {
      ival[x][j] = 255;
      return;
    }
  else
    threshold(x,y,i, j+1, high, ival);
}
int Histogram(double ival[MAX][MAX],double percent)
{
  int hist[MAX];
  int i,j, sum = 0;
  int cutoff = percent * MAX * MAX;
  for (i = 0; i< MAX; i++)
    hist[i] = 0;
  for(i = 1; i< MAX-1; i++)
  {
    for(j = 1; j< MAX-1; j++)
      {
        hist[(int)ival[i][j]]++;
      }
  }
  for (i = 255; i> 0; i--)
    {
      sum += hist[i];
      if (sum >= cutoff)
        {
        return i;
        }
    }

}