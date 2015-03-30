#include <vector>
#include <cmath>
#include <limits>
#include <cstdlib> //for randomizing purposes
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include <iostream>

//We're getting back from the "higher performance code" to the "readable code" here

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);
Model *model = NULL;
const int width = 800; //pixels
const int height = 500; //pixels

void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color) //vector = (x, y) a.k.a. one dot
{
    bool steep = false; //steep = true if length < height
    if (std::abs(p0.x - p1.x)<std::abs(p0.y - p1.y)) //if the line is steep, we transpose the image
    {
        std::swap(p0.x, p0.y);
        std::swap(p1.x, p1.y);
        steep = true;
    }
    if (p0.x>p1.x) //to avoid the symmetry problem: line(x0,y0,x1,y1)!=line(x1,y1,x0,y0)
    {
        //make it left-to-right
        std::swap(p0, p1);
    }


    for (int x=p0.x; x<=p1.x; x++)
    {
        float t = (x-p0.x)/(float)(p1.x-p0.x); //again, this one is from the line equation
        int y = p0.y*(1.-t) + p1.y*t + .5;       //(y-y1)/(y2-y1) = (x-x1)/(x2-x1) = t
        /* the + .5 part was added in the 2d z-buffer learning version
            this seems to be-- pardon, this IS a hack to make an ordinary float -> int
            cast (which simply gets rid of the fractional component, i.e. 5.9 -> 5)
            more like normal Math.round thing, i.e. 5.9 -> 6 (5.9 + 0.5 = 6.8 -> 6)
        */
        if(steep)
        {
            image.set(y, x,  color); //if transposed, de-transpose
        }
        else
        {
            image.set(x, y,  color);
        }


    }
}


void rasterize(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color, int ybuffer[])
{
    if (p0.x>p1.x){
        std::swap(p0, p1);//rendering from left to right
    }
    for (int x=p0.x; x<=p1.x; x++){
        float t = (x-p0.x)/(float)(p1.x-p0.x);
        int y = p0.y*(1.-t) + p1.y*t + .5;//going through our scene; if there is an object
        if (ybuffer[x]<y){ //closer to the camera than that written in the buffer
            ybuffer[x] = y;//then we put the new y-coordinate to the buffer
            image.set(x,0,color);//and draw the pixel
        }
    }

}


int main(int argc, char** argv)
{

//the 2d scene which we'll have to render aka project to the "screen"
    TGAImage scene (width, height, TGAImage::RGB);

    line(Vec2i(20, 34), Vec2i(744, 400), scene, red);
    line(Vec2i(120, 434), Vec2i(444, 400), scene, green);
    line(Vec2i(330, 463), Vec2i(594, 200), scene, blue);

//the "screen" line

    line(Vec2i(10, 10), Vec2i(790, 10), scene, white);
    scene.flip_vertically(); // we want to have the origin at the left bottom corner of the image
    scene.write_tga_file("scene.tga");





    TGAImage render(width, 16, TGAImage::RGB);
    int ybuffer[width];
    for (int i=0; i<width;i++){
        ybuffer[i] = std::numeric_limits<int>::min();//we initialize the ybuffer with minus infinity
    }
    rasterize(Vec2i(20,34), Vec2i(744,400), render, red, ybuffer);
    rasterize(Vec2i(120,434), Vec2i(444,400), render, green, ybuffer);
    rasterize(Vec2i(330,463), Vec2i(594,200), render, blue, ybuffer);

    //since one-pixel-wide image is hard on the eyes, we've made it 16px
    for(int i=0; i<width; i++){
        for(int j=1;j<16;j++){//we start from 1 since we've already put the pixel in j=0 with the rasterize function; we're just expanding the pic here
            render.set(i, j, render.get(i, 0));
        }
    }
    render.flip_vertically();
    render.write_tga_file("render.tga");

    //  image.flip_vertically(); // we want to have the origin at the left bottom corner of the image
    //image.write_tga_file("output.tga");
    //  delete model; //get out from the memory, you sneaky thing
    return 0;
}
