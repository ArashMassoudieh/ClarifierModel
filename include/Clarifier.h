#ifndef CLARIFIER_H
#define CLARIFIER_H

#include <vector>
#include <string>
#include "BTC.h"

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
        vector<double> F;
        double inflow;
        double underflow;
        double upflow;
        double dz;
        double dt;
        CBTC inflowconcentration;
        CVector_arma GetResidual(CVector_arma &X);

};

#endif // CLARIFIER_H
