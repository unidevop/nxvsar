
#include <uf_defs.h>
#include <Vsar_Train.hxx>

//#include <boost/filesystem.hpp>
#include <boost/cast.hpp>

#include <NXOpen/Session.hxx>
//#include <NXOpen/NXException.hxx>
#include <NXOpen/PartCollection.hxx>
#include <NXOpen/Part.hxx>
#include <NXOpen/Assemblies_ComponentAssembly.hxx>
#include <NXOpen/Assemblies_Component.hxx>
#include <NXOpen/ExpressionCollection.hxx>
#include <NXOpen/Expression.hxx>
#include <NXOpen/Body.hxx>
//#include <NXOpen/BodyCollection.hxx>
#include <NXOpen/CAE_FemPart.hxx>
#include <NXOpen/CAE_FEModel.hxx>

#include <Vsar_Names.hxx>
#include <Vsar_Project.hxx>
#include <Vsar_Utils.hxx>
#include <Vsar_Init_Utils.hxx>

using namespace boost;
using namespace NXOpen;
using namespace NXOpen::CAE;
using namespace NXOpen::Assemblies;
using namespace Vsar;

//------------------------------------------------------------------------------
// Declaration of global variables
//------------------------------------------------------------------------------
namespace Vsar
{
    static CompAttrInfo attrExpInfo[] =
    {
        {"carriageCount",               "train",    "Carriage_Count"},
        //{"carriageLength",              "carriage", "Length"},
        {"carriageDistance",            "carriage", "Distance"},
        {"carriageWeight",              "carriage", "Weight"},
        {"bogieLength",                 "carriage", "BogieLength"},
        {"halfBogieDistance",           "carriage", "HalfBogieDistance"},
        {"bogieWeight",                 "carriage", "BogieWeight"},
        {"wheelSetInterval",            "carriage", "WheelSetInterval"},
        {"wheelSetWeight",              "carriage", "WheelSetWeight"},
        {"wheelSetSglStgSusp",          "carriage", "WheelSetSglStgSusp"},
        {"wheelSetSglStgSuspDamp",      "carriage", "WheelSetSglStgSuspDamp"},
        {"wheelSetTwoStgSuspStiff",     "carriage", "WheelSetTwoStgSuspStiff"},
        {"wheelSetTwoStgSuspDamp",      "carriage", "WheelSetTwoStgSuspDamp"},
        {"wheelSetContactCoefElast",    "carriage", "WheelSetContactCoefElast"}
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

    void Train::UpdateRailSlabModel()
    {
        if (GetCarriageCount() > m_oldCarriageCount)
        {
            BaseProjectProperty *pPrjProp       = Project::Instance()->GetProperty();
            CAE::FemPart        *pRailSlabFem   = pPrjProp->GetRailSlabFemPart();
            std::vector<Body*>   bodyOccs;
            std::vector<Body*>   tmpBodyOccs;

            //  Carriage Settings
            tmpBodyOccs = GetGeoModelOccs(pRailSlabFem, CARRIAGE_PART_NAME, CARRIAGE_BODY_NAME);
            bodyOccs.insert(bodyOccs.end(), tmpBodyOccs.begin(), tmpBodyOccs.end());

            //  Slab Settings
            tmpBodyOccs = GetGeoModelOccs(pRailSlabFem, SLAB_PART_NAME, SLAB_BODY_NAME);
            bodyOccs.insert(bodyOccs.end(), tmpBodyOccs.begin(), tmpBodyOccs.end());

            SetFeGeometryData(pRailSlabFem, bodyOccs);

            //std::string meshName = std::string("Mesh[").append(SLAB_MESH_NAME).append("]");

            //EditSweptMeshData(pRailSlabFem->BaseFEModel(), meshName, tmpBodyOccs);

            UpdateSwepMesh(pRailSlabFem->BaseFEModel(), GetCaeBodies(tmpBodyOccs),
                           SLAB_MESH_COLLECTOR_NAME, SLAB_MESH_NAME,
                           pRailSlabFem->Expressions()->FindObject("Slab_Element_Size"));

            UpdateRailSlabConnection(pRailSlabFem);
        }
    }

    void Train::UpdateBraseModel()
    {
        if (GetCarriageCount() > m_oldCarriageCount)
        {
            BaseProjectProperty *pPrjProp       = Project::Instance()->GetProperty();
            
            if (pPrjProp->GetProjectType() == Project::ProjectType_Bridge)
            {
                CAE::FemPart        *pBridgeFem     = pPrjProp->GetBraceFemPart();
                std::vector<Body*>   bodyOccs;

                bodyOccs = GetGeoModelOccs(pBridgeFem, BRIDGE_BEAM_PART_NAME, BRIDGE_BEAM_BODY_NAME);

                SetFeGeometryData(pBridgeFem, bodyOccs);

                UpdateSwepMesh_sf(pBridgeFem->BaseFEModel(), GetCaeBodies(bodyOccs),
                               BRIDGE_MESH_COLLECTOR_NAME, BRIDGE_MESH_NAME,
                               pBridgeFem->Expressions()->FindObject("Bridge_Element_Size"));
            }
        }
    }

    std::vector<Body*> Train::GetGeoModelOccs(FemPart *pFemPart, const std::string &bodyPrtName, const std::string &bodyName)
    {
        std::vector<Body*> bodyOccs;

        if (pFemPart)
        {
            // Set fem part to display part
            PartCollection *pPrtCol  = Session::GetSession()->Parts();

            //Part* pRootGeo = Project::Instance()->GetProperty()->GetGeometryPart();
            Part* pRootGeo = pFemPart->IdealizedPart();

            //  get body part prototype
            Part *pBodyPrt = polymorphic_cast<Part*>(pPrtCol->FindObject(bodyPrtName.c_str()));

            //  get body part occurrence
            Component *pRootComp = pRootGeo->ComponentAssembly()->RootComponent();

            std::vector<Assemblies::Component*>  prtOccs(GetOccInCompTree(pRootComp, pBodyPrt));

            std::vector<Body*> bodyProtos(GetBodyByName(pBodyPrt, bodyName));

            for (unsigned int idx = 0; idx < prtOccs.size(); idx++)
            {
                for (unsigned int jdx = 0; jdx < bodyProtos.size(); jdx++)
                {
                    Body *pBodyOcc = dynamic_cast<Body*>(prtOccs[idx]->FindOccurrence(bodyProtos[jdx]));

                    if (pBodyOcc)
                        bodyOccs.push_back(pBodyOcc);
                }
            }
        }

        return bodyOccs;
    }

    void Train::UpdateRailSlabConnection(FemPart *pFemPart)
    {
        std::vector<Point*>  railConnectPts;

        //railConnectPts = GetPointByAttrName(pFemPart, RAIL_CONNECTION_NAME);
        railConnectPts = GetPointByAttrName(pFemPart, SLAB_CONNECT_TO_RAIL_NAME);

        railConnectPts = GetPointByLayer(pFemPart, RAIL_CONNECTION_POINT_LAYER);
    }

    void Train::OnInit()
    {
        m_oldCarriageCount = GetCarriageCount();
    }

    void Train::SetFeGeometryData( FemPart * pFemPart, const std::vector<Body*> &bodyOccs )
    {
        scoped_ptr<FemSynchronizeOptions> psyncData;

        psyncData.reset(pFemPart->NewFemSynchronizeOptions());
        psyncData->SetSynchronizeLinesFlag(true);
        psyncData->SetSynchronizePointsFlag(true);

        pFemPart->SetGeometryData(FemPart::UseBodiesOptionSelectedBodies, bodyOccs, psyncData.get());

        //pFemPart->BaseFEModel()->UpdateFemodel();
    }
}
