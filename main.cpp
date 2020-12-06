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
#include <math.h>
#include <filesystem>
#include <chrono>
using namespace std;
using namespace chrono;

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

bool readBMP(string *filename, BMP* bmp)
{
    FILE *f = fopen(filename->c_str(), "rb");

    // read the first part of the header
    bmp->header.resize(14);
    fread(&bmp->header[0], sizeof(byte), 14, f);
    int start = *(int *)&bmp->header[10];

    // read the remaining part of the header
    fseek(f, 0, SEEK_SET);
    bmp->header.resize(start);
    fread(&bmp->header[0], sizeof(byte), start, f);

    // Comprobacion de erroresen el header
    byte dims = *(byte *)&bmp->header[27] << 8 | *(byte *)&bmp->header[26];
    if((int)dims != 1){
        cerr << "Header error:" << endl;
        cerr << "Illegal number of planes" << endl;
        return false;
    }

    byte pointSize = *(byte *)&bmp->header[29] << 8 | *(byte *)&bmp->header[28];
    if((int)pointSize != 24){
        cerr << "Header error:" << endl;
        cerr << "Bit count is not 24" << endl;
        return false;
    }

    int compression = *(int *)&bmp->header[30];
    if(compression != 0) {
        cerr << "Header error:" << endl;
        cerr << "Compression value is not 0" << endl;
        return false;
    }

    cout << *filename << endl;

    int width = *(int *)&bmp->header[18];
    int height = *(int *)&bmp->header[22];

    bmp->data.resize(height);

    // Padding for each row
    int padding = (4 - (3 * width % 4)) % 4;

    for (int i = 0; i < height; i++) {
        bmp->data[i].resize(width);
        for (int j = 0; j < width; j++){
            Color color;
            fread(&color.B, sizeof(byte), 1, f);
            fread(&color.G, sizeof(byte), 1, f);
            fread(&color.R, sizeof(byte), 1, f);
            bmp->data[i][j] = color;
        }
        // skipping padding
        fseek(f, padding, SEEK_CUR);
    }

    fclose(f);
    return true;
}

void writeInByteArray(byte* dest, byte info[], uint infoSize ){
    for (int i = 0; i < infoSize; i++){
        dest[i] =  info[i];
    }
}

void writeBMP(string *filename, BMP *bmp)
{
    FILE *f = fopen(filename->c_str(), "wb");
    int width = bmp->data[0].size();
    int height = bmp->data.size();
    int padding = (4 - (3 * width % 4)) % 4;

    int zero = 0;
    byte* zeroArray = static_cast<byte*> (static_cast<void*>(&zero));

    //write size of file in header
    int sizeFile = 3 * width * height + height*padding + 54 ;
    byte* sizeFileArrayByte = static_cast<byte*> (static_cast<void*>(&sizeFile));
    writeInByteArray(&bmp->header[2], sizeFileArrayByte, sizeof(int));

    //write zero in header[6]
    writeInByteArray(&bmp->header[6], zeroArray, sizeof(int));

    //write data offset
    int dataStart = 54;
    byte* dataStartArray = static_cast<byte*> (static_cast<void*>(&dataStart));
    writeInByteArray(&bmp->header[10], dataStartArray, sizeof(int));

    //write bitmap header size
    int bitMapHSize = 40;
    byte* bitMapHSizeArray = static_cast<byte*> (static_cast<void*>(&bitMapHSize));
    writeInByteArray(&bmp->header[14], bitMapHSizeArray, sizeof(int));

    //write image Width in pixels
    byte* widthImageArray  = static_cast<byte*> (static_cast<void*>(&width));
    writeInByteArray(&bmp->header[18], widthImageArray, sizeof(int));

    //write image height in pixels
    byte* heightImageArray  = static_cast<byte*> (static_cast<void*>(&height));
    writeInByteArray(&bmp->header[22], heightImageArray, sizeof(int));

    //write dims number only first 2 bytes
    int dims = 1;
    byte* dimsArray  = static_cast<byte*> (static_cast<void*>(&dims));
    writeInByteArray(&bmp->header[26], dimsArray, 2);

    //write  in header[28]
    int pointSize = 24;
    byte* pointSizeArray  = static_cast<byte*> (static_cast<void*>(&pointSize));
    writeInByteArray(&bmp->header[28], pointSizeArray, 2);

    //write compretion value
    writeInByteArray(&bmp->header[30], zeroArray, sizeof(int));

    //write image data size in bytes
    int dataSize = 3 * width * height + height*padding;
    byte* dataSizeArray  = static_cast<byte*> (static_cast<void*>(&dataSize));
    writeInByteArray(&bmp->header[34], dataSizeArray, sizeof(int));

    // write horizontal resolution
    int horizontalResolution = 2835;
    byte* horizontalResolutionArray  = static_cast<byte*> (static_cast<void*>(&horizontalResolution));
    writeInByteArray(&bmp->header[38], horizontalResolutionArray, sizeof(int));

    // write vertical resolution
    int verticalResolution = 2835;
    byte* verticalResolutionArray  = static_cast<byte*> (static_cast<void*>(&verticalResolution));
    writeInByteArray(&bmp->header[42], verticalResolutionArray, sizeof(int));

    writeInByteArray(&bmp->header[46], zeroArray, sizeof(int));

    writeInByteArray(&bmp->header[50], zeroArray, sizeof(int));

    fwrite(&bmp->header[0], sizeof(byte), 54, f);

    byte zeros[8] = { };

    for (int i = 0; i < height; i++) {
        for(int j = 0; j < width; j++) {
            Color pixel = bmp->data[i][j];
            fwrite(&pixel.B, sizeof(byte), 1, f);
            fwrite(&pixel.G, sizeof(byte), 1, f);
            fwrite(&pixel.R, sizeof(byte), 1, f);
        }
        fwrite(&zeros[0], sizeof(byte), padding, f);
    }

    fclose(f);
}

bool printError(int argc, string *command, string *indir, string *outdir)
{
    if (argc != 4)
    {
        cerr << "Wrong format:" << endl;
        cerr << "\timage-seq operation in_path out_path\n\t\toperation: copy, gauss, sobel\n";
        return true;
    }

    if (command->compare("copy") && command->compare("gauss") && command->compare("sobel"))
    {
        cerr << "Unexpected operation:" << *command << "\n"
             << endl;
        cerr << "\timage-seq operation in_path out_path\n\t\toperation: copy, gauss, sobel\n";
        return true;
    }

     DIR *dr = opendir(indir->c_str());

     if (dr == NULL)
     {
         cerr << "Input path: " << *indir << "\nOutput path: " << *outdir << "\nCannot open file [" << *indir << "]" << endl;
         cerr << "\timage-seq operation in_path out_path\n\t\toperation: copy, gauss, sobel\n";
         return true;
     }
     closedir(dr);

    dr = opendir(outdir->c_str());

    if (dr == NULL)
    {
        cerr << "Input path: " << *indir << "\nOutput path: " << *outdir << "\nOutput directory [" << *indir << "] does not exist" << endl;
        cerr << "\timage-seq operation in_path out_path\n\t\toperation: copy, gauss, sobel\n";
    }
    closedir(dr);

    return false;
}


void gauss (BMP *bmp){

    vector<vector<int>> m = {{1, 4, 7, 4, 1},
                             {4, 16, 26, 16, 4},
                             {7, 26, 41, 26, 7},
                             {4, 16, 26, 16, 4},
                             {1, 4, 7, 4, 1}}
    ;


    int w = 273;
    vector<vector<Color>> dataResult;
    dataResult.resize(bmp->data.size());


    for(int i = 0; i < bmp->data.size(); i++){
        for(int j = 0; j < bmp->data[i].size(); j++){
            dataResult[i].resize(bmp->data[i].size());
            int R=0;
            int G=0;
            int B=0;
            for(int s = -2; s <= 2; s++){
                for(int t = -2; t <= 2; t++){
                    if(!((i+s) < 0 || (i+s) >= bmp->data.size() || (j+t) < 0 || (j+t) > bmp->data[i].size())) {
                        R += m[s + 2][t + 2] * (int) bmp->data[i + s][j + t].R;
                        G += m[s + 2][t + 2] * (int) bmp->data[i + s][j + t].G;
                        B += m[s + 2][t + 2] * (int) bmp->data[i + s][j + t].B;
                    }
                }
            }
            dataResult[i][j].R = (byte) (R / w);
            dataResult[i][j].G = (byte) (G / w);
            dataResult[i][j].B = (byte) (B / w);
        }
    }
    bmp->data = dataResult;
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
    vector<vector<Color>> dataResult;
    dataResult.resize(bmp->data.size());


    for(int i =0; i<bmp->data.size(); i++){
        for(int j=0; j< bmp->data[i].size(); j++){
            dataResult[i].resize(bmp->data[i].size());
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

            dataResult[i][j].R = (byte)((abs(RX) + abs(RY)) / w);
            dataResult[i][j].G = (byte)((abs(GX) + abs(GY)) / w);
            dataResult[i][j].B = (byte)((abs(BX) + abs(BY)) / w);
        }
    }
    bmp->data = dataResult;

}

void restaBMP (BMP *bmp1, BMP * bmp2){

    if (bmp1->data.size() == bmp2->data.size() && bmp1->data[0].size() == bmp2->data[0].size()) {
        for (int i = 0; i < bmp1->data.size(); i++) {
            for (int j = 0; j < bmp1->data[i].size(); j++) {
                bmp2->data[i][j].R = (byte)(abs((int) bmp1->data[i][j].R - (int) bmp2->data[i][j].R));
                bmp2->data[i][j].G = (byte)(abs((int) bmp1->data[i][j].G - (int) bmp2->data[i][j].G));
                bmp2->data[i][j].B = (byte)(abs((int) bmp1->data[i][j].B - (int) bmp1->data[i][j].B));
            }
        }
    }
}


int main(int argc, char *argv[])
{
    auto startTotal= system_clock::now();
    string command = string(argv[1]);
    string indirPath = string(argv[2]);
    string outdirPath = string(argv[3]);

     if (printError(argc, &command, &indirPath, &outdirPath))
     {
         return -1;
     }

    for (const auto & entry : filesystem::directory_iterator(indirPath)) {
        auto start = system_clock::now();
        // Lectura del archivo bmp
        BMP bmp;
        auto path = entry.path();

        string infilePath = path.relative_path().string();
        string infileName = path.filename().string();
        string outfilePath = outdirPath + '/' + infileName;

        auto startLoad = system_clock::now();
        bool load = readBMP(&infilePath, &bmp);
        auto endLoad = system_clock::now();
        auto loadTime = duration_cast<microseconds>(endLoad - startLoad);
        cout << "Load time: " << loadTime.count() << " microseconds" << endl;
        if(load) {
            // Ejecución de las funciones
            if(!command.compare("gauss"))
            {
                auto startGauss = system_clock::now();
                gauss(&bmp);
                auto endGauss = system_clock::now();
                auto gaussTime = duration_cast<microseconds>(endGauss - startGauss);
                cout << "Gauss time: " << gaussTime.count() << " microseconds" << endl;
            }
            if(!command.compare("sobel"))
            {
                auto startGauss = system_clock::now();
                gauss(&bmp);
                auto endGauss = system_clock::now();
                auto gaussTime = duration_cast<microseconds>(endGauss - startGauss);
                cout << "Gauss time: " << gaussTime.count() << " microseconds" << endl;

                auto startSobel = system_clock::now();
                sobel(&bmp);
                auto endSobel = system_clock::now();
                auto sobelTime = duration_cast<microseconds>(endGauss - startGauss);
                cout << "Sobel time: " << sobelTime.count() << " microseconds" << endl;
            }
            auto startStore = system_clock::now();
            writeBMP(&outfilePath, &bmp);
            auto endStore = system_clock::now();
            auto storeTime = duration_cast<microseconds>(endStore - startStore);
            cout << "Store time: " << storeTime.count() << " microseconds" << endl;
        }
        auto end = system_clock::now();
        auto time = duration_cast<microseconds>(end - start);
        cout << "Total time from picture: " << time.count() << " microseconds \n" << endl;



    }

//    else if(!strcmp(argv[1], "minus"))
//    {
//        BMP bmp2;
//        string filename2 = string (argv[4]);
//        readBMP(&filename2, &bmp2);
//        restaBMP(&bmp, &bmp2);
//        writeBMP(&bmp2, &dir);
//    }
    auto endTotal = system_clock::now();
    auto totalTime = duration_cast<microseconds>(endTotal - startTotal);
    cout<<" \n GLOBAL TIME: "<<totalTime.count()<< " microseconds"<<endl;

    return 0;
}
