
#include <uf_defs.h>
#include <Vsar_Train.hxx>

//#include <boost/filesystem.hpp>
//#include <boost/shared_ptr.hpp>
//#include <boost/bind.hpp>
#include <boost/cast.hpp>

//#include <NXOpen/Session.hxx>
//#include <NXOpen/NXException.hxx>
//#include <NXOpen/PartCollection.hxx>
//#include <NXOpen/Part.hxx>
//#include <NXOpen/Assemblies_ComponentAssembly.hxx>
//#include <NXOpen/Assemblies_Component.hxx>
//#include <NXOpen/ExpressionCollection.hxx>
#include <NXOpen/Expression.hxx>
//#include <NXOpen/Body.hxx>
//#include <NXOpen/Point.hxx>
//#include <NXOpen/BodyCollection.hxx>
//#include <NXOpen/CAE_FemPart.hxx>
//#include <NXOpen/CAE_FEModel.hxx>

#include <Vsar_Names.hxx>
//#include <Vsar_Project.hxx>
//#include <Vsar_Utils.hxx>
#include <Vsar_Init_Utils.hxx>

using namespace boost;
using namespace NXOpen;
//using namespace NXOpen::CAE;
//using namespace NXOpen::Assemblies;
//using namespace Vsar;

//------------------------------------------------------------------------------
// Declaration of global variables
//------------------------------------------------------------------------------
namespace Vsar
{
    static CompAttrInfo attrExpInfo[] =
    {
        {TRAIN_CARRIAGE_COUNT_ID_NAME,               TRAIN_PRT_PART_NAME,    TRAIN_CARRIAGE_COUNT_EXP_NAME},
        //{"carriageLength",              CARRIAGE_PRT_PART_NAME, "Length"},
        {TRAIN_CARRIAGE_DISTANCE_ID_NAME,            CARRIAGE_PRT_PART_NAME, TRAIN_CARRIAGE_DISTANCE_EXP_NAME},
        {TRAIN_CARRIAGE_WEIGHT_ID_NAME,              CARRIAGE_PRT_PART_NAME, TRAIN_CARRIAGE_WEIGHT_EXP_NAME},
        {TRAIN_BOGIE_LENGTH_ID_NAME,                 CARRIAGE_PRT_PART_NAME, TRAIN_BOGIE_LENGTH_EXP_NAME},
        {TRAIN_HALF_BOGIE_DISTANCE_ID_NAME,          CARRIAGE_PRT_PART_NAME, TRAIN_HALF_BOGIE_DISTANCE_EXP_NAME},
        {TRAIN_BOGIE_WEIGHT_ID_NAME,                 CARRIAGE_PRT_PART_NAME, TRAIN_BOGIE_WEIGHT_EXP_NAME},
        {TRAIN_WHEELSET_INTERVAL_ID_NAME,            CARRIAGE_PRT_PART_NAME, TRAIN_WHEELSET_INTERVAL_EXP_NAME},
        {TRAIN_WHEELSET_WEIGHT_ID_NAME,              CARRIAGE_PRT_PART_NAME, TRAIN_WHEELSET_WEIGHT_EXP_NAME},
        {TRAIN_WHEELSET_SGL_STG_SUSP_STIFF_ID_NAME,  CARRIAGE_PRT_PART_NAME, TRAIN_WHEELSET_SGL_STG_SUSP_STIFF_EXP_NAME},
        {TRAIN_WHEELSET_SGL_STG_SUSP_DAMP_ID_NAME,   CARRIAGE_PRT_PART_NAME, TRAIN_WHEELSET_SGL_STG_SUSP_DAMP_EXP_NAME},
        {TRAIN_WHEELSET_TWO_STG_SUSP_STIFF_ID_NAME,  CARRIAGE_PRT_PART_NAME, TRAIN_WHEELSET_TWO_STG_SUSP_STIFF_EXP_NAME},
        {TRAIN_WHEELSET_TWO_STG_SUSP_DAMP_ID_NAME,   CARRIAGE_PRT_PART_NAME, TRAIN_WHEELSET_TWO_STG_SUSP_DAMP_EXP_NAME},
        {TRAIN_WHEELSET_CONTACT_COEF_ELAST_ID_NAME,  CARRIAGE_PRT_PART_NAME, TRAIN_WHEELSET_CONTACT_COEF_ELAST_EXP_NAME}
    };

    Train::Train() : BaseComponent(attrExpInfo, N_ELEMENTS(attrExpInfo)),
                                   m_oldCarriageCount(0)
    {
    }

    Train::~Train()
    {

    }

    int Train::GetCarriageCount() const
    {
        Expression *pCarriageCntExp = NULL;

        if (!m_compAttrs.empty())
            pCarriageCntExp = GetExpression(m_compAttrs[0].m_partName, m_compAttrs[0].m_expName);

        return pCarriageCntExp ? numeric_cast<int>(pCarriageCntExp->Value()) : 0;
    }

    //void Train::UpdateRailSlabModel()
    //{
    //    int carriageCnt = GetCarriageCount();

    //    if (carriageCnt == m_oldCarriageCount)
    //        return;

    //    BaseProjectProperty *pPrjProp       = Project::Instance()->GetProperty();
    //    CAE::FemPart        *pRailSlabFem   = pPrjProp->GetRailSlabFemPart();

    //    if (carriageCnt > m_oldCarriageCount)
    //    {
    //        std::vector<Body*>   bodyOccs;
    //        std::vector<Body*>   tmpBodyOccs;

    //        //  Carriage Settings
    //        tmpBodyOccs = GetGeoModelOccs(pRailSlabFem, CARRIAGE_PART_NAME, CARRIAGE_BODY_NAME);
    //        bodyOccs.insert(bodyOccs.end(), tmpBodyOccs.begin(), tmpBodyOccs.end());

    //        //  Slab Settings
    //        tmpBodyOccs = GetGeoModelOccs(pRailSlabFem, SLAB_PART_NAME, SLAB_BODY_NAME);
    //        bodyOccs.insert(bodyOccs.end(), tmpBodyOccs.begin(), tmpBodyOccs.end());

    //        SetFeGeometryData(pRailSlabFem, bodyOccs, true);

    //        //std::string meshName = std::string("Mesh[").append(SLAB_MESH_NAME).append("]");

    //        //EditSweptMeshData(pRailSlabFem->BaseFEModel(), meshName, tmpBodyOccs);

    //        UpdateSweptMesh(pRailSlabFem->BaseFEModel(), GetCaeBodies(tmpBodyOccs),
    //                       SLAB_MESH_COLLECTOR_NAME, SLAB_MESH_NAME,
    //                       SLAB_ELEMENT_SIZE_NAME);
    //    }

    //    UpdateRailSlabConnection(pRailSlabFem);
    //    MergeDuplicateNodes();
    //}

    //void Train::UpdateBraseModel()
    //{
    //    int carriageCnt = GetCarriageCount();

    //    if (carriageCnt == m_oldCarriageCount)
    //        return;

    //    BaseProjectProperty *pPrjProp    = Project::Instance()->GetProperty();
    //    CAE::FemPart        *pBaseFem    = pPrjProp->GetBraceFemPart();

    //    if (carriageCnt > m_oldCarriageCount)
    //    {
    //        if (pPrjProp->GetProjectType() == Project::ProjectType_Bridge)
    //        {
    //            std::vector<Body*>   bodyOccs;

    //            bodyOccs = GetGeoModelOccs(pBaseFem, BRIDGE_BEAM_PART_NAME, BRIDGE_BEAM_BODY_NAME);

    //            SetFeGeometryData(pBaseFem, bodyOccs, false);

    //            UpdateSweptMesh(pBaseFem->BaseFEModel(), GetCaeBodies(bodyOccs),
    //                           BRIDGE_MESH_COLLECTOR_NAME, BRIDGE_MESH_NAME,
    //                           BRIDGE_ELEMENT_SIZE_NAME);
    //        }
    //    }

    //    UpdateBaseSlabConnection(pBaseFem);
    //    MergeDuplicateNodes();
    //}

    //std::vector<Body*> Train::GetGeoModelOccs(FemPart *pFemPart, const std::string &bodyPrtName, const std::string &bodyName)
    //{
    //    std::vector<Body*> bodyOccs;

    //    if (pFemPart)
    //    {
    //        // Set fem part to display part
    //        PartCollection *pPrtCol  = Session::GetSession()->Parts();

    //        //Part* pRootGeo = Project::Instance()->GetProperty()->GetGeometryPart();
    //        Part* pRootGeo = pFemPart->IdealizedPart();

    //        //  get body part prototype
    //        Part *pBodyPrt = polymorphic_cast<Part*>(pPrtCol->FindObject(bodyPrtName.c_str()));

    //        //  get body part occurrence
    //        Component *pRootComp = pRootGeo->ComponentAssembly()->RootComponent();

    //        std::vector<Assemblies::Component*>  prtOccs(GetOccInCompTree(pRootComp, pBodyPrt));

    //        std::vector<Body*> bodyProtos(GetBodyByName(pBodyPrt, bodyName));

    //        for (unsigned int idx = 0; idx < prtOccs.size(); idx++)
    //        {
    //            for (unsigned int jdx = 0; jdx < bodyProtos.size(); jdx++)
    //            {
    //                Body *pBodyOcc = dynamic_cast<Body*>(prtOccs[idx]->FindOccurrence(bodyProtos[jdx]));

    //                if (pBodyOcc)
    //                    bodyOccs.push_back(pBodyOcc);
    //            }
    //        }
    //    }

    //    return bodyOccs;
    //}

    //void Train::UpdateRailSlabConnection(FemPart *pFemPart)
    //{
    //    std::vector<TaggedObject*>  railConnectPts;
    //    std::vector<TaggedObject*>  slabConnectPts;

    //    //railConnectPts = GetPointByAttrName(pFemPart, RAIL_CONNECTION_NAME);
    //    //slabConnectPts = GetPointByAttrName(pFemPart, SLAB_CONNECT_TO_RAIL_NAME);

    //    railConnectPts = GetPointByLayer(pFemPart, RAIL_CONNECTION_POINT_LAYER);
    //    slabConnectPts = GetPointByLayer(pFemPart, SLAB_CONNECT_TO_RAIL_POINT_LAYER);

    //    Update1DConnection(pFemPart->BaseFEModel(), railConnectPts, slabConnectPts,
    //        RAIL_SLAB_CONNECTION_NAME, RAIL_SLAB_CONNECTION_COLLECTOR_NAME);
    //}

    //void Train::UpdateBaseSlabConnection(FemPart *pFemPart)
    //{
    //    std::vector<TaggedObject*>  slabConnectPts;
    //    std::vector<TaggedObject*>  baseConnectPts;

    //    std::vector<TaggedObject*>  slabPartConnectPts;
    //    std::vector<TaggedObject*>  basePartConnectPts;

    //    //  get left side points
    //    slabPartConnectPts = GetPointByLayer(pFemPart, SLAB_CONNECT_TO_BASE_LEFT_POINT_LAYER);
    //    basePartConnectPts = GetPointByLayer(pFemPart, BASE_CONNECT_TO_SLAB_LEFT_POINT_LAYER);

    //    slabConnectPts.insert(slabConnectPts.end(), slabPartConnectPts.begin(), slabPartConnectPts.end());
    //    baseConnectPts.insert(baseConnectPts.end(), basePartConnectPts.begin(), basePartConnectPts.end());

    //    //  get right side points
    //    slabPartConnectPts = GetPointByLayer(pFemPart, SLAB_CONNECT_TO_BASE_RIGHT_POINT_LAYER);
    //    basePartConnectPts = GetPointByLayer(pFemPart, BASE_CONNECT_TO_SLAB_RIGHT_POINT_LAYER);

    //    slabConnectPts.insert(slabConnectPts.end(), slabPartConnectPts.begin(), slabPartConnectPts.end());
    //    baseConnectPts.insert(baseConnectPts.end(), basePartConnectPts.begin(), basePartConnectPts.end());

    //    Update1DConnection(pFemPart->BaseFEModel(), slabConnectPts, baseConnectPts,
    //        SLAB_BASE_CONNECTION_NAME, SLAB_BASE_CONNECTION_COLLECTOR_NAME);
    //}

    void Train::OnInit()
    {
        m_oldCarriageCount = GetCarriageCount();
    }

    //void Train::SetFeGeometryData( FemPart * pFemPart, const std::vector<Body*> &bodyOccs, bool syncLines )
    //{
    //    scoped_ptr<FemSynchronizeOptions> psyncData;

    //    psyncData.reset(pFemPart->NewFemSynchronizeOptions());
    //    psyncData->SetSynchronizeLinesFlag(syncLines);
    //    psyncData->SetSynchronizePointsFlag(true);

    //    pFemPart->SetGeometryData(FemPart::UseBodiesOptionSelectedBodies, bodyOccs, psyncData.get());

    //    //pFemPart->BaseFEModel()->UpdateFemodel();
    //}

    bool Train::CanUpdateRailSlabFEModel() const
    {
        return GetCarriageCount() > m_oldCarriageCount;
    }

    bool Train::CanUpdateBraseFEModel() const
    {
        return GetCarriageCount() > m_oldCarriageCount;
    }

    bool Train::CanUpdateRailSlabConnection() const
    {
        return GetCarriageCount() != m_oldCarriageCount;
    }

    bool Train::CanUpdateBraseConnection() const
    {
        return GetCarriageCount() != m_oldCarriageCount;
    }
}
