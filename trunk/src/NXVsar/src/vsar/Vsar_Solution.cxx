
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
    static CompAttrInfo attrExpInfo[] =
    {
        {TRAIN_SPEED_ID_NAME,       "", TRAIN_SPEED_EXP_NAME},
        {COMPUTE_TIME_STEP_ID_NAME, "", COMPUTE_TIME_STEP_EXP_NAME}
    };

    Solution::Solution() : BaseComponent(attrExpInfo, N_ELEMENTS(attrExpInfo))
    {
        //  Get result path name
        BaseProjectProperty *pPrjProp = Project::Instance()->GetProperty();

        std::string  strSimPrt(pPrjProp->GetProjectName().append("_s"));

        for (int idx = 0; idx < m_compAttrs.size(); idx++)
        {
            m_compAttrs[idx].m_partName = strSimPrt;
        }
    }

    Solution::~Solution()
    {

    }

    void Solution::OnInit()
    {
    }

    bool Solution::CanUpdateRailSlabFEModel() const
    {
        return false;
    }

    bool Solution::CanUpdateBraseFEModel() const
    {
        return false;
    }

    bool Solution::CanUpdateRailSlabConnection() const
    {
        return false;
    }

    bool Solution::CanUpdateBraseConnection() const
    {
        return false;
    }
}