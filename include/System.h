#pragma once

#include "Block.h"
#include "Link.h"
#include "Object.h"
#include "Vector_arma.h"
#include "Matrix_arma.h"
#include "MetaModel.h"
#include "BTCSet.h"

#ifdef QT_version
    #include "runtimeWindow.h"
    class GWidget;
    class logWindow;
#endif
struct solversettings
{
    double C_N_weight; //Crank-Nicholson Weight
    double NRtolerance = 1e-6; //Newton Raphson Tolerance
    double NR_coeff_reduction_factor = 0.8; //The coefficient to reduce the Newton-Raphson coefficient
    double NR_timestep_reduction_factor = 0.75;
    double NR_timestep_reduction_factor_fail = 0.3;
    double minimum_timestep = 1e-7;
    int NR_niteration_lower=20;
    int NR_niteration_upper=40;
    int NR_niteration_max=100;
    bool makeresultsuniform = false;

};

struct outputs
{
    CBTCSet AllOutputs;
    CBTCSet ObservedOutputs;
};

struct solvertemporaryvars
{
    CMatrix_arma Inverse_Jacobian;
    double NR_coefficient = 1;
    bool updatejacobian = true;
    int numiterations;
    int epoch_count=0;
    string fail_reason;
    double t;
    double dt;


};

struct simulationparameters
{
    double tstart = 0; //start time of simulation
    double tend = 1; //end time of simulation
    double dt0 = 0.01; // initial time-step size
};

class System: public Object
{
    public:
        System();
#ifdef QT_version
        System(GraphWidget* DiagramViewer,runtimeWindow *rtw);
        System(GraphWidget* diagramviewer,runtimeWindow *_rtw, const string &modelfilename);
        void GetModelConfiguration();
#endif
        virtual ~System();
        System(const System& other);
        System& operator=(const System& other);
        double &GetTime() {return SolverTempVars.t;}
        bool AddBlock(Block &blk);
        bool AddLink(Link &lnk, const string &source, const string &destination);
        Block *block(const string &s);
        Link *link(const string &s);
        int blockid(const string &s);
        int linkid(const string &s);
        bool GetQuanTemplate(const string &filename);
        void CopyQuansToMembers();
        double &dt() {return SolverTempVars.dt;}
        double &tend() {return SimulationParameters.tend;}
        double &tstart() {return SimulationParameters.tstart;}
        bool OneStepSolve(const string &s);
		bool Renew(const string &variable);
		bool Update(const string &variable);
		bool Solve(const string &variable);
		bool SetProp(const string &s, const double &val);
        CBTCSet& GetOutputs() {return Outputs.AllOutputs;}
        vector<string> GetAllBlockTypes();
        vector<string> GetAllLinkTypes();
        void SetVariableParents();
        MetaModel *GetMetaModel() {return  &metamodel;}
        QuanSet* GetModel(const string &type) {return metamodel[type];}
        void clear();
        int lookup_observation(const string &s) {return 0;}
        int EpochCount() {return SolverTempVars.epoch_count;}
#ifdef QT_version
        logWindow *LogWindow() {return logwindow;}
        void SetLogWindow(logWindow *lgwnd) {logwindow=lgwnd;}
        bool stop_triggered = false;
#endif

    protected:

    private:
        vector<Block> blocks;
        vector<Link> links;
        string last_error;
        MetaModel metamodel;
        CVector_arma GetResiduals(const string &variable, CVector_arma &X);
        CVector_arma CalcStateVariables(const string &variable, const Expression::timing &tmg = Expression::timing::past);
        CVector_arma GetStateVariables(const string &variable, const Expression::timing &tmg = Expression::timing::past);
        solversettings SolverSettings;
        simulationparameters SimulationParameters;
        bool OneStepSolve();
        CMatrix_arma Jacobian(const string &variale, CVector_arma &X);
        CVector_arma Jacobian(const string &variable, CVector_arma &V, CVector_arma &F0, int i);  //Works also w/o reference (&)
        bool CalculateFlows(const string &var, const Expression::timing &tmg = Expression::timing::present);
        void SetStateVariables(const string &variable, CVector_arma &X, const Expression::timing &tmg = Expression::timing::present);
        solvertemporaryvars SolverTempVars;
        outputs Outputs;
        void InitiateOutputs();
        void PopulateOutputs();
        void TransferQuantitiesFromMetaModel();

#ifdef QT_version
        GraphWidget *diagramview;
        runtimeWindow *rtw;
        void updateProgress(bool finished);
        logWindow *logwindow = nullptr;
#endif
};


