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
    CVector_arma Res(X.num);
    for (int i=1; i<X.num-1; i++)
    {
        Res[i] = (X[i]-rho[i])/dt - (pos(v_s[i-1])*rho[i-1]-pos(v_s[i])*rho[i]-pos(-v_s[i-1])*rho[i]+pos(-v_s[i])*rho[i+1])/dz;
    }
    Res[X.num-1] = X[X.num-1];
    Res[0] = X[0]-X[1];
    return Res;
}

double Clarifier::CD(const double &Re)
{
    if (Re<1)
        return 24/Re;
    else
        return 24/Re + 3/sqrt(Re) + 0.34;
}

double Clarifier::Residual_vs(int i, double vs)
{
    double rho_avg = 0.5*(rho[i-1]+rho[i]);
    double rho_grad = (rho[i]-rho[i-1])/dz;
    double Re = physical_params.gamma_w/g*vs*physical_params.d/physical_params.mu;
    double residual = physical_params.gamma_s-physical_params.gamma_w - CD(Re)*physical_params.gamma_w/(2*g*physical_params.d)*(F[i]-vs)*fabs(F[i]-vs)/(1-rho_avg)+physical_params.k_sigma*physical_params.n*pow(rho_avg,physical_params.n-1)*rho_grad;
    return residual;
}

double Clarifier::Residual_vs_grad(int i, double vs_star)
{
    return (Residual_vs(i,vs_star+1e-6)-Residual_vs(i,vs_star))/1e-6;
}


double Clarifier::Calculate_vs(int i)
{
    double err0 = Residual_vs(i,v_s[i]);
    double err = err0;
    double vs_star = v_s[i];
    while (fabs(err/err0)>1e-6)
    {
        vs_star -= err/Residual_vs_grad(i,vs_star);
        err = Residual_vs(i,vs_star);
    }
    return vs_star;
}

void Clarifier::FillVprime_s()
{
    for (int i=1; i<nz; i++)
        v_s[i] = Calculate_vs(i);

    return;

}
