#ifndef CLARIFIER_H
#define CLARIFIER_H

#include <vector>
#include <string>
#include "BTC.h"

struct _physical_params
{
    double gamma_s;
    double gamma_w;
    double d;
    double mu;
    double k_sigma;
    double n;
};

using namespace std;

class Clarifier
{
    public:
        Clarifier();
        Clarifier(int);
        virtual ~Clarifier();
        Clarifier(const string &filename);
        bool Read(const string &filename);


    protected:

    private:
        int numberofcells;
        int inflowcell;
        vector<double> rho;
        vector<double> v_s;
        vector<double> v_s_prime;
        vector<double> F;
        double inflow;
        double underflow;
        double upflow;
        double dz;
        double dt;
        double nz;
        _physical_params physical_params;
        CBTC inflowconcentration;
        CVector_arma GetResidual(CVector_arma &X);
        double CD(const double &Re);
        void FillVprime_s();

};

#endif // CLARIFIER_H
