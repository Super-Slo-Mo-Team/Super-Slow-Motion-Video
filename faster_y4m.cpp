#include <iostream>
#include <fstream>
#include <string>
#include <ctype.h>
#include <cstring>
#include <vector>
#include <iterator>

using namespace std;

int main() {
    int wid = 0, hei = 0, frames = 0;
    bool wcheck = false, hcheck = false;
    char temp = 0;
    int i = 1;

    ifstream file;
    file.open("akiyo_cif.y4m", ios::binary);

    char* tempBuffer = nullptr;

    // read char until hit W or H
    while (file.get(temp)) {
        if ((temp) == 'W' && !wcheck) {
            string val = "";
            file.get(temp);
            while (isdigit(temp)) {
                val += (temp);
                file.get(temp);
            }
            wid = stoi(val);
            cout << "width: " << wid << endl;
            wcheck = 1;
        }

        if ((temp) == 'H' && !hcheck) {
            string val = "";
            file.get(temp);
            while (isdigit(temp)) {
                val += (temp);
                file.get(temp);
            }
            hei = stoi(val);
            cout << "height: " << hei << endl;
            hcheck = 1;
        }


        if (temp == 'F') {
            string cur = "F";
            int j = 0;
            while (file.get(temp) && j < 4) {
                cur += temp;
                j++;
            }
            if (cur.compare("FRAME") == 0) {
                frames++;


                int size = wid * hei * 3 / 2;
                
                if (tempBuffer == nullptr)
                {
                    tempBuffer = (char*)malloc(size);
                }

                file.read(tempBuffer, size);

                string outputName = "input_" + to_string(wid) + "x" + to_string(hei) + "_" + to_string(i) + ".yuv";
                ofstream output;
                output.open(outputName, ios::binary);

                ostream_iterator<unsigned char> it(output);

                output.write(tempBuffer, size);

                output.close();

                i++;
            }
        }

    }

    file.close();

    cout << "found " << frames << " frames" << endl;

    if (tempBuffer != nullptr)
    {
        free(tempBuffer);
    }

    return 0;
}