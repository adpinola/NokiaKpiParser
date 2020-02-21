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
#include <cstring>
#include <vector>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

using namespace std;
using namespace tinyxml2;

#define COUNTER_LIMIT 100

int main(int argc, char *argv[])
{
    vector<string> MOs;        // Guardo aca todos los MO del reporte
    vector<string> contadores; // Maximo 200 contadores
    XMLDocument doc;           // Reporte de KPIs
    int N_COUNTERS = 0;
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
                std::cout << "Reporte encontrado: " << Filename << Extension << endl;
                doc.LoadFile(dir->d_name);
                std::cout << "Parse: " << doc.ErrorName() << endl
                     << endl;
                strKPIReport.close();
            }
            if (Extension == ".txt" && Filename == "ReportTemplate")
            {
                // Tengo el template de contadores
                ifstream strTemplate(dir->d_name);
                std::cout << "Template encontrado: " << Filename << Extension << endl;
                int i = 0;
                while (!strTemplate.eof() && i < COUNTER_LIMIT)
                {
                    string aux;
                    getline(strTemplate, aux, ',');
                    contadores.push_back(move(aux));
                    std::cout << "Contador #" << i << ": " << contadores.at(i) << endl;
                    i++;
                }
                N_COUNTERS = i;
                strTemplate.close();
            }
        }
        std::cout << endl;
        closedir(d);
    }
    // Tenemos en XMLDocument doc el XML parseado, ahora tenemos que extraer informacion
    const XMLAttribute *startTime = doc.FirstChildElement("OMeS")->FirstChildElement("PMSetup")->FindAttribute("startTime");
    std::cout << "> startTime: " << startTime->Value() << endl;
    const XMLAttribute *interval = doc.FirstChildElement("OMeS")->FirstChildElement("PMSetup")->FindAttribute("interval");
    std::cout << "> interval: " << interval->Value() << endl;

    // Dada la lista de contadores que tenemos en contadores[] vamos a empezar a armar la tabla
    /* Formato de Tabla a devolver
    *  period,MO,Counter#1,Counter#3,Counter#2,Counter#4
    *  
    *  Una linea por cada MO, si ningun MO tiene los contadores pedidos, la linea no va.
    *  Para algun contador que no corresponda a algun objeto, quedara el espacio en blanco
    */
    // Aca obtenemos el DN al cual pertenece el KPI

    // En primer lugar vamos a guardar todos los MOs del reporte asi los tenemos listados
    // Luego el objetivo es agrupar KPIs por MO en lugar de por Medicion para asi sacar en el reporte una linea por MO
    XMLElement *parent = doc.FirstChildElement("OMeS")->FirstChildElement("PMSetup")->FirstChildElement("PMMOResult");
    while (parent)
    {
        string MOdn(parent->FirstChildElement("MO")->FirstChildElement("DN")->FirstChild()->ToText()->Value());
        if (MOs.empty())
        {
            MOs.push_back(MOdn);
        }
        else if (MOs.back() != MOdn)
        {
            MOs.push_back(MOdn);
        }
        parent = parent->NextSiblingElement("PMMOResult");
    }
    parent = doc.FirstChildElement("OMeS")->FirstChildElement("PMSetup")->FirstChildElement("PMMOResult");
    for (auto itrMO = MOs.begin(); itrMO != MOs.end(); ++itrMO)
    {
        std::cout << *itrMO << ",";
        XMLElement *CounterNODE = parent->FirstChildElement("NE-WBTS_1.0")->FirstChildElement();
        for (auto itr = contadores.begin(); itr != contadores.end(); ++itr)
        {
            XMLElement *root = doc.FirstChildElement("OMeS")->FirstChildElement("PMSetup")->FirstChildElement("PMMOResult");
            while (root)
            {
                if( strcmp((*itrMO).c_str(),root->FirstChildElement("MO")->FirstChildElement("DN")->FirstChild()->ToText()->Value()) == 0)
                {
                    // Contadores del MO bajo analisis
                    CounterNODE = root->FirstChildElement("NE-WBTS_1.0")->FirstChildElement();
                    while (CounterNODE)
                    {
                        if (strcmp(CounterNODE->Name(), (*itr).c_str()) == 0)
                        {
                            std::cout << CounterNODE->FirstChild()->Value();
                            break;
                        }
                        CounterNODE = CounterNODE->NextSiblingElement();
                    }
                }
                root = root->NextSiblingElement("PMMOResult");
            }
            std::cout << ",";
        }
        std::cout << endl;
    }
    return 0;
}