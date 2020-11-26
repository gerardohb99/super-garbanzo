#include <iostream>
#include <fstream>
#include <dirent.h>
#include <stdio.h>
#include <sys/types.h>
#include <cstring>
#include <cstdlib>
#include <string>
#include <vector>
#include <cstddef>
using namespace std;

struct Color {
    byte R;
    byte G;
    byte B;
};

struct BMP
{
    vector<byte> header;
    vector<vector<Color>> data;
};


void readBMP(string *filename, BMP* bmp)
{
    ifstream in {filename->c_str()};
    FILE *f = fopen(filename->c_str(), "rb");

    // byte b;
    // // Read the next line from File untill it reaches the end.
    // while (in.readsome((char *) b, sizeof byte))
    // {
    //     // Line contains string of length > 0 then save it in vector
    //     if(b.size() > 0)
    //         bmp->header.push_back(b);
    // }
    // //Close The File
    // in.close();

    // read the 54-byte header
    bmp->header.resize(54);

    fread(&bmp->header[0], sizeof(byte), 54, f);

    // extract image height and width from header
    int width = *(int *)&bmp->header[18];
    int height = *(int *)&bmp->header[22];

    cout << width << " - " << height << endl;

    // allocate 3 bytes per pixel
    // int size = 3 * width * height;
    bmp->data.resize(height);

    for (int i = 0; i < height; i++) {
        bmp->data[i].resize(width);
        for (int j = 0; j < width; j++){
            Color color;
            fread(&color.R, sizeof(byte), 1, f);
            fread(&color.G, sizeof(byte), 1, f);
            fread(&color.B, sizeof(byte), 1, f);
            bmp->data[i][j] = color;
        }
    }

    fclose(f);

    // for(i = 0; i < size; i += 3)
    // {
    //     // flip the order of every 3 bytes
    //     unsigned char tmp = data[i];
    //     data[i] = data[i+2];
    //     data[i+2] = tmp;
    // }
}

void writeBMP(BMP *bmp, string *dir)
{
    dir->append("/out.bmp");

    vector<byte> image {bmp->header};
    for (int i = 0; i < bmp->data.size(); i++) {
        for(int j = 0; j < bmp->data[i].size(); j++) {
            Color pixel = bmp->data[i][j];
            image.push_back(pixel.R);
            image.push_back(pixel.G);
            image.push_back(pixel.B);
        }
    }

    FILE *f = fopen(dir->c_str(), "wb");
    fwrite(&image[0], sizeof(byte), image.size(), f);
}

bool printError(int argc, char *argv[])
{
    if (argc != 4)
    {
        cerr << "Wrong format:" << endl;
        cerr << "\timage-seq operation in_path out_path\n\t\toperation: copy, gauss, sobel\n";
        return true;
    }

    if (strcmp(argv[1], "copy") != 0 && strcmp(argv[1], "gauss") != 0 && strcmp(argv[1], "sobel") != 0)
    {
        cerr << "Unexpected operation:" << argv[1] << "\n"
             << endl;
        cerr << "\timage-seq operation in_path out_path\n\t\toperation: copy, gauss, sobel\n";
        return true;
    }

    FILE *pFile = fopen(argv[2], "rb");

    if (pFile == NULL)
    {
        cerr << "Input path: " << argv[2] << "\nOutput path: " << argv[3] << "\nCannot open file [" << argv[2] << "]" << endl;
        cerr << "\timage-seq operation in_path out_path\n\t\toperation: copy, gauss, sobel\n";
        return true;
    }
    fclose(pFile);

    DIR *dr;
    dr = opendir(argv[3]);

    if (dr == NULL)
    {
        cerr << "Input path: " << argv[2] << "\nOutput path: " << argv[3] << "\nOutput directory [" << argv[3] << "] does not exist" << endl;
        cerr << "\timage-seq operation in_path out_path\n\t\toperation: copy, gauss, sobel\n";
        return true;
    }
    closedir(dr);

    return false;
}

// bool copyFile(const char *SRC, const char* DEST)
// {
//     ifstream src(SRC, ios::binary);
//     ofstream dest(DEST, ios::binary);
//     dest << src.rdbuf();
//     return src && dest;
// }

void gauss (BMP *bmp){

    vector<vector<int>> m = {{1, 4, 7, 4, 1},
                             {4, 16, 26, 16, 4},
                             {7, 26, 41, 26, 7},
                             {4, 16, 26, 16, 4},
                             {1, 4, 7, 4, 1}}
    ;
    int w = 273;



    for(int i = 0; i < bmp->data.size()  ; i++){
        for(int j = 0; j < bmp->data[i].size() ; j++){
            cout<< i << " - " << j << endl;
            int R=0;
            int G=0;
            int B=0;
            for(int s = -2; s <= 2 ; s++){
                for(int t = -2; t <= 2 ; t++){
                    if(!((i+s) < 0 || (i+s) >= bmp->data.size() || (j+t) < 0 || (j+t) >= bmp->data[i].size())) {
                        R += m[s + 2][t + 2] * (int) bmp->data[i + s][j + t].R;
                        G += m[s + 2][t + 2] * (int) bmp->data[i + s][j + t].G;
                        B += m[s + 2][t + 2] * (int) bmp->data[i + s][j + t].B;
                    }
                }
            }
            bmp->data[i][j].R = (byte)(R/w);
            bmp->data[i][j].G = (byte)(G/w);
            bmp->data[i][j].B = (byte)(B/w);
            cout << R/w << "," << G/w << "," << B/w << endl;


        }
    }
}

void sobel (BMP *bmp)
{


}


int main(int argc, char *argv[])
{
    // if (!strcmp(getenv("DEBUGGING"), "true"))
    // {
    //     char *argv2[] = {(char *) "path", (char *) "copy", (char *) "1.bmp", (char *) "out"};
    //     argc = sizeof(argv2) / sizeof(char*);
    //     argv = argv2;
    // }

    if (printError(argc, argv))
    {
        return -1;
    }


    if(!strcmp(argv[1], "copy"))
    {

        BMP bmp;
        string filename = string (argv[2]);
        string dir = string (argv[3]);
        readBMP(&filename, &bmp);
        writeBMP(&bmp, &dir);

        //   return copyFile(argv[2], argv[3]) ? 0 : 1;

    }

    if(!strcmp(argv[1], "gauss"))
    {

        BMP bmp;
        string filename = string (argv[2]);
        string dir = string (argv[3]);
        readBMP(&filename, &bmp);
        gauss(&bmp);
        writeBMP(&bmp, &dir);
    }


    if(!strcmp(argv[1], "sobel"))
    {
        BMP bmp;
        string filename = string (argv[2]);
        string dir = string (argv[3]);
        readBMP(&filename, &bmp);
        gauss(&bmp);
        sobel(&bmp);
        writeBMP(&bmp, &dir);
    }

    // char* path = "./1.bmp";
    // BMP bmp = readBMP(path);

    return 0;
}
