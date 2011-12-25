
#include <uf_defs.h>
#include <Vsar_Bridge.hxx>

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
        {WIDTH_ID_NAME,                 BEAM_PRT_PART_NAME,     WIDTH_EXP_NAME},
        {HEIGHT_ID_NAME,                BEAM_PRT_PART_NAME,     HEIGHT_EXP_NAME},
        {SPAN_COUNT_ID_NAME,            BRIDGE_PRT_PART_NAME,   SPAN_COUNT_EXP_NAME},

        {ELASTIC_MODULUS_ID_NAME,       BRIDGE_FEM_PART_NAME,   BRIDGE_ELASTIC_MODULUS_EXP_NAME},
        {BRIDGE_MASS_DENSITY_ID_NAME,   BRIDGE_FEM_PART_NAME,   BRIDGE_MASS_DENSITY_EXP_NAME},
        {POISSON_RATIO_ID_NAME,         BRIDGE_FEM_PART_NAME,   BRIDGE_POISSON_RATIO_EXP_NAME}
    };

    Bridge::Bridge() : BaseComponent(attrExpInfo, N_ELEMENTS(attrExpInfo)),
        m_oldHeight(1), m_oldSpanCount(2)
    {
    }

    Bridge::~Bridge()
    {

    }

    void Bridge::OnInit()
    {
        m_oldHeight    = GetHeight();
        m_oldSpanCount = GetSpanCount();
    }

    int Bridge::GetSpanCount() const
    {
        Expression *pSupportCntExp = NULL;

        if (!m_compAttrs.empty())
            pSupportCntExp = GetExpression(m_compAttrs[2].m_partName, m_compAttrs[2].m_expName);

        return pSupportCntExp ? pSupportCntExp->IntegerValue() : 0;
    }

    double Bridge::GetHeight() const
    {
        Expression *pSupportCntExp = NULL;

        if (!m_compAttrs.empty())
            pSupportCntExp = GetExpression(m_compAttrs[1].m_partName, m_compAttrs[1].m_expName);

        return pSupportCntExp ? pSupportCntExp->Value() : 0;
    }

    bool Bridge::CanUpdateRailSlabFEModel() const
    {
        return false;
    }

    bool Bridge::CanUpdateBraseFEModel() const
    {
        return (GetHeight() != m_oldHeight) || (GetSpanCount() != m_oldSpanCount);
    }

    bool Bridge::CanUpdateRailSlabConnection() const
    {
        return false;
    }

    bool Bridge::CanUpdateBraseConnection() const
    {
        return GetSpanCount() != m_oldSpanCount;
    }
}