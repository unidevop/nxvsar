
#include <uf_defs.h>
#include <Vsar_Component.hxx>

#include <algorithm>
#include <boost/scope_exit.hpp>
#include <boost/bind.hpp>
#include <boost/cast.hpp>

#include <NXOpen/Session.hxx>
//#include <NXOpen/NXException.hxx>
#include <NXOpen/PartCollection.hxx>
#include <NXOpen/BasePart.hxx>
#include <NXOpen/ExpressionCollection.hxx>

#include <NXOpen/CAE_SimPart.hxx>
#include <NXOpen/CAE_FemPart.hxx>
#include <NXOpen/CAE_AssyFemPart.hxx>
#include <NXOpen/CAE_BaseFEModel.hxx>
#include <NXOpen/CAE_CAEBody.hxx>
#include <NXOpen/CAE_CAEFace.hxx>

#include <Vsar_Project.hxx>
#include <Vsar_Init_Utils.hxx>
#include <Vsar_Names.hxx>
#include <Vsar_Utils.hxx>

using namespace boost;
using namespace NXOpen;
using namespace NXOpen::CAE;
using namespace Vsar;

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

        LoadGeometryPart();
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

        UpdateFECompModel(pRailSlabFem, boost::bind(&BaseComponent::UpdateRailSlabModel, this));

        CAE::FemPart        *pBraceFem      = pPrjProp->GetBraceFemPart();

        UpdateFECompModel(pBraceFem, boost::bind(&BaseComponent::UpdateBraseModel, this));
    }

    template <typename UpdateCallback>
    void BaseComponent::UpdateFECompModel(CAE::FemPart *pFem, UpdateCallback updateCb)
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
        catch (...)
        {
            pSession->UndoToLastVisibleMark();
            throw;
        }

        //pSession->DeleteUndoMark(undoMark, NULL);
    }

    void BaseComponent::UpdateRailSlabModel()
    {
    }

    void BaseComponent::UpdateBraseModel()
    {
    }

    void BaseComponent::OnInit()
    {
    }

    void BaseComponent::UpdateSwepMesh(IFEModel *pFeModel, const std::vector<CAEBody*> &pPolygonBodies,
                                       const std::string &meshColName, const std::string &meshName,
                                       Expression *pEleSize)
    {
        std::vector<CAEFace*>  vCaeFaces;

        for (unsigned int idx = 0; idx < pPolygonBodies.size(); idx++)
        {
            vCaeFaces = GetCaeFaceByName(pPolygonBodies[idx], FACE_NAME_TOP);

            CAEFace *pTopFace    = vCaeFaces.empty() ? NULL : vCaeFaces[0];

            vCaeFaces = GetCaeFaceByName(pPolygonBodies[idx], FACE_NAME_BOTTOM);

            CAEFace *pBottomFace = vCaeFaces.empty() ? NULL : vCaeFaces[0];

            CreateSweptMesh(pFeModel, meshColName, meshName, pTopFace, pBottomFace, pEleSize);
        }
    }

    void BaseComponent::UpdateSwepMesh_sf(IFEModel *pFeModel, const std::vector<CAEBody*> &pPolygonBodies,
                                       const std::string &meshColName, const std::string &meshName,
                                       Expression *pEleSize)
    {
        std::vector<CAEFace*>  vCaeFaces;

        for (unsigned int idx = 0; idx < pPolygonBodies.size(); idx++)
        {
            vCaeFaces = GetCaeFaceByName(pPolygonBodies[idx], FACE_NAME_TOP);

            CAEFace *pTopFace    = vCaeFaces.empty() ? NULL : vCaeFaces[0];

            //vCaeFaces = GetCaeFaceByName(pPolygonBodies[idx], FACE_NAME_BOTTOM);

            //CAEFace *pBottomFace = vCaeFaces.empty() ? NULL : vCaeFaces[0];

            CreateSweptMesh_sf(pFeModel, pPolygonBodies[idx], meshColName, meshName, pTopFace, pEleSize);
        }
    }
}
