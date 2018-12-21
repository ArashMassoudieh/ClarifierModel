#ifndef OBJECT_H
#define OBJECT_H

#include <string>
#include <vector>
#include <map>
#include "Quan.h"
#include "MetaModel.h"

using namespace std;

class Object
{
    public:
        Object();
        virtual ~Object();
        Object(const Object& other);
        Object& operator=(const Object& other);
        double CalcVal(const string& s, const Expression::timing &tmg=Expression::timing::past);
        double GetVal(const string& s, const Expression::timing &tmg=Expression::timing::past, bool limit=false);
        bool AddQnantity(const string &name,const Quan &Q);
        bool SetQuantities(MetaModel &m, const string& typ);
        bool SetVal(const string& s, double value, const Expression::timing &tmg = Expression::timing::both);
        bool SetVal(const string& s, const string & value, const Expression::timing &tmg = Expression::timing::both);
        double GetProperty(const string&);
        System *GetParent() const {return parent;}
        string GetName() const;
        bool SetName(const string &_name);
        Object* GetConnectedBlock(Expression::loc l);
        void SetConnectedBlock(Expression::loc l, const string &blockname);
        void AppendError(const string &s);
        void SetParent(System *s);
        Quan* CorrespondingFlowVariable(const string &s);
        Quan* Variable(const string &s);
        void SetType(const string &typ) {type = typ;}
        string GetType() {return type;}
        int s_Block_No() {return s_Block_no; }
        int e_Block_No() {return e_Block_no; }
        void Set_s_Block(Object *O) { s_Block = O; }
        void Set_e_Block(Object *O) { e_Block = O; }
		bool Renew(const string &variable);
		bool Update(const string &variable);
		QuanSet* GetVars()
            {
                return &var;
            }
        void SetOutflowLimitFactor(const double &val) {outflowlimitfactor = val;}
        double GetOutflowLimitFactor() {return outflowlimitfactor;}
        void SetLimitedOutflow(bool x) {limitoutflow = x;}
        bool GetLimitedOutflow() {return limitoutflow;}
		void SetVariableParents();
    protected:

    private:
        QuanSet var;
        vector<string> errors;
        string last_error;
        bool last_operation_success;
        map<string, string> setting;
        System *parent;
        string name;
        Object *s_Block;
        Object *e_Block;
        int s_Block_no, e_Block_no;
        string type;
        double outflowlimitfactor = 1;
        bool limitoutflow = false;

};

#endif // OBJECT_H
