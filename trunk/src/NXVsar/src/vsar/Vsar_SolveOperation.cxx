
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
    BaseSolveOperation::BaseSolveOperation()
    {
    }

    BaseSolveOperation::~BaseSolveOperation()
    {
    }

    void BaseSolveOperation::Execute()
    {
        CreateWorkDir();
        //CleanResult();

        //  Solve 103 solution

        //  Solve vsar sol

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
        m_workDir = filesystem::unique_path(filesystem::path("workDir-%%%%%%")).string();
    }

    SolveResponseOperation::SolveResponseOperation() : BaseSolveOperation()
    {
    }

    SolveResponseOperation::~SolveResponseOperation()
    {
    }
}
