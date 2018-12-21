#include "Link.h"

Link::Link():Object::Object()
{
    //ctor
}

Link::~Link()
{
    //dtor
}

Link::Link(const Link& other):Object::Object(other)
{
    //copy ctor
}

Link& Link::operator=(const Link& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    Object::operator=(rhs);
    //assignment operator
    return *this;
}



