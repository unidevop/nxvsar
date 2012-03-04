
#include <uf_defs.h>
#include <Vsar_Solution.hxx>

//#include <algorithm>
//#include <boost/bind.hpp>

//#include <NXOpen/Expression.hxx>

#include <Vsar_Names.hxx>
#include <Vsar_Init_Utils.hxx>
#include <Vsar_Project.hxx>

using namespace NXOpen;
using namespace boost;

//------------------------------------------------------------------------------
// Declaration of global variables
//------------------------------------------------------------------------------
namespace Vsar
{

    BaseSolution::~BaseSolution()
    {
    }

    void BaseSolution::OnInit()
    {
    }

    bool BaseSolution::HasGeometryDependency() const
    {
        return true;
    }

    bool BaseSolution::CanUpdateRailSlabFEModel() const
    {
        return false;
    }

    bool BaseSolution::CanUpdateBraseFEModel() const
    {
        return false;
    }

    bool BaseSolution::CanUpdateRailSlabConnection() const
    {
        return false;
    }

    bool BaseSolution::CanUpdateBraseConnection() const
    {
        return false;
    }


    static CompAttrInfo attrExpInfo[] =
    {
        {TRAIN_SPEED_ID_NAME,       "", TRAIN_SPEED_EXP_NAME},
        {COMPUTE_TIME_STEP_ID_NAME, "", COMPUTE_TIME_STEP_EXP_NAME}
    };

    ResponseSolution::ResponseSolution() : BaseSolution(attrExpInfo, N_ELEMENTS(attrExpInfo))
    {
        //  Get result path name
        BaseProjectProperty *pPrjProp = Project::Instance()->GetProperty();

        std::string  strSimPrt(pPrjProp->GetProjectName().append("_s"));

        for (unsigned int idx = 0; idx < m_compAttrs.size(); idx++)
        {
            m_compAttrs[idx].m_partName = strSimPrt;
        }
    }

    ResponseSolution::~ResponseSolution()
    {
    }


    NoiseSolution::NoiseSolution() : BaseSolution(NULL, 0)
    {
    }

    NoiseSolution::~NoiseSolution()
    {
    }
}