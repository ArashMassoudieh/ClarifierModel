#ifndef BLOCK_H
#define BLOCK_H
#include <map>
#include <string>
#include "Expression.h"
#include "Quan.h"
#include "Object.h"

using namespace std;

class System;
class Link;

class Block: public Object
{
    public:
        Block();
        Block(System *parent);
        Block(const Block& other);
        Block& operator=(const Block& rhs);
        virtual ~Block();
        void AppendLink(Link* l, const Expression::loc &loc);


    protected:

    private:
        vector<Link*> links_from;
        vector<Link*> links_to;
};

#endif // BLOCK_H
