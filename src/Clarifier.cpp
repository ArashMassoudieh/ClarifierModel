#include "Clarifier.h"
#include <string>
#include <iostream>
#include <fstream>
#include "StringOP.h"
#include "Utilities.h"

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
    v_s_prime.resize(n+1);
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
            return false;
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

double Clarifier::CD(const double &Re)
{
    if (Re<1)
        return 24/Re;
    else
        return 24/Re + 3/sqrt(Re) + 0.34;
}

void Clarifier::FillVprime_s()
{
    for (int i=1; i<nz; i++)
    {
        double rho_avg = 0.5*(rho[i-1]+rho[i]);
        double rho_grad = (rho[i]-rho[i-1])/dz;
        double Re = physical_params.gamma_s/g*v_s_prime[i]*physical_params.d/physical_params.mu;
        double Drag_coeff = CD(Re);
        double term1 = physical_params.gamma_s/physical_params.gamma_w - 1 + physical_params.k_sigma*physical_params.n*pow(rho_avg,physical_params.n)*rho_grad;
        v_s_prime[i] = sgn(term1)*sqrt(2*g*physical_params.d/Drag_coeff*fabs(term1)*pow(1-rho_avg,3));
    }
    return;

}
