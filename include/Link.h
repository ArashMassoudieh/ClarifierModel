#ifndef LINK_H
#define LINK_H
#include <string>
#include <map>
#include "Quan.h"
#include "Object.h"

using namespace std;

class Block;
class System;

class Link: public Object
{
    public:
        Link();
        Link(System *parent);
        virtual ~Link();
        Link(const Link& other);
        Link& operator=(const Link& other);
        Block* GetConnectedBlock(Expression::loc l);

    protected:

    private:


};

#endif // LINK_H
