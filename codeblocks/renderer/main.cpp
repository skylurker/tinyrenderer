#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"


//We're getting back from the "higher performance code" to the "readable code" here

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green = TGAColor(0, 255, 0, 255);
Model *model = NULL;
const int width = 800; //pixels
const int height = 800; //pixels

void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color) //vector = (x, y) a.k.a. one dot
{
    bool steep = false; //steep = true if length < height
    if (std::abs(p0.x - p1.x)<std::abs(p0.y - p1.y)) //if the line is sttep, we transpose the image
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
        int y = p0.y*(1.-t) + p1.y*t;       //(y-y1)/(y2-y1) = (x-x1)/(x2-x1) = t

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


void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color)
{
    if (t0.y==t1.y && t0.y==t2.y) return; //if it is a single dot, get outta here
        //sorting the vertices by y coordinate (t0 lower, t2 upper), bubblesort
        if (t0.y>t1.y) std::swap(t0,t1);
        if (t0.y>t2.y) std::swap(t0,t2);
        if (t1.y>t2.y) std::swap(t1,t2);


       int total_height = t2.y-t0.y; //of the whole triangle

       for (int i=0; i<total_height; i++){
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
       Vec2i A = t0 + (t2-t0)*alpha;
       Vec2i B = second_half ? t1 + (t2-t1)*beta : t0 + (t1-t0)*beta;
       if (A.x>B.x) std::swap(A, B); //if point A is to the right of point B
       for (int j=A.x; j<=B.x; j++){
        image.set(j, t0.y+i, color); //due to int casts t0.y+i != A.y
       }

       }




    }

/* Trying to draw the triangles */
int main(int argc, char** argv)
/* Параметр argc содержит количество аргументов
в командной строке и является целым числом,
причем он всегда не меньше 1, потому что первым аргументом
считается имя программы. */
{
   /* if (2==argc)
    {
        model = new Model(argv[1]); //if there is an argument which specifies the model we use
    }
    else
    {
        model = new Model("obj/african_head.obj"); //by default
    }*/
    TGAImage image(width, height, TGAImage::RGB);
   /* for (int i=0; i<model->nfaces(); i++)
    {
        // a->b Обращение к члену структуры («член b объекта, на который указывает a»)
        std::vector<int> face = model->face(i);
        for (int j=0; j<3; j++)
        {
            Vec3f v0 = model->vert(face[j]);
            Vec3f v1 = model->vert(face[(j+1)%3]);
            int x0 = (v0.x+1.)*width/2.; //for explanation lurk below
            int y0 = (v0.y+1.)*height/2.;
            int x1 = (v1.x+1.)*width/2.;
            int y1 = (v1.y+1.)*height/2.;*/
            /* All the vertices appear to have values ranging from -1 to 1.
            Thus, by adding 1 to each we get 0 to 2. Multiplying these by width
            and dividing by 2, we get a range of 0 to width as a result. Yay! */
          /*  line(x0, y0, x1, y1, image, white);
        }

    }*/

    Vec2i t0[3] = {Vec2i(10,70), Vec2i(50,160), Vec2i(70,80)};
    Vec2i t1[3] = {Vec2i(180,50), Vec2i(150,1), Vec2i(70,180)};
    Vec2i t2[3] = {Vec2i(180,150), Vec2i(120,160), Vec2i(130,180)};

    triangle(t0[0], t0[1], t0[2], image, red);
    triangle(t1[0], t1[1], t1[2], image, white);
    triangle(t2[0], t2[1], t2[2], image, green);

    image.flip_vertically(); // we want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    delete model; //get out from the memory, you sneaky thing
    return 0;
}
