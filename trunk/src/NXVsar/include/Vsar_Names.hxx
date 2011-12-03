
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

    // Bridge
    const char * const BRIDGE_BEAM_PART_NAME                    = "beam.prt";
    const char * const BRIDGE_BEAM_BODY_NAME                    = "BRIDGE_BEAM_BODY";
    const char * const BRIDGE_MESH_NAME                         = "Bridge_Mesh";
    const char * const BRIDGE_MESH_COLLECTOR_NAME               = "Bridge_Mesh_Collector";
    const char * const BRIDGE_ELEMENT_SIZE_NAME                 = "Bridge_Element_Size";

    const char * const CARRIAGE_PART_NAME                       = "carriage.prt";
    const char * const CARRIAGE_BODY_NAME                       = "CARRIAGE_BODY";

    // Rail
    const char * const RAIL_CONNECTION_NAME                     = "RAIL_CONNECT_POINT";
    const char * const RAIL_MESH_NAME                           = "Rail_Mesh";
    const char * const RAIL_SLAB_CONNECTION_MESH_NAME           = "Rail_Slab_Connection_Mesh";
    const char * const RAIL_SLAB_CONNECTION_COLLECTOR_NAME      = "Rail_Slab_Connection_Collector";
    const char * const RAIL_SLAB_CONNECTION_NAME                = "RAIL_SLAB_CONNECTION";

    // Slab
    const char * const SLAB_CONNECT_TO_RAIL_NAME                = "SLAB_CONNECT_TO_RAIL_POINT";

    const char * const SLAB_BASE_CONNECTION_MESH_NAME           = "Slab_Base_Connection_Mesh";
    const char * const SLAB_BASE_CONNECTION_COLLECTOR_NAME      = "Slab_Base_Connection_Collector";
    const char * const SLAB_BASE_CONNECTION_NAME                = "SLAB_BASE_CONNECTION";

    // Base
    const char * const BASE_MESH_COLLECTOR_NAME                 = "Base_Mesh_Collector";

    const char * const VSAR_SOLUTION_NAME                       = "Vsar103Sol";

    //  Part Name
    const char * const RAIL_SLAB_FEM_PART_NAME                  = "RailSlab_f";
    const char * const BRIDGE_FEM_PART_NAME                     = "Bridge_f";
    const char * const SLAB_PRT_PART_NAME                       = "slab";
    const char * const BEAM_PRT_PART_NAME                       = "beam";
    const char * const BRIDGE_PRT_PART_NAME                     = "bridge";
    const char * const BASE_PRT_PART_NAME                       = "base";
    const char * const TUNNEL_PRT_PART_NAME                     = "tunnel";
    const char * const TRAIN_PRT_PART_NAME                      = "train";
    const char * const CARRIAGE_PRT_PART_NAME                   = "carriage";

    //////////////////////////////////////////////////////////////////////////
    //  Expression Name-Value
    //  Rail
    const char * const MASS_DENSITY_ID_NAME                     = "density";      // common id name
    const char * const RAIL_DENSITY_EXPRESSION_NAME             = "Rail_Mass_Density";

    const char * const ELASTIC_MODULUS_ID_NAME                  = "elasticModulus";      // common id name
    const char * const RAIL_ELASTIC_MODULUS_EXP_NAME            = "Rail_Elastic_Modulus";

    const char * const POISSON_RATIO_ID_NAME                    = "poissonRatio";      // common id name
    const char * const RAIL_POISSON_RATIO_EXP_NAME              = "Rail_Poisson_Ratio";

    const char * const FASTENER_STIFFNESS_ID_NAME               = "fastenerStiffness";      // common id name
    const char * const RAIL_FASTENER_STIFFNESS_EXP_NAME         = "Rail_Fastener_Stiffness";

    const char * const FASTENER_DAMPING_ID_NAME                 = "fastenerDamping";      // common id name
    const char * const RAIL_FASTENER_DAMPING_EXP_NAME           = "Rail_Fastener_Damping";

    // Slab
    const char * const SLAB_SUPPORT_COUNT_ID_NAME               = "supportCnt";
    const char * const SLAB_SUPPORT_COUNT_EXP_NAME              = "Support_Count";

    const char * const MASS_RATIO_ID_NAME                       = "massRatio";      // common id name
    const char * const SLAB_MASS_RATIO_EXP_NAME                 = "Slab_Mass_Ratio";

    const char * const STIFFNESS_RATIO_ID_NAME                  = "stiffnessRatio";      // common id name
    const char * const SLAB_STIFFNESS_RATIO_EXP_NAME            = "Slab_Stiffness_Ratio";

    const char * const DAMPING_RATIO_ID_NAME                    = "dampingRatio";      // common id name
    const char * const SLAB_DAMPING_RATIO_EXP_NAME              = "Slab_Damping_Ratio";

    const char * const SLAB_ELASTIC_MODULUS_EXP_NAME            = "Slab_Elastic_Modulus";

    const char * const SLAB_POISSON_RATIO_EXP_NAME              = "Slab_Poisson_Ratio";

    // Bridge
    const char * const BRIDGE_ELASTIC_MODULUS_EXP_NAME          = "Bridge_Elastic_Modulus";

    const char * const BRIDGE_POISSON_RATIO_EXP_NAME            = "Bridge_Poisson_Ratio";

    const char * const UNIT_WEIGHT_ID_NAME                      = "unitWeight";      // common id name
    const char * const BRIDGE_UNIT_WEIGHT_EXP_NAME              = "Bridge_UnitWeight";

    const char * const WIDTH_ID_NAME                            = "width";      // common id name
    const char * const WIDTH_EXP_NAME                           = "Width";

    const char * const HEIGHT_ID_NAME                           = "height";      // common id name
    const char * const HEIGHT_EXP_NAME                          = "Height";

    const char * const SPAN_COUNT_ID_NAME                       = "spanCnt";      // common id name
    const char * const SPAN_COUNT_EXP_NAME                      = "Beam_Span";

    //  Tunnel
    const char * const DIAMETER_ID_NAME                         = "diameter";      // common id name
    const char * const DIAMETER_EXP_NAME                        = "Diameter";

    const char * const TUNNEL_H1_ID_NAME                        = "h1";
    const char * const TUNNEL_H1_EXP_NAME                       = "Height_Top";

    const char * const TUNNEL_H2_ID_NAME                        = "h2";
    const char * const TUNNEL_H2_EXP_NAME                       = "Height_Mid";

    const char * const TUNNEL_H3_ID_NAME                        = "h3";
    const char * const TUNNEL_H3_EXP_NAME                       = "Height_Bottom";

    // Train
    const char * const TRAIN_CARRIAGE_COUNT_ID_NAME             = "carriageCount";
    const char * const TRAIN_CARRIAGE_COUNT_EXP_NAME            = "Carriage_Count";

    const char * const TRAIN_CARRIAGE_DISTANCE_ID_NAME          = "carriageDistance";
    const char * const TRAIN_CARRIAGE_DISTANCE_EXP_NAME         = "Distance";

    const char * const TRAIN_CARRIAGE_WEIGHT_ID_NAME            = "carriageWeight";
    const char * const TRAIN_CARRIAGE_WEIGHT_EXP_NAME           = "Weight";

    const char * const TRAIN_BOGIE_LENGTH_ID_NAME               = "bogieLength";
    const char * const TRAIN_BOGIE_LENGTH_EXP_NAME              = "BogieLength";

    const char * const TRAIN_HALF_BOGIE_DISTANCE_ID_NAME        = "halfBogieDistance";
    const char * const TRAIN_HALF_BOGIE_DISTANCE_EXP_NAME       = "HalfBogieDistance";

    const char * const TRAIN_BOGIE_WEIGHT_ID_NAME               = "bogieWeight";
    const char * const TRAIN_BOGIE_WEIGHT_EXP_NAME              = "BogieWeight";

    const char * const TRAIN_WHEELSET_INTERVAL_ID_NAME          = "wheelSetInterval";
    const char * const TRAIN_WHEELSET_INTERVAL_EXP_NAME         = "WheelSetInterval";

    const char * const TRAIN_WHEELSET_WEIGHT_ID_NAME            = "wheelSetWeight";
    const char * const TRAIN_WHEELSET_WEIGHT_EXP_NAME           = "WheelSetWeight";

    const char * const TRAIN_WHEELSET_SGL_STG_SUSP_ID_NAME      = "wheelSetSglStgSusp";
    const char * const TRAIN_WHEELSET_SGL_STG_SUSP_EXP_NAME     = "WheelSetSglStgSusp";

    const char * const TRAIN_WHEELSET_SGL_STG_SUSP_DAMP_ID_NAME  = "wheelSetSglStgSuspDamp";
    const char * const TRAIN_WHEELSET_SGL_STG_SUSP_DAMP_EXP_NAME = "WheelSetSglStgSuspDamp";

    const char * const TRAIN_WHEELSET_TWO_STG_SUSP_STIFF_ID_NAME  = "wheelSetTwoStgSuspStiff";
    const char * const TRAIN_WHEELSET_TWO_STG_SUSP_STIFF_EXP_NAME = "WheelSetTwoStgSuspStiff";

    const char * const TRAIN_WHEELSET_TWO_STG_SUSP_DAMP_ID_NAME   = "wheelSetTwoStgSuspDamp";
    const char * const TRAIN_WHEELSET_TWO_STG_SUSP_DAMP_EXP_NAME  = "WheelSetTwoStgSuspDamp";

    const char * const TRAIN_WHEELSET_CONTACT_COEF_ELAST_ID_NAME  = "wheelSetContactCoefElast";
    const char * const TRAIN_WHEELSET_CONTACT_COEF_ELAST_EXP_NAME = "WheelSetContactCoefElast";


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
