#ifndef CLARIFIER_H
#define CLARIFIER_H

#include <vector>

using namespace std;

class Clarifier
{
    public:
        Clarifier();
        Clarifier(int);
        virtual ~Clarifier();

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

};

#endif // CLARIFIER_H
