
#ifndef VSAR_INIT_NAMES_H_INCLUDED
#define VSAR_INIT_NAMES_H_INCLUDED

//------------------------------------------------------------------------------
//These includes are needed for the following template code
//------------------------------------------------------------------------------

namespace Vsar
{
    const char * const ROOT_PART_NAME                           = "rail-transit.prt";
    const char * const RAIL_SLAB_FEM_BASE_NAME                  = "RailSlab";

    const char * const PROJECT_TYPE_NAME_BRIDGE                 = "BRIDGE";
    const char * const TEMPLATE_BASE_NAME_BRIDGE                = "RailSlabBridge";
    const char * const TEMPLATE_BRACE_BASE_NAME_BRIDGE          = "Bridge";

    const char * const PROJECT_TYPE_NAME_SELMI_INFINITE         = "SELMI-INFINITE";
    const char * const TEMPLATE_BASE_NAME_SELMI_INFINITE        = "RailSlabBase";
    const char * const TEMPLATE_BRACE_BASE_NAME_SELMI_INFINITE  = "Base";

    const char * const PROJECT_TYPE_NAME_TUNNEL          = "TUNNEL";
    const char * const TEMPLATE_BASE_NAME_TUNNEL         = "RailSlabTunnel";
    const char * const TEMPLATE_BRACE_BASE_NAME_TUNNEL   = "Tunnel";

    const char * const TEMPLATE_FOLDER_NAME              = "template";
    const char * const TEMPLATE_COMMON_FOLDER_NAME       = "common";

    const char * const ATTRIBUTE_PROJECT_NAME            = "VSAR_PROJECT_NAME";
    const char * const ATTRIBUTE_PROJECT_TYPE            = "VSAR_PROJECT_TYPE";
    const char * const ATTRIBUTE_PROJECT_STATUS          = "VSAR_PROJECT_STATUS";

    const char * const ATTRIBUTE_PROJECT_STATUS_DEFINED  = "PROJECT_DEFINED";
    const char * const ATTRIBUTE_PROJECT_STATUS_RESPONSE_SOLVED   = "PROJECT_RESPONSE_SOLVED";
    const char * const ATTRIBUTE_PROJECT_STATUS_NOISE_SOLVED      = "PROJECT_NOISE_SOLVED";

    const char * const MENU_ITEM_NAME_NEW_PROJECT        = "VSAR_NEW_PROJECT";
    const char * const MENU_ITEM_NAME_SET_TRAIN          = "VSAR_SET_TRAIN";
    const char * const MENU_ITEM_NAME_SET_RAIL           = "VSAR_SET_RAIL";
    const char * const MENU_ITEM_NAME_SET_SLAB           = "VSAR_SET_SLAB";
    const char * const MENU_ITEM_NAME_SET_BRACE          = "VSAR_SET_BRACE";
    const char * const MENU_ITEM_NAME_SET_BRIDGE         = "VSAR_SET_BRIDGE";
    const char * const MENU_ITEM_NAME_SET_BASE           = "VSAR_SET_BASE";
    const char * const MENU_ITEM_NAME_SET_TUNNEL         = "VSAR_SET_TUNNEL";
    const char * const MENU_ITEM_NAME_EXECUTE_SOLVE      = "VSAR_EXECUTE_SOLVE";
    const char * const MENU_ITEM_NAME_SOLVE_RESPONSE     = "VSAR_SOLVE_RESPONSE";
    const char * const MENU_ITEM_NAME_SOLVE_NOISE        = "VSAR_SOLVE_NOISE";
    const char * const MENU_ITEM_NAME_VIEW_RESULT        = "VSAR_VIEW_RESULT";

}

#endif //VSAR_INIT_NAMES_H_INCLUDED
