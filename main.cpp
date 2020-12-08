#include <iostream>
#include <dirent.h>
#include <cstdlib>
#include <string>
#include <vector>
#include <cstddef>
#include <filesystem>
#include <chrono>
#include <fstream>
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
    ifstream f (filename->c_str(), ios::in | ios::binary);

    // read the first part of the header
    bmp->header.resize(14);
    f.read((char*)&bmp->header[0], 14);
    int start = *(int *)&bmp->header[10];

    // read the remaining part of the header
    f.seekg(0);
    bmp->header.resize(start);
    f.read((char*)&bmp->header[0], start);

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

            Color color {};
            f.read((char*)&color.B, 1);
            f.read((char*)&color.G, 1);
            f.read((char*)&color.R, 1);

            bmp->data[i][j] = color;
        }
        // skipping padding
        f.seekg((int)f.tellg() + padding);
    }

    f.close();
    return true;
}

void writeInByteArray(byte* dest, byte info[], int infoSize){
    for (int i = 0; i < infoSize; i++){
        dest[i] =  info[i];
    }
}

void writeBMP(string *filename, BMP *bmp)
{
    //FILE *f = fopen(filename->c_str(), "wb");
    ofstream f (filename->c_str(), ios::out | ios::binary);
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

    f.write((char*)&bmp->header[0], 54);

    byte zeros[8] = { };

    for (int i = 0; i < height; i++) {
        for(int j = 0; j < width; j++) {
            Color pixel = bmp->data[i][j];
            f.write((char*)&pixel.B, 1);
            f.write((char*)&pixel.G, 1);
            f.write((char*)&pixel.R, 1);
        }
        f.write((char*)&zeros[0], padding);
    }

    f.close();
}

bool printError(int argc, string *command, string *indir, string *outdir)
{
    if (argc != 4)
    {
        cerr << "Wrong format:" << endl;
        cerr << "\timage-seq operation in_path out_path\n\t\toperation: copy, gauss, sobel\n";
        return true;
    }

    if (*command != "copy" && *command !="gauss" && *command != "sobel")
    {
        cerr << "Unexpected operation:" << *command << "\n"
             << endl;
        cerr << "\timage-seq operation in_path out_path\n\t\toperation: copy, gauss, sobel\n";
        return true;
    }

    DIR *dr = opendir(indir->c_str());

    if (dr == nullptr)
    {
        cerr << "Input path: " << *indir << "\nOutput path: " << *outdir << "\nCannot open file [" << *indir << "]" << endl;
        cerr << "\timage-seq operation in_path out_path\n\t\toperation: copy, gauss, sobel\n";
        return true;
    }
    closedir(dr);

    dr = opendir(outdir->c_str());

    if (dr == nullptr)
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

    int height = bmp->data.size();
    int width = bmp->data[0].size();

    int w = 273;
    vector<vector<Color>> dataResult;
    dataResult.resize(height);

    for(int i = 0; i < height; i++){
        dataResult[i].resize(width);
        for(int j = 0; j < width; j++){
            int R = 0;
            int G = 0;
            int B = 0;
            for(int s = -2; s <= 2; s++){
                for(int t = -2; t <= 2; t++){
                    if((i + s) >= 0 && (i + s) < height && (j + t) >= 0 && (j + t) < width) {
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

    int height = bmp->data.size();
    int width = bmp->data[0].size();

    int w = 8;
    vector<vector<Color>> dataResult;
    dataResult.resize(height);

    for(int i = 0; i < height; i++){
        dataResult[i].resize(width);
        for(int j = 0; j< width; j++){
            //Colors for matrix X
            int RX = 0;
            int GX = 0;
            int BX = 0;
            //Colors for matrix Y
            int RY = 0;
            int GY = 0;
            int BY = 0;
            for(int s = -1; s <= 1 ; s++) {
                for (int t = -1; t <= 1; t++) {
                    if((i + s) >= 0 && (i + s) < height && (j + t) >= 0 && (j + t) < width) {
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
        auto &path = entry.path();

        string infilePath = path.relative_path().string();
        string infileName = path.filename().string();
        string outfilePath;
        outfilePath.append(outdirPath).append("/").append(infileName);

        auto startLoad = system_clock::now();
        bool load = readBMP(&infilePath, &bmp);
        auto endLoad = system_clock::now();
        auto loadTime = duration_cast<microseconds>(endLoad - startLoad);
        cout << "Load time: " << loadTime.count() << " microseconds" << endl;
        if(load) {
            // Ejecución de las funciones
            if(command == "gauss")
            {
                auto startGauss = system_clock::now();
                gauss(&bmp);
                auto endGauss = system_clock::now();
                auto gaussTime = duration_cast<microseconds>(endGauss - startGauss);
                cout << "Gauss time: " << gaussTime.count() << " microseconds" << endl;
            }
            if(command == "sobel")
            {
                auto startGauss = system_clock::now();
                gauss(&bmp);
                auto endGauss = system_clock::now();
                auto gaussTime = duration_cast<microseconds>(endGauss - startGauss);
                cout << "Gauss time: " << gaussTime.count() << " microseconds" << endl;

                auto startSobel = system_clock::now();
                sobel(&bmp);
                auto endSobel = system_clock::now();
                auto sobelTime = duration_cast<microseconds>(endSobel - startSobel);
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

    auto endTotal = system_clock::now();
    auto totalTime = duration_cast<microseconds>(endTotal - startTotal);
    cout<<" \n GLOBAL TIME: "<<totalTime.count()<< " microseconds"<<endl;

    return 0;
}
