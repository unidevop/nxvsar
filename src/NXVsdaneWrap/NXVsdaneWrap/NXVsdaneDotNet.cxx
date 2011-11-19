// This is the main DLL file.

#include "NXVsdaneDotNet.hxx"

using namespace NXOpen;
using namespace NXOpen::CAE;

namespace Vsdane
{
    void NXVsdane::CreateSweptMeshDotNet(IFEModel ^pFeModel,
        String^ meshColName, String ^meshName,
        CAEFace ^pSrcFace, CAEFace ^pTargetFace, Expression ^pEleSize)
    {
        Session ^pSession = Session::GetSession();

        MeshManager ^pMeshMgr = safe_cast<MeshManager^>(pFeModel->MeshManager);

        Mesh3dHexBuilder ^pMesh3dHexBuilder = pMeshMgr->CreateMesh3dHexBuilder(nullptr);

        Session::UndoMarkId undoMark = pSession->SetUndoMark(Session::MarkVisibility::Invisible, "Create Swept Mesh");

        try
        {
            String ^meshColFullName = "MeshCollector[" + meshColName + "]";

            MeshCollector ^meshCol(safe_cast<MeshCollector^>(pMeshMgr->FindObject(meshColFullName)));

            ElementTypeBuilder ^pEleTypeBuilder = pMesh3dHexBuilder->ElementType;

            pEleTypeBuilder->ElementDimension = ElementTypeBuilder::ElementType::SweepSolid;
            pEleTypeBuilder->ElementTypeName = "CHEXA(20)";

            DestinationCollectorBuilder ^pDstColBulder   = pEleTypeBuilder->DestinationCollector;

            pDstColBulder->ElementContainer = meshCol;
            pDstColBulder->AutomaticMode = false;

            pMesh3dHexBuilder->CreationType = Mesh3dHexBuilder::Type::Manual;

            //DisplayableObject^[] srcFaces(1, pSrcFace);

            bool bSrcFaceAdded = pMesh3dHexBuilder->SourceFaceList->Add(pSrcFace);

            pMesh3dHexBuilder->TargetFace->Value = pTargetFace;

            PropertyTable ^pPropTable = pMesh3dHexBuilder->PropertyTable;

            pPropTable->SetBooleanPropertyValue("mapped mesh option bool", false);
            pPropTable->SetIntegerPropertyValue("quad only option", 0);
            pPropTable->SetBooleanPropertyValue("project vertices option", false);
            pPropTable->SetBooleanPropertyValue("target face smoothing option", false);

            //  set element size
            Expression ^pEleSizeExp = pPropTable->GetScalarPropertyValue("source element size");

            pEleSizeExp->RightHandSide = pEleSize->RightHandSide;
            //        pPropTable->SetScalarPropertyValue("source element size", pEleSizeExp);

            int nErrs = pSession->UpdateManager->DoUpdate(undoMark);

#ifdef DEBUG
            if (nErrs > 0)
            {
                ListingWindow ^pLstWnd = pSession->ListingWindow;

                pLstWnd->Open();
                pLstWnd->WriteLine("\n");

                ErrorList ^pErrLst = pSession->UpdateManager->ErrorList;

                for (int idx = 0; idx < nErrs; idx++)
                {
                    pLstWnd->WriteLine(pErrLst->GetErrorInfo(idx)->ErrorCode.ToString());
                    pLstWnd->WriteLine(pErrLst->GetErrorInfo(idx)->Description);
                    pLstWnd->WriteLine(pErrLst->GetErrorInfo(idx)->ErrorObjectDescription);
                    pLstWnd->WriteLine("\n");
                }

                pErrLst->Clear();
            }
#endif

            array<CAE::Mesh^>^ createdMeshes = pMesh3dHexBuilder->CommitMesh();
            int idx = 0;
            for each(Mesh^ swepMesh in createdMeshes)
                swepMesh->SetName(meshName + "_" + ++idx);
        }
        finally
        {
            pMesh3dHexBuilder->Destroy();
            pSession->DeleteUndoMark(undoMark, nullptr);
        }
    }
}