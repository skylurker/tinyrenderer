#include "tgaimage.h"
#include <cmath>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color)
{
    bool steep = false; //steep = true if length < height
    if (std::abs(x0-x1)<std::abs(y0-y1)) //if the line is sttep, we transpose the image
    {
        std::swap(x0,y0);
        std::swap(x1,y1);
        steep = true;
    }
    if (x0>x1) //to avoid the symmetry problem: line(x0,y0,x1,y1)!=line(x1,y1,x0,y0)
    {
        //make it left-to-right
        std::swap(x0,x1);
        std::swap(y0,y1);
    }

    int dx = x1-x0;
    int dy = y1-y0;
    int derror2 = std::abs(dy)*2;
   // float error = 0; //from our pixel to the perfect line
   int error2 = 0;
    int y = y0;
    for (int x=x0; x<=x1; x++)
    {

        if(steep)
        {
            image.set(y, x,  TGAColor(255, 1)); //if transposed, de-transpose
        }
        else
        {
            image.set(x, y,  TGAColor(255, 1));
        }
        error2 += derror2;

        if(error2>dx){
            y += (y1>y0?1:-1);
            error2 -= dx*2;
        }

    }
}


int main(int argc, char** argv)
{
    TGAImage image(100, 100, TGAImage::RGB);
    //image.set(52, 41, red); //a red dot!
    for(int i=0;i<1000000;i++){
         line(20, 13, 40, 80, image, red);
    }

    image.flip_vertically(); // we want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    return 0;
}
