
//------------------------------------------------------------------------------
//These includes are needed for the following template code
//------------------------------------------------------------------------------
#include <uf_defs.h>
#include <Vsar_Project.hxx>

//#include <uf.h>
//#include <uf_clone.h>
//#include <uf_part.h>

#include <algorithm>
//#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>
#include <boost/bind.hpp>

#include <NXOpen/Session.hxx>
#include <NXOpen/NXException.hxx>
#include <NXOpen/PartCollection.hxx>
#include <NXOpen/CAE_SimPart.hxx>
#include <NXOpen/CAE_FemPart.hxx>
#include <NXOpen/CAE_AssyFemPart.hxx>
#include <NXOpen/CAE_FTK_DataManager.hxx>
#include <NXOpen/ListingWindow.hxx>
#include <NXOpen/UI.hxx>
#include <NXOpen/MenuBar_MenuBarManager.hxx>
#include <NXOpen/MenuBar_MenuButton.hxx>

#include <Vsar_Init_Names.hxx>
#include <Vsar_Init_Utils.hxx>

using namespace boost;
using namespace NXOpen;
using namespace NXOpen::CAE;
using namespace NXOpen::MenuBar;

//------------------------------------------------------------------------------
// Declaration of global variables
//------------------------------------------------------------------------------
namespace Vsar
{

    boost::scoped_ptr<Project> Project::m_prjInstance;
    Status              Project::m_prjStatus;

    Status::Status()
    {
    }

    Status::~Status()
    {
    }

    static Status::MenuItemSensitivity s_menuItemSens[] = 
    {
        {MENU_ITEM_NAME_NEW_PROJECT,    Status::ProjectStatus_None | Status::ProjectStatus_Defined | Status::ProjectStatus_ResponseSolved | Status::ProjectStatus_ResponseNoiseSolved | Status::ProjectStatus_NoiseSolved },
        {MENU_ITEM_NAME_SET_TRAIN,      Status::ProjectStatus_Defined | Status::ProjectStatus_ResponseSolved | Status::ProjectStatus_ResponseNoiseSolved | Status::ProjectStatus_NoiseSolved},
        {MENU_ITEM_NAME_SET_RAIL,       Status::ProjectStatus_Defined | Status::ProjectStatus_ResponseSolved | Status::ProjectStatus_ResponseNoiseSolved | Status::ProjectStatus_NoiseSolved},
        {MENU_ITEM_NAME_SET_SLAB,       Status::ProjectStatus_Defined | Status::ProjectStatus_ResponseSolved | Status::ProjectStatus_ResponseNoiseSolved | Status::ProjectStatus_NoiseSolved},
        {MENU_ITEM_NAME_SET_BRACE,      Status::ProjectStatus_Defined | Status::ProjectStatus_ResponseSolved | Status::ProjectStatus_ResponseNoiseSolved | Status::ProjectStatus_NoiseSolved},
        {MENU_ITEM_NAME_SET_BRIDGE,     0},
        {MENU_ITEM_NAME_SET_BASE,       0},
        {MENU_ITEM_NAME_SET_TUNNEL,     0},
        {MENU_ITEM_NAME_EXECUTE_SOLVE,  Status::ProjectStatus_Defined | Status::ProjectStatus_ResponseSolved | Status::ProjectStatus_ResponseNoiseSolved | Status::ProjectStatus_NoiseSolved},
        {MENU_ITEM_NAME_SOLVE_RESPONSE, Status::ProjectStatus_Defined | Status::ProjectStatus_ResponseSolved | Status::ProjectStatus_ResponseNoiseSolved | Status::ProjectStatus_NoiseSolved},
        {MENU_ITEM_NAME_SOLVE_NOISE,    Status::ProjectStatus_ResponseNoiseSolved | Status::ProjectStatus_NoiseSolved},
        {MENU_ITEM_NAME_LOAD_RESULT,    Status::ProjectStatus_Defined | Status::ProjectStatus_ResponseSolved | Status::ProjectStatus_ResponseNoiseSolved | Status::ProjectStatus_NoiseSolved}
    };

    void Status::SetMenuItemSensitivity(const MenuItemSensitivity &menuItemData, ProjectStatus status)
    {
        try
        {
            MenuButton *pMenuButton = UI::GetUI()->MenuBarManager()->GetButtonFromName(menuItemData.m_buttonName.c_str());

            if (pMenuButton)
            {
                MenuButton::SensitivityStatus sens;

                if (menuItemData.m_status)
                {
                    sens = (menuItemData.m_status & status) ?
                        MenuButton::SensitivityStatusSensitive : MenuButton::SensitivityStatusInsensitive;
                }
                else
                {
                    sens = (menuItemData.m_buttonName == Project::Instance()->GetProperty()->GetBraceMenuItemName()) ?
                        MenuButton::SensitivityStatusSensitive : MenuButton::SensitivityStatusInsensitive;
                }

                pMenuButton->SetButtonSensitivity(sens);
            }
        }
        catch (std::exception &)
        {
        }
    }

    void Status::Switch(ProjectStatus status)
    {
        std::for_each(s_menuItemSens, s_menuItemSens + N_ELEMENTS(s_menuItemSens),
            boost::bind(&Status::SetMenuItemSensitivity, this, _1, status));

        std::string  prjStatusVal;

        switch (status)
        {
        case ProjectStatus_Defined:
            {
                prjStatusVal = ATTRIBUTE_PROJECT_STATUS_DEFINED;
                break;
            }
        case ProjectStatus_ResponseSolved:
            {
                prjStatusVal = ATTRIBUTE_PROJECT_STATUS_RESPONSE_SOLVED;
                break;
            }
        case ProjectStatus_ResponseNoiseSolved:
            {
                prjStatusVal = ATTRIBUTE_PROJECT_STATUS_RESPONSE_NOISE_SOLVED;
                break;
            }
        case ProjectStatus_NoiseSolved:
            {
                prjStatusVal = ATTRIBUTE_PROJECT_STATUS_NOISE_SOLVED;
                break;
            }
        default:
            break;
        }

        if (!prjStatusVal.empty())
        {
            Project::Instance()->GetProperty()->GetSimPart()->SetAttribute(ATTRIBUTE_PROJECT_STATUS,
                prjStatusVal.c_str(), Update::OptionLater);
        }
    }

    BaseProjectProperty::BaseProjectProperty(NXOpen::CAE::SimPart* pSimPrt) : m_simPrt(pSimPrt)
    {
    }

    BaseProjectProperty::~BaseProjectProperty()
    {
    }

    std::string BaseProjectProperty::GetProjectName() const
    {
        return m_simPrt->GetStringAttribute(ATTRIBUTE_PROJECT_NAME).GetUTF8Text();
    }

    std::string BaseProjectProperty::GetProjectPath() const
    {
        boost::filesystem::path  fullPath(m_simPrt->FullPath().GetUTF8Text());

        return fullPath.parent_path().string();
    }

    AssyFemPart* BaseProjectProperty::GetAFemPart() const
    {
        BasePart *pAfemPart = m_simPrt->FemPart();

        //  Load afem part
        if (pAfemPart == NULL)
        {
            std::string       prtName(GetProjectName().append("_a.afm"));

            OpenCompPart(prtName);
        }

        return dynamic_cast<AssyFemPart*>(pAfemPart);
    }

    BasePart* BaseProjectProperty::OpenCompPart( const std::string & prtName ) const
    {
        namespace bfs = boost::filesystem;

        PartCollection *pPrtCol  = Session::GetSession()->Parts();
        BasePart       *pCompPrt = NULL;

        //  Get template sim path name
        const bfs::path   prjPath(GetProjectPath());
        bfs::path         compPathName(prjPath / prtName);

        PartLoadStatus *pPrtLoadStatus = NULL;

        DELETE_CLASS_POINTER(pPrtLoadStatus);
        pCompPrt = pPrtCol->OpenBase(compPathName.string().c_str(), &pPrtLoadStatus);

        return pCompPrt;
    }

    BasePart* BaseProjectProperty::GetCompPart(const std::string &prtName) const
    {
        BasePart       *pCompPrt = NULL;

        try
        {
            PartCollection *pPrtCol  = Session::GetSession()->Parts();

            pCompPrt = pPrtCol->FindObject(prtName.c_str());
        }
        catch(std::exception&)
        {
            pCompPrt = OpenCompPart(prtName);
        }

        return pCompPrt;
    }

    FemPart* BaseProjectProperty::GetRailSlabFemPart() const
    {
        std::string     femName(std::string(RAIL_SLAB_FEM_BASE_NAME).append("_f.fem"));
        BasePart       *pFemPrt = GetCompPart(femName);

        return dynamic_cast<FemPart*>(pFemPrt);
    }

    Part* BaseProjectProperty::GetRailSlabIdeaPart() const
    {
        std::string     prtName(std::string(RAIL_SLAB_FEM_BASE_NAME).append("_f_i.prt"));
        BasePart       *pPrt = GetCompPart(prtName);

        return dynamic_cast<Part*>(pPrt);
    }

    FemPart* BaseProjectProperty::GetBraceFemPart() const
    {
        std::string     femName(GetBraceTemplateBaseName().append("_f.fem"));
        BasePart       *pFemPrt = GetCompPart(femName);

        return dynamic_cast<FemPart*>(pFemPrt);
    }

    Part* BaseProjectProperty::GetBraceIdeaPart() const
    {
        std::string     prtName(GetBraceTemplateBaseName().append("_f_i.prt"));
        BasePart       *pPrt = GetCompPart(prtName);

        return dynamic_cast<Part*>(pPrt);
    }

    Part* BaseProjectProperty::GetGeometryPart() const
    {
        BasePart       *pPrt = GetCompPart(GetRootPartName());

        return dynamic_cast<Part*>(pPrt);
    }

    int BaseProjectProperty::GetTemplatePartFiles(std::vector<std::string> &fileNames) const
    {
        fileNames.clear();

        const char *commonParts[] = {"base.prt", "beam.prt", "bridge.prt", "carriage.prt",
                                   "rail-transit.prt", "rail.prt", "rails.prt", "slab.prt",
                                   "slabs.prt", "train.prt", "tunnel.prt"};

        int eleCnt = sizeof(commonParts) / sizeof(commonParts[0]);

        fileNames.insert(fileNames.begin(), commonParts, commonParts + eleCnt);

        return static_cast<int>(fileNames.size());
    }

    std::string BaseProjectProperty::GetRootPartName() const
    {
        return ROOT_PART_NAME;
    }

    BridgeProperty::BridgeProperty(NXOpen::CAE::SimPart* pSimPrt)
        : BaseProjectProperty(pSimPrt)
    {
    }

    BridgeProperty::~BridgeProperty()
    {
    }

    Project::ProjectType BridgeProperty::GetProjectType() const
    {
        return Project::ProjectType_Bridge;
    }

    std::string BridgeProperty::GetProjectTypeName() const
    {
        return PROJECT_TYPE_NAME_BRIDGE;
    }

    std::string BridgeProperty::GetTemplateBaseName() const
    {
        return TEMPLATE_BASE_NAME_BRIDGE;
    }

    std::string BridgeProperty::GetBraceTemplateBaseName() const
    {
        return TEMPLATE_BRACE_BASE_NAME_BRIDGE;
    }

    std::string BridgeProperty::GetBraceMenuItemName() const
    {
        return MENU_ITEM_NAME_SET_BRIDGE;
    }

    SelmiInfiniteBaseProperty::SelmiInfiniteBaseProperty(NXOpen::CAE::SimPart* pSimPrt)
        : BaseProjectProperty(pSimPrt)
    {
    }

    SelmiInfiniteBaseProperty::~SelmiInfiniteBaseProperty()
    {
    }

    Project::ProjectType SelmiInfiniteBaseProperty::GetProjectType() const
    {
        return Project::ProjectType_Selmi_Infinite;
    }

    std::string SelmiInfiniteBaseProperty::GetProjectTypeName() const
    {
        return PROJECT_TYPE_NAME_SELMI_INFINITE;
    }

    std::string SelmiInfiniteBaseProperty::GetBraceTemplateBaseName() const
    {
        return TEMPLATE_BRACE_BASE_NAME_SELMI_INFINITE;
    }

    std::string SelmiInfiniteBaseProperty::GetBraceMenuItemName() const
    {
        return MENU_ITEM_NAME_SET_BASE;
    }

    std::string SelmiInfiniteBaseProperty::GetTemplateBaseName() const
    {
        return TEMPLATE_BASE_NAME_SELMI_INFINITE;
    }

    TunnelProperty::TunnelProperty(NXOpen::CAE::SimPart* pSimPrt)
        : BaseProjectProperty(pSimPrt)
    {
    }

    TunnelProperty::~TunnelProperty()
    {
    }

    Project::ProjectType TunnelProperty::GetProjectType() const
    {
        return Project::ProjectType_Tunnel;
    }

    std::string TunnelProperty::GetProjectTypeName() const
    {
        return PROJECT_TYPE_NAME_TUNNEL;
    }

    std::string TunnelProperty::GetTemplateBaseName() const
    {
        return TEMPLATE_BASE_NAME_TUNNEL;
    }

    std::string TunnelProperty::GetBraceTemplateBaseName() const
    {
        return TEMPLATE_BRACE_BASE_NAME_TUNNEL;
    }

    std::string TunnelProperty::GetBraceMenuItemName() const
    {
        return MENU_ITEM_NAME_SET_TUNNEL;
    }

    void Project::New(const std::string &prjName, const std::string &prjPath, ProjectType prjType)
    {
        m_prjInstance.reset(new Project(prjName, prjPath, prjType));

        m_prjStatus.Switch(Status::ProjectStatus_Defined);
    }

    Project::ProjectType Project::GetProjectTypeOfPart(BasePart* pOpenedPrt)
    {
        CAE::SimPart *pSimPrt = dynamic_cast<CAE::SimPart*>(pOpenedPrt);
        Project::ProjectType   prjType;
        bool                   isPrjPrt = false;

        if (pSimPrt)
        {
            std::string prjAttrName(pSimPrt->GetStringAttribute(ATTRIBUTE_PROJECT_NAME).GetUTF8Text());
            std::string prjAttrType(pSimPrt->GetStringAttribute(ATTRIBUTE_PROJECT_TYPE).GetUTF8Text());

            if (!prjAttrName.empty() && !prjAttrType.empty())
            {
                if (prjAttrType.compare(PROJECT_TYPE_NAME_BRIDGE) == 0)
                {
                    prjType = Project::ProjectType_Bridge;
                    isPrjPrt = true;
                }
                else if (prjAttrType.compare(PROJECT_TYPE_NAME_SELMI_INFINITE) == 0)
                {
                    prjType = Project::ProjectType_Selmi_Infinite;
                    isPrjPrt = true;
                }
                else if (prjAttrType.compare(PROJECT_TYPE_NAME_TUNNEL) == 0)
                {
                    prjType = Project::ProjectType_Tunnel;
                    isPrjPrt = true;
                }
            }
        }

        if (!isPrjPrt)
            throw NXException::Create(MSGTXT("The part is not a vsar project part."));

        return prjType;
    }

    Status::ProjectStatus Project::GetProjectStatusOfPart(BasePart* pOpenedPrt)
    {
        CAE::SimPart *pSimPrt = dynamic_cast<CAE::SimPart*>(pOpenedPrt);
        Status::ProjectStatus  prjStatus = Status::ProjectStatus_None;
        bool                   isPrjPrt = false;

        if (pSimPrt)
        {
            std::string prjAttrName(pSimPrt->GetStringAttribute(ATTRIBUTE_PROJECT_NAME).GetUTF8Text());
            std::string prjAttrStatus(pSimPrt->GetStringAttribute(ATTRIBUTE_PROJECT_STATUS).GetUTF8Text());

            if (!prjAttrName.empty() && !prjAttrStatus.empty())
            {
                if (prjAttrStatus.compare(ATTRIBUTE_PROJECT_STATUS_DEFINED) == 0)
                {
                    prjStatus = Status::ProjectStatus_Defined;
                    isPrjPrt = true;
                }
                else if (prjAttrStatus.compare(ATTRIBUTE_PROJECT_STATUS_RESPONSE_SOLVED) == 0)
                {
                    prjStatus = Status::ProjectStatus_ResponseSolved;
                    isPrjPrt = true;
                }
                else if (prjAttrStatus.compare(ATTRIBUTE_PROJECT_STATUS_RESPONSE_NOISE_SOLVED) == 0)
                {
                    prjStatus = Status::ProjectStatus_ResponseNoiseSolved;
                    isPrjPrt = true;
                }
                else if (prjAttrStatus.compare(ATTRIBUTE_PROJECT_STATUS_NOISE_SOLVED) == 0)
                {
                    prjStatus = Status::ProjectStatus_NoiseSolved;
                    isPrjPrt = true;
                }
            }
        }

        if (!isPrjPrt)
            throw NXException::Create(MSGTXT("The part is not a vsar project part."));

        return prjStatus;
    }

    void Project::Open(BasePart* pOpenedPrt)
    {
        try
        {
            ProjectType   prjType(GetProjectTypeOfPart(pOpenedPrt));

            m_prjInstance.reset(new Project(dynamic_cast<CAE::SimPart*>(pOpenedPrt), prjType));

            m_prjStatus.Switch(GetProjectStatusOfPart(pOpenedPrt));

            //  Load result
            BaseProjectProperty  *pPrjProp = m_prjInstance->GetProperty();

            // Load response result
            filesystem::path  resultPathName(filesystem::path(pPrjProp->GetProjectPath()) /
                                             pPrjProp->GetResponseOp2ResultName());

            LoadResult(resultPathName.string());

            resultPathName = filesystem::path(pPrjProp->GetProjectPath()) /
                                              pPrjProp->GetResponseAfuResultName();

            LoadResult(resultPathName.string());

            // Load noise
            resultPathName = filesystem::path(pPrjProp->GetProjectPath()) /
                                              pPrjProp->GetNoiseResultName();

            LoadResult(resultPathName.string());
        }
        catch (std::exception &)
        {
        }
    }

    void Project::Close(BasePart* pOpenedPrt)
    {
        try
        {
            ProjectType   prjType(GetProjectTypeOfPart(pOpenedPrt));

            m_prjInstance.reset();

            m_prjStatus.Switch(Status::ProjectStatus_None);
        }
        catch (std::exception &)
        {
        }
    }

    Project* Project::Instance()
    {
        return m_prjInstance.get();
    }

    //------------------------------------------------------------------------------
    // Constructor for NX Styler class
    //------------------------------------------------------------------------------
    Project::Project(const std::string &prjName, const std::string &prjPath, ProjectType prjType)
    {
        switch (prjType)
        {
        case ProjectType_Bridge:
            m_prjProperty.reset(new BridgeProperty());
            break;
        case ProjectType_Selmi_Infinite:
            m_prjProperty.reset(new SelmiInfiniteBaseProperty());
            break;
        case ProjectType_Tunnel:
            m_prjProperty.reset(new TunnelProperty());
            break;
        default:
            break;
        }

        Initilize(prjName, prjPath);
    }

    Project::Project(CAE::SimPart* pSimPrt, ProjectType prjType)
    {
        switch (prjType)
        {
        case ProjectType_Bridge:
            m_prjProperty.reset(new BridgeProperty(pSimPrt));
            break;
        case ProjectType_Selmi_Infinite:
            m_prjProperty.reset(new SelmiInfiniteBaseProperty(pSimPrt));
            break;
        case ProjectType_Tunnel:
            m_prjProperty.reset(new TunnelProperty(pSimPrt));
            break;
        default:
            break;
        }
    }

    //------------------------------------------------------------------------------
    // Destructor for NX Styler class
    //------------------------------------------------------------------------------
    Project::~Project()
    {
    }

    void Project::Initilize(const std::string &prjName, const std::string &prjPath)
    {
        LoadPrtPart(prjPath);
        LoadSimPart(prjName, prjPath);
        SetProjectAttribute(prjName);
    }

    void Project::LoadPrtPart(const std::string &prjPath)
    {
        namespace bfs = boost::filesystem;

        //  Copy part files to location
        const bfs::path          templatePath(bfs::path(GetInstallPath()) / TEMPLATE_FOLDER_NAME);

        //ListingWindow *pLstWnd = Session::GetSession()->ListingWindow();

        //pLstWnd->Open();
        CopyDirectory(bfs::path(templatePath / TEMPLATE_COMMON_FOLDER_NAME).string(), prjPath);
        CopyDirectory(bfs::path(templatePath / m_prjProperty->GetTemplateBaseName()).string(), prjPath);

        //  copy Idealized Part
        //const std::string  idealPrtName(m_prjProperty->GetTemplateBaseName().append("_f_i.prt"));

        //fromFilePathName = templatePath / m_prjProperty->GetTemplateBaseName() / idealPrtName;
        //toFilePathName   = toPath / idealPrtName;
        //if (bfs::exists(toFilePathName))
        //    bfs::remove_all(toFilePathName);

        //bfs::copy_file(fromFilePathName, toFilePathName);
    }

    void Project::LoadSimPart(const std::string &prjName, const std::string &prjPathName)
    {
        //  Open sim and fem, save as to location
        Session        *pSession  = Session::GetSession();
        PartCollection *pPrtCol   = pSession->Parts();
        CAE::SimPart   *pSimPrt   = NULL;
        CAE::CaePart   *pFemPrt   = NULL;

        namespace bfs = boost::filesystem;

        //  Get template sim path name
        const bfs::path   prjPath(prjPathName);
        bfs::path         tempPrtPathName(prjPath / m_prjProperty->GetTemplateSimName());

        PartLoadStatus *pPrtLoadStatus = NULL;

        //  Open template sim part
        {
            DELETE_CLASS_POINTER(pPrtLoadStatus);
            pSimPrt = dynamic_cast<CAE::SimPart*>(pPrtCol->OpenBaseDisplay(tempPrtPathName.string().c_str(),
                                                                           &pPrtLoadStatus));
        }

        //  Save template sim parts to project folder
        try
        {
            bfs::path         toFilePathName;

            pFemPrt = pSimPrt->FemPart();

            //  save as afem part
            toFilePathName = prjPath / std::string(prjName).append("_a.afm");
            SaveAsComp(pFemPrt, toFilePathName);

            //  save as sim file
            toFilePathName = prjPath / std::string(prjName).append("_s.sim");
            SaveAsComp(pSimPrt, toFilePathName);

            const std::string tempBaseName(m_prjProperty->GetTemplateBaseName());

            //  delete original afm part
            std::string  tempSimName(std::string(tempBaseName).append("_s.sim"));
            std::string  tempAfmName(std::string(tempBaseName).append("_a.afm"));

            boost::filesystem::remove_all(prjPath / tempSimName);
            boost::filesystem::remove_all(prjPath / tempAfmName);
        }
        catch(std::exception &)
        {
            pSimPrt->Close(BasePart::CloseWholeTreeTrue, BasePart::CloseModifiedCloseModified, NULL);
            throw;
        }

        m_prjProperty->SetSimPart(pSimPrt);
    }

    void Project::SaveAsComp( NXOpen::BasePart * pPrt,
                              const boost::filesystem::path &toFilePathName)
    {
        PartSaveStatus * pPrtSaveStatus = NULL;

        // remove file in the target path
        if (boost::filesystem::exists(toFilePathName))
            boost::filesystem::remove_all(toFilePathName);

        DELETE_CLASS_POINTER(pPrtSaveStatus);
        pPrtSaveStatus = pPrt->SaveAs(toFilePathName.string().c_str());
    }

    void Project::SetProjectAttribute(const std::string &prjName)
    {
        CAE::SimPart *pSimPrt = GetProperty()->GetSimPart();
        Session *theSession = Session::GetSession();

        Session::UndoMarkId markId;

        markId = theSession->SetUndoMark(Session::MarkVisibilityInvisible, "Set Project Attribute");

        pSimPrt->SetAttribute(ATTRIBUTE_PROJECT_NAME, prjName.c_str(), Update::OptionLater);
        pSimPrt->SetAttribute(ATTRIBUTE_PROJECT_TYPE, m_prjProperty->GetProjectTypeName().c_str(), Update::OptionLater);
        pSimPrt->SetAttribute(ATTRIBUTE_PROJECT_STATUS, ATTRIBUTE_PROJECT_STATUS_DEFINED, Update::OptionLater);

        Update *pUpdateMgr = theSession->UpdateManager();

        if (pUpdateMgr->DoUpdate(markId) != 0)
        {
            NXException::Create(pUpdateMgr->ErrorList()->GetErrorInfo(0)->ErrorCode());
        }

        pSimPrt->Save(BasePart::SaveComponentsTrue, BasePart::CloseAfterSaveFalse);
    }

    //void Project::LoadFromTemplate(const std::string &prjPath)
    //{
    //    int         rtc = 0;
    //    UF_PART_load_status_t  loadStatus = {0};

    //    UF_CLONE_initialise(UF_CLONE_clone_operation);

    //    std::string strAssemName(GetInstallPath().append("template"));

    //    strAssemName.push_back(PATH_DELIM);
    //    strAssemName.append("rail-transit.prt");

    //    rtc = UF_CALL(UF_CLONE_add_assembly(strAssemName.c_str(), &loadStatus));
    //    rtc = UF_CALL(UF_CLONE_set_def_action(UF_CLONE_clone));
    //    rtc = UF_CALL(UF_CLONE_set_def_directory(prjPath.c_str()));
    //    rtc = UF_CALL(UF_CLONE_set_def_naming(UF_CLONE_naming_rule));

    //    std::string outputPartName(m_prjName + "_");//(prjPath);

    //    //if (*prjPath.rbegin() != PATH_DELIM)
    //    //    outputPartName.push_back(PATH_DELIM);
    //    //outputPartName.append(m_prjName).append("_main.prt");

    //    //rtc = UF_CALL(UF_CLONE_set_naming(strAssemName, UF_CLONE_user_name, outputPartName.c_str()));
    //    UF_CLONE_name_rule_def_t nameRule = {UF_CLONE_prepend_string, NULL,
    //                                         const_cast<char*>(outputPartName.c_str())};
    //    UF_CLONE_naming_failures_t  nameFailures = {0};

    //    rtc = UF_CALL(UF_CLONE_init_naming_failures(&nameFailures));
    //    rtc = UF_CALL(UF_CLONE_set_name_rule(&nameRule, &nameFailures));
    //    rtc = UF_CALL(UF_CLONE_perform_clone(&nameFailures));

    //    BOOST_SCOPE_EXIT((&loadStatus)(&nameFailures))
    //    {
    //        UF_PART_free_load_status(&loadStatus);

    //        if (nameFailures.n_failures > 0)
    //        {
    //            UF_free(nameFailures.statuses);
    //            UF_free_string_array(nameFailures.n_failures, nameFailures.input_names);
    //            UF_free_string_array(nameFailures.n_failures, nameFailures.output_names);
    //        }

    //        UF_CLONE_terminate();
    //    }
    //    BOOST_SCOPE_EXIT_END
    //}
}
