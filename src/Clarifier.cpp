#include "Clarifier.h"
#include <string>
#include <iostream>
#include <fstream>
#include "StringOP.h"

using namespace std;

Clarifier::Clarifier()
{
    //ctor
}

Clarifier::~Clarifier()
{
    //dtor
}

Clarifier::Clarifier(int n)
{
    numberofcells = n;
    rho.resize(n);
    v_s.resize(n+1);
    F.resize(n+1);
}

Clarifier::Clarifier(const string &filename)
{
    Read(filename);
}

bool Clarifier::Read(const string &filename)
{
    ifstream file(filename);
	if (!file.good())
        {
            cout << "File " + filename + "was not found!" << endl;
            return;
        }
        else
        {
            cout << "Reading " + filename + "..." << endl;
        }
	vector<string> s;

	while (!file.eof())
	{
        s = getline(file,'=');
        if (s.size()>=2)
        {
            if (tolower(s[0]) == "inflow")
            {
                inflow = atof(s[1].c_str());
                cout<<"Inflow = "<<inflow<<endl;
            }

            if (tolower(s[0]) == "underflow")
            {
                underflow = atof(s[1].c_str());
                cout<<"Underflow = "<<underflow<<endl;
            }

            if (tolower(s[0]) == "numberofcells")
            {
                numberofcells = atoi(s[1].c_str());
                cout << "Number of Cells = " << numberofcells;
            }

            if (tolower(s[0]) == "inflowcell")
            {
                inflowcell == atoi(s[1].c_str());
                cout << "Inflow cell = " << inflowcell;
            }

            if (tolower(s[0])== "dz")
            {
                dz = atoi(s[1].c_str());
                cout << "Cell size = " << dz;
            }
            if (tolower(s[0])== "inflowconcentration")
            {
                CBTC temp = CBTC(s[1]);
                if (temp.C.size())
                {
                    inflowconcentration = temp;
                    cout<<"Inflow concentration was read successfully"<<endl;
                }
                else
                {
                    cout << "Inflow concentration file not cannot be opened!" << endl;
                    return false;
                }
            }
        }
	}
    upflow = inflow - underflow;
    rho.resize(numberofcells);
    v_s.resize(numberofcells+1);
    F.resize(numberofcells+1);
    for (int i=0; i<numberofcells+1; i++)
        if (i<=inflowcell)
            F[i] = -underflow;
        else
            F[i] = upflow;

    return true;
}

CVector_arma Clarifier::GetResidual(CVector_arma &X)
{

}
