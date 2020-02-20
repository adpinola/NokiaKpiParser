/*
	Compilar:
	c++ --std=c++11 main.cpp tinyxml2/tinyxml2.cpp -o NokiaKpiParser
*/
#include "tinyxml2/tinyxml2.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <ctime>
#include <string.h>
#include <dirent.h>

using namespace std;

int main(int argc, char *argv[])
{
    // Primero abrimos directorio donde estan los KPIs
    // En este caso va a ser en el mismo lugar de ejecucion del programa
    DIR *d;
    int dot_pos;
    struct dirent *dir;
    d = opendir(".");
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            string Filename = dir->d_name;
            string Extension = Filename;
            dot_pos = Filename.find_last_of(".");
            if(dot_pos != -1)
            {
                Filename.erase(dot_pos, Filename.length());
                Extension.erase(0, dot_pos);    
            }
            else
            {
                Extension.clear();
            }
            //Ya tenemos el nombre del archivo sin extension
            if (Extension == ".xml")
            {
                // Es un XML entonces lo abro.
                ifstream strTemplate(dir->d_name);
                cout << "Archivo encontrado: " << Filename << Extension << endl;
            }
        }
        cout << endl;
        closedir(d);
    }
    return 0;
}