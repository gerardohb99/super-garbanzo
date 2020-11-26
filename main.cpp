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

    // read the 54-byte header
    bmp->header.resize(54);

    fread(&bmp->header[0], sizeof(byte), 54, f);


    // extract image height and width from header
    int width = *(int *)&bmp->header[18];
    int height = *(int *)&bmp->header[22];

    //cout << width << " - " << height << endl;

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
            //cout<< i << " - " << j << endl;
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
            //cout << R/w << "," << G/w << "," << B/w << endl;


        }
    }
}

void sobel (BMP *bmp)
{
    vector<vector<int>> x = {{1, 2, 1},
                            {0, 0, 0},
                            {-1, -2, -1}};

    vector<vector<int>> y = {{-1, 0, 1},
                        {-2, 0, 2},
                        {-1, 0, 1}};

    int w = 8;


    for(int i =0; i<bmp->data.size(); i++){
        for(int j=0; j< bmp->data[i].size(); j++){
            //Colors for matrix X
            int RX=0;
            int GX=0;
            int BX=0;
            //Colors for matrix Y
            int RY=0;
            int GY=0;
            int BY=0;
            for(int s = -1; s <= 1 ; s++) {
                for (int t = -1; t <= 1; t++) {
                    if(!((i+s) < 0 || (i+s) >= bmp->data.size() || (j+t) < 0 || (j+t) >= bmp->data[i].size())) {
                        //res x
                        RX += x[s + 1][t + 1] * (int) bmp->data[i + s][j + t].R;
                        GX += x[s + 1][t + 1] * (int) bmp->data[i + s][j + t].G;
                        BX += x[s + 1][t + 1] * (int) bmp->data[i + s][j + t].B;
                        //res y
                        RY += y[s + 1][t + 1] * (int) bmp->data[i + s][j + t].R;
                        GY += y[s + 1][t + 1] * (int) bmp->data[i + s][j + t].G;
                        BY += y[s + 1][t + 1] * (int) bmp->data[i + s][j + t].B;
                    }
                }
            }
            RX = RX/w;
            GX = GX/w;
            BX = BX/w;

            RY = RY/w;
            GY = GY/w;
            BY = BY/w;

            bmp->data[i][j].R = (byte)(abs(RX) + abs(RY));
            bmp->data[i][j].G = (byte)(abs(GX) + abs(GY));
            bmp->data[i][j].B = (byte)(abs(BX) + abs(BY));
        }
    }

}


int main(int argc, char *argv[])
{
    if (printError(argc, argv))
    {
        return -1;
    }

    // Lectura del archivo bmp
    BMP bmp;
    string filename = string (argv[2]);
    string dir = string (argv[3]);
    readBMP(&filename, &bmp);

    // Comprobacion de erroresen el header
    byte dims = *(byte *)bmp.header[27] << 8 | *(byte *)bmp.header[26];
    if((int)dims != 1){
        cerr << "Header error:" << endl;
        cerr << "Illegal number of planes" << endl;
        return -1;
    }

    byte pointSize = *(byte *)bmp.header[29] << 8 | *(byte *)bmp.header[28];
    if((int)pointSize != 24){
        cerr << "Header error:" << endl;
        cerr << "Bit count is not 24" << endl;
        return -1;
    }

    int compression = *(int *)bmp.header[30];
    if(compression != 0) {
        cerr << "Header error:" << endl;
        cerr << "Compression value is not 0" << endl;
        return -1;
    }

    // Ejecución de las funciones
    if(!strcmp(argv[1], "copy"))
    {
        writeBMP(&bmp, &dir);
    } else if(!strcmp(argv[1], "gauss"))
    {
        gauss(&bmp);
        writeBMP(&bmp, &dir);
    } else if(!strcmp(argv[1], "sobel"))
    {
        gauss(&bmp);
        sobel(&bmp);
        writeBMP(&bmp, &dir);
    }

    return 0;
}
