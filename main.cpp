/*
	Compilar:
	c++ --std=c++11 main.cpp tinyxml2/tinyxml2.cpp -o NokiaKpiParser
*/
#include "tinyxml2/tinyxml2.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <ctime>
#include <string>
#include <vector>
#include <string.h>
#include <dirent.h>

using namespace std;
using namespace tinyxml2;

#define COUNTER_LIMIT 100

int main(int argc, char *argv[])
{
    vector<string> contadores; // Maximo 200 contadores
    XMLDocument doc;           // Reporte de KPIs
    int N_COUNTERS = 0;
    char aux_contador[12];
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
            if (dot_pos != -1)
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
                ifstream strKPIReport(dir->d_name);
                cout << "Reporte encontrado: " << Filename << Extension << endl;
                doc.LoadFile(dir->d_name);
                cout << "Parse: " << doc.ErrorName() << endl
                     << endl;
                strKPIReport.close();
            }
            if (Extension == ".txt" && Filename == "ReportTemplate")
            {
                // Tengo el template de contadores
                ifstream strTemplate(dir->d_name);
                cout << "Template encontrado: " << Filename << Extension << endl;
                int i = 0;
                memset(aux_contador, 0, 12);
                while (!strTemplate.eof() && i < COUNTER_LIMIT)
                {
                    memset(aux_contador, 0, 12);
                    strTemplate.getline(aux_contador, 12);
                    contadores.push_back(aux_contador);
                    cout << "Contador #" << i << ": " << contadores[i] << endl;
                    i++;
                }
                N_COUNTERS = i;
                strTemplate.close();
            }
        }
        cout << endl;
        closedir(d);
    }
    // Tenemos en XMLDocument doc el XML parseado, ahora tenemos que extraer informacion
    const XMLAttribute *startTime = doc.FirstChildElement("OMeS")->FirstChildElement("PMSetup")->FindAttribute("startTime");
    cout << "> startTime: " << startTime->Value() << endl;
    const XMLAttribute *interval = doc.FirstChildElement("OMeS")->FirstChildElement("PMSetup")->FindAttribute("interval");
    cout << "> interval: " << interval->Value() << endl;

    // Dada la lista de contadores que tenemos en contadores[] vamos a empezar a armar la tabla
    /* Formato de Tabla a devolver
    *  period,MO,Counter#1,Counter#3,Counter#2,Counter#4
    *  
    *  Una linea por cada MO, si ningun MO tiene los contadores pedidos, la linea no va.
    *  Para algun contador que no corresponda a algun objeto, quedara el espacio en blanco
    */
    // Aca obtenemos el DN al cual pertenece el KPI
    XMLNode *parent = doc.FirstChildElement("OMeS")->FirstChildElement("PMSetup")->FirstChildElement("PMMOResult");
    while (parent)
    {
        cout << parent->FirstChildElement("MO")->FirstChildElement("DN")->FirstChild()->ToText()->Value() << "," << endl;
        for (auto itr = contadores.begin(); itr != contadores.end(); ++itr)
        {
            XMLNode *CounterNODE = parent->FirstChildElement("NE-WBTS_1.0")->FirstChildElement((*itr).c_str());
            if(CounterNODE)
            {
                cout << CounterNODE->ToElement()->Value() << ",";
            }
        }
        cout << ",";
        cout << endl;
        parent = parent->NextSiblingElement("PMMOResult");
        parent = 0;
    }
    return 0;
}