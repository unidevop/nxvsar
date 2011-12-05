
#include <uf_defs.h>
#include <Vsar_SelmiInfiniteBase.hxx>

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
        {WIDTH_ID_NAME,    BASE_PRT_PART_NAME, WIDTH_EXP_NAME},
        {HEIGHT_ID_NAME,   BASE_PRT_PART_NAME, HEIGHT_EXP_NAME}
    };

    SelmiInfiniteBase::SelmiInfiniteBase() : BaseComponent(attrExpInfo, N_ELEMENTS(attrExpInfo)),
        m_oldHeight(1)
    {
    }

    SelmiInfiniteBase::~SelmiInfiniteBase()
    {

    }

    void SelmiInfiniteBase::OnInit()
    {
        m_oldHeight    = GetHeight();
    }

    double SelmiInfiniteBase::GetHeight() const
    {
        Expression *pSupportCntExp = NULL;

        if (!m_compAttrs.empty())
            pSupportCntExp = GetExpression(m_compAttrs[1].m_partName, m_compAttrs[1].m_expName);

        return pSupportCntExp ? pSupportCntExp->Value() : 0;
    }

    bool SelmiInfiniteBase::CanUpdateRailSlabFEModel() const
    {
        return false;
    }

    bool SelmiInfiniteBase::CanUpdateBraseFEModel() const
    {
        return GetHeight() != m_oldHeight;
    }

    bool SelmiInfiniteBase::CanUpdateRailSlabConnection() const
    {
        return false;
    }

    bool SelmiInfiniteBase::CanUpdateBraseConnection() const
    {
        return GetHeight() != m_oldHeight;
    }
}