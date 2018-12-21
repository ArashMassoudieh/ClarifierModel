#include "MetaModel.h"
#include <json/json.h>

MetaModel::MetaModel()
{
    //ctor
}

MetaModel::~MetaModel()
{
    //dtor
}

MetaModel::MetaModel(const MetaModel& other)
{
    metamodel = other.metamodel;
}

MetaModel& MetaModel::operator=(const MetaModel& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    metamodel = rhs.metamodel;
    return *this;
}

bool MetaModel::Append(const string &s, const QuanSet &q)
{
    if (metamodel.count(s)>0)
    {
        last_error = "Object type " + s + " Already exist";
        errors.push_back(last_error);
        return false;
    }
    else
        metamodel[s] = q;
    return true;
}

QuanSet* MetaModel::operator[] (const string &typ)
{
    if (metamodel.count(typ)==0)
    {
        last_error = "Type " + typ + " was not found!";
        return nullptr;
    }
    else
        return &(metamodel[typ]);
}

QuanSet* MetaModel::GetItem(const string &typ)
{
    if (metamodel.count(typ)==0)
    {
        last_error = "Type " + typ + " was not found!";
        return nullptr;
    }
    else
        return &(metamodel[typ]);
}


bool MetaModel::GetFromJsonFile(const string &filename)
{
    Clear();
    Json::Value root;
    Json::Reader reader;

    std::ifstream file(filename);
	if (!file.good())
	{
		cout << "File " + filename + " was not found!";
		return false;
	}

    file >> root;

    if(!reader.parse(file, root, true)){
            //for some reason it always fails to parse
        std::cout  << "Failed to parse configuration\n"
                   << reader.getFormattedErrorMessages();
        last_error = "Failed to parse configuration\n";
    }


    for (Json::ValueIterator object_types=root.begin(); object_types!=root.end(); ++object_types)
    {
        QuanSet quanset;
        quanset.Name() = object_types.key().asString();
        for (Json::ValueIterator it=object_types->begin(); it!=object_types->end(); ++it)
        {
            if (it.key()=="icon")
                quanset.IconFileName() = (*it)["filename"].asString();
            if (it.key()=="type")
            {
                if ((*object_types)[it.key().asString()]=="block")
                    quanset.BlockLink = blocklink::block;
                if ((*object_types)[it.key().asString()]=="link")
                    quanset.BlockLink = blocklink::link;
            }
            else
            {
                Quan Q;
                Q.SetName(it.key().asString());
                if ((*it)["type"].asString()=="balance")
                {
                    Q.SetType(Quan::_type::balance);
                    Q.SetCorrespondingFlowVar((*it)["flow"].asString());
                }
                if ((*it)["type"].asString()=="constant")
                    Q.SetType(Quan::_type::constant);
                if ((*it)["type"].asString()=="expression")
                {
                    Q.SetType(Quan::_type::expression);
                    Q.SetExpression((*it)["expression"].asString());
                }
                if ((*it)["type"].asString()=="global")
                    Q.SetType(Quan::_type::global_quan);
                if ((*it)["type"].asString()=="timeseries")
                    Q.SetType(Quan::_type::timeseries);
                if ((*it)["type"].asString()=="value")
                    Q.SetType(Quan::_type::value);
                if (it->isMember("includeinoutput"))
                {
                    if ((*it)["includeinoutput"].asString()=="true")
                        Q.SetIncludeInOutput(true);
                    else
                        Q.SetIncludeInOutput(false);
                }
                else
                    Q.SetIncludeInOutput(false);
                if (it->isMember("description"))
                {
                    Q.Description() = (*it)["description"].asString();
                }

                if (it->isMember("unit"))
                    Q.Unit() = (*it)["unit"].asString();

                if (it->isMember("default_unit"))
                    Q.DefaultUnit() = (*it)["default_unit"].asString();

                if (it->isMember("default"))
                    Q.Default() = (*it)["default"].asString();

                if (it->isMember("delegate"))
                    Q.Delegate() = (*it)["delegate"].asString();

                if (it->isMember("category"))
                    Q.Category() = (*it)["category"].asString();

                if (it->isMember("input"))
                    Q.Input() = (*it)["input"].asString();

                if (it->isMember("experiment_dependent"))
                {   if ((*it)["experiment_dependent"].asString()=="Yes")
                        Q.ExperimentDependent() = true;
                    else
                        Q.ExperimentDependent() = false;

                }

                if (it->isMember("description_code"))
                    Q.DescriptionCode() = (*it)["description_code"].asString();

                if (it->isMember("criteria"))
                    Q.Criteria() = (*it)["criteria"].asString();

                if (it->isMember("warningerror"))
                    Q.WarningError() = (*it)["warningerror"].asString();

                if (it->isMember("warningmessage"))
                    Q.WarningMessage() = (*it)["warningmessage"].asString();

                if (it->isMember("inputtype"))
                    Q.InputType() = (*it)["inputtype"].asString();

                if (it->isMember("ask_user"))
                {    if (tolower((*it)["ask_user"].asString())=="true")
                        Q.AskFromUser() = true;
                }
                else
                   Q.AskFromUser() = false;

                //cout<<it.key().asString()<<endl;
                quanset.Append(it.key().asString(),Q);
                //AddQnantity(it.key().asString(), Q);
            }
        }
        Append(object_types.key().asString(),quanset);
    }
	return true;
}

void MetaModel::Clear()
{
    metamodel.clear();
    errors.clear();
    last_error = "";
}

string MetaModel::ToString(int _tabs)
{
    string out = tabs(_tabs) + "root:\n";
    out += tabs(_tabs) + "{\n";
    for (map<string, QuanSet>::iterator it = metamodel.begin(); it!=metamodel.end(); it++)
        out += metamodel[it->first].ToString(_tabs+1) + "\n";
    out += "}\n";
    return out;
}
