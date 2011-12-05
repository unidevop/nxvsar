
#include <uf_defs.h>
#include <Vsar_Tunnel.hxx>

//#include <boost/cast.hpp>

//#include <NXOpen/Expression.hxx>

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
        {DIAMETER_ID_NAME,     TUNNEL_PRT_PART_NAME, DIAMETER_EXP_NAME},
        {WIDTH_ID_NAME,        TUNNEL_PRT_PART_NAME, WIDTH_EXP_NAME},
        {TUNNEL_H1_ID_NAME,    TUNNEL_PRT_PART_NAME, TUNNEL_H1_EXP_NAME},
        {TUNNEL_H2_ID_NAME,    TUNNEL_PRT_PART_NAME, TUNNEL_H2_EXP_NAME},
        {TUNNEL_H3_ID_NAME,    TUNNEL_PRT_PART_NAME, TUNNEL_H3_EXP_NAME}
    };

    Tunnel::Tunnel() : BaseComponent(attrExpInfo, N_ELEMENTS(attrExpInfo))
    {
    }

    Tunnel::~Tunnel()
    {

    }

    void Tunnel::OnInit()
    {
    }

    bool Tunnel::CanUpdateRailSlabFEModel() const
    {
        return false;
    }

    bool Tunnel::CanUpdateBraseFEModel() const
    {
        return true;
    }

    bool Tunnel::CanUpdateRailSlabConnection() const
    {
        return false;
    }

    bool Tunnel::CanUpdateBraseConnection() const
    {
        return true;
    }
}