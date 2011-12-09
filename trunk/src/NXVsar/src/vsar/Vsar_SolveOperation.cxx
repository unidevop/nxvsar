
#include <uf_defs.h>
#include <Vsar_SolveOperation.hxx>

//#include <uf.h>
//#include <uf_ui.h>
//
#include <boost/filesystem.hpp>

#include <NXOpen/Session.hxx>
#include <NXOpen/NXException.hxx>
#include <NXOpen/CAE_FTK_DataManager.hxx>

#include <Vsar_Project.hxx>

#include <Vsar_Result.hxx>

using namespace boost;
using namespace NXOpen;
using namespace Vsar;

//------------------------------------------------------------------------------
// Declaration of global variables
//------------------------------------------------------------------------------
namespace Vsar
{
    BaseSolveOperation::BaseSolveOperation() : m_workDir(), m_solDir()
    {
        //  Get result path name
        BaseProjectProperty *pPrjProp = Project::Instance()->GetProperty();

        m_solDir = filesystem::path(pPrjProp->GetProjectPath());
    }

    BaseSolveOperation::~BaseSolveOperation()
    {
    }

    void BaseSolveOperation::Execute()
    {
        PreExecute();

        filesystem::path  oldWorkPath(filesystem::current_path());

        try
        {
            CreateWorkDir();
            //CleanResult();

            //  Solve 103 solution

            //  Solve vsar sol
        }
        catch (std::exception &)
        {
            
        }

        LoadResult();
    }

    void BaseSolveOperation::CleanResult()
    {
        ResponseResult   respResult;
        std::string      resultPathName(respResult.GetResultPathName());

        try
        {
            Session::GetSession()->DataManager()->UnloadFile(resultPathName.c_str());
        }
        catch (NXException &)
        {
        }

        if (filesystem::exists(resultPathName))
            boost::filesystem::remove_all(resultPathName);
    }

    void BaseSolveOperation::LoadResult()
    {
        ResponseResult   respResult;

        respResult.Load();
    }

    void BaseSolveOperation::CreateWorkDir()
    {
        filesystem::path strScratchDir(m_solDir);   //  TODO: Use custom scratch dir in future
        filesystem::path workFolder;
        filesystem::path workPath;

        //  set work dir
        filesystem::current_path(strScratchDir);

        do
        {
            workFolder = filesystem::unique_path(filesystem::path("workDir-%%%%%%"));
            workPath   = strScratchDir / workFolder;
        } while (!filesystem::exists(workPath));

        filesystem::create_directory(workPath);

        m_workDir = workPath;
    }

    SolveResponseOperation::SolveResponseOperation() : BaseSolveOperation()
    {
    }

    SolveResponseOperation::~SolveResponseOperation()
    {
    }

    void SolveResponseOperation::PreExecute()
    {

    }

    void SolveResponseOperation::PrepareInputFiles() const
    {

    }
}
