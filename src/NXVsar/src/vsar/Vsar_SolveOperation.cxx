
#include <uf_defs.h>
#include <Vsar_SolveOperation.hxx>

#include <fstream>
//
#include <boost/filesystem.hpp>
#include <boost/scope_exit.hpp>
#include <boost/lexical_cast.hpp>

#include <uf_unit_types.h>

#include <NXOpen/Session.hxx>
#include <NXOpen/PartCollection.hxx>
#include <NXOpen/Part.hxx>
#include <NXOpen/Expression.hxx>
#include <NXOpen/ExpressionCollection.hxx>
#include <NXOpen/UnitCollection.hxx>
#include <NXOpen/NXException.hxx>
#include <NXOpen/CAE_FTK_DataManager.hxx>

#include <Vsar_Init_Utils.hxx>
#include <Vsar_Project.hxx>

#include <Vsar_Component.hxx>
#include <Vsar_Names.hxx>
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

        //  remove work dir
        BOOST_SCOPE_EXIT((&m_workDir))
        {
            if (filesystem::exists(m_workDir))
                filesystem::remove_all(m_workDir);

            m_workDir.clear();
        }
        BOOST_SCOPE_EXIT_END

        //try
        {
            CreateWorkDir();

            PrepareInputFiles();

            //CleanResult();

            //  Solve 103 solution

            //  Solve vsar sol

            //LoadResult();
        }
        //catch (std::exception &)
        {
            
        }
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

        BaseProjectProperty *pPrjProp = Project::Instance()->GetProperty();

        std::string  workFolderTemp(pPrjProp->GetProjectName() + "_%%%%%%");

        //  set work dir
        filesystem::current_path(strScratchDir);

        do
        {
            workFolder = filesystem::unique_path(workFolderTemp);
            workPath   = strScratchDir / workFolder;
        } while (filesystem::exists(workPath));

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
        ExcitationInput excitationInput(m_workDir);

        excitationInput.Generate();
    }

    ExcitationInput::ExcitationInput(const boost::filesystem::path &targetDir) : m_targetDir(targetDir)
    {
    }

    ExcitationInput::~ExcitationInput()
    {
    }

    void ExcitationInput::Generate() const
    {
        CopyIrrData();
        WriteVehicleData();
        WriteRailData();
        WriteSlabData();
        WriteBeamData();
        WriteCalculationData();
    }

    void ExcitationInput::CopyIrrData() const
    {
        filesystem::path  irrTmpPath = filesystem::path(GetInstallPath()) /
            SOLVER_FOLDER_NAME / SOLVER_DATA_FOLDER_NAME / IRR_DATA_FILE_NAME;

        filesystem::path toFilePathName(m_targetDir / IRR_DATA_FILE_NAME);

        if (boost::filesystem::exists(toFilePathName))
            boost::filesystem::remove_all(toFilePathName);

        boost::filesystem::copy_file(irrTmpPath, toFilePathName);
    }

    void ExcitationInput::WriteInputData(const StlInputItemVector &vInputItems, const std::string &fileName) const
    {
        if (vInputItems.empty())
            return;

        std::ofstream   inputFile(filesystem::path(m_targetDir / fileName).string().c_str());

        for (int idx = 0; idx < vInputItems.size(); idx++)
        {
            double expVal = 0.0;

            try
            {
                Expression *pExp = BaseComponent::GetExpression(vInputItems[idx].m_partName,
                                                                vInputItems[idx].m_expName);

                std::string  strExpType = pExp->Type().GetText();

                if (strExpType == "Number")
                    expVal = pExp->GetValueUsingUnits(Expression::UnitsOptionExpression); //pExp->Value();
                else if (strExpType == "Integer")
                    expVal = pExp->IntegerValue();

                //  Convert to SI Unit System
                if (!vInputItems[idx].m_targetUnitName.empty())
                {
                    Session         *pSession = Session::GetSession();
                    BasePart        *pExpPrt  = pSession->Parts()->FindObject(vInputItems[idx].m_partName.c_str());

                    UnitCollection *pUnitCol = pExpPrt->UnitCollection();

                    Unit *pSiUnit = pUnitCol->FindObject(vInputItems[idx].m_targetUnitName.c_str());

                    expVal = pUnitCol->Convert(pExp->Units(), pSiUnit, expVal);
                }
            }
            catch (NXException &)
            {
                expVal = boost::lexical_cast<double>(vInputItems[idx].m_expName);
            }

            inputFile << expVal << ",";
        }

        inputFile.seekp(-1, std::ios_base::cur);
        inputFile << "\n";
        inputFile.close();
    }

    void ExcitationInput::WriteVehicleData() const
    {
        InputItem inputDataItem[] =
        {
            {TRAIN_PRT_PART_NAME,       TRAIN_CARRIAGE_COUNT_EXP_NAME,      ""},
            {CARRIAGE_PRT_PART_NAME,    TRAIN_CARRIAGE_WEIGHT_EXP_NAME,     UF_UNIT_MASS_kg},
            {CARRIAGE_PRT_PART_NAME,    TRAIN_CARRIAGE_LENGTH_EXP_NAME,     UF_UNIT_LENGTH_m},
            {CARRIAGE_PRT_PART_NAME,    TRAIN_CARRIAGE_DISTANCE_EXP_NAME,   UF_UNIT_LENGTH_m},
            {CARRIAGE_PRT_PART_NAME,    TRAIN_BOGIE_WEIGHT_EXP_NAME,        UF_UNIT_MASS_kg},
            {CARRIAGE_PRT_PART_NAME,    TRAIN_WHEELSET_WEIGHT_EXP_NAME,     UF_UNIT_MASS_kg},
            {CARRIAGE_PRT_PART_NAME,    TRAIN_WHEELSET_SGL_STG_SUSP_STIFF_EXP_NAME, UF_UNIT_PRESSUREONEDGE_N__m},
            {CARRIAGE_PRT_PART_NAME,    TRAIN_WHEELSET_SGL_STG_SUSP_DAMP_EXP_NAME,  UF_UNIT_MASSFLOW_kg__sec},
            {CARRIAGE_PRT_PART_NAME,    TRAIN_WHEELSET_TWO_STG_SUSP_STIFF_EXP_NAME, UF_UNIT_PRESSUREONEDGE_N__m},
            {CARRIAGE_PRT_PART_NAME,    TRAIN_WHEELSET_TWO_STG_SUSP_DAMP_EXP_NAME,  UF_UNIT_MASSFLOW_kg__sec},
            {CARRIAGE_PRT_PART_NAME,    TRAIN_HALF_BOGIE_DISTANCE_EXP_NAME, UF_UNIT_LENGTH_m},
            {CARRIAGE_PRT_PART_NAME,    TRAIN_WHEELSET_INTERVAL_EXP_NAME,   UF_UNIT_LENGTH_m}
        };

        StlInputItemVector vInputItems(inputDataItem, inputDataItem + N_ELEMENTS(inputDataItem));

        WriteInputData(vInputItems, VEHICLE_INPUT_FILE_NAME);
    }

    void ExcitationInput::WriteRailData() const
    {
        InputItem inputDataItem[] =
        {
            {RAIL_SLAB_FEM_PART_NAME,    RAIL_ELASTIC_MODULUS_EXP_NAME, UF_UNIT_PRESSURE_N__m2},
            {RAIL_SLAB_FEM_PART_NAME,    RAIL_MASS_DENSITY_EXP_NAME,    UF_UNIT_MASSDENSITY_kg__m3},
            {RAIL_SLAB_FEM_PART_NAME,    "0",   UF_UNIT_AREA_m2},
            {RAIL_SLAB_FEM_PART_NAME,    "0",   UF_UNIT_MOMENT_OF_INERTIA_m4},
            {RAIL_SLAB_FEM_PART_NAME,    RAIL_ELEMENT_SIZE_EXP_NAME,    UF_UNIT_LENGTH_m}
        };

        StlInputItemVector vInputItems(inputDataItem, inputDataItem + N_ELEMENTS(inputDataItem));

        WriteInputData(vInputItems, RAIL_INPUT_FILE_NAME);
    }

    void ExcitationInput::WriteSlabData() const
    {
        InputItem inputDataItem[] =
        {
            {SLABS_PRT_PART_NAME,       SLAB_COUNT_EXP_NAME,                ""},
            {SLAB_PRT_PART_NAME,        SLAB_LENGTH_EXP_NAME,               UF_UNIT_LENGTH_m},
            {RAIL_SLAB_FEM_PART_NAME,   SLAB_ELEMENT_SIZE_EXP_NAME,         UF_UNIT_LENGTH_m},
            {RAIL_SLAB_FEM_PART_NAME,   SLAB_MASS_RATIO_EXP_NAME,           ""},
            {RAIL_SLAB_FEM_PART_NAME,   SLAB_FASTENER_STIFFNESS_EXP_NAME,   UF_UNIT_PRESSUREONEDGE_N__m},
            {RAIL_SLAB_FEM_PART_NAME,   SLAB_FASTENER_DAMPING_EXP_NAME,     UF_UNIT_MASSFLOW_kg__sec},
            {RAIL_SLAB_FEM_PART_NAME,   SLAB_STIFFNESS_RATIO_EXP_NAME,      ""},
            {RAIL_SLAB_FEM_PART_NAME,   SLAB_DAMPING_RATIO_EXP_NAME,        ""},
            {RAIL_SLAB_FEM_PART_NAME,   SLAB_ELASTIC_MODULUS_EXP_NAME,      UF_UNIT_PRESSURE_N__m2},
            {RAIL_SLAB_FEM_PART_NAME,   "1",    UF_UNIT_MOMENT_OF_INERTIA_m4},
            {SLAB_PRT_PART_NAME,        SLAB_SUPPORT_COUNT_EXP_NAME,        ""}
        };

        StlInputItemVector vInputItems(inputDataItem, inputDataItem + N_ELEMENTS(inputDataItem));

        WriteInputData(vInputItems, SLAB_INPUT_FILE_NAME);
    }

    void ExcitationInput::WriteBeamData() const
    {
        // TODO: Handle Base and Tunnel
        // Bridge
        InputItem inputDataItem[] =
        {
            {BRIDGE_FEM_PART_NAME,       BRIDGE_ELASTIC_MODULUS_EXP_NAME,   UF_UNIT_PRESSURE_N__m2},
            {BRIDGE_FEM_PART_NAME,       BRIDGE_UNIT_WEIGHT_EXP_NAME,       UF_UNIT_MASS_kg},
            {BRIDGE_FEM_PART_NAME,       "0",   UF_UNIT_AREA_m2},
            {BRIDGE_FEM_PART_NAME,       "0",   UF_UNIT_MOMENT_OF_INERTIA_m4},
            {BEAM_PRT_PART_NAME,         WIDTH_EXP_NAME,                    UF_UNIT_LENGTH_m}
        };

        StlInputItemVector vInputItems(inputDataItem, inputDataItem + N_ELEMENTS(inputDataItem));

        WriteInputData(vInputItems, BEAM_INPUT_FILE_NAME);
    }

    void ExcitationInput::WriteCalculationData() const
    {
        //  Get result path name
        BaseProjectProperty *pPrjProp = Project::Instance()->GetProperty();

        std::string  strSimPrt(pPrjProp->GetProjectName().append("_s"));

        InputItem inputDataItem[] =
        {
            {strSimPrt,       TRAIN_SPEED_EXP_NAME},
            {strSimPrt,       COMPUTE_TIME_STEP_EXP_NAME}/*,
            {BRIDGE_FEM_PART_NAME,       ""},
            {BRIDGE_FEM_PART_NAME,       ""}*/
        };

        StlInputItemVector vInputItems(inputDataItem, inputDataItem + N_ELEMENTS(inputDataItem));

        WriteInputData(vInputItems, CALCULATION_INPUT_FILE_NAME);
    }
}
