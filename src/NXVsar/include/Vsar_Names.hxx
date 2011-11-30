
#ifndef VSAR_NAMES_H_INCLUDED
#define VSAR_NAMES_H_INCLUDED

//------------------------------------------------------------------------------
//These includes are needed for the following template code
//------------------------------------------------------------------------------

namespace Vsar
{
    const char * const SLAB_PART_NAME                           = "slab.prt";
    const char * const SLAB_BODY_NAME                           = "SLAB_BODY";
    const char * const SLAB_ASSEM_NAME                          = "slabs.prt";
    const char * const SLAB_MESH_NAME                           = "Slab_Mesh";
    const char * const SLAB_MESH_COLLECTOR_NAME                 = "Slab_Mesh_Collector";
    const char * const SLAB_ELEMENT_SIZE_NAME                   = "Slab_Element_Size";

    const char * const FACE_NAME_TOP                            = "TOP_FACE";
    const char * const FACE_NAME_BOTTOM                         = "BOTTOM_FACE";

    const char * const BRIDGE_BEAM_PART_NAME                    = "beam.prt";
    const char * const BRIDGE_BEAM_BODY_NAME                    = "BRIDGE_BEAM_BODY";
    const char * const BRIDGE_MESH_NAME                         = "Bridge_Mesh";
    const char * const BRIDGE_MESH_COLLECTOR_NAME               = "Bridge_Mesh_Collector";
    const char * const BRIDGE_ELEMENT_SIZE_NAME                 = "Bridge_Element_Size";

    const char * const CARRIAGE_PART_NAME                       = "carriage.prt";
    const char * const CARRIAGE_BODY_NAME                       = "CARRIAGE_BODY";

    const char * const RAIL_CONNECTION_NAME                     = "RAIL_CONNECT_POINT";
    const char * const RAIL_MESH_NAME                           = "Rail_Mesh";
    const char * const RAIL_SLAB_CONNECTION_MESH_NAME           = "Rail_Slab_Connection_Mesh";
    const char * const RAIL_SLAB_CONNECTION_COLLECTOR_NAME      = "Rail_Slab_Connection_Collector";
    const char * const RAIL_SLAB_CONNECTION_NAME                = "Rail_Slab_Connection";

    const char * const SLAB_CONNECT_TO_RAIL_NAME                = "SLAB_CONNECT_TO_RAIL_POINT";

    const char * const SLAB_BASE_CONNECTION_MESH_NAME           = "Slab_Base_Connection_Mesh";
    const char * const SLAB_BASE_CONNECTION_COLLECTOR_NAME      = "Slab_Base_Connection_Collector";
    const char * const SLAB_BASE_CONNECTION_NAME                = "Slab_Base_Connection";

    const char * const VSAR_SOLUTION_NAME                       = "Vsar103Sol";

    enum CompLayer
    {
        RAIL_CONNECTION_POINT_LAYER                             = 128,
        SLAB_CONNECT_TO_RAIL_POINT_LAYER                        = 129,
        SLAB_CONNECT_TO_BASE_LEFT_POINT_LAYER                   = 130,
        SLAB_CONNECT_TO_BASE_RIGHT_POINT_LAYER                  = 131,
        BASE_CONNECT_TO_SLAB_LEFT_POINT_LAYER                   = 132,
        BASE_CONNECT_TO_SLAB_RIGHT_POINT_LAYER                  = 133
    };
}

#endif //VSAR_NAMES_H_INCLUDED
