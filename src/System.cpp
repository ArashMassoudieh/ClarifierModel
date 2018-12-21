#include "System.h"
#include <fstream>
#include <json/json.h>
#pragma warning(disable : 4996)

#ifdef QT_version
#include "node.h"
#include "edge.h"
#include "GWidget.h"
#include "QDebug"
#include "logwindow.h"
#endif

System::System():Object::Object()
{
    //ctor
}

#ifdef QT_version
System::System(GraphWidget* diagramviewer,runtimeWindow *_rtw):Object::Object()
{
    diagramview = diagramviewer;
    rtw = _rtw;
    GetModelConfiguration();

}
System::System(GraphWidget* diagramviewer,runtimeWindow *_rtw, const string &modelfilename):Object::Object()
{
    diagramview = diagramviewer;
    rtw = _rtw;
    GetQuanTemplate(modelfilename);
    GetModelConfiguration();

}

#endif

System::~System()
{
    //dtor
}

System::System(const System& other):Object::Object(other)
{
    blocks = other.blocks;
    links = other.links;
}

System& System::operator=(const System& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    Object::operator=(rhs);
    blocks = rhs.blocks;
    links = rhs.links;
    return *this;
}

bool System::AddBlock(Block &blk)
{
    blocks.push_back(blk);
    block(blk.GetName())->SetParent(this);
    block(blk.GetName())->SetQuantities(metamodel, blk.GetType());
	return true;
}

bool System::AddLink(Link &lnk, const string &source, const string &destination)
{
    links.push_back(lnk);
    link(lnk.GetName())->SetParent(this);
    link(lnk.GetName())->SetConnectedBlock(Expression::loc::source, source);
    link(lnk.GetName())->SetConnectedBlock(Expression::loc::destination, destination);
    block(source)->AppendLink(link(lnk.GetName()),Expression::loc::source);
    block(destination)->AppendLink(link(lnk.GetName()),Expression::loc::destination);
	link(lnk.GetName())->SetQuantities(metamodel, lnk.GetType());
	return true;
}

Block *System::block(const string &s)
{
    for (unsigned int i=0; i<blocks.size(); i++)
        if (blocks[i].GetName() == s) return &blocks[i];

    AppendError("Block " + s + " was not found");
    return nullptr;
}

int System::blockid(const string &s)
{
    for (unsigned int i=0; i<blocks.size(); i++)
        if (blocks[i].GetName() == s) return int(i);

    AppendError("Block " + s + " was not found");
    return -1;
}

int System::linkid(const string &s)
{
    for (unsigned int i=0; i<links.size(); i++)
        if (links[i].GetName() == s) return int(i);

    AppendError("Link " + s + " was not found");
    return -1;
}

Link *System::link(const string &s)
{
    for (unsigned int i=0; i<links.size(); i++)
        if (links[i].GetName() == s) return &links[i];

    AppendError("Link " + s + " was not found");
    return nullptr;
}

bool System::GetQuanTemplate(const string &filename)
{
    metamodel.GetFromJsonFile(filename);
    TransferQuantitiesFromMetaModel();
    return true;
}

void System::CopyQuansToMembers()
{
    for (unsigned int i=0; i<blocks.size(); i++)
    {
        blocks[i].SetQuantities(metamodel,blocks[i].GetType());
    }
    for (unsigned int i=0; i<links.size(); i++)
        links[i].SetQuantities(metamodel,blocks[i].GetType());

}

bool System::OneStepSolve()
{
	return true;
}

bool System::Solve(const string &variable)
{
    if (LogWindow())
    {
        LogWindow()->append("Simulation started!");
    }
    InitiateOutputs();
    PopulateOutputs();

    SolverTempVars.dt = SimulationParameters.dt0;
    SolverTempVars.t = SimulationParameters.tstart;

    while (SolverTempVars.t<SimulationParameters.tend+SolverTempVars.dt)
    {
        #ifdef Debug_mode
        cout << "t = " << SolverSettings.t << ", dt = " << SolverSettings.dt << ", SolverTempVars.numiterations =" << SolverTempVars.numiterations << endl;
        #endif // Debug_mode
        if (rtw)
        {
            updateProgress(false);
        }
        bool success = OneStepSolve(variable);
        if (!success)
        {
            #ifdef Debug_mode
            cout<<"failed!"<<endl;
            #endif // Debug_mode
            SolverTempVars.dt *= SolverSettings.NR_timestep_reduction_factor_fail;
            SolverTempVars.updatejacobian = true;
        }
        else
        {
            SolverTempVars.t += SolverTempVars.dt;
            if (SolverTempVars.numiterations>SolverSettings.NR_niteration_upper)
            {
                SolverTempVars.dt = max(SolverTempVars.dt*SolverSettings.NR_timestep_reduction_factor,SolverSettings.minimum_timestep);
                SolverTempVars.updatejacobian = true;
            }
            if (SolverTempVars.numiterations<SolverSettings.NR_niteration_lower)
                SolverTempVars.dt /= SolverSettings.NR_timestep_reduction_factor;
            PopulateOutputs();
            Update(variable);
        }

    }

    updateProgress(true);
    if (LogWindow())
    {
        LogWindow()->append("Simulation finished!");
    }
    return true;
}

#ifdef QT_version
void System::updateProgress(bool finished)
{
    // t, dtt (graph), epoch_count
    if (rtw != nullptr)
    {
        QMap<QString, QVariant> vars;
        vars["mode"] = "forward";
        if (finished)
        {
            vars["progress"] = 100;
            vars["finished"] = true;
        }
        else
        {
            int progress;
            progress = 100.0*(SolverTempVars.t - SimulationParameters.tstart) / (SimulationParameters.tend - SimulationParameters.tstart);
            vars["t"] = SolverTempVars.t;
            vars["progress"] = progress;
            vars["dtt"] = SolverTempVars.dt;
            vars["epoch count"] = SolverTempVars.epoch_count;
            QString reason = QString::fromStdString(SolverTempVars.fail_reason);
            ////qDebug() << reason;
            if (!reason.toLower().contains("none"))
                vars["label"] = reason;
            ////qDebug()<< t<<dtt;

            if (rtw->sln_dtl_active)
                if (!reason.toLower().contains("none"))
                    rtw->slndetails_append(QString::number(SolverTempVars.epoch_count) + ":" + reason + ", time step size: " + QString::number(SolverTempVars.dt));
        }
        rtw->update(vars);
        if (finished)
        {
            //QMessageBox::StandardButton reply;
            //QMessageBox::question(runtimewindow, "Simulation Ended", "Simulation Finished!", QMessageBox::Ok);
        }
    }
}
#endif

bool System::SetProp(const string &s, const double &val)
{
    if (s=="cn_weight")
    {   SolverSettings.C_N_weight = val; return true;}
    if (s=="nr_tolerance")
    {   SolverSettings.NRtolerance = val; return true;}
    if (s=="nr_coeff_reduction_factor")
    {   SolverSettings.NR_coeff_reduction_factor = val; return true;}
    if (s=="nr_timestep_reduction_factor")
    {   SolverSettings.NR_timestep_reduction_factor = val; return true;}
    if (s=="nr_timestep_reduction_factor_fail")
    {   SolverSettings.NR_timestep_reduction_factor_fail = val; return true;}
    if (s=="minimum_timestep")
    {   SolverSettings.minimum_timestep = val; return true;}
    if (s=="nr_niteration_lower")
    {   SolverSettings.NR_niteration_lower=int(val); return true;}
    if (s=="nr_niteration_upper")
    {   SolverSettings.NR_niteration_upper=int(val); return true;}
    if (s=="nr_niteration_max")
    {   SolverSettings.NR_niteration_max=int(val); return true;}
    if (s=="make_results_uniform")
    {   SolverSettings.makeresultsuniform = bool(val); return true;}

    if (s=="tstart")
    {   SimulationParameters.tstart = val; return true;}
    if (s=="tend")
    {   SimulationParameters.tend = val; return true;}
    if (s=="tend")
    {   SimulationParameters.dt0 = val; return true;}

    return false;
}

void System::InitiateOutputs()
{
    Outputs.AllOutputs.clear();
    for (unsigned int i=0; i<blocks.size(); i++)
    {
        for (map<string, Quan>::iterator it = blocks[i].GetVars()->begin(); it != blocks[i].GetVars()->end(); it++)
            if (it->second.IncludeInOutput())
                Outputs.AllOutputs.append(CBTC(), blocks[i].GetName() + "_" + it->first);
    }

    for (unsigned int i=0; i<links.size(); i++)
    {
        for (map<string, Quan>::iterator it = links[i].GetVars()->begin(); it != links[i].GetVars()->end(); it++)
            if (it->second.IncludeInOutput())
                Outputs.AllOutputs.append(CBTC(), links[i].GetName() + "_" + it->first);
    }

}


void System::PopulateOutputs()
{
    for (unsigned int i=0; i<blocks.size(); i++)
    {
        for (map<string, Quan>::iterator it = blocks[i].GetVars()->begin(); it != blocks[i].GetVars()->end(); it++)
            if (it->second.IncludeInOutput())
                Outputs.AllOutputs[blocks[i].GetName() + "_" + it->first].append(SolverTempVars.t,blocks[i].GetVal(it->first,Expression::timing::present));
    }

    for (unsigned int i=0; i<links.size(); i++)
    {
        for (map<string, Quan>::iterator it = links[i].GetVars()->begin(); it != links[i].GetVars()->end(); it++)
            if (it->second.IncludeInOutput())
            {
                Outputs.AllOutputs[links[i].GetName() + "_" + it->first].append(SolverTempVars.t,links[i].GetVal(it->first,Expression::timing::present,true));
            }
    }

}


bool System::OneStepSolve(const string &variable)
{
	Renew(variable);
	#ifdef Debug_mode
//  cout << "Calculating Residuals" <<endl;
    #endif // Debug_mode
    CVector_arma X = GetStateVariables(variable, Expression::timing::past);
    CVector_arma X_past = X;
//  cout<<"X: " << X.toString()<<endl;
    CVector_arma F = GetResiduals(variable, X);
//  cout<<"F: " << F.toString()<<endl;
    double err_ini = F.norm2();
    double err;
    double err_p = err = err_ini;
    SolverTempVars.numiterations = 0;
    bool switchvartonegpos = true;
    int attempts = 0;
    while (attempts<2 && switchvartonegpos)
    {
        while (err/err_ini>SolverSettings.NRtolerance)
        {
            SolverTempVars.numiterations++;
            if (SolverTempVars.updatejacobian)
            {
                SolverTempVars.Inverse_Jacobian = Invert(Jacobian(variable,X));
                SolverTempVars.updatejacobian = false;
                SolverTempVars.NR_coefficient = 1;
            }
            X = X - SolverTempVars.NR_coefficient*SolverTempVars.Inverse_Jacobian*F;
            F = GetResiduals(variable, X);
            err_p = err;
            err = F.norm2();
            #ifdef Debug_mode
            cout << err << endl;
            #endif // Debug_mode
            if (err>err_p)
                SolverTempVars.NR_coefficient*=SolverSettings.NR_coeff_reduction_factor;
            //else
            //    SolverTempVars.NR_coefficient/=SolverSettings.NR_coeff_reduction_factor;
            if (SolverTempVars.numiterations>SolverSettings.NR_niteration_max)
                return false;
        }
        switchvartonegpos = false;
        for (unsigned int i=0; i<blocks.size(); i++)
        {
            if (X[i]<-1e-13 && !blocks[i].GetLimitedOutflow())
            {
                blocks[i].SetLimitedOutflow(true);
                switchvartonegpos = true;
                SolverTempVars.updatejacobian = true;
            }
            else if (X[i]>1 && blocks[i].GetLimitedOutflow())
            {
                blocks[i].SetLimitedOutflow(false);
                switchvartonegpos = true;
                SolverTempVars.updatejacobian = true;
            }
        }
    }

	#ifdef Debug_mode
//	CMatrix_arma M = Jacobian("Storage",X);
//	M.writetofile("M.txt");
	#endif // Debug_mode
	return true;
}

bool System::Renew(const string & variable)
{
	bool out = true;
	for (unsigned int i = 0; i < blocks.size(); i++)
		out &= blocks[i].Renew(variable);

	for (unsigned int i = 0; i < links.size(); i++)
		out &= links[i].Renew(variable);

	return out;
}

bool System::Update(const string & variable)
{
	bool out = true;
	for (unsigned int i = 0; i < blocks.size(); i++)
		out &= blocks[i].Update(variable);

	for (unsigned int i = 0; i < links.size(); i++)
		out &= links[i].Update(variable);

	return out;
}


CVector_arma System::CalcStateVariables(const string &variable, const Expression::timing &tmg)
{
    CVector_arma X(blocks.size());
    for (unsigned int i=0; i<blocks.size(); i++)
        X[i] = blocks[i].CalcVal(variable,tmg);
    return X;
}

CVector_arma System::GetStateVariables(const string &variable, const Expression::timing &tmg)
{
    CVector_arma X(blocks.size());
    for (unsigned int i=0; i<blocks.size(); i++)
        X[i] = blocks[i].GetVal(variable,tmg);
    return X;
}

void System::SetStateVariables(const string &variable, CVector_arma &X, const Expression::timing &tmg)
{
    for (unsigned int i=0; i<blocks.size(); i++)
    {
        blocks[i].SetVal(variable,X[i],tmg);
        #ifdef Debug_mode
//      cout<<"Variable :"<< variable << "in " + blocks[i].GetName() << " was set to " + numbertostring(blocks[i].GetVal(variable,tmg)) << endl;
        #endif // Debug_mode
    }
}


CVector_arma System::GetResiduals(const string &variable, CVector_arma &X)
{
    CVector_arma F(blocks.size());
    SetStateVariables(variable,X,Expression::timing::present);
    CalculateFlows(Variable(variable)->GetCorrespondingFlowVar(),Expression::timing::present);

    for (unsigned int i=0; i<blocks.size(); i++)
    {
        if (blocks[i].GetLimitedOutflow())
        {
            blocks[i].SetOutflowLimitFactor(X[i]);
            blocks[i].SetVal(variable,0);
            F[i] = (0-blocks[i].GetVal(variable,Expression::timing::past))/dt();
        }
        else
            F[i] = (X[i]-blocks[i].GetVal(variable,Expression::timing::past))/dt();
    }


    for (unsigned int i=0; i<links.size(); i++)
    {
        if (blocks[links[i].s_Block_No()].GetLimitedOutflow() && links[i].GetVal(blocks[links[i].s_Block_No()].Variable(variable)->GetCorrespondingFlowVar(),Expression::timing::present)>0)
            links[i].SetOutflowLimitFactor(blocks[links[i].s_Block_No()].GetOutflowLimitFactor());
        if (blocks[links[i].e_Block_No()].GetLimitedOutflow() && links[i].GetVal(blocks[links[i].e_Block_No()].Variable(variable)->GetCorrespondingFlowVar(),Expression::timing::present)<0)
            links[i].SetOutflowLimitFactor(blocks[links[i].e_Block_No()].GetOutflowLimitFactor());

    }

    for (unsigned int i=0; i<links.size(); i++)
    {
        F[links[i].s_Block_No()] += links[i].GetVal(blocks[links[i].s_Block_No()].Variable(variable)->GetCorrespondingFlowVar(),Expression::timing::present)*links[i].GetOutflowLimitFactor();
        F[links[i].e_Block_No()] -= links[i].GetVal(blocks[links[i].s_Block_No()].Variable(variable)->GetCorrespondingFlowVar(),Expression::timing::present)*links[i].GetOutflowLimitFactor();
    }
    return F;
}

bool System::CalculateFlows(const string &var, const Expression::timing &tmg)
{
    for (int i=0; i<links.size(); i++)
    {
        links[i].SetVal(var,links[i].CalcVal(var,tmg));
    }
	return true;
}

CMatrix_arma System::Jacobian(const string &variable, CVector_arma &X)
{
    CMatrix_arma M(X.num);

    CVector_arma F0 = GetResiduals(variable, X);
    for (int i=0; i < X.num; i++)
    {
        CVector_arma V = Jacobian(variable, X, F0, i);
        for (int j=0; j<X.num; j++)
            M(i,j) = V[j];
    }

  return Transpose(M);
}


CVector_arma System::Jacobian(const string &variable, CVector_arma &V, CVector_arma &F0, int i)  //Works also w/o reference (&)
{
  double epsilon;
  epsilon = -1e-6;
  CVector_arma V1(V);
  V1[i] += epsilon;
  #ifdef Debug_mode
//cout<<i<<":"<<V1.toString()<<endl;
  #endif // Debug_mode
  CVector_arma F1;
  F1 = GetResiduals(variable,V1);
  CVector_arma grad = (F1 - F0) / epsilon;
  if (grad.norm2() == 0)
  {
    epsilon = 1e-6;
    V1 = V;
    V1[i] += epsilon;
    F1 = GetResiduals(variable,V1);
    grad = (F1 - F0) / epsilon;
  }
  return grad;

}

void System::SetVariableParents()
{
	for (unsigned int i = 0; i < links.size(); i++)
	{
		links[i].SetVariableParents();
        links[i].Set_s_Block(&blocks[int(links[i].s_Block_No())]);
		links[i].Set_e_Block(&blocks[links[i].e_Block_No()]);
	}

	for (unsigned int i = 0; i < blocks.size(); i++)
	{
		blocks[i].SetVariableParents();

	}
}

vector<string> System::GetAllBlockTypes()
{
    vector<string> out;
    for (map<string, QuanSet>::iterator it = metamodel.GetMetaModel()->begin(); it != metamodel.GetMetaModel()->end(); it++)
        if (it->second.BlockLink == blocklink::block)
        {
            cout<<it->first<<endl;
            out.push_back(it->first);
        }

    return out;

}

vector<string> System::GetAllLinkTypes()
{
    vector<string> out;
    for (map<string, QuanSet>::iterator it = metamodel.GetMetaModel()->begin(); it != metamodel.GetMetaModel()->end(); it++)
        if (it->second.BlockLink == blocklink::link)
        {
            cout<<it->first<<endl;
            out.push_back(it->first);
        }

    return out;
}

void System::clear()
{
    blocks.clear();
    links.clear();
    Outputs.AllOutputs.clear();
    Outputs.ObservedOutputs.clear();
}

void System::TransferQuantitiesFromMetaModel()
{
    vector<string> out;
    for (map<string, QuanSet>::iterator it = metamodel.GetMetaModel()->begin(); it != metamodel.GetMetaModel()->end(); it++)
        GetVars()->Append(it->second);
}

#ifdef QT_version
void System::GetModelConfiguration()
{
    QList <Node*> nodes = diagramview->Nodes();
    QStringList nodenames_sorted = diagramview->nodeNames();
    nodenames_sorted.sort();

    for (int i = 0; i < nodenames_sorted.count(); i++)
    {
        Node* n = nodes[diagramview->nodeNames().indexOf(nodenames_sorted[i])];
        Block B;
        B.SetName(n->Name().toStdString());
        B.SetType(n->GetObjectType().toStdString());
        AddBlock(B);
        QStringList codes = n->codes();

        foreach (mProp mP , n->getmList(n->objectType).GetList())
        {
            QString code = mP.VariableCode;
            if (!n->val(code).isEmpty() && n->val(code) != ".")
                block(n->Name().toStdString())->SetVal(code.toStdString(), n->val(code).toFloat());
            if (mP.Delegate == "Browser" && !n->val(code).isEmpty() && n->val(code) != ".")
                block(n->Name().toStdString())->Variable(code.toStdString())->SetTimeSeries(fullFilename(n->val(code), diagramview->modelPathname()).toStdString()+n->val(code).toQString().toStdString());
            qDebug()<<code<<"  "<<QString::fromStdString(block(n->Name().toStdString())->GetName())<<"  "<<QString::fromStdString(block(n->Name().toStdString())->GetType())<<"    "<<block(n->Name().toStdString())->GetVal(code.toStdString());
        }



/*      foreach (QString code , n->codes()) //Parameters
        {
            if (gw->EntityNames("Parameter").contains(n->val(code).toQString()))
            {
                if (lookup_parameters(n->val(code).toStdString()) != -1) {
                    parameters()[lookup_parameters(n->val(code).toStdString())].location.push_back(Blocks.size() - 1);  // Check for everything
                    parameters()[lookup_parameters(n->val(code).toStdString())].conversion_factor.push_back(n->val(code).conversionCoefficient(n->val(code).unit, n->val(code).defaultUnit));
                    parameters()[lookup_parameters(n->val(code).toStdString())].quan.push_back(code.toStdString());
                    parameters()[lookup_parameters(n->val(code).toStdString())].location_type.push_back(0);
                    parameters()[lookup_parameters(n->val(code).toStdString())].experiment_id.push_back(name);
                }
            }
        } //Controller
        foreach (QString code , n->codes())
        {
            if (gw->EntityNames("Controller").contains(n->val(code).toQString()))
            {
                if (lookup_controllers(n->val(code).toStdString()) != -1) {
                    controllers()[lookup_controllers(n->val(code).toStdString())].application_spec.location.push_back(Blocks.size() - 1);  // Check for everything
                    controllers()[lookup_controllers(n->val(code).toStdString())].application_spec.conversion_factor.push_back(n->val(code).conversionCoefficient(n->val(code).unit, n->val(code).defaultUnit));
                    controllers()[lookup_controllers(n->val(code).toStdString())].application_spec.quan.push_back(code.toStdString());
                    controllers()[lookup_controllers(n->val(code).toStdString())].application_spec.location_type.push_back(0);
                    controllers()[lookup_controllers(n->val(code).toStdString())].application_spec.experiment_id.push_back(name);
                }
            }
        }
*/
    }

    QList <Edge*> edges = diagramview->Edges();
    QStringList edgenames_sorted = diagramview->edgeNames();
    edgenames_sorted.sort();
//#pragma omp parallel for
    for (int i = 0; i < edges.count(); i++)
    {
        Edge *e = edges[diagramview->edgeNames().indexOf(edgenames_sorted[i])];
        Link L;
        L.SetName(e->Name().toStdString());
        L.SetType(e->GetObjectType().toStdString());
        AddLink(L,e->sourceNode()->Name().toStdString(),e->destNode()->Name().toStdString());

        foreach (mProp mP ,e->getmList(e->objectType).GetList())
        {   QString code = mP.VariableCode;
            if (!e->val(code).isEmpty() && e->val(code) != ".") link(e->Name().toStdString())->SetVal(code.toStdString(), e->val(code).toFloat());
            if (mP.Delegate == "Browser" && !e->val(code).isEmpty() && e->val(code) != ".")
                link(e->Name().toStdString())->Variable(code.toStdString())->SetTimeSeries(fullFilename(e->val(code), diagramview->modelPathname()).toStdString());

        }


        //progress->setValue(progress->value() + 1);

/*
        foreach (QString code , e->codes()) //Parameters
        {
            if (gw->EntityNames("Parameter").contains(e->val(code).toQString()))
            {
                if (lookup_parameters(e->val(code).toStdString()) != -1) {
                    parameters()[lookup_parameters(e->val(code).toStdString())].location.push_back(Connectors.size() - 1);  // Check for everything
                    parameters()[lookup_parameters(e->val(code).toStdString())].conversion_factor.push_back(e->val(code).conversionCoefficient(e->val(code).unit, e->val(code).defaultUnit));
                    parameters()[lookup_parameters(e->val(code).toStdString())].quan.push_back(code.toStdString());
                    parameters()[lookup_parameters(e->val(code).toStdString())].location_type.push_back(1);
                    parameters()[lookup_parameters(e->val(code).toStdString())].experiment_id.push_back(name);
                }
            }
        }
        foreach (QString code , e->codes()) //Controllers
        {
            if (gw->EntityNames("Controller").contains(e->val(code).toQString()))
            {
                if (lookup_controllers(e->val(code).toStdString()) != -1) {
                    controllers()[lookup_controllers(e->val(code).toStdString())].application_spec.location.push_back(Connectors.size() - 1);  // Check for everything
                    controllers()[lookup_controllers(e->val(code).toStdString())].application_spec.conversion_factor.push_back(e->val(code).conversionCoefficient(e->val(code).unit, e->val(code).defaultUnit));
                    controllers()[lookup_controllers(e->val(code).toStdString())].application_spec.quan.push_back(code.toStdString());
                    controllers()[lookup_controllers(e->val(code).toStdString())].application_spec.location_type.push_back(1);
                    controllers()[lookup_controllers(e->val(code).toStdString())].application_spec.experiment_id.push_back(name);
                }
            }
        }
*/


    }
}
#endif
