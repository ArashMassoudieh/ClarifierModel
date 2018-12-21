#include "Object.h"
#include "System.h"

Object::Object()
{
    //ctor
}

Object::~Object()
{
    //dtor
}

Object::Object(const Object& other)
{
    var = other.var;
    setting = other.setting;
    name = other.GetName();
    parent = other.GetParent();
	s_Block_no = other.s_Block_no;
	e_Block_no = other.e_Block_no;
}

Object& Object::operator=(const Object& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    setting = rhs.setting;
    name = rhs.GetName();
    parent = rhs.GetParent();
    var = rhs.var;
	s_Block_no = rhs.s_Block_no;
	e_Block_no = rhs.e_Block_no;
    return *this;
}

double Object::CalcVal(const string& s,const Expression::timing &tmg)
{
    if (var.Count(s)==1)
    {
        #ifdef Debug_mode
        cout<<"Object: "<<name<<" Variable: "<<s<< " Value: " << var[s].CalcVal(tmg) <<endl;
        #endif // Debug_mode
        return var[s].CalcVal(tmg);
    }
    else
    {
        last_error = "property '" + s + "' does not exist!";
        last_operation_success = false;
        return 0;
    }

}

double Object::GetVal(const string& s,const Expression::timing &tmg, bool limit)
{
    if (var.Count(s)==1)
    {
        #ifdef Debug_mode
//      cout<<"Object: "<<name<<" Variable: "<<s<< " Value: " << var[s].GetVal(tmg) <<endl;
        #endif // Debug_mode
        if (!limit)
            return var[s].GetVal(tmg);
        else
            return var[s].GetVal(tmg)*outflowlimitfactor;
    }
    else
    {
        last_error = "property '" + s + "' does not exist!";
        last_operation_success = false;
        return 0;
    }

}


bool Object::AddQnantity(const string &name,const Quan &Q)
{
    //cout<<int(var.find(name)->first)<<"   "<<int(var.end()) << endl;
    if (var.find(name)!=var.end() && !var.size())
    {
        last_error = "Variable " + name + " already exists! ";
        return false;
    }
    else
    {
		var.Append(name, Q);
        return true;
    }

}

bool Object::SetQuantities(MetaModel &m, const string& typ )
{
    if (m.Count(typ)==0)
    {
        last_error = "Type " + typ + "was not found";
		return false;
	}
    else
        var = *m[typ];
    for (map<string, Quan>::const_iterator s = var.begin(); s != var.end(); ++s)
        var[s->first].SetParent(this);
	return true;
}

bool Object::SetVal(const string& s, double value, const Expression::timing &tmg)
{
    if (var.find(s)!=var.end())
    {
        var[s].SetVal(value,tmg);
        return true;
    }
    else
    {
        last_error = "Variable " + s + " was not found!";
        return false;
    }
}

bool Object::SetVal(const string& s, const string & value, const Expression::timing &tmg)
{
    if (var.find(s)!=var.end())
    {
        var[s].SetVal(atof(value),tmg);
        return true;
    }
    else
    {
        last_error = "Variable " + s + " was not found!";
        return false;
    }
}

string Object::GetName() const
{
    return name;
}

bool Object::SetName(const string &s)
{
    name = s;
    return true;
}


Object* Object::GetConnectedBlock(Expression::loc l)
{
    if (l==Expression::loc::destination)
        return e_Block;
    if (l==Expression::loc::source)
        return s_Block;
    if (l==Expression::loc::self)
        return this;

}

void Object::SetConnectedBlock(Expression::loc l, const string &blockname)
{
    if (GetParent()->block(blockname)==nullptr)
    {
        last_error = "Block '" +blockname + "' does not exist!";
        GetParent()->AppendError(last_error);
    }
    else
    {
        if (l==Expression::loc::source)
        {
            s_Block = GetParent()->block(blockname);
            s_Block_no = GetParent()->blockid(blockname);
        }
        if (l==Expression::loc::destination)
        {
            e_Block = GetParent()->block(blockname);
            e_Block_no = GetParent()->blockid(blockname);
        }
    }

}

void Object::AppendError(const string &s)
{
	errors.push_back(s);
    last_error = s;
}

void Object::SetParent(System *s)
{
    parent = s;
}

Quan* Object::CorrespondingFlowVariable(const string &s)
{
    if (var.Count(Variable(s)->GetCorrespondingFlowVar())==0)
    {
        AppendError("Variable '" + s +"' does not exist!");
        return nullptr;
    }
    else
        return Variable(Variable(s)->GetCorrespondingFlowVar());
}

Quan* Object::Variable(const string &s)
{
    if (var.Count(s)==0)
    {
#ifdef Debug_mode
		cout << "In '" + name + "': " + "Variable '" + s + "' does not exist!" << endl;
#endif
		AppendError("Variable '" + s + "' does not exist!");
        return nullptr;
    }
    else
        return &var[s];
}

bool Object::Renew(const string & variable)
{
	if (!Variable(variable))
	{
		AppendError("Variable '" + variable + "' does not exist!");
		return false;
	}
	else
		Variable(variable)->Renew();
	return true;

}

bool Object::Update(const string & variable)
{
	if (!Variable(variable))
	{
		AppendError("Variable " + variable + " does not exist!");
		return false;
	}
	else
		Variable(variable)->Update();
	return true;

}

void Object::SetVariableParents()
{
	for (map<string, Quan>::const_iterator s = var.begin(); s != var.end(); ++s)
		var[s->first].SetParent(this);
}
