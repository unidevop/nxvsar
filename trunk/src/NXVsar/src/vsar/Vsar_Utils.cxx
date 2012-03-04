
#include <uf_defs.h>
#include <Vsar_Utils.hxx>

#include <uf.h>
#include <uf_ui.h>
#include <uf_obj.h>

#if 0
#include <uf_assem.h>
#include <boost/shared_array.hpp>
#endif

#include <sstream>
#include <functional>
#include <algorithm>

#include <boost/filesystem.hpp>
#include <boost/scope_exit.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/bind.hpp>
//#include <boost/lambda/lambda.hpp>
//#include <boost/lambda/bind.hpp>
#include <boost/cast.hpp>
#include <boost/foreach.hpp>

#include <NXOpen/Session.hxx>
#include <NXOpen/NXException.hxx>
#include <NXOpen/PartCollection.hxx>
#include <NXOpen/Part.hxx>
//#include <NXOpen/CAE_FemPart.hxx>
#include <NXOpen/Body.hxx>
#include <NXOpen/Point.hxx>
#include <NXOpen/BodyCollection.hxx>
#include <NXOpen/PointCollection.hxx>
#include <NXOpen/NXObjectManager.hxx>
//#include <NXOpen/UnitCollection.hxx>
//#include <NXOpen/Unit.hxx>
//#include <NXOpen/ExpressionCollection.hxx>
#include <NXOpen/Expression.hxx>
#include <NXOpen/SelectDisplayableObjectList.hxx>
#include <NXOpen/SelectDisplayableObject.hxx>
#include <NXOpen/Assemblies_Component.hxx>

#include <NXOpen/CAE_BaseFEModel.hxx>
#include <NXOpen/CAE_BaseFemPart.hxx>
#include <NXOpen/CAE_CAEBody.hxx>
#include <NXOpen/CAE_CAEFace.hxx>
#include <NXOpen/CAE_MeshManager.hxx>
#include <NXOpen/CAE_SweptMesh.hxx>
#include <NXOpen/CAE_Mesh3dHexBuilder.hxx>
#include <NXOpen/CAE_MeshCollector.hxx>
#include <NXOpen/CAE_FENode.hxx>
#include <NXOpen/CAE_ElementTypeBuilder.hxx>
#include <NXOpen/CAE_DestinationCollectorBuilder.hxx>
#include <NXOpen/CAE_PropertyTable.hxx>

#include <NXOpen/CAE_CAEConnectionCollection.hxx>
#include <NXOpen/CAE_CAEConnection.hxx>
#include <NXOpen/CAE_CAEConnectionBuilder.hxx>
#include <NXOpen/CAE_FEModelOccurrence.hxx>
#include <NXOpen/CAE_IMeshManager.hxx>
#include <NXOpen/CAE_SimPart.hxx>
#include <NXOpen/CAE_SimSimulation.hxx>
#include <NXOpen/CAE_SmartSelectionManager.hxx>
#include <NXOpen/CAE_RelatedNodeMethod.hxx>
#include <NXOpen/SelectTaggedObjectList.hxx>


#include <uf_sf.h>
//#include <uf_so.h>

#ifdef DEBUG
#include <boost/lexical_cast.hpp>
#include <NXOpen/ListingWindow.hxx>
#endif

#include <Vsar_Project.hxx>
#include <Vsar_Names.hxx>

using namespace boost;
using namespace NXOpen;
using namespace NXOpen::Assemblies;
using namespace NXOpen::CAE;
using namespace Vsar;

//------------------------------------------------------------------------------
// Declaration of global variables
//------------------------------------------------------------------------------
namespace Vsar
{
    int ReportError(const std::string &file, int line, const std::string &call, int irc)
    {
        if (irc)
        {
            char messg[MAX_LINE_SIZE + 1]/*, str1[100]*/;

            std::stringstream  errMsgStream;

            if (UF_get_fail_message(irc, messg))
                errMsgStream << "returned a " << irc << "\n";
            else
                errMsgStream << "returned error " << irc << ":  " << messg << "\n";
#if defined(DEBUG) || defined(_DEBUG)
            {
                filesystem::path  filePath(file);

                std::string  errMsg(errMsgStream.str());

                errMsgStream.clear();
                errMsgStream << filePath.filename() << ", line " << line << ":  " << call << "\n" << errMsg;

                UF_UI_open_listing_window();
                UF_UI_write_listing_window(errMsgStream.str().c_str());
            }
#else
            UF_UI_set_status(const_cast<char*>(errMsgStream.str().c_str()));
#endif
            throw NXException::Create(irc);
        }
        return(irc);
    }

    void LoadRootPart()
    {
        Project* pPrj = Project::Instance();

        if (pPrj)
        {
            Session        *pSession = Session::GetSession();
            PartCollection *pPrtCol  = pSession->Parts();
            BaseProjectProperty *pPrjProp = pPrj->GetProperty();

            boost::filesystem::path    rootPathName(pPrjProp->GetProjectPath());

            rootPathName /= pPrjProp->GetRootPartName();

            PartLoadStatus *pPrtLoadStatus = NULL;

            Part* pPrt = pPrtCol->Open(rootPathName.string().c_str(), &pPrtLoadStatus);

            pPrt->LoadFully();

            BOOST_SCOPE_EXIT((&pPrtLoadStatus))
            {
                delete pPrtLoadStatus;
            }
            BOOST_SCOPE_EXIT_END

            //pPrtRoot->LoadFully();
        }
    }

    std::vector<Body*> GetBodyByName( Part * pBodyPrt, const std::string &bodyName )
    {
        BodyCollection    *pBodyProtoCol = pBodyPrt->Bodies();
        std::vector<Body*> bodyProtos;

        //  Get body prototypes
        for (BodyCollection::iterator iter = pBodyProtoCol->begin(); iter != pBodyProtoCol->end(); ++iter)
        {
            if (bodyName.compare((*iter)->Name().GetText()) == 0)
                bodyProtos.push_back(*iter);
        }

        return bodyProtos;
    }

    //  Select points by Attribute Name
    std::vector<TaggedObject*> GetPointByAttrName( BasePart * pPtPrt, const std::string &attrName )
    {
        PointCollection    *pPtOccCol = pPtPrt->Points();
        std::vector<TaggedObject*> ptSel;

        typedef std::vector<NXObject::AttributeInformation> StlAttrInfoVector;

        //boost::function<bool (StlAttrInfoVector::iterator)> findAlgo = ;
        //  Get body prototypes
        for (PointCollection::iterator iter = pPtOccCol->begin(); iter != pPtOccCol->end(); ++iter)
        {
            StlAttrInfoVector vAttrInfo((*iter)->GetAttributeTitlesByType(NXObject::AttributeTypeNull));

            //if ( std::find_if(vAttrInfo.begin(), vAttrInfo.end(),
            //    boost::bind(&std::string::compare, attrName, 
            //                 boost::bind<const char*>(&NXString::GetText, 
            //                            boost::bind<NXString>(&NXObject::AttributeInformation::Title, _1))) == 0)
            //           != vAttrInfo.end() )
            for (unsigned int idx = 0; idx < vAttrInfo.size(); idx++)
            {
                if (vAttrInfo[idx].Title.GetText() == attrName)
                {
                    ptSel.push_back(*iter);
                    break;
                }
            }
        }

        return ptSel;
    }

    //  Select points by layer
    std::vector<TaggedObject*> GetPointByLayer( BasePart * pPtPrt, int layer )
    {
        PointCollection    *pPtCol = pPtPrt->Points();
        std::vector<TaggedObject*> ptSel;

        //  Get body prototypes
        for (PointCollection::iterator iter = pPtCol->begin(); iter != pPtCol->end(); ++iter)
        {
            if ((*iter)->Layer() == layer)
                ptSel.push_back(*iter);
        }

        return ptSel;
    }

    std::vector<Assemblies::Component*> GetOccInCompTree(Assemblies::Component *pAssemTree, Part *pPrt)
    {
        std::vector<Assemblies::Component*>  prtOccs;

        if (pAssemTree && pPrt)
        {
            //  NOTE: There's problem with this API: Component::GetChildren()
            std::vector<Assemblies::Component*>  childComps(pAssemTree->GetChildren());

            for (unsigned int idx = 0; idx < childComps.size(); idx++)
            {
                Component *pChildOcc = childComps[idx];

                if (pChildOcc && pChildOcc->Prototype() == pPrt)
                    prtOccs.push_back(pChildOcc);
                else
                {
                    std::vector<Assemblies::Component*>  childOccComps(GetOccInCompTree(pChildOcc, pPrt));

                    prtOccs.insert(prtOccs.end(), childOccComps.begin(), childOccComps.end());
                }
            }
        }

        return prtOccs;
    }

    void EditSweptMeshData(IFEModel *pFeModel, const std::string &meshName,
                        const std::vector<Body*> &srcBodies)
    {
        if (!pFeModel)
            return;

        //MeshManager   *pMeshMgr = polymorphic_cast<MeshManager*>(pFeModel->MeshManager());
        //SweptMesh     *pMesh3d  = polymorphic_cast<SweptMesh*>(pMeshMgr->FindObject(meshName.c_str()));
        //boost::shared_ptr<Mesh3dHexBuilder> pMesh3dBuilder(pMeshMgr->CreateMesh3dHexBuilder(pMesh3d),
        //                                    boost::bind(&Builder::Destroy, _1));

        //std::vector<DisplayableObject*> meshBody(GetCaeBodies(srcBodies));

        //logical success = pMesh3dBuilder->SelectionList()->Add(meshBody);
        //std::vector<Mesh *> pvMesh = pMesh3dBuilder->CommitMesh();
    }

    void CreateSweptMesh( MeshManager *pMeshMgr, MeshCollector *pMeshCol, const std::string &meshName,
                          CAEFace* pSrcFace, CAEFace *pTargetFace, const std::string &eleSizeExpName)
    {
        Session *pSession = Session::GetSession();

        boost::shared_ptr<Mesh3dHexBuilder> pMesh3dHexBuilder(pMeshMgr->CreateMesh3dHexBuilder(NULL),
                                                              boost::bind(&Builder::Destroy, _1));

        ElementTypeBuilder          *pEleTypeBuilder = pMesh3dHexBuilder->ElementType();

        pEleTypeBuilder->SetElementDimension(CAE::ElementTypeBuilder::ElementTypeSweepSolid);
        pEleTypeBuilder->SetElementTypeName("CHEXA(8)");

        DestinationCollectorBuilder *pDstColBulder   = pEleTypeBuilder->DestinationCollector();

        pDstColBulder->SetElementContainer(pMeshCol);
        pDstColBulder->SetAutomaticMode(false);

        pMesh3dHexBuilder->SetCreationType(Mesh3dHexBuilder::TypeManual);

        bool bSrcFaceAdded = pMesh3dHexBuilder->SourceFaceList()->Add(pSrcFace);

        pMesh3dHexBuilder->TargetFace()->SetValue(pTargetFace);

        PropertyTable *pPropTable = pMesh3dHexBuilder->PropertyTable();

        pPropTable->SetBooleanPropertyValue("mapped mesh option bool", false);
        pPropTable->SetIntegerPropertyValue("quad only option", 1);
        pPropTable->SetBooleanPropertyValue("project vertices option", false);
        pPropTable->SetBooleanPropertyValue("target face smoothing option", false);

        //  set element size
        Expression *pEleSizeExp = pPropTable->GetScalarPropertyValue("source element size");

        pEleSizeExp->SetRightHandSide(eleSizeExpName);
//        pPropTable->SetScalarPropertyValue("source element size", pEleSizeExp);

        Session::UndoMarkId undoMark = pSession->SetUndoMark(Session::MarkVisibilityInvisible, "Create Swept Mesh");

        int nErrs = pSession->UpdateManager()->DoUpdate(undoMark);

#ifdef DEBUG
        if (nErrs > 0)
        {
            ListingWindow *pLstWnd = pSession->ListingWindow();

            pLstWnd->Open();
            pLstWnd->WriteLine("\n");

            ErrorList *pErrLst = pSession->UpdateManager()->ErrorList();

            for (int idx = 0; idx < nErrs; idx++)
            {
                pLstWnd->WriteLine((lexical_cast<std::string>(pErrLst->GetErrorInfo(idx)->ErrorCode())).c_str());
                pLstWnd->WriteLine(pErrLst->GetErrorInfo(idx)->Description());
                pLstWnd->WriteLine(pErrLst->GetErrorInfo(idx)->ErrorObjectDescription());
                pLstWnd->WriteLine("\n");
            }

            pErrLst->Clear();
        }
#endif

        std::vector<CAE::Mesh*> createdMeshes(pMesh3dHexBuilder->CommitMesh());
        std::for_each(createdMeshes.begin(), createdMeshes.end(),
            boost::bind(&NXObject::SetName, _1, meshName.c_str()));

        pSession->DeleteUndoMark(undoMark, NULL);
    }

    void CreateSweptMesh_sf(IFEModel *pFeModel, CAEBody *pSolidBody,
                         const std::string &meshColName, const std::string &meshName,
                         CAEFace* pSrcFace, Expression *pEleSize)
    {
        tag_t  tSweptMesh;

        int err = UF_CALL(UF_SF_create_swept_hex_mesh(pSolidBody->Tag(), pSrcFace->Tag(), true,
                    pEleSize->Value(), &tSweptMesh));
#if 0
        if (err == 0)
        {
            MeshManager   *pMeshMgr = polymorphic_cast<MeshManager*>(pFeModel->MeshManager());
            SweptMesh     *pMesh    = dynamic_cast<SweptMesh*>(NXObjectManager::Get(tSweptMesh));

            pMesh->SetName(meshName.c_str());

            std::string srcMeshColName = std::string("MeshCollector[").append(meshColName).append("]");

            MeshCollector *pDstMeshCol(polymorphic_cast<MeshCollector*>(pMeshMgr->FindObject(srcMeshColName.c_str())));

            boost::shared_ptr<Mesh3dHexBuilder> pMesh3dHexBuilder(pMeshMgr->CreateMesh3dHexBuilder(pMesh),
                                                                  boost::bind(&Builder::Destroy, _1));

            ElementTypeBuilder          *pEleTypeBuilder = pMesh3dHexBuilder->ElementType();
            DestinationCollectorBuilder *pSrcColBulder   = pEleTypeBuilder->DestinationCollector();

            //pMeshMgr->DragNDropMesh(pMesh, pSrcColBulder->ElementContainer(), pDstMeshCol);
            PropertyTable *pPropTable = pMesh3dHexBuilder->PropertyTable();

            //  set element size
            Expression *pEleSizeExp = pPropTable->GetScalarPropertyValue("source element size");

            pEleSizeExp->SetRightHandSide(pEleSize->RightHandSide());
            pMesh3dHexBuilder->CommitMesh();
        }
#endif
    }

    void Update1DConnection(BaseFEModel *pFeModel,
                            const std::vector<TaggedObject*> &railConnectPts,
                            const std::vector<TaggedObject*> &slabConnectPts,
                            const std::string &connName,
                            const std::string &connColName)
    {
        CAEConnectionCollection  *pCaeConnCol     = pFeModel->CaeConnections();
        CAEConnection            *pCaeConn        = NULL;
        bool                      createMode      = false;

        try
        {
            pCaeConn = pCaeConnCol->FindObject(std::string("Connection[") + connName + "]");
        }
        catch (NXException&)
        {
            pCaeConn    = NULL;
            createMode  = true;
        }

        boost::shared_ptr<CAEConnectionBuilder> pCaeConnBuilder(pCaeConnCol->CreateConnectionBuilder(pCaeConn), boost::bind(&Builder::Destroy, _1));

        pCaeConnBuilder->SourceSelection()->Add(railConnectPts);
        pCaeConnBuilder->TargetSelection()->Add(slabConnectPts);

        MeshManager   *pMeshMgr = polymorphic_cast<MeshManager*>(pFeModel->MeshManager());

        std::string meshColFullName((boost::format(FIND_MESH_COL_PATTERN_NAME) % connColName).str());

        MeshCollector *pMeshCol = polymorphic_cast<MeshCollector*>(pMeshMgr->FindObject(meshColFullName.c_str()));

        DestinationCollectorBuilder *pDstCol = pCaeConnBuilder->ElementType()->DestinationCollector();
        pDstCol->SetAutomaticMode(false);
        pDstCol->SetElementContainer(pMeshCol);

        pCaeConnBuilder->ElementType()->SetElementTypeName("CBUSH");
        pCaeConnBuilder->SetType(CAEConnectionBuilder::ConnectionTypeEnumPointToPoint);
        pCaeConnBuilder->SetMethodType(CAEConnectionBuilder::MethodTypeEnumProximity);

        pCaeConn = dynamic_cast<CAEConnection*>(pCaeConnBuilder->Commit());

        if (createMode)
        {
            pCaeConn->SetName(connName);
        }
    }

    std::vector<CAEBody*> GetCaeBodies(const std::vector<Body*> &srcBodies)
    {
        std::vector<CAEBody*> caeBodies;

        //  Get CAEBody tag
        caeBodies.reserve(srcBodies.size());
        for (unsigned int idx = 0; idx < srcBodies.size(); idx++)
        {
            tag_t tCaeBody = NULL_TAG;

            UF_SF_modl_body_ask_body(srcBodies[idx]->Tag(), &tCaeBody);
            caeBodies.push_back(dynamic_cast<CAEBody*>(NXObjectManager::Get(tCaeBody)));
        }

        return caeBodies;
    }

    std::vector<CAEFace*> GetCaeFaceByName(CAEBody *pCaeBody, const std::string &faceName)
    {
        std::vector<CAEFace*> caeFaces;
        int                   numFaces;
        tag_t                *tFaces   = NULL;

        UF_SF_body_ask_faces(pCaeBody->Tag(), &numFaces, &tFaces);
        boost::shared_array<tag_t>  tFacesArray(tFaces, UF_free);

        for (int idx = 0; idx < numFaces; idx++)
        {
            CAEFace    *pCaeFace = dynamic_cast<CAEFace*>(NXObjectManager::Get(tFaces[idx]));
            const char *strName  = pCaeFace->Name().GetText();

            if (pCaeFace && faceName.compare(strName ? strName : "") == 0)
            {
                caeFaces.push_back(pCaeFace);
            }
        }

        return caeFaces;
    }

    template<class GeomClass>
    static std::vector<GeomClass*> GetCaeGeomByName(CaePart *pPrt, const std::string &geomName, int geomType)
    {
        std::vector<GeomClass*>  caeGeoms;

        int                  rtc       = 0;
        tag_t                tCaeGeom  = NULL_TAG;

        rtc = UF_OBJ_cycle_by_name_and_type(pPrt->Tag(), geomName.c_str(), geomType, true, &tCaeGeom);
        //rtc = UF_OBJ_cycle_by_name(const_cast<char*>(bodyName), &tCaeBody);
        while (tCaeGeom != NULL_TAG)
        {
            GeomClass *pCaeBody = dynamic_cast<GeomClass*>(NXObjectManager::Get(tCaeGeom));

            if (pCaeBody)
            {
                caeGeoms.push_back(pCaeBody);
            }
            rtc = UF_OBJ_cycle_by_name_and_type(pPrt->Tag(), geomName.c_str(), geomType, true, &tCaeGeom);
        }

        return caeGeoms;
    }

    std::vector<CAEBody*> GetCaeBodyByName(CaePart *pPrt, const std::string &bodyName)
    {
        return GetCaeGeomByName<CAEBody>(pPrt, bodyName, UF_caegeom_type);
    }

    std::vector<CAEFace*> GetCaeFaceByName(CaePart *pPrt, const std::string &faceName)
    {
        return GetCaeGeomByName<CAEFace>(pPrt, faceName, UF_caegeom_type);
    }

    std::vector<CAEFace*> GetCaeFacesOfBodyByName(CaePart *pPrt, const std::string &bodyName, const std::string &faceName)
    {
        std::vector<CAEFace*>  pAllFaces;

        std::vector<CAEBody*>  pBodies(GetCaeBodyByName(pPrt, bodyName));

        for (int idx = 0; idx < int(pBodies.size()); idx++)
        {
            std::vector<CAEFace*>  pFaces(GetCaeFaceByName(pBodies[idx], faceName));

            pAllFaces.insert(pAllFaces.end(), pFaces.begin(), pFaces.end());
        }

        return pAllFaces;
    }

    FEModelOccurrence* GetFEModelOccByMeshName(IHierarchicalFEModel *pHieFeModel, const std::string &meshName)
    {
        FEModelOccurrence   *pFEModelOcc = NULL;

        std::string strMeshFindName((boost::format(FIND_MESH_OCC_PATTERN_NAME) % meshName).str());

        std::vector<FEModelOccurrence*>  childFeModelOcc(pHieFeModel->GetChildren());

        for (std::vector<FEModelOccurrence*>::iterator iter = childFeModelOcc.begin();
            iter != childFeModelOcc.end(); ++iter)
        {
            IMeshManager        *pMeshMgr    = (*iter)->MeshManager();

            try
            {
                Mesh *pRailMesh = polymorphic_cast<Mesh*>(pMeshMgr->FindObject(strMeshFindName.c_str()));
                pFEModelOcc = *iter;
                break;
            }
            catch (std::exception&)
            {
            }
        }

        return pFEModelOcc;
    }

    Mesh* GetMeshByName(IFEModel *pFEModel, const std::string &meshNamePattern, const std::string &meshName)
    {
        IMeshManager *pMeshMgr  = pFEModel->MeshManager();

        std::string    strMeshFindName((boost::format(meshNamePattern) % meshName).str());

        return polymorphic_cast<Mesh*>(pMeshMgr->FindObject(strMeshFindName.c_str()));
    }

    void DeleteMeshesInCollector(IFEModel *pFeModel, const std::string &meshColName)
    {
        std::vector<Mesh*>  existingMeshes(GetMeshesInCollector(pFeModel, FIND_MESH_COL_PATTERN_NAME, meshColName));

        if (!existingMeshes.empty())
        {
            Session *pSession   = Session::GetSession();
            Update  *pUpdateMgr = pSession->UpdateManager();

            BOOST_FOREACH(Mesh *pMesh, existingMeshes)
            {
                pUpdateMgr->AddToDeleteList(pMesh);
            }

            //std::for_each(existingMeshes.begin(), existingMeshes.end(),
            //    boost::bind<void, NXObject*>(boost::bind<void, NXObject*>(&Update::AddToDeleteList, pUpdateMgr), _1));

            pUpdateMgr->DoUpdate(pSession->GetNewestUndoMark(Session::MarkVisibilityAnyVisibility));
        }
    }

    std::vector<Mesh*> GetMeshesInCollector(IFEModel *pFeModel, const std::string &meshNamePattern, const std::string &meshColName)
    {
        IMeshManager   *pMeshMgr = pFeModel->MeshManager();

        std::string meshColFullName((boost::format(meshNamePattern) % meshColName).str());

        IMeshCollector *pMeshCol = polymorphic_cast<IMeshCollector*>(pMeshMgr->FindObject(meshColFullName.c_str()));

        //  delete meshes in the collector first
        std::vector<Mesh*>  existingMeshes(pMeshMgr->GetMeshes());

        if (!existingMeshes.empty())
        {
            existingMeshes.erase(std::remove_if(existingMeshes.begin(), existingMeshes.end(),
                boost::bind(std::not_equal_to<IMeshCollector*>(), boost::bind(&Mesh::MeshCollector, _1), pMeshCol)),
                existingMeshes.end());

            //existingMeshes.erase(std::remove_if(existingMeshes.begin(), existingMeshes.end(),
            //    boost::bind(&Mesh::MeshCollector, _1) != lambda::var(pMeshCol)), existingMeshes.end());
        }

        return existingMeshes;
    }

    std::vector<FENode*> GetNodesOnFace(CaePart *pCaePrt, const std::vector<CAEFace*> &pFaces)
    {
        std::vector<FENode*>    pAllNodes;

        SmartSelectionManager *pSelMgr = pCaePrt->SmartSelectionMgr();

        for (int idx = 0; idx < int(pFaces.size()); idx++)
        {
            boost::scoped_ptr<RelatedNodeMethod> pRelNodeMethod(pSelMgr->CreateRelatedNodeMethod(pFaces[idx]));

            std::vector<FENode*>  slabNodes(pRelNodeMethod->GetNodes());

            pAllNodes.insert(pAllNodes.end(), slabNodes.begin(), slabNodes.end());
        }

        return pAllNodes;
    }

    std::vector<FENode*> GetNodeOcc(FEModelOccurrence *pFeModelOcc, int nodeOffset, const std::vector<FENode*> &nodeProtos)
    {
        std::vector<FENode*> nodeOccs;

        boost::scoped_ptr<FENodeLabelMap> pNodeLabelMap(pFeModelOcc->FenodeLabelMap());

        nodeOccs.reserve(nodeProtos.size());
        BOOST_FOREACH(FENode* pNodeProto, nodeProtos)
        {
            FENode *pNodeOcc = pNodeLabelMap->GetNode(nodeOffset + pNodeProto->Label());

            if (pNodeOcc)
                nodeOccs.push_back(pNodeOcc);
        }

        return nodeOccs;
    }

    int GetNodeOffset(FEModelOccurrence *pFeModelOcc)
    {
        int nodeOffset;
        int elemOffset;
        int csysOffset;

        pFeModelOcc->GetLabelOffsets(&nodeOffset, &elemOffset, &csysOffset);

        return nodeOffset;
    }

    Mesh* GetMesh(FENode *pNode)
    {
        Mesh *pMesh = NULL;

        if (pNode)
        {
            tag_p_t                     tMeshes  = NULL;
            int                         meshCnt;

            UF_SF_find_mesh(pNode->Tag(), UF_SF_DIMENSION_ANY, &meshCnt, &tMeshes);

            boost::shared_array<tag_t>  tMeshArray(tMeshes, UF_free);

            if (meshCnt > 0)
            {
                pMesh = dynamic_cast<Mesh*>(NXObjectManager::Get(tMeshes[0]));
            }
        }

        return pMesh;
    }

    FEModelOccurrence* GetFEModelOccOfNode(FEModelOccurrence *pParentFEModel, FENode *pNodeProto)
    {
        if (!pNodeProto)
            return NULL;

        // Get mesh name
        Mesh *pMesh = GetMesh(pNodeProto);
        if (!pMesh)
            return NULL;

        std::string meshName(pMesh->Name().GetUTF8Text());

        return GetFEModelOccByMeshName(pParentFEModel, meshName);
    }

#if 0
    std::vector<Assemblies::Component*> GetOccInCompTree(Assemblies::Component *pAssemTree, Part *pPrt)
    {
        std::vector<Assemblies::Component*>  prtOccs;

        if (pAssemTree && pPrt)
        {
            //  NOTE: There's problem with this API: Component::GetChildren()
            //std::vector<Assemblies::Component*>  childComps(pAssemTree->GetChildren());

            tag_t *tChildOccs = NULL;
            boost::shared_array<tag_t>  childOccArray(tChildOccs, UF_free);

            int tChildCnt = UF_ASSEM_ask_part_occ_children(pAssemTree->Tag(), &tChildOccs);
            for (int idx = 0; idx < tChildCnt; idx++)
            {
                Component *pChildOcc = dynamic_cast<Component*>(NXObjectManager::Get(tChildOccs[idx]));

                if (pChildOcc && pChildOcc->Prototype() == pPrt)
                    prtOccs.push_back(pChildOcc);
                else
                {
                    std::vector<Assemblies::Component*>  childOccComps(GetOccInCompTree(pChildOcc, pPrt));

                    prtOccs.insert(prtOccs.end(), childOccComps.begin(), childOccComps.end());
                }
            }
        }

        return prtOccs;
    }
#endif
}
