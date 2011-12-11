
#include <uf_defs.h>
#include <Vsar_Slab.hxx>

//#include <boost/cast.hpp>

#include <NXOpen/Expression.hxx>

#include <Vsar_Names.hxx>
#include <Vsar_Init_Utils.hxx>

using namespace NXOpen;
using namespace boost;

//------------------------------------------------------------------------------
// Declaration of global variables
//------------------------------------------------------------------------------
namespace Vsar
{
    static CompAttrInfo attrExpInfo[] =
    {
        {SLAB_SUPPORT_COUNT_ID_NAME,    SLAB_PRT_PART_NAME,       SLAB_SUPPORT_COUNT_EXP_NAME},

        {FASTENER_STIFFNESS_ID_NAME,    RAIL_SLAB_FEM_PART_NAME, SLAB_FASTENER_STIFFNESS_EXP_NAME},
        {FASTENER_DAMPING_ID_NAME,      RAIL_SLAB_FEM_PART_NAME, SLAB_FASTENER_DAMPING_EXP_NAME},
        {MASS_RATIO_ID_NAME,            RAIL_SLAB_FEM_PART_NAME, SLAB_MASS_RATIO_EXP_NAME},
        {STIFFNESS_RATIO_ID_NAME,       RAIL_SLAB_FEM_PART_NAME, SLAB_STIFFNESS_RATIO_EXP_NAME},
        {DAMPING_RATIO_ID_NAME,         RAIL_SLAB_FEM_PART_NAME, SLAB_DAMPING_RATIO_EXP_NAME},
        {ELASTIC_MODULUS_ID_NAME,       RAIL_SLAB_FEM_PART_NAME, SLAB_ELASTIC_MODULUS_EXP_NAME},
        {POISSON_RATIO_ID_NAME,         RAIL_SLAB_FEM_PART_NAME, SLAB_POISSON_RATIO_EXP_NAME},
    };

    Slab::Slab() : BaseComponent(attrExpInfo, N_ELEMENTS(attrExpInfo)),
        m_oldSupportCount(0)
    {
    }

    Slab::~Slab()
    {

    }

    void Slab::OnInit()
    {
        m_oldSupportCount = GetSupportCount();
    }

    int Slab::GetSupportCount() const
    {
        Expression *pSupportCntExp = NULL;

        if (!m_compAttrs.empty())
            pSupportCntExp = GetExpression(m_compAttrs[0].m_partName, m_compAttrs[0].m_expName);

        return pSupportCntExp ? pSupportCntExp->IntegerValue() : 0;
    }

    bool Slab::CanUpdateRailSlabFEModel() const
    {
        return false;
    }

    bool Slab::CanUpdateBraseFEModel() const
    {
        return false;
    }

    bool Slab::CanUpdateRailSlabConnection() const
    {
        return false;
    }

    bool Slab::CanUpdateBraseConnection() const
    {
        return GetSupportCount() != m_oldSupportCount;
    }
}