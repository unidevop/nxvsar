
#include <uf_defs.h>
#include <Vsar_SolveOperation.hxx>

#include <cstdlib>
#include <fstream>

#include <boost/filesystem.hpp>
#include <boost/scope_exit.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/cast.hpp>

#include <uf_unit_types.h>
#include <uf.h>
#include <uf_sf.h>

#include <NXOpen/Session.hxx>
#include <NXOpen/PartCollection.hxx>
#include <NXOpen/Part.hxx>
#include <NXOpen/Expression.hxx>
#include <NXOpen/ExpressionCollection.hxx>
#include <NXOpen/UnitCollection.hxx>
#include <NXOpen/NXException.hxx>
#include <NXOpen/CAE_FTK_DataManager.hxx>
#include <NXOpen/CAE_CaeGroup.hxx>
#include <NXOpen/CAE_CaeGroupCollection.hxx>
#include <NXOpen/CAE_FemPart.hxx>
#include <NXOpen/CAE_SimPart.hxx>
#include <NXOpen/CAE_SimSimulation.hxx>
#include <NXOpen/CAE_SimSolution.hxx>
#include <NXOpen/CAE_FEModel.hxx>
#include <NXOpen/CAE_FEModelOccurrence.hxx>
#include <NXOpen/CAE_Mesh.hxx>
#include <NXOpen/CAE_IMeshManager.hxx>
#include <NXOpen/CAE_ModelingObjectPropertyTable.hxx>
#include <NXOpen/CAE_ModelingObjectPropertyTableCollection.hxx>
#include <NXOpen/CAE_PropertyTable.hxx>

#include <Vsar_Init_Utils.hxx>
#include <Vsar_Project.hxx>

#include <Vsar_Component.hxx>
#include <Vsar_Names.hxx>
#include <Vsar_Result.hxx>

using namespace boost;
using namespace NXOpen;
using namespace NXOpen::CAE;
using namespace Vsar;

#pragma warning(push)
#pragma warning(disable: 4355)
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
        //PreExecute();

        filesystem::path  oldWorkPath(filesystem::current_path());

        CreateWorkDir();

        //  remove work dir
        BOOST_SCOPE_EXIT((&m_workDir)(&oldWorkPath))
        {

#if !defined(_DEBUG) && !defined(DEBUG)
            if (filesystem::exists(m_workDir))
                filesystem::remove_all(m_workDir);
#endif
            m_workDir.clear();

            filesystem::current_path(oldWorkPath);
        }
        BOOST_SCOPE_EXIT_END

        CleanResult();

        PreExecute();

        //  Solve
        Solve();

        //LoadResult();
    }

    void BaseSolveOperation::CleanAfuFile(const std::string &resultPathName)
    {
        try
        {
            if (filesystem::exists(resultPathName))
            {
                try
                {
                    Session::GetSession()->DataManager()->UnloadFile(resultPathName.c_str());
                }
                catch(NXException&) // maybe the file is not loaded, delete it anyway
                {
                }
                filesystem::remove_all(resultPathName);
            }
        }
        catch (std::exception &)
        {
            throw NXException::Create("Failed to clean old result.");
        }
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

    SolveResponseOperation::SolveResponseOperation() : BaseSolveOperation(),
        m_computeExcitation(this), m_convertExcitation(this)
    {
    }

    SolveResponseOperation::~SolveResponseOperation()
    {
    }

    void SolveResponseOperation::PreExecute()
    {
        m_computeExcitation.Run();

        m_convertExcitation.Run();
    }

    void SolveResponseOperation::CleanResult()
    {
        ResponseResult   respResult;

        CleanAfuFile(respResult.GetResultPathName());
    }

    void SolveResponseOperation::Solve()
    {
        BaseProjectProperty *pPrjProp  = Project::Instance()->GetProperty();
        SimPart             *pSimPart  = pPrjProp->GetSimPart();

        SimSimulation *pSim = pSimPart->Simulation();
        std::string    strSol(std::string("Solution[").append(VSDANE_SOLUTION_NAME).append("]"));

        SimSolution * pSolution(dynamic_cast<SimSolution*>(pSim->FindObject(strSol)));

        pSolution->Solve(SimSolution::SolveOptionSolve,
            SimSolution::SetupCheckOptionDoNotCheck);
    }

    void SolveResponseOperation::LoadResult()
    {
        ResponseResult   respResult;

        respResult.Create();
    }

    BaseTask::BaseTask(const BaseSolveOperation *solOper) : m_solOper(solOper)
    {
    }

    BaseTask::~BaseTask()
    {
    }

    void BaseTask::Run()
    {
        CleanResults();

        PrepareInput();

        CallExecutable();

        PostSolveCheck();

        MoveOutputs();
    }

    void BaseTask::CleanResults() const
    {
        std::vector<std::string>  results(GetOutputResults());

        for (std::vector<std::string>::iterator iter = results.begin(); iter != results.end(); ++iter)
        {
            filesystem::path resultPath = m_solOper->GetSolutionDir() / *iter;

            if (filesystem::exists(resultPath))
                filesystem::remove_all(resultPath);
        }
    }

    void BaseTask::CallExecutable() const
    {
        filesystem::path  exePathName = filesystem::path(GetInstallPath()) /
            SOLVER_FOLDER_NAME / GetExecutableName();

        //  set work dir
        filesystem::current_path(m_solOper->GetWorkDir());

        std::system(exePathName.string().c_str());
    }

    void BaseTask::PostSolveCheck() const
    {
        filesystem::path failLogPath = m_solOper->GetWorkDir() / GetFailLog();

        if (filesystem::exists(failLogPath))
            throw NXException::Create("Failed to solve excitation.");
    }

    void BaseTask::MoveOutputs() const
    {
        std::vector<std::string>  results(GetOutputResults());

        for (std::vector<std::string>::iterator iter = results.begin(); iter != results.end(); ++iter)
        {
            filesystem::path srcPath = m_solOper->GetWorkDir() / *iter;
            filesystem::path dstPath = m_solOper->GetSolutionDir() / *iter;

            if (filesystem::exists(srcPath))
                filesystem::copy_file(srcPath, dstPath);
            else
                throw NXException::Create("Failed to solve.");
        }
    }

    ComputeExcitationTask::ComputeExcitationTask(const BaseSolveOperation *solOper) : BaseTask(solOper)
    {
    }

    ComputeExcitationTask::~ComputeExcitationTask()
    {
    }

    void ComputeExcitationTask::PrepareInput()
    {
        ExcitationInput excitationInput(m_solOper->GetWorkDir());

        excitationInput.Generate();
    }

    std::string ComputeExcitationTask::GetExecutableName() const
    {
        return SOLVER_ELASTIC_EXE_NAME;
    }

    std::string ComputeExcitationTask::GetSuccessLog() const
    {
        return SOLVE_ELASTIC_SUCCESS_LOG_NAME;
    }

    std::string ComputeExcitationTask::GetFailLog() const
    {
        return SOLVE_ELASTIC_FAIL_LOG_NAME;
    }

    std::vector<std::string> ComputeExcitationTask:: GetOutputResults() const
    {
        return std::vector<std::string>();
    }

    ConvertExcitationTask::ConvertExcitationTask(const BaseSolveOperation *solOper) : BaseTask(solOper), m_nodeOffset(0)
    {
    }

    ConvertExcitationTask::~ConvertExcitationTask()
    {
    }

    FEModelOccurrence* ConvertExcitationTask::GetRailFEModelOcc() const
    {
        BaseProjectProperty *pPrjProp  = Project::Instance()->GetProperty();
        SimPart             *pSimPart  = pPrjProp->GetSimPart();

        FEModelOccurrence   *pSimFEModel = pSimPart->Simulation()->Femodel();

        std::vector<FEModelOccurrence*> feModelChildren(pSimFEModel->GetChildren());

        if (feModelChildren.size() == 2)
        {
            for (unsigned int idx = 0; idx < feModelChildren.size(); idx++)
            {
                std::string strName = feModelChildren[idx]->Name().GetText();
                bool  isOcc = feModelChildren[idx]->IsOccurrence();
                feModelChildren[idx]->Print();
                std::string strID = feModelChildren[idx]->JournalIdentifier().GetText();

                BasePart *pPrt = feModelChildren[idx]->OwningPart();

                Assemblies::Component* pComp = feModelChildren[idx]->OwningComponent();

                IFEModel *pParentModel = feModelChildren[idx]->Parent();
            }
            //feModelChildren[0]->SetLabelOffsets(0, 0, 0);

            //feModelChildren[1]->SetLabelOffsets(GetMaxNodeLabel(feModelChildren[0]->FenodeLabelMap()),
            //    GetMaxElementLabel(feModelChildren[0]->FeelementLabelMap()), 0);
        }

        return pSimFEModel;
    }

    std::vector<tag_t> ConvertExcitationTask::GetRailNodes()
    {
        BaseProjectProperty *pPrjProp  = Project::Instance()->GetProperty();

        Mesh *pRailMesh = NULL;

        std::string strRailMeshName(std::string("MeshOccurrence[").append(RAIL_MESH_NAME).append("]"));
        SimPart             *pSimPart  = pPrjProp->GetSimPart();

        FEModelOccurrence   *pSimFEModel = pSimPart->Simulation()->Femodel();

        std::vector<FEModelOccurrence*>  childFeModelOcc(pSimFEModel->GetChildren());

        for (std::vector<FEModelOccurrence*>::iterator iter = childFeModelOcc.begin();
            iter != childFeModelOcc.end(); ++iter)
        {
            IMeshManager        *pMeshMgr    = (*iter)->MeshManager();

            try
            {
                pRailMesh = polymorphic_cast<Mesh*>(pMeshMgr->FindObject(strRailMeshName.c_str()));

                int elemOffset = 0;
                int csysOffset = 0;

                (*iter)->GetLabelOffsets(&m_nodeOffset, &elemOffset, &csysOffset);
                break;
            }
            catch (std::exception&)
            {
            }
        }
#if 0
        FemPart      *pFemPart  = pPrjProp->GetRailSlabFemPart();
        IMeshManager *pMeshMgr  = pFemPart->BaseFEModel()->MeshManager();

        std::string strRailMeshName(std::string("Mesh[").append(RAIL_MESH_NAME).append("]"));

        pRailMesh = polymorphic_cast<Mesh*>(pMeshMgr->FindObject(strRailMeshName.c_str()));
#endif
        int    nodeCnt = 0;
        int    errCode = 0;
        tag_t *tNodes  = NULL;

        errCode = UF_SF_locate_nodes_on_mesh(pRailMesh->Tag(), &nodeCnt, &tNodes);
        if (errCode != 0)
            throw NXException::Create(errCode);

        boost::shared_ptr<tag_t> pNodes(tNodes, UF_free);

        return std::vector<tag_t>(tNodes, tNodes + nodeCnt);
    }

    class NodePosComparer : public std::binary_function<tag_t, tag_t, bool>
    {
    public:
        NodePosComparer()
        {
        }

        ~NodePosComparer()
        {
        }

        bool operator () (tag_t tNode1, tag_t tNode2) const
        {
            int errCode = 0;
            int label   = 0;
            UF_SF_node_btype_t    bType;
            UF_SF_mid_node_type_t eType;
            double  absPos1[3];
            double  absPos2[3];

            errCode = UF_SF_ask_node(tNode1, &label, &bType, &eType, absPos1);
            if (errCode != 0)
                throw NXException::Create(errCode);

            errCode = UF_SF_ask_node(tNode2, &label, &bType, &eType, absPos2);
            if (errCode != 0)
                throw NXException::Create(errCode);

            return absPos1[2] < absPos2[2];
        }
    };

    void ConvertExcitationTask::WriteInputData(std::vector<tag_t> &railNodes) const
    {
        std::ofstream  inputFile(filesystem::path(m_solOper->GetWorkDir() /
            CONVERT_EXCITATION_INPUT_FILE_NAME).string().c_str());

        int errCode = 0;
        int label   = 0;
        UF_SF_node_btype_t    bType;
        UF_SF_mid_node_type_t eType;
        double  absPos[3];

        for (std::vector<tag_t>::iterator iter = railNodes.begin(); iter != railNodes.end(); ++iter)
        {
            errCode = UF_SF_ask_node(*iter, &label, &bType, &eType, absPos);
            if (errCode != 0)
                throw NXException::Create(errCode);

            inputFile << (label + m_nodeOffset) << std::endl;
        }
    }

    void ConvertExcitationTask::PrepareInput()
    {
        std::vector<tag_t>   railNodes(GetRailNodes());

        std::sort(railNodes.begin(), railNodes.end(), NodePosComparer());

        WriteInputData(railNodes);
    }

    std::string ConvertExcitationTask::GetExecutableName() const
    {
        return SOLVER_ELASTIC_CONVERT_EXE_NAME;
    }

    std::string ConvertExcitationTask::GetSuccessLog() const
    {
        return "";
    }

    std::string ConvertExcitationTask::GetFailLog() const
    {
        return SOLVE_CONVERT_ELASTIC_FAIL_LOG_NAME;
    }

    std::vector<std::string> ConvertExcitationTask:: GetOutputResults() const
    {
        std::vector<std::string>  results;

        results.reserve(3);
        results.push_back("force.dat");
        results.push_back("moment.dat");
        results.push_back("dload.dat");

        return results;
    }

    ExcitationInput::ExcitationInput(const filesystem::path &targetDir) : m_targetDir(targetDir)
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

        if (filesystem::exists(toFilePathName))
            filesystem::remove_all(toFilePathName);

        filesystem::copy_file(irrTmpPath, toFilePathName);
    }

    void ExcitationInput::WriteInputData(const StlInputItemVector &vInputItems, const std::string &fileName) const
    {
        if (vInputItems.empty())
            return;

        std::ofstream   inputFile(filesystem::path(m_targetDir / fileName).string().c_str());

        for (unsigned int idx = 0; idx < vInputItems.size(); idx++)
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
                expVal = lexical_cast<double>(vInputItems[idx].m_expName);
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
            //{RAIL_SLAB_FEM_PART_NAME,    "0",   UF_UNIT_AREA_m2},
            {RAIL_SLAB_FEM_PART_NAME,    RAIL_SECTION_INERTIA_EXP_NAME, UF_UNIT_MOMENT_OF_INERTIA_m4},
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
            {RAIL_SLAB_FEM_PART_NAME,   SLAB_SECTION_INERTIA_EXP_NAME,      UF_UNIT_MOMENT_OF_INERTIA_m4},
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
            {BEAM_PRT_PART_NAME,         SECTION_AREA_EXP_NAME,             UF_UNIT_AREA_m2},
            {BRIDGE_FEM_PART_NAME,       BRIDGE_SECTION_INERTIA_EXP_NAME,   UF_UNIT_MOMENT_OF_INERTIA_m4},
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
            {strSimPrt,       COMPUTE_TIME_STEP_EXP_NAME}
        };

        StlInputItemVector vInputItems(inputDataItem, inputDataItem + N_ELEMENTS(inputDataItem));

        WriteInputData(vInputItems, CALCULATION_INPUT_FILE_NAME);
    }

    SolveSettings::SolveSettings(bool bOutputElems, const std::vector<TaggedObject*> &outputElems,
        bool bOutputNodes, const std::vector<TaggedObject*> &outputNodes,
        bool bOutputNodesForNoise) : m_bOutputElems(bOutputElems), m_outputElems(outputElems),
        m_bOutputNodes(bOutputNodes), m_outputNodes(outputNodes), m_bOutputNodesForNoise(bOutputNodesForNoise)
    {
    }

    void SolveSettings::Apply()
    {
        if (m_bOutputElems)
            SetEntityGroup(ELEMENT_FOR_RESPONSE_GROUP_NAME, m_outputElems);

        if (m_bOutputNodes)
            SetEntityGroup(NODE_FOR_RESPONSE_GROUP_NAME, m_outputNodes);

        SetNoiseOutput();

        SetTimeStep();
    }

    void SolveSettings::SetEntityGroup(const std::string &groupName,
        const std::vector<TaggedObject*> &outputEntities)
    {
        BaseProjectProperty *pPrjProp  = Project::Instance()->GetProperty();
        CAE::SimPart        *pSimPart  = pPrjProp->GetSimPart();

        CaeGroup *pGroup = pSimPart->CaeGroups()->FindObject(groupName);

        pGroup->SetEntities(outputEntities);
    }

    void SolveSettings::SetNoiseOutput()
    {
        BaseProjectProperty *pPrjProp  = Project::Instance()->GetProperty();
        CAE::SimPart        *pSimPart  = pPrjProp->GetSimPart();

        std::string  strModelingObj(std::string("SsmoPropTable[").append(NOISE_STRUCTURAL_OUTPUT_OBJECT_NAME).append("]"));

        ModelingObjectPropertyTable *pModelingObjPT(pSimPart->ModelingObjectPropertyTables()->FindObject(strModelingObj));

        PropertyTable *pPropTab = pModelingObjPT->PropertyTable();

        pPropTab->SetBooleanPropertyValue("Velocity - Enable", m_bOutputNodesForNoise);
    }

    void SolveSettings::SetTimeStep()
    {
        BaseProjectProperty *pPrjProp  = Project::Instance()->GetProperty();
        CAE::SimPart        *pSimPart  = pPrjProp->GetSimPart();

        std::string  strModelingObj(std::string("SsmoPropTable[").append(TIME_STEP_OUTPUT_OBJECT_NAME).append("]"));

        ModelingObjectPropertyTable *pModelingObjPT(pSimPart->ModelingObjectPropertyTables()->FindObject(strModelingObj));

        PropertyTable *pPropTab = pModelingObjPT->PropertyTable();

        Expression *pExp = pSimPart->Expressions()->FindObject(NUM_OF_TIME_STEPS_EXP_NAME);

        pPropTab->SetIntegerPropertyValue("Number of Time Steps", numeric_cast<int>(pExp->Value()));
    }
}

#pragma warning(pop)