#include "Clarifier.h"

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
