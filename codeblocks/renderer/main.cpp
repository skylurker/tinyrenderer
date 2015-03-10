#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"


//We're getting back from the "higher performance code" to the "readable code" here

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
Model *model = NULL;
const int width = 800; //pixels
const int height = 800; //pixels

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


    for (int x=x0; x<=x1; x++)
    {
        float t = (x-x0)/(float)(x1-x0); //again, this one is from the line equation
        int y = y0*(1.-t) + y1*t;       //(y-y1)/(y2-y1) = (x-x1)/(x2-x1) = t

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


int main(int argc, char** argv)
/* Параметр argc содержит количество аргументов
в командной строке и является целым числом,
причем он всегда не меньше 1, потому что первым аргументом
считается имя программы. */
{
  if (2==argc){
    model = new Model(argv[1]); //if there is an argument which specifies the model we use
  } else{
    model = new Model("obj/african_head.obj"); //by default
  }
  TGAImage image(width, height, TGAImage::RGB);
  for (int i=0; i<model->nfaces(); i++){
    // a->b Обращение к члену структуры («член b объекта, на который указывает a»)
    std::vector<int> face = model->face(i);
    for (int j=0; j<3; j++){
    Vec3f v0 = model->vert(face[j]);
    Vec3f v1 = model->vert(face[(j+1)%3]);
    int x0 = (v0.x+1.)*width/2.; //for explanation lurk below
    int y0 = (v0.y+1.)*height/2.;
    int x1 = (v1.x+1.)*width/2.;
    int y1 = (v1.y+1.)*height/2.;
    /* All the vertices appear to have values ranging from -1 to 1.
    Thus, by adding 1 to each we get 0 to 2. Multiplying these by width
    and dividing by 2, we get a range of 0 to width as a result. Yay! */
    line(x0, y0, x1, y1, image, white);
  }

  }

    image.flip_vertically(); // we want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    delete model; //get out from the memory, you sneaky thing
    return 0;
}
