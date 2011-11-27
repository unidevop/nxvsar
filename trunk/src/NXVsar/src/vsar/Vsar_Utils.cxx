
#include <uf_defs.h>
#include <Vsar_Utils.hxx>

#include <uf.h>
#include <uf_ui.h>

#if 0
#include <uf_assem.h>
#include <boost/shared_array.hpp>
#endif

#include <sstream>

#include <boost/filesystem.hpp>
#include <boost/scope_exit.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/bind.hpp>
#include <boost/cast.hpp>

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

#include <NXOpen/CAE_IFEModel.hxx>
#include <NXOpen/CAE_CAEBody.hxx>
#include <NXOpen/CAE_CAEFace.hxx>
#include <NXOpen/CAE_MeshManager.hxx>
#include <NXOpen/CAE_SweptMesh.hxx>
#include <NXOpen/CAE_Mesh3dHexBuilder.hxx>
#include <NXOpen/CAE_MeshCollector.hxx>
#include <NXOpen/CAE_ElementTypeBuilder.hxx>
#include <NXOpen/CAE_DestinationCollectorBuilder.hxx>
#include <NXOpen/CAE_PropertyTable.hxx>

#include <uf_sf.h>
//#include <uf_so.h>

#ifdef DEBUG
#include <boost/lexical_cast.hpp>
#include <NXOpen/ListingWindow.hxx>
#endif

#include <Vsar_Project.hxx>

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
    int ReportError(char *file, int line, char *call, int irc)
    {
        if (irc)
        {
            char messg[MAX_LINE_SIZE + 1]/*, str1[100]*/;

            //UF_get_fail_message(irc, messg) ?
            //    sprintf(str1, "returned a %d\n", irc) :
            //sprintf(str1, "returned error %d:  %s\n", irc, messg);

            std::stringstream  errMsgStream;

            if (UF_get_fail_message(irc, messg))
                errMsgStream << "returned a " << irc << "\n";
            else
                errMsgStream << "returned error " << irc << ":  " << messg << "\n";
#ifdef DEBUG
            {
                char /*str[300], */fName[MAX_ENTITY_NAME_SIZE+1], ext[_MAX_EXT];

                _splitpath(file, NULL, NULL, fName, ext);
                //sprintf(str, "%s%s, line %d:  %s\n", fName, ext, line, call);
                //strcat(str, str1);

                std::string  errMsg(errMsgStream.str());

                errMsgStream.clear();
                errMsgStream << fName << ext << ", line " << line << ":  " << call << "\n" << errMsg;

                UF_UI_open_listing_window();
                UF_UI_write_listing_window(errMsgStream.str().c_str());
                //UF_UI_write_listing_window(str);
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
            if (bodyName.compare((*iter)->Name().GetUTF8Text()) == 0)
                bodyProtos.push_back(*iter);
        }

        return bodyProtos;
    }

    //  Select points by Attribute Name
    std::vector<Point*> GetPointByAttrName( BasePart * pPtPrt, const std::string &attrName )
    {
        PointCollection    *pPtOccCol = pPtPrt->Points();
        std::vector<Point*> ptSel;

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
    std::vector<Point*> GetPointByLayer( BasePart * pPtPrt, int layer )
    {
        PointCollection    *pPtCol = pPtPrt->Points();
        std::vector<Point*> ptSel;

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
        pEleTypeBuilder->SetElementTypeName("CHEXA(20)");

        DestinationCollectorBuilder *pDstColBulder   = pEleTypeBuilder->DestinationCollector();

        pDstColBulder->SetElementContainer(pMeshCol);
        pDstColBulder->SetAutomaticMode(false);

        pMesh3dHexBuilder->SetCreationType(Mesh3dHexBuilder::TypeManual);

        bool bSrcFaceAdded = pMesh3dHexBuilder->SourceFaceList()->Add(pSrcFace);

        pMesh3dHexBuilder->TargetFace()->SetValue(pTargetFace);

        PropertyTable *pPropTable = pMesh3dHexBuilder->PropertyTable();

        pPropTable->SetBooleanPropertyValue("mapped mesh option bool", false);
        pPropTable->SetIntegerPropertyValue("quad only option", 0);
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

    std::vector<CAEFace*> GetCaeFaceByName(const CAEBody *pCaeBody, const std::string &faceName)
    {
        std::vector<CAEFace*> caeFaces;
        int                   numFaces;
        tag_t                *tFaces   = NULL;
        boost::shared_array<tag_t>  tFacesArray(tFaces, UF_free);

        UF_SF_body_ask_faces(pCaeBody->Tag(), &numFaces, &tFaces);

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
