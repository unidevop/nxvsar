
#ifndef VSAR_PROJECT_H_INCLUDED
#define VSAR_PROJECT_H_INCLUDED

//------------------------------------------------------------------------------
//These includes are needed for the following template code
//------------------------------------------------------------------------------
#include <string>
#include <vector>
#include <boost/format.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/filesystem.hpp>
#include <NXOpen/MenuBar_MenuButton.hxx>
#include <Vsar_Init_Names.hxx>
#include <libvsarinit_exports.h>

//------------------------------------------------------------------------------
// Namespaces needed for following template
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Forward declaration for Class
//------------------------------------------------------------------------------
namespace NXOpen
{
    class BasePart;
    class Part;

    namespace CAE
    {
        class SimPart;
        class AssyFemPart;
        class FemPart;
    }
}

#pragma warning(push)
#pragma warning(disable:4251)   // get rid of dll import and export warning

namespace Vsar
{
    class BaseProjectProperty;

    class VSARINITEXPORT Status
    {
    public:
        enum ProjectStatus
        {
            ProjectStatus_None                  = 1 << 1,
            ProjectStatus_Defined               = 1 << 2,
            ProjectStatus_ResponseSolved        = 1 << 3,
            ProjectStatus_ResponseNoiseSolved   = 1 << 4,
            ProjectStatus_NoiseSolved           = 1 << 5
        };

        typedef struct
        {
            std::string                                      m_buttonName;
            //NXOpen::MenuBar::MenuButton::SensitivityStatus   m_sensStatus;
            unsigned int                                     m_status;
        }MenuItemSensitivity;

    public:
        Status();
        ~Status();

        void Switch(ProjectStatus status);

    protected:
        void SetMenuItemSensitivity(const MenuItemSensitivity &menuItemData, ProjectStatus status);
    };

    class VSARINITEXPORT Project
    {
        // class members
    public:

        typedef enum ProjectType
        {
            ProjectType_Bridge,
            ProjectType_Selmi_Infinite,
            ProjectType_Tunnel
        }ProjectType;

        ~Project();

        static void New(const std::string &prjName, const std::string &prjPath, ProjectType prjType);

        static void Open(NXOpen::BasePart* pOpenedPrt);

        static void Close(NXOpen::BasePart* pOpenedPrt);

        static Project* Instance();

        static Status* GetStatus()
        {
            return &m_prjStatus;
        }

        BaseProjectProperty* GetProperty() const
        {
            return m_prjProperty.get();
        }

    protected:

        Project(const std::string &prjName, const std::string &prjPath, ProjectType prjType);

        Project(NXOpen::CAE::SimPart* pSimPrt, ProjectType prjType);

        void Initilize(const std::string &prjName, const std::string &prjPath);

        void LoadPrtPart(const std::string &prjPath);

        void LoadSimPart(const std::string &prjName, const std::string &prjPath);

        void SaveAsComp( NXOpen::BasePart * pPrt,
                         const boost::filesystem::path &toFilePathName);

        void SetProjectAttribute(const std::string &prjName);

        static ProjectType Project::GetProjectTypeOfPart(NXOpen::BasePart* pOpenedPrt);

        static Status::ProjectStatus Project::GetProjectStatusOfPart(NXOpen::BasePart* pOpenedPrt);

    private:
        boost::scoped_ptr<BaseProjectProperty>  m_prjProperty;

        static boost::scoped_ptr<Project>       m_prjInstance;

        static Status                    m_prjStatus;
    };

    class VSARINITEXPORT BaseProjectProperty
    {
    public:
        friend class Project;

        BaseProjectProperty(NXOpen::CAE::SimPart* pSimPrt = NULL);
        virtual ~BaseProjectProperty();

        std::string GetProjectName() const;

        std::string GetProjectPath() const;

        std::string GetResponseOp2ResultName() const
        {
            return (boost::format(RESPONSE_OP2_RESULT_FILE_PATTERN_NAME) %
                GetProjectName() % VSDANE_SOLUTION_NAME).str();
        }

        std::string GetResponseAfuResultName() const
        {
            return (boost::format(RESPONSE_AFU_RESULT_FILE_PATTERN_NAME) %
                GetProjectName() % VSDANE_SOLUTION_NAME).str();
        }

        std::string GetNoiseIntermediateResultName() const
        {
            return (boost::format(NOISE_INTERMEDIATE_RESULT_FILE_PATTERN_NAME) % GetProjectName()).str();
        }

        std::string GetNoiseResultName() const
        {
            return (boost::format(NOISE_RESULT_FILE_PATTERN_NAME) % GetProjectName()).str();
        }

        NXOpen::CAE::SimPart* GetSimPart() const
        {
            return m_simPrt;
        }

        NXOpen::CAE::AssyFemPart* GetAFemPart() const;

        NXOpen::CAE::FemPart* GetRailSlabFemPart() const;

        NXOpen::Part* GetRailSlabIdeaPart() const;

        NXOpen::CAE::FemPart* GetBraceFemPart() const;

        NXOpen::Part* GetBraceIdeaPart() const;

        NXOpen::Part* GetGeometryPart() const;

        virtual int GetTemplatePartFiles(std::vector<std::string> &fileNames) const;

        virtual std::string GetTemplateBaseName() const = 0;

        virtual Project::ProjectType GetProjectType() const = 0;

        virtual std::string GetProjectTypeName() const = 0;

        virtual std::string GetBraceTemplateBaseName() const = 0;

        virtual std::string GetBraceMenuItemName() const = 0;

        std::string GetTemplateSimName() const
        {
            return GetTemplateBaseName() + "_s.sim";
        }

        std::string GetRootPartName() const;

    protected:

        void SetSimPart(NXOpen::CAE::SimPart* pSimPrt)
        {
            m_simPrt = pSimPrt;
        }

        NXOpen::BasePart* GetCompPart(const std::string &prtName) const;

        NXOpen::BasePart* OpenCompPart( const std::string & prtName ) const;

    private:

        NXOpen::CAE::SimPart *m_simPrt;
    };

    class BridgeProperty : public BaseProjectProperty
    {
    public:
        BridgeProperty(NXOpen::CAE::SimPart* pSimPrt = NULL);
        virtual ~BridgeProperty();

        virtual std::string GetTemplateBaseName() const;

        virtual Project::ProjectType GetProjectType() const;

        virtual std::string GetProjectTypeName() const;

        virtual std::string GetBraceTemplateBaseName() const;

        virtual std::string GetBraceMenuItemName() const;
    };

    class SelmiInfiniteBaseProperty : public BaseProjectProperty
    {
    public:
        SelmiInfiniteBaseProperty(NXOpen::CAE::SimPart* pSimPrt = NULL);
        virtual ~SelmiInfiniteBaseProperty();

        virtual std::string GetTemplateBaseName() const;

        virtual Project::ProjectType GetProjectType() const;

        virtual std::string GetProjectTypeName() const;

        virtual std::string GetBraceTemplateBaseName() const;

        virtual std::string GetBraceMenuItemName() const;
    };

    class TunnelProperty : public BaseProjectProperty
    {
    public:
        TunnelProperty(NXOpen::CAE::SimPart* pSimPrt = NULL);
        virtual ~TunnelProperty();

        virtual std::string GetTemplateBaseName() const;

        virtual Project::ProjectType GetProjectType() const;

        virtual std::string GetProjectTypeName() const;

        virtual std::string GetBraceTemplateBaseName() const;

        virtual std::string GetBraceMenuItemName() const;
    };
}

#pragma warning(pop)

#endif //VSAR_PROJECT_H_INCLUDED
