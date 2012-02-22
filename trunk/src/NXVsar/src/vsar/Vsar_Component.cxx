
#include <uf_defs.h>
#include <Vsar_Component.hxx>

#include <algorithm>
#include <boost/scope_exit.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/cast.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

#include <NXOpen/Session.hxx>
#include <NXOpen/NXException.hxx>
#include <NXOpen/LogFile.hxx>
#include <NXOpen/PartCollection.hxx>
#include <NXOpen/Part.hxx>
#include <NXOpen/Expression.hxx>
#include <NXOpen/ExpressionCollection.hxx>
#include <NXOpen/Body.hxx>
#include <NXOpen/Point.hxx>
#include <NXOpen/Assemblies_ComponentAssembly.hxx>
#include <NXOpen/Assemblies_Component.hxx>
#include <NXOpen/ExpressionCollection.hxx>

#include <NXOpen/CAE_SimPart.hxx>
#include <NXOpen/CAE_FemPart.hxx>
#include <NXOpen/CAE_AssyFemPart.hxx>
#include <NXOpen/CAE_BaseFEModel.hxx>
#include <NXOpen/CAE_AssyFEModel.hxx>
#include <NXOpen/CAE_MeshManager.hxx>
#include <NXOpen/CAE_MeshCollector.hxx>
#include <NXOpen/CAE_CAEBody.hxx>
#include <NXOpen/CAE_CAEFace.hxx>
#include <NXOpen/CAE_FEModelOccurrence.hxx>
#include <NXOpen/CAE_FEElementLabelMap.hxx>
#include <NXOpen/CAE_FENodeLabelMap.hxx>
#include <NXOpen/CAE_Mesh.hxx>

#include <uf.h>
#include <uf_sf.h>

#include <Vsar_Project.hxx>
#include <Vsar_Init_Utils.hxx>
#include <Vsar_Names.hxx>
#include <Vsar_Utils.hxx>

using namespace boost;
using namespace NXOpen;
using namespace NXOpen::Assemblies;
using namespace NXOpen::CAE;
//using namespace Vsar;

//------------------------------------------------------------------------------
// Declaration of global variables
//------------------------------------------------------------------------------
namespace Vsar
{
    BaseComponent::BaseComponent(const CompAttrInfo *pCompAttrs, int compAttrCnt) :
                        m_compAttrs(pCompAttrs, pCompAttrs + compAttrCnt)
    {
        //  set root sim part as display part
        Project             *pPrj     = Project::Instance();
        BaseProjectProperty *pPrjProp = pPrj->GetProperty();
        PartLoadStatus *pPrtLoadStatus = NULL;

        DELETE_CLASS_POINTER(pPrtLoadStatus);

        Session::GetSession()->Parts()->SetDisplay(pPrjProp->GetSimPart(),
            false, true, &pPrtLoadStatus);
    }

    BaseComponent::~BaseComponent()
    {
    }

    void BaseComponent::LoadGeometryPart(bool onDemandLoad)
    {
        Project             *pPrj     = Project::Instance();
        BaseProjectProperty *pPrjProp = pPrj->GetProperty();

        CAE::AssyFemPart *pAfem = pPrjProp->GetAFemPart();
        //pAfem->LoadFully();

        bool loadAllGeometry = onDemandLoad ? HasGeometryDependency() : true;

        //  load idea part and geometry part
        CAE::FemPart *pRailSlabFem = pPrjProp->GetRailSlabFemPart();
        CAE::FemPart *pBraceFem    = pPrjProp->GetBraceFemPart();

        if (loadAllGeometry)
        {
            Part         *pRailSab_i   = pPrjProp->GetRailSlabIdeaPart();
            pRailSab_i->LoadFully();

            Part         *pBrace_i     = pPrjProp->GetBraceIdeaPart();
            pBrace_i->LoadFully();

            Part         *pRootGeo     = pPrjProp->GetGeometryPart();
            pRootGeo->LoadFully();
        }
    }

    Expression* BaseComponent::GetExpression(const std::string &attrName)
    {
        Expression      *pExp     = NULL;

        for (StlCompAttrInfoVector::iterator iter = m_compAttrs.begin();
             iter != m_compAttrs.end(); ++iter)
        {
            if (iter->m_attrName == attrName)
            {
                pExp = GetExpression(iter->m_partName, iter->m_expName);
                break;
            }
        }

        return pExp;
    }

    Expression* BaseComponent::GetExpression(const std::string &partName, const std::string &expName)
    {
        Session         *pSession = Session::GetSession();
        PartCollection  *pPrtCol  = pSession->Parts();
        BasePart        *pExpPrt  = NULL;
        Expression      *pExp     = NULL;

        pExpPrt = pPrtCol->FindObject(partName.c_str());

        if (pExpPrt)
        {
            pExp = pExpPrt->Expressions()->FindObject(expName.c_str());
        }

        return pExp;
    }

    bool BaseComponent::HasGeometryDependency() const
    {
        Project             *pPrj     = Project::Instance();
        BaseProjectProperty *pPrjProp = pPrj->GetProperty();

        //  Load all the geometry parts
        std::vector<std::string> fileNames;
        bool                     needUpdate = false;

        pPrjProp->GetTemplatePartFiles(fileNames);

        for (StlCompAttrInfoVector::const_iterator iter = m_compAttrs.begin();
            iter != m_compAttrs.end(); ++iter)
        {
            if (std::find(fileNames.begin(), fileNames.end(),
                          iter->m_partName + ".prt") != fileNames.end())
            {
                needUpdate = true;
                break;
            }
        }

        return needUpdate;
    }

    void BaseComponent::UpdateModel()
    {
        if (HasGeometryDependency())
        {
            Project             *pPrj     = Project::Instance();
            BaseProjectProperty *pPrjProp = pPrj->GetProperty();
            PartLoadStatus *pPrtLoadStatus = NULL;

            DELETE_CLASS_POINTER(pPrtLoadStatus);

            BOOST_SCOPE_EXIT((&pPrjProp)(&pPrtLoadStatus))
            {
                Session::GetSession()->Parts()->SetDisplay(pPrjProp->GetSimPart(),
                    false, true, &pPrtLoadStatus);
            }
            BOOST_SCOPE_EXIT_END

            UpdateGeometryModel();
            UpdateFEModel();
        }
    }

    void BaseComponent::UpdateGeometryModel()
    {
        Project             *pPrj     = Project::Instance();
        BaseProjectProperty *pPrjProp = pPrj->GetProperty();

        //PartLoadStatus *pPrtLoadStatus = NULL;

        //DELETE_CLASS_POINTER(pPrtLoadStatus);

        PartCollection  *pPrtCol = Session::GetSession()->Parts();

        //pPrtCol->SetDisplay(pPrjProp->GetGeometryPart(),
        //                    false, true, &pPrtLoadStatus);

        //  Load all the geometry parts
        std::vector<std::string> fileNames;

        pPrjProp->GetTemplatePartFiles(fileNames);
        for (unsigned int idx = 0; idx < fileNames.size(); idx++)
        {
            BasePart *pPrt = pPrtCol->FindObject(fileNames[idx].c_str());

            if (pPrt)
                pPrt->LoadFully();
        }

        Session *pSession = Session::GetSession();
        Session::UndoMarkId undoMarkId;
        NXString            undoMarkName("Update Geometry Model");

        undoMarkId = pSession->SetUndoMark(Session::MarkVisibilityInvisible, undoMarkName);

        Update  *pUpdateMgr = pSession->UpdateManager();

        pUpdateMgr->DoUpdate(undoMarkId);
        pUpdateMgr->DoInterpartUpdate(undoMarkId);
        pUpdateMgr->DoAssemblyConstraintsUpdate(undoMarkId);

        pSession->DeleteUndoMark(undoMarkId, undoMarkName);
    }

    void BaseComponent::UpdateFEModel()
    {
        BaseProjectProperty *pPrjProp       = Project::Instance()->GetProperty();
        CAE::FemPart        *pRailSlabFem   = pPrjProp->GetRailSlabFemPart();
        bool                 needMerge      = false;

        if (CanUpdateRailSlabFEModel() || CanUpdateRailSlabConnection())
        {
            UpdateFECompModel(pRailSlabFem, boost::bind(&BaseComponent::UpdateRailSlabModel, this));
            needMerge = true;
        }

        CAE::FemPart        *pBraceFem      = pPrjProp->GetBraceFemPart();

        if (CanUpdateBraseFEModel() || CanUpdateBraseConnection())
        {
            UpdateFECompModel(pBraceFem, boost::bind(&BaseComponent::UpdateBraseModel, this));
            needMerge = true;
        }

        CAE::BaseFemPart    *pAssemFem      = pPrjProp->GetAFemPart();

        if (needMerge)
            UpdateFECompModel(pAssemFem, boost::bind(&BaseComponent::UpdateAssembleModel, this));

    }

    template <typename UpdateCallback>
    void BaseComponent::UpdateFECompModel(CAE::BaseFemPart *pFem, UpdateCallback updateCb)
    {
        PartLoadStatus      *pPrtLoadStatus = NULL;

        DELETE_CLASS_POINTER(pPrtLoadStatus);

        Session *pSession = Session::GetSession();

        pSession->Parts()->SetDisplay(pFem, false, true, &pPrtLoadStatus);

        Session::UndoMarkId  undoMark;

        try
        {
            undoMark = pSession->SetUndoMark(Session::MarkVisibilityVisible, "Setting Vsar Component");

            updateCb();

            pFem->BaseFEModel()->UpdateFemodel();
        }
        catch (std::exception &ex)
        {
            pSession->LogFile()->WriteLine(ex.what());
            pSession->UndoToLastVisibleMark();
            throw;
        }

        //pSession->DeleteUndoMark(undoMark, NULL);
    }

    void BaseComponent::UpdateRailSlabModel()
    {
        if (CanUpdateRailSlabFEModel())
        {
            BaseProjectProperty *pPrjProp       = Project::Instance()->GetProperty();
            CAE::FemPart        *pRailSlabFem   = pPrjProp->GetRailSlabFemPart();

            std::vector<Body*>   bodyOccs;
            std::vector<Body*>   tmpBodyOccs;

            //  Carriage Settings
            //tmpBodyOccs = GetGeoModelOccs(pRailSlabFem, CARRIAGE_PART_NAME, CARRIAGE_BODY_NAME);
            //bodyOccs.insert(bodyOccs.end(), tmpBodyOccs.begin(), tmpBodyOccs.end());

            //  Slab Settings
            tmpBodyOccs = GetGeoModelOccs(pRailSlabFem, SLAB_PART_NAME, SLAB_BODY_NAME);
            bodyOccs.insert(bodyOccs.end(), tmpBodyOccs.begin(), tmpBodyOccs.end());

            SetFeGeometryData(pRailSlabFem, bodyOccs, true);

            // delete slab meshes first in case of update error.
            DeleteMeshesInCollector(pRailSlabFem->BaseFEModel(), SLAB_MESH_COLLECTOR_NAME);

            UpdateSweptMesh(pRailSlabFem->BaseFEModel(), GetCaeBodies(tmpBodyOccs),
                SLAB_MESH_COLLECTOR_NAME, SLAB_MESH_NAME,
                SLAB_ELEMENT_SIZE_NAME);
        }

        if (CanUpdateRailSlabConnection())
            UpdateRailSlabConnection();

        if (CanUpdateRailSlabFEModel() || CanUpdateRailSlabConnection())
        {
            std::vector<Mesh*> meshToMergeNodes;

            MergeDuplicateNodes(meshToMergeNodes);
        }
    }

    void BaseComponent::UpdateBraseModel()
    {
        BaseProjectProperty *pPrjProp    = Project::Instance()->GetProperty();

        if (CanUpdateBraseFEModel())
        {
            if (pPrjProp->GetProjectType() == Project::ProjectType_Bridge)
            {
                CAE::FemPart        *pBaseFem    = pPrjProp->GetBraceFemPart();

                std::vector<Body*>   bodyOccs;

                bodyOccs = GetGeoModelOccs(pBaseFem, BRIDGE_BEAM_PART_NAME, BRIDGE_BEAM_BODY_NAME);

                SetFeGeometryData(pBaseFem, bodyOccs, false);

                UpdateSweptMesh(pBaseFem->BaseFEModel(), GetCaeBodies(bodyOccs),
                    BRIDGE_MESH_COLLECTOR_NAME, BRIDGE_MESH_NAME,
                    BRIDGE_ELEMENT_SIZE_NAME);
            }
        }

        if (CanUpdateBraseConnection())
            UpdateBaseSlabConnection();

        if (CanUpdateBraseFEModel() || CanUpdateBraseConnection())
        {
            std::vector<Mesh*> meshToMergeNodes;

            if (pPrjProp->GetProjectType() == Project::ProjectType_Tunnel)
            {
                CAE::BaseFEModel *pBaseFeModel    = pPrjProp->GetBraceFemPart()->BaseFEModel();

                meshToMergeNodes.push_back(GetMeshByName(pBaseFeModel,
                    FIND_MESH_PATTERN_NAME, TUNNEL_CONCRETE_MESH_NAME));

                meshToMergeNodes.push_back(GetMeshByName(pBaseFeModel,
                    FIND_MESH_PATTERN_NAME, SLAB_BASE_CONNECTION_MESH_NAME));
            }

            MergeDuplicateNodes(meshToMergeNodes);
        }
    }

    std::vector<Body*> BaseComponent::GetGeoModelOccs(FemPart *pFemPart, const std::string &bodyPrtName, const std::string &bodyName)
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

    void BaseComponent::UpdateRailSlabConnection()
    {
        BaseProjectProperty *pPrjProp   = Project::Instance()->GetProperty();
        CAE::FemPart        *pFemPart   = pPrjProp->GetRailSlabFemPart();

        std::vector<TaggedObject*>  railConnectPts;
        std::vector<TaggedObject*>  slabConnectPts;

        railConnectPts = GetPointByLayer(pFemPart, RAIL_CONNECTION_POINT_LAYER);
        slabConnectPts = GetPointByLayer(pFemPart, SLAB_CONNECT_TO_RAIL_POINT_LAYER);

        Update1DConnection(pFemPart->BaseFEModel(), railConnectPts, slabConnectPts,
            RAIL_SLAB_CONNECTION_NAME, RAIL_SLAB_CONNECTION_COLLECTOR_NAME);
    }

    void BaseComponent::UpdateBaseSlabConnection()
    {
        BaseProjectProperty *pPrjProp   = Project::Instance()->GetProperty();
        CAE::FemPart        *pFemPart   = pPrjProp->GetBraceFemPart();

        std::vector<TaggedObject*>  slabConnectPts;
        std::vector<TaggedObject*>  baseConnectPts;

        std::vector<TaggedObject*>  slabPartConnectPts;
        std::vector<TaggedObject*>  basePartConnectPts;

        //  get left side points
        slabPartConnectPts = GetPointByLayer(pFemPart, SLAB_CONNECT_TO_BASE_LEFT_POINT_LAYER);
        basePartConnectPts = GetPointByLayer(pFemPart, BASE_CONNECT_TO_SLAB_LEFT_POINT_LAYER);

        slabConnectPts.insert(slabConnectPts.end(), slabPartConnectPts.begin(), slabPartConnectPts.end());
        baseConnectPts.insert(baseConnectPts.end(), basePartConnectPts.begin(), basePartConnectPts.end());

        //  get right side points
        slabPartConnectPts = GetPointByLayer(pFemPart, SLAB_CONNECT_TO_BASE_RIGHT_POINT_LAYER);
        basePartConnectPts = GetPointByLayer(pFemPart, BASE_CONNECT_TO_SLAB_RIGHT_POINT_LAYER);

        slabConnectPts.insert(slabConnectPts.end(), slabPartConnectPts.begin(), slabPartConnectPts.end());
        baseConnectPts.insert(baseConnectPts.end(), basePartConnectPts.begin(), basePartConnectPts.end());

        Update1DConnection(pFemPart->BaseFEModel(), slabConnectPts, baseConnectPts,
            SLAB_BASE_CONNECTION_NAME, SLAB_BASE_CONNECTION_COLLECTOR_NAME);
    }

    void BaseComponent::SetFeGeometryData( FemPart * pFemPart, const std::vector<Body*> &bodyOccs, bool syncLines )
    {
        scoped_ptr<FemSynchronizeOptions> psyncData;

        psyncData.reset(pFemPart->NewFemSynchronizeOptions());
        psyncData->SetSynchronizeLinesFlag(syncLines);
        psyncData->SetSynchronizePointsFlag(true);

        pFemPart->SetGeometryData(FemPart::UseBodiesOptionSelectedBodies, bodyOccs, psyncData.get());

        //pFemPart->BaseFEModel()->UpdateFemodel();
    }

    class FELabelUpdater
    {
    public:
        FELabelUpdater()
        {
        }

        ~FELabelUpdater()
        {
        }

        void Update();
    protected:

        int GetMaxElementLabel(FEElementLabelMap *pElemLabelMap) const;
        int GetMaxNodeLabel(FENodeLabelMap *pNodeLabelMap) const;

    private:
        FEModelOccurrence* GetFEModelOcc(const IFEModel *pFEModel) const;
    };

    void FELabelUpdater::Update()
    {
        BaseProjectProperty *pPrjProp    = Project::Instance()->GetProperty();
        CAE::AssyFemPart    *pAssemPart  = pPrjProp->GetAFemPart();

        AssyFEModel         *pAssyFEModel = dynamic_cast<AssyFEModel*>(pAssemPart->BaseFEModel());

        std::vector<FEModelOccurrence*> feModelChildren(pAssyFEModel->GetChildren());

        if (feModelChildren.size() == 2)
        {
            feModelChildren[0]->SetLabelOffsets(0, 0, 0);

            feModelChildren[1]->SetLabelOffsets(GetMaxNodeLabel(feModelChildren[0]->FenodeLabelMap()),
                GetMaxElementLabel(feModelChildren[0]->FeelementLabelMap()), 0);
        }
    }

    int FELabelUpdater::GetMaxElementLabel(FEElementLabelMap *pElemLabelMap) const
    {
        int   maxLabel = 0;
        int   curLabel = 0;

        for (int idx = 0; idx < pElemLabelMap->NumElements(); idx++)
        {
            curLabel = pElemLabelMap->AskNextElementLabel(curLabel);
            if (curLabel > maxLabel)
                maxLabel = curLabel;
        }

        return maxLabel;
    }

    int FELabelUpdater::GetMaxNodeLabel(FENodeLabelMap *pNodeLabelMap) const
    {
        int   maxLabel = 0;
        int   curLabel = 0;

        for (int idx = 0; idx < pNodeLabelMap->NumNodes(); idx++)
        {
            curLabel = pNodeLabelMap->AskNextNodeLabel(curLabel);
            if (curLabel > maxLabel)
                maxLabel = curLabel;
        }

        return maxLabel;
    }

    void BaseComponent::UpdateAssembleModel()
    {
        // Update Node and Element offset anyway
//        if (CanUpdateRailSlabFEModel())
        {
            FELabelUpdater feLabelUpdater;

            feLabelUpdater.Update();
        }

        // merge duplicate nodes
        std::vector<Mesh*> meshToMergeNodes;

        BaseProjectProperty *pPrjProp    = Project::Instance()->GetProperty();

        if (pPrjProp->GetProjectType() == Project::ProjectType_Tunnel)
        {
            CAE::BaseFemPart        *pBaseFem    = pPrjProp->GetAFemPart();

            AssyFEModel *pAFEModel = polymorphic_cast<AssyFEModel*>(pBaseFem->BaseFEModel());

            FEModelOccurrence *pRailFeModelOcc = GetFEModelOccByMeshName(pAFEModel, RAIL_MESH_NAME);

            //  add to slab meshes to merge list
            std::vector<Mesh*> slabMeshes(GetMeshesInCollector(pRailFeModelOcc,
                FIND_MESH_COL_OCC_PATTERN_NAME, SLAB_MESH_COLLECTOR_NAME));

            meshToMergeNodes.insert(meshToMergeNodes.end(), slabMeshes.begin(), slabMeshes.end());

            IMeshManager *pMeshMgr  = pRailFeModelOcc->MeshManager();

            //std::string    strMeshFindName((boost::format(FIND_MESH_COL_OCC_PATTERN_NAME) % SLAB_MESH_COLLECTOR_NAME).str());
            //polymorphic_cast<IMeshCollector*>(pMeshMgr->FindObject(strMeshFindName.c_str()));
            //meshToMergeNodes.push_back(GetMeshByName(pRailFeModelOcc,
            //    FIND_MESH_OCC_PATTERN_NAME, SLAB_MESH_NAME));

            std::vector<FEModelOccurrence*>  childFeModelOcc(pAFEModel->GetChildren());

            std::vector<FEModelOccurrence*>::iterator iter = std::find_if(childFeModelOcc.begin(), childFeModelOcc.end(),
                lambda::_1 != pRailFeModelOcc);

            FEModelOccurrence *pBaseFeModelOcc = (iter != childFeModelOcc.end()) ? *iter : NULL;

            //  add slab-base connection mesh to merge list
            meshToMergeNodes.push_back(GetMeshByName(pBaseFeModelOcc,
                FIND_MESH_OCC_PATTERN_NAME, SLAB_BASE_CONNECTION_MESH_NAME));
        }

        MergeDuplicateNodes(meshToMergeNodes);
    }

    void BaseComponent::OnInit()
    {
    }

    void BaseComponent::UpdateSweptMesh(IFEModel *pFeModel, const std::vector<CAEBody*> &pPolygonBodies,
                                       const std::string &meshColName, const std::string &meshName,
                                       const std::string &eleSizeExpName)
    {
        std::vector<CAEFace*>  vCaeFaces;

        MeshManager   *pMeshMgr = polymorphic_cast<MeshManager*>(pFeModel->MeshManager());

        std::string meshColFullName((boost::format(FIND_MESH_COL_PATTERN_NAME) % meshColName).str());

        MeshCollector *pMeshCol = polymorphic_cast<MeshCollector*>(pMeshMgr->FindObject(meshColFullName.c_str()));

        for (int idx = int(pPolygonBodies.size()) - 1; idx >= 0; idx--)
        {
            vCaeFaces = GetCaeFaceByName(pPolygonBodies[idx], FACE_NAME_TOP);

            CAEFace *pTopFace    = vCaeFaces.empty() ? NULL : vCaeFaces[0];

            vCaeFaces = GetCaeFaceByName(pPolygonBodies[idx], FACE_NAME_BOTTOM);

            CAEFace *pBottomFace = vCaeFaces.empty() ? NULL : vCaeFaces[0];

            std::string  curMeshName(meshName + "(" + boost::lexical_cast<std::string>(pPolygonBodies.size() - idx) + ")");

            CreateSweptMesh(pMeshMgr, pMeshCol, curMeshName, pTopFace, pBottomFace, eleSizeExpName);
        }
    }

    void BaseComponent::UpdateSweptMesh_sf(IFEModel *pFeModel, const std::vector<CAEBody*> &pPolygonBodies,
                                       const std::string &meshColName, const std::string &meshName,
                                       Expression *pEleSize)
    {
        std::vector<CAEFace*>  vCaeFaces;

        for (unsigned int idx = 0; idx < pPolygonBodies.size(); idx++)
        {
            vCaeFaces = GetCaeFaceByName(pPolygonBodies[idx], FACE_NAME_TOP);

            CAEFace *pTopFace    = vCaeFaces.empty() ? NULL : vCaeFaces[0];

            CreateSweptMesh_sf(pFeModel, pPolygonBodies[idx], meshColName, meshName, pTopFace, pEleSize);
        }
    }

    void BaseComponent::MergeDuplicateNodes(const std::vector<Mesh*> &meshToMergeNodes)
    {
        int    numDuplicates = 0;
        double tolerance     = 0.001;

        std::vector<tag_t>  tMeshesToMearge(meshToMergeNodes.size());

        for (int idx = 0; idx < (int)(meshToMergeNodes.size()); idx++)
        {
            tag_p_t                     tMeshPtrs  = NULL;
            int                         meshCnt;

            UF_SF_locate_all_meshes(meshToMergeNodes[idx]->Tag(), &meshCnt, &tMeshPtrs);

            boost::shared_array<tag_t>  tMeshPtrArray(tMeshPtrs, UF_free);

            if (meshCnt > 0)
                tMeshesToMearge[idx] = tMeshPtrs[0];
        }
        //std::transform(meshToMergeNodes.begin(), meshToMergeNodes.end(),
        //               tMeshesToMearge.begin(), boost::bind(&Mesh::Tag, _1));

        int iErr = UF_SF_check_model_duplicate_nodes(static_cast<int>(tMeshesToMearge.size()),
            tMeshesToMearge.empty() ? NULL_TAG : &tMeshesToMearge[0],
            true, tolerance, &numDuplicates);

        if (iErr != 0)
            throw NXException::Create(iErr);
    }

    bool BaseComponent::CanUpdateRailSlabFEModel() const
    {
        return false;
    }

    bool BaseComponent::CanUpdateBraseFEModel() const
    {
        return false;
    }

    bool BaseComponent::CanUpdateRailSlabConnection() const
    {
        return false;
    }

    bool BaseComponent::CanUpdateBraseConnection() const
    {
        return false;
    }
}
