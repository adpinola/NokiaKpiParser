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
    vector<string> RAWFiles;   // Nombre de arhivos con KPIs
    XMLDocument doc;
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
            if (Extension == ".xml" || Extension == ".raw")
            {
                // Es un XML entonces lo abro.
                //ifstream strKPIReport(dir->d_name);
                std::cout << "Reporte encontrado: " << Filename << Extension << endl;
                // Guardo el nombre del archivo
                RAWFiles.push_back(dir->d_name);
                //strKPIReport.close();
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

    // En primer lugar vamos a guardar todos los MOs del reporte asi los tenemos listados
    // Luego el objetivo es agrupar KPIs por MO en lugar de por Medicion para asi sacar en el reporte una linea por MO
    // los KPIs se van a guardar en un archivo report .csv

    ofstream report("report.csv");

    // Escribimos el titulo de las columnas
    report << "periodStartTime,MO,";
    for (auto itr = contadores.begin(); itr != contadores.end(); ++itr)
    {
        report << *itr << ",";
    }
    report << endl;
    for (auto itrRAW = RAWFiles.begin(); itrRAW != RAWFiles.end(); ++itrRAW)
    {
        // Barremos todos los documentos que trajimos
        doc.LoadFile((*itrRAW).c_str());
        std::cout << "Parse: " << *itrRAW << " - " << doc.ErrorName() << endl;
        // Tenemos en XMLDocument doc el XML parseado, ahora tenemos que extraer informacion
        const XMLAttribute *startTime = doc.FirstChildElement("OMeS")->FirstChildElement("PMSetup")->FindAttribute("startTime");
        string periodStartTime(startTime->Value());
        periodStartTime.replace(periodStartTime.find("T"), 1, " ");
        periodStartTime.resize(19);
        std::cout << "> startTime: " << periodStartTime << endl;
        const XMLAttribute *interval = doc.FirstChildElement("OMeS")->FirstChildElement("PMSetup")->FindAttribute("interval");
        std::cout << "> interval: " << interval->Value() << endl
                  << endl;

        // Dada la lista de contadores que tenemos en contadores[] vamos a empezar a armar la tabla
        /* Formato de Tabla a devolver
        *  period,MO,Counter#1,Counter#3,Counter#2,Counter#4
        *  
        *  Una linea por cada MO, si ningun MO tiene los contadores pedidos, la linea no va.
        *  Para algun contador que no corresponda a algun objeto, quedara el espacio en blanco
        */
        // Aca obtenemos el DN al cual pertenece el KPI
        // Ahora empezamos a recorrer el XML
        bool f_existKPI = false;
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
            f_existKPI = false;
            streampos line_start = report.tellp();
            report << periodStartTime << "," << *itrMO << ",";
            XMLElement *CounterNODE = parent->FirstChildElement("NE-WBTS_1.0")->FirstChildElement();
            for (auto itr = contadores.begin(); itr != contadores.end(); ++itr)
            {
                XMLElement *root = doc.FirstChildElement("OMeS")->FirstChildElement("PMSetup")->FirstChildElement("PMMOResult");
                while (root)
                {
                    if (strcmp((*itrMO).c_str(), root->FirstChildElement("MO")->FirstChildElement("DN")->FirstChild()->ToText()->Value()) == 0)
                    {
                        // Contadores del MO bajo analisis
                        CounterNODE = root->FirstChildElement("NE-WBTS_1.0")->FirstChildElement();
                        while (CounterNODE)
                        {
                            if (strcmp(CounterNODE->Name(), (*itr).c_str()) == 0)
                            {
                                report << CounterNODE->FirstChild()->Value();
                                f_existKPI = true;
                                break;
                            }
                            CounterNODE = CounterNODE->NextSiblingElement();
                        }
                    }
                    root = root->NextSiblingElement("PMMOResult");
                }
                report << ",";
            }
            if (f_existKPI)
                report << endl;
            else
                report.seekp(line_start);
        }
        //report << "EOF " << *itrRAW << endl;
        doc.Clear();
        MOs.clear();
    }
    report.close();
    cout << "Output file generated: report.csv" << endl;
    return 0;
}