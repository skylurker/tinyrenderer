#include <vector>
#include <cmath>
#include <limits>
//#include <cstdlib> //for randomizing purposes
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include <iostream>
//DON'T FORGET to add the new cpp and h files to the project or you'll get the linker error
//smth like "undefined reference"


//We're getting back from the "higher performance code" to the "readable code" here

//const TGAColor white = TGAColor(255, 255, 255, 255);
//const TGAColor red   = TGAColor(255, 0,   0,   255);
//const TGAColor green = TGAColor(0, 255, 0, 255);
//const TGAColor blue = TGAColor(0, 0, 255, 255);
const int width = 800; //pixels
//const int height = 500; //pixels LOL FORGOT TO CHANGE BACK TO 800 (the result looks pretty funny btw)
const int height = 800; //pixels
const int depth = 255;

Model *model = NULL;
Vec3f light_dir(0,0,-1);
int *zbuffer = NULL;
/*
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
        // the + .5 part was added in the 2d z-buffer learning version
       //     this seems to be-- pardon, this IS a hack to make an ordinary float -> int
        //    cast (which simply gets rid of the fractional component, i.e. 5.9 -> 5)
         //   more like normal Math.round thing, i.e. 5.9 -> 6 (5.9 + 0.5 = 6.8 -> 6)

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

*/

void triangle(Vec3i t0, Vec3i t1, Vec3i t2, Vec2i uv0, Vec2i uv1, Vec2i uv2, TGAImage &image, float intensity, int *zbuffer)
{
    if (t0.y==t1.y && t0.y==t2.y) return; //if it is a single dot, get outta here
    //sorting the vertices by y coordinate (t0 lower, t2 upper), bubblesort
    if (t0.y>t1.y) {std::swap(t0,t1); std::swap(uv0, uv1);}
    if (t0.y>t2.y) {std::swap(t0,t2); std::swap(uv0, uv2);}
    if (t1.y>t2.y) {std::swap(t1,t2); std::swap(uv1, uv2);}


    int total_height = t2.y-t0.y; //of the whole triangle

    for (int i=0; i<total_height; i++)
    {
        //kinda local coordinate system: t0.y is treated as 0 (i=0) here

        //if we've reached the upper half OR if the t1t0 rib is horizontal
        bool second_half = i>t1.y-t0.y || t1.y==t0.y;
        //segment_height is calculated depending on whether it is an upper part or the lower
        int segment_height = second_half ? t2.y-t1.y : t1.y-t0.y;
        /*Далее - коэффициенты подобия треугольников. Большой треугольник для beta
        образован точками t0, t1 и перпендикуляром от неё к горизонтали t0. Малый -
        t0, B (текущей точкой растеризации) и перпендикуляром от неё к горизонтали t0.*/
        float alpha = (float)i/total_height;
        float beta = (float)(i-(second_half ? t1.y-t0.y : 0))/segment_height;
        /*There will be no division by zero here.
        If segment_height of the lower part is gonna be 0 (t1.y==t0.y)
        then second_half==true, and segment_height = t2.y-t1.y!       */
        /*Для определения текущей ординаты: точка отсчёта - t0; расстояние до текущей
         ординаты есть горизонтальная сторона малого треугольника, определяется как
         произведение горизонтальной стороны большого (t1-t0) и коэф. подобия */
        Vec3i A = t0 + Vec3f(t2-t0)*alpha;
        Vec3i B = second_half ? t1 + Vec3f(t2-t1)*beta : t0 + Vec3f(t1-t0)*beta;
        Vec2i uvA = uv0 + (uv2-uv0)*alpha;
        Vec2i uvB = second_half ? uv1 + (uv2-uv1)*beta : uv0 + (uv1-uv0)*beta;
        if (A.x>B.x) {std::swap(A, B); std::swap(uvA, uvB);}//if point A is to the right of point B
        for (int j=A.x; j<=B.x; j++)
        {//we're interpolating the z coordinate here
            //see http://habrahabr.ru/post/248179/#comment_8256309
            //that's linear interpolation http://en.wikipedia.org/wiki/Linear_interpolation
            float phi = B.x==A.x ? 1. : (float)(j-A.x)/(float)(B.x-A.x);
            Vec3i P = Vec3f(A) + Vec3f(B-A)*phi; //kindly reminder that i means integer and f means float
            Vec2i uvP = uvA + (uvB-uvA)*phi;
            int idx = P.x+P.y*width;
            if (zbuffer[idx]<P.z){
                zbuffer[idx] = P.z;
                TGAColor color = model->diffuse(uvP);
                image.set(P.x, P.y, TGAColor(color.r*intensity, color.g*intensity, color.b*intensity));
            }

        }

    }




}



/*
void rasterize(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color, int ybuffer[])
{
    if (p0.x>p1.x)
    {
        std::swap(p0, p1);//rendering from left to right
    }
    for (int x=p0.x; x<=p1.x; x++)
    {
        float t = (x-p0.x)/(float)(p1.x-p0.x);
        int y = p0.y*(1.-t) + p1.y*t + .5;//going through our scene; if there is an object
        if (ybuffer[x]<y)  //closer to the camera than that written in the buffer
        {
            ybuffer[x] = y;//then we put the new y-coordinate to the buffer
            image.set(x,0,color);//and draw the pixel
        }
    }

}
*/

int main(int argc, char** argv)
{
    if (2==argc)
    {
        model = new Model(argv[1]); //if there is an argument which specifies the model we use
    }
    else
    {
        model = new Model("obj/african_head.obj"); //by default
    }




    zbuffer = new int[width*height];
    //HOWTO CONVERT
    //from two to one:
    //int idx = x + y*width;
    //from one to two:
    //int x = idx % width;
    //int y = idx / width;
    for (int i=0; i<width*height; i++)
    {
        zbuffer[i] = std::numeric_limits<int>::min();//initializing with minus infinity
    }


    {
        //drawing the model here

        TGAImage image(width, height, TGAImage::RGB);

        for (int i=0; i<model->nfaces(); i++)
        {
            std::vector<int> face = model->face(i); //face = polygon
            Vec3i screen_coords[3];
            Vec3f world_coords[3];
            for (int j=0; j<3; j++)
            {
                Vec3f v = model->vert(face[j]); //every face consists of three numbers, each of which is an index of a vertex
                screen_coords[j] = Vec3i((v.x+1.)*width/2., (v.y+1.)*height/2., (v.z+1.)*depth/2.);
                // (coordinate + 1)*width/2 is due to the range of the values in obj file, which is [-1, 1]
                /*Basically, world_coords are constant and exist in the obj file;
                screen_coords values depend on the screen width/height */
                world_coords[j] = v;
            }
            Vec3f n = (world_coords[2]-world_coords[0])^(world_coords[1]-world_coords[0]);
            /* IT'S NOT XOR IT'S FROM THE GEOMETRY.H DAMNIT
            inline Vec3<t> operator ^(const Vec3<t> &v) const
            { return Vec3<t>(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x); }
            */
            n.normalize();
            /* From the tutorial:

            Нулевую освещённость мы получим, если полигон параллелен вектору света.
            Перефразируем: интенсивность освещённости равна скалярному произведению
            вектора света и нормали к данному треугольнику.
            Нормаль к треугольнику может быть посчитана просто
            как векторное произведение двух его рёбер.
            Но ведь скалярное произведение может быть отрицательным, что это означает?
            Это означает, что свет падает позади полигона.*/
            float intensity = n*light_dir;
            if (intensity>0)
            {
                Vec2i uv[3];
                for (int k=0; k<3; k++){
                    uv[k] = model->uv(i,k);
                }
                triangle(screen_coords[0], screen_coords[1], screen_coords[2], uv[0], uv[1], uv[2], image, intensity, zbuffer);
            }


        }


        image.flip_vertically(); // we want to have the origin at the left bottom corner of the image
        image.write_tga_file("output.tga");
    }




    /*
     { // dump z-buffer (debugging purposes only)
    TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
    for (int i=0; i<width; i++) {
    for (int j=0; j<height; j++) {
    zbimage.set(i, j, TGAColor(zbuffer[i+j*width], 1));
    }
    }
    zbimage.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    zbimage.write_tga_file("zbuffer.tga");
    }

    */


    delete model; //get out from the memory, you sneaky thing
    delete [] zbuffer;
    return 0;
}
