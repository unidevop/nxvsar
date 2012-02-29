
#include <uf_defs.h>
#include <Vsar_SolveOperation.hxx>

#include <cstdlib>
#include <cmath>
#include <regex>
#include <fstream>
#include <iomanip>

#include <boost/filesystem.hpp>
#include <boost/scope_exit.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/cast.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>

#include <uf_unit_types.h>
#include <uf.h>
#include <uf_sf.h>

#include <NXOpen/Session.hxx>
#include <NXOpen/PartCollection.hxx>
#include <NXOpen/Part.hxx>
#include <NXOpen/Expression.hxx>
#include <NXOpen/ExpressionCollection.hxx>
#include <NXOpen/UnitCollection.hxx>
#include <NXOpen/Point.hxx>
#include <NXOpen/PointCollection.hxx>
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
#include <NXOpen/CAE_FENode.hxx>
#include <NXOpen/CAE_FENodeLabelMap.hxx>
#include <NXOpen/CAE_ModelingObjectPropertyTable.hxx>
#include <NXOpen/CAE_ModelingObjectPropertyTableCollection.hxx>
#include <NXOpen/CAE_PropertyTable.hxx>
#include <NXOpen/CAE_AfuManager.hxx>
#include <NXOpen/CAE_AfuDataConvertor.hxx>

#include <Vsar_Init_Utils.hxx>
#include <Vsar_Project.hxx>

#include <Vsar_Component.hxx>
#include <Vsar_Names.hxx>
#include <Vsar_Result.hxx>
#include <Vsar_Utils.hxx>

using namespace boost;
using namespace NXOpen;
using namespace NXOpen::CAE;
using namespace Vsar;

//#pragma warning(push)
//#pragma warning(disable: 4355)
//------------------------------------------------------------------------------
// Declaration of global variables
//------------------------------------------------------------------------------
namespace Vsar
{

    class NoiseDatumPointsUpdater
    {
    public:
        NoiseDatumPointsUpdater()
        {
        }

        ~NoiseDatumPointsUpdater()
        {
        }

        void Update();

        Point* GetSlabCenter() const;

    protected:
        std::vector<TaggedObject*> GetDatumNodes() const;

        std::vector<Point3d> GetDatumPoints() const;

        Point3d GetSlabDim() const;
    };

    void NoiseDatumPointsUpdater::Update()
    {
        BaseProjectProperty *pPrjProp  = Project::Instance()->GetProperty();
        CAE::SimPart        *pSimPart  = pPrjProp->GetSimPart();

        CaeGroup *pGroup = pSimPart->CaeGroups()->FindObject(NODES_FOR_NOISE_GROUP_NAME);

        // Update noise datum points manually
        if (pGroup->GetEntities().size() != 14)
        {
            std::vector<TaggedObject*>  datumNodes(GetDatumNodes());

            if (datumNodes.size() == 14)
                pGroup->SetEntities(datumNodes);
            else
                throw NXException::Create("Failed to update noise datum points.");
        }
    }

    std::vector<TaggedObject*> NoiseDatumPointsUpdater::GetDatumNodes() const
    {
        std::vector<Point3d>  datumPts(GetDatumPoints());

        std::vector<TaggedObject*>  datumNodes;

        return datumNodes;
    }

    std::vector<Point3d> NoiseDatumPointsUpdater::GetDatumPoints() const
    {
        std::vector<Point3d>  datumPts;

        datumPts.reserve(14);

        Point3d  slabDim(GetSlabDim());
        Point3d  slabCenter(GetSlabCenter()->Coordinates());

        for (int idx = 0; idx < 7; idx++)
        {
            datumPts.push_back(Point3d(slabCenter.X - slabDim.X/2, slabCenter.Y, slabCenter.Z - slabDim.Z/2 + idx * slabDim.Z/6));
            datumPts.push_back(Point3d(slabCenter.X + slabDim.X/2, slabCenter.Y, slabCenter.Z - slabDim.Z/2 + idx * slabDim.Z/6));
        }

        return datumPts;
    }

    Point* NoiseDatumPointsUpdater::GetSlabCenter() const
    {
        BaseProjectProperty *pPrjProp  = Project::Instance()->GetProperty();
        CAE::SimPart        *pSimPart  = pPrjProp->GetSimPart();
        PointCollection     *pPoints   = pSimPart->Points();

        std::string        pointName(SLAB_CENTER_POINT_NAME);
        //  Get points
        for (PointCollection::iterator iter = pPoints->begin(); iter != pPoints->end(); ++iter)
        {
            if (pointName.compare((*iter)->Name().GetText()) == 0)
                return *iter;
        }

        return NULL;
    }

    Point3d NoiseDatumPointsUpdater::GetSlabDim() const
    {
        Expression *pLengthExp = BaseComponent::GetExpression(SLAB_PRT_PART_NAME, SLAB_LENGTH_EXP_NAME);
        Expression *pWidghExp  = BaseComponent::GetExpression(SLAB_PRT_PART_NAME, SLAB_WIDTH_EXP_NAME);

        return Point3d(pWidghExp->Value(), 0.0, pLengthExp->Value());
    }


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
            try
            {
                if (filesystem::exists(m_workDir))
                    filesystem::remove_all(m_workDir);
            }
            catch (std::exception &)
            {
            }
#endif
            m_workDir.clear();

            filesystem::current_path(oldWorkPath);
        }
        BOOST_SCOPE_EXIT_END

        CleanResult();

        PreExecute();

        //  Solve
        Solve();

        if (CanAutoLoadResult())
        {
            LoadResult();

            // save parts
            CAE::SimPart *pSimPrt = Project::Instance()->GetProperty()->GetSimPart();

            pSimPrt->Save(BasePart::SaveComponentsTrue, BasePart::CloseAfterSaveFalse);
        }
    }

    void BaseSolveOperation::CleanResultFile(const std::string &resultPathName)
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
        filesystem::path strScratchDir(m_solDir);   //  Use custom scratch dir in future
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
        ComputeExcitationTask  computeExcitation(this);

        computeExcitation.Run();

        ConvertExcitationTask  convertExcitation(this);

        convertExcitation.Run();
    }

    void SolveResponseOperation::CleanResult()
    {
        ResponseOp2Result   respResult;

        CleanResultFile(respResult.GetResultPathName());

        ResponseAfuResult   respAfuResult;

        CleanResultFile(respAfuResult.GetResultPathName());

        NoiseIntermResult noiseIntermResult;

        CleanResultFile(noiseIntermResult.GetResultPathName());
    }

    void SolveResponseOperation::Solve()
    {
        BaseProjectProperty *pPrjProp  = Project::Instance()->GetProperty();
        SimPart             *pSimPart  = pPrjProp->GetSimPart();

        SimSimulation *pSim = pSimPart->Simulation();
        std::string    strSol((boost::format(FIND_SOLUTION_PATTERN_NAME) % VSDANE_SOLUTION_NAME).str());

        SimSolution * pSolution(dynamic_cast<SimSolution*>(pSim->FindObject(strSol)));

        pSolution->Solve(SimSolution::SolveOptionSolve,
            SimSolution::SetupCheckOptionDoNotCheck);
    }

    bool SolveResponseOperation::CanAutoLoadResult() const
    {
        return true;
    }

    void SolveResponseOperation::LoadResult()
    {
        ResponseOp2Result   respResult;

        respResult.Load();

        ResponseAfuResult   respAfuResult;

        respAfuResult.Create();

        // may don't have noise intermediate result
        NoiseIntermResult noiseIntermResult;

        try
        {
            noiseIntermResult.Create();
        }
        catch (NXException &)
        {
            // does nothing if failed to extract noise intermediate results, the empty result has been deleted
        }

        //  modify project status
        if (noiseIntermResult.IsResultExist())
        {
            FTK::DataManager *pDataMgr = Session::GetSession()->DataManager();

            pDataMgr->UnloadFile(noiseIntermResult.GetResultPathName().c_str());
            Project::GetStatus()->Switch(Status::ProjectStatus_ResponseNoiseSolved);
        }
        else if (respResult.IsResultExist() && respAfuResult.IsResultExist())
            Project::GetStatus()->Switch(Status::ProjectStatus_ResponseSolved);
    }

    SolveNoiseOperation::SolveNoiseOperation(const std::vector<NXOpen::Point*> &pts)
        : BaseSolveOperation(), m_outputPoints(pts)
    {
    }

    SolveNoiseOperation::~SolveNoiseOperation()
    {
    }

    void SolveNoiseOperation::PreExecute()
    {
        NoiseDatumPointsUpdater datumPtsUpdater;

        datumPtsUpdater.Update();

        // convert to FFT
        NoiseInput  noiseInput(m_workDir, m_outputPoints);

        noiseInput.Generate();
    }

    void SolveNoiseOperation::CleanResult()
    {
        NoiseResult   respResult(m_workDir, m_outputPoints);

        CleanResultFile(respResult.GetResultPathName());
    }

    void SolveNoiseOperation::Solve()
    {
        filesystem::path  exePathName = filesystem::path(GetInstallPath()) /
            SOLVER_FOLDER_NAME / SOLVER_NOISE_EXE_NAME;

        //  set work dir
        filesystem::current_path(GetWorkDir());

        std::system(exePathName.string().c_str());

        filesystem::path failLogPath = GetWorkDir() / SOLVE_NOISE_FAIL_LOG_NAME;

        if (filesystem::exists(failLogPath))
            throw NXException::Create("Failed to solve noise.");
    }

    bool SolveNoiseOperation::CanAutoLoadResult() const
    {
        return true;
    }

    void SolveNoiseOperation::LoadResult()
    {
        NoiseResult   noiseResult(m_workDir, m_outputPoints);

        noiseResult.Create();

        //  modify project status
        if (noiseResult.IsResultExist())
            Project::GetStatus()->Switch(Status::ProjectStatus_NoiseSolved);
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

    std::vector<std::string> ComputeExcitationTask::GetOutputResults() const
    {
        std::vector<std::string>  results;

        results.reserve(3);
        results.push_back(VEHICLE_OUTPUT_FILE_NAME);
        results.push_back(WHEEL_OUTPUT_FILE_NAME);
        results.push_back(TURN_OUTPUT_FILE_NAME);

        return results;
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

        std::string strRailMeshName((boost::format(FIND_MESH_OCC_PATTERN_NAME) % RAIL_MESH_NAME).str());

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

        //FEModelOccurrence *pRailFEModelOcc = GetFEModelOccByMeshName(RAIL_MESH_NAME);

        //int elemOffset = 0;
        //int csysOffset = 0;

        //pRailFEModelOcc->GetLabelOffsets(&m_nodeOffset, &elemOffset, &csysOffset);
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

    class NodePosZComparer : public std::binary_function<tag_t, tag_t, bool>
    {
    public:
        NodePosZComparer()
        {
        }

        ~NodePosZComparer()
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

        std::sort(railNodes.begin(), railNodes.end(), NodePosZComparer());

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
            {RAIL_SLAB_FEM_PART_NAME,    RAIL_LINEAR_DENSITY_EXP_NAME,  UF_UNIT_MASSPERLENGTH_kg__m},
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
        // Bridge
        BaseProjectProperty *pPrjProp = Project::Instance()->GetProperty();

        StlInputItemVector vInputItems;

        switch (pPrjProp->GetProjectType())
        {
        case Project::ProjectType_Bridge:
            {
                InputItem inputDataItem[] =
                {
                    {BRIDGE_FEM_PART_NAME,       BRIDGE_ELASTIC_MODULUS_EXP_NAME,   UF_UNIT_PRESSURE_N__m2},
                    {BRIDGE_FEM_PART_NAME,       BRIDGE_MASS_DENSITY_EXP_NAME,      UF_UNIT_MASSDENSITY_kg__m3},
                    {BEAM_PRT_PART_NAME,         SECTION_AREA_EXP_NAME,             UF_UNIT_AREA_m2},
                    {BRIDGE_FEM_PART_NAME,       BRIDGE_SECTION_INERTIA_EXP_NAME,   UF_UNIT_MOMENT_OF_INERTIA_m4}/*,
                    {BEAM_PRT_PART_NAME,         WIDTH_EXP_NAME,                    UF_UNIT_LENGTH_m}*/
                };

                vInputItems.insert(vInputItems.end(), inputDataItem, inputDataItem + N_ELEMENTS(inputDataItem));
            }
            break;
        case Project::ProjectType_Selmi_Infinite:
            {
                InputItem inputDataItem[] =
                {
                    {BASE_FEM_PART_NAME,     BASE_ELASTIC_MODULUS_EXP_NAME,   UF_UNIT_PRESSURE_N__m2},
                    {BASE_FEM_PART_NAME,     BASE_MASS_DENSITY_EXP_NAME,      UF_UNIT_MASSDENSITY_kg__m3},
                    {BASE_PRT_PART_NAME,     SECTION_AREA_EXP_NAME,           UF_UNIT_AREA_m2},
                    {BASE_FEM_PART_NAME,     BASE_SECTION_INERTIA_EXP_NAME,   UF_UNIT_MOMENT_OF_INERTIA_m4}
                };

                vInputItems.insert(vInputItems.end(), inputDataItem, inputDataItem + N_ELEMENTS(inputDataItem));
            }
            break;
        case Project::ProjectType_Tunnel:
            {
                InputItem inputDataItem[] =
                {
                    {TUNNEL_FEM_PART_NAME,   TUNNEL_CONCRETE_ELASTIC_MODULUS_EXP_NAME,   UF_UNIT_PRESSURE_N__m2},
                    {TUNNEL_FEM_PART_NAME,   TUNNEL_CONCRETE_MASS_DENSITY_EXP_NAME,      UF_UNIT_MASSDENSITY_kg__m3},
                    {TUNNEL_PRT_PART_NAME,   SECTION_AREA_EXP_NAME,                      UF_UNIT_AREA_m2},
                    {TUNNEL_FEM_PART_NAME,   TUNNEL_SECTION_INERTIA_EXP_NAME,            UF_UNIT_MOMENT_OF_INERTIA_m4}
                };

                vInputItems.insert(vInputItems.end(), inputDataItem, inputDataItem + N_ELEMENTS(inputDataItem));
            }
            break;
        default:
            break;
        }

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


    class NodePosXZComparer : public std::binary_function<TaggedObject*, TaggedObject*, bool>
    {
    public:
        NodePosXZComparer()
        {
        }

        ~NodePosXZComparer()
        {
        }

        bool operator () (TaggedObject *pNode1, TaggedObject *pNode2) const
        {
            int errCode = 0;
            int label   = 0;
            UF_SF_node_btype_t    bType;
            UF_SF_mid_node_type_t eType;
            double  absPos1[3];
            double  absPos2[3];

            errCode = UF_SF_ask_node(pNode1->Tag(), &label, &bType, &eType, absPos1);
            if (errCode != 0)
                throw NXException::Create(errCode);

            errCode = UF_SF_ask_node(pNode2->Tag(), &label, &bType, &eType, absPos2);
            if (errCode != 0)
                throw NXException::Create(errCode);

            return (absPos1[0] > absPos2[0]) || (absPos1[2] < absPos2[2]);
        }
    };

    void NoiseInput::ConstructRefNodeSequence()
    {
        //  Get all ref nodes
        BaseProjectProperty *pPrjProp  = Project::Instance()->GetProperty();
        CAE::SimPart        *pSimPart  = pPrjProp->GetSimPart();

        CaeGroup *pGroup = pSimPart->CaeGroups()->FindObject(NODES_FOR_NOISE_GROUP_NAME);

        m_refNodeSeq = pGroup->GetEntities();

        std::sort(m_refNodeSeq.begin(), m_refNodeSeq.end(), NodePosXZComparer());
    }

    void NoiseInput::Generate() const
    {
        WriteOutputPoints();

        WriteFrequenceData();
    }

    void NoiseInput::WriteFrequenceData() const
    {
        std::string afuFileName(GetIntermediateResult());

        //  unload intermediate result
        BOOST_SCOPE_EXIT((&afuFileName))
        {
            try
            {
                FTK::DataManager *pDataMgr = Session::GetSession()->DataManager();
                pDataMgr->UnloadFile(afuFileName.c_str());
            }
            catch (std::exception &)
            {
            }
        }
        BOOST_SCOPE_EXIT_END

        AfuManager *pAfuMgr = Session::GetSession()->AfuManager();

        AfuDataConvertor *pAfuConvert = pAfuMgr->AfuDataConvertor();

        std::vector<int>  recordIndices(pAfuMgr->GetRecordIndexes(afuFileName.c_str()));

        BOOST_FOREACH(int idx, recordIndices)
        {
            AfuData *pAfuData = NULL;

            pAfuMgr->GetAfuData(afuFileName.c_str(), idx, &pAfuData);

            std::string recordName(pAfuData->RecordName().GetText());

            if(std::tr1::regex_search(recordName, std::tr1::regex("-Node-\\d+-Y$")))
            {
                std::vector<double> xValues, yValues;

                yValues = pAfuData->GetRealData(xValues);

                std::vector<double> freqVals, yReals, yImags;
                yImags = pAfuConvert->GetFftFrequencyData(xValues, yValues, freqVals, yReals);

                WriteRecord(recordName, freqVals, yReals, yImags);
            }
        }
    }

    void NoiseInput::WriteRecord(const std::string &recordName, const std::vector<double> &freqVals,
                                 const std::vector<double> &yReals, const std::vector<double> &yImags) const
    {
        std::ofstream  inputFile(filesystem::path(m_targetDir / GetTargetInputName(recordName)).string().c_str());

        for (unsigned int idx = 0; idx < freqVals.size(); idx++)
        {
            //  write values
            inputFile << std::setw(15) << freqVals[idx] << " " <<
                         std::setw(15) << mmToMConvert * yReals[idx] << " " <<
                         std::setw(15) << mmToMConvert * yImags[idx] << std::endl;
        }
    }

    std::string NoiseInput::GetIntermediateResult() const
    {
        NoiseIntermResult  noiseIntermResult;

        std::string intermResultPathName(noiseIntermResult.GetResultPathName());

        if (filesystem::exists(intermResultPathName))
        {
            // Load it anyway
            try
            {
                FTK::DataManager *pDataMgr = Session::GetSession()->DataManager();
                pDataMgr->LoadFile(intermResultPathName.c_str());
            }
            catch (std::exception &)
            {
            }
        }
        else
            noiseIntermResult.Create();

        return intermResultPathName;
    }

    std::string NoiseInput::GetTargetInputName(const std::string &recordName) const
    {
        int nodeLabel = 0;

        //  Get Node Label
        std::tr1::regex reg("-Node-(\\d+)-");

        std::tr1::smatch what;
        if(std::tr1::regex_search(recordName, what, reg) && what.size() == 2)
        {
            nodeLabel = boost::lexical_cast<int>(what[1]);
        }

        BaseProjectProperty *pPrjProp  = Project::Instance()->GetProperty();
        SimPart             *pSimPart  = pPrjProp->GetSimPart();
        FEModelOccurrence   *pSimFEModel = pSimPart->Simulation()->Femodel();

        FEModelOccurrence *pRailFEModelOcc       = GetFEModelOccByMeshName(pSimFEModel, RAIL_MESH_NAME);
        FENodeLabelMap    *pRailSlabNodeLabelMap = pRailFEModelOcc->FenodeLabelMap();
        FENode            *pFENode               = pRailSlabNodeLabelMap->GetNode(nodeLabel);

        int nodeIndex = static_cast<int>(std::find(m_refNodeSeq.begin(), m_refNodeSeq.end(), pFENode) - m_refNodeSeq.begin() + 1);

        return (boost::format(NOISE_FREQUENCE_INPUT_FILE_NAME) % nodeIndex).str();
    }

    void NoiseInput::WriteOutputPoints() const
    {
        Point *pCenterPt = GetSlabCenter();

        if (!pCenterPt)
            throw NXException::Create("The slab center point has been deleted.");

        Point3d centerCoord(pCenterPt->Coordinates());

        std::ofstream   inputFile(filesystem::path(m_targetDir / NOISE_COORDINATE_INPUT_FILE_NAME).string().c_str());

        BOOST_FOREACH(Point *pOutputPt, m_outputPoints)
        {
            //  convert the coordinate to first slab
            Point3d coord(pOutputPt->Coordinates());

            coord.Z = std::fmod(coord.Z, centerCoord.Z * 2);

            //  write relative coordinate
            inputFile << std::setw(15) << mmToMConvert * (coord.X - centerCoord.X) << " " <<
                         std::setw(15) << mmToMConvert * (coord.Y - centerCoord.Y) << " " <<
                         std::setw(15) << mmToMConvert * (coord.Z - centerCoord.Z) << std::endl;
        }
    }

    Point* NoiseInput::GetSlabCenter() const
    {
        NoiseDatumPointsUpdater datumPtsUpdater;

        return datumPtsUpdater.GetSlabCenter();
    }

    SolveSettings::SolveSettings(bool bOutputElems, const std::vector<TaggedObject*> &outputElems,
        bool bOutputNodes, const std::vector<TaggedObject*> &outputNodes,
        bool bOutputNodesForNoise) : m_bOutputElems(bOutputElems), m_outputElems(outputElems),
        m_bOutputNodes(bOutputNodes), m_outputNodes(outputNodes), m_bOutputNodesForNoise(bOutputNodesForNoise)
    {
    }

    void SolveSettings::Apply()
    {
        SetRunJobInForeground();

        CheckConstraints();

        SetResponseOutput();

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

    void SolveSettings::SetRunJobInForeground()
    {
        BaseProjectProperty *pPrjProp  = Project::Instance()->GetProperty();
        CAE::SimPart        *pSimPart  = pPrjProp->GetSimPart();
        std::string    strSol((boost::format(FIND_SOLUTION_PATTERN_NAME) % VSDANE_SOLUTION_NAME).str());

        SimSimulation *pSim = pSimPart->Simulation();

        SimSolution * pSolution(dynamic_cast<SimSolution*>(pSim->FindObject(strSol)));

        PropertyTable *pPropTab = pSolution->PropertyTable();

        pPropTab->SetBooleanPropertyValue("Foreground", true);
    }

    void SolveSettings::SetResponseOutput()
    {
        if (m_bOutputElems)
            SetEntityGroup(ELEMENT_FOR_RESPONSE_GROUP_NAME, m_outputElems);

        if (m_bOutputNodes)
            SetEntityGroup(NODE_FOR_RESPONSE_GROUP_NAME, m_outputNodes);

        std::vector<OutputRequestItem> outputReqItems;

        outputReqItems.reserve(3);
        outputReqItems.push_back(OutputRequestItem("Acceleration - Enable", m_bOutputNodes));
        outputReqItems.push_back(OutputRequestItem("Displacement - Enable", m_bOutputNodes));
        outputReqItems.push_back(OutputRequestItem("Stress - Enable", m_bOutputElems));

        OpenOutputRequests(RESPONSE_STRUCTURAL_OUTPUT_OBJECT_NAME, outputReqItems);
    }

    void SolveSettings::CheckConstraints()
    {
        // TODO: check constraints
    }

    void SolveSettings::SetNoiseOutput()
    {
        //  Check Noise Datum Points
        if (m_bOutputNodesForNoise)
        {
            NoiseDatumPointsUpdater datumPtsUpdater;

            datumPtsUpdater.Update();
        }

        std::vector<OutputRequestItem> outputReqItems;

        outputReqItems.push_back(OutputRequestItem("Velocity - Enable", m_bOutputNodesForNoise));

        OpenOutputRequests(NOISE_STRUCTURAL_OUTPUT_OBJECT_NAME, outputReqItems);
    }

    void SolveSettings::OpenOutputRequests(const std::string &oObjName, const std::vector<OutputRequestItem> &outputReqItems)
    {
        BaseProjectProperty *pPrjProp  = Project::Instance()->GetProperty();
        CAE::SimPart        *pSimPart  = pPrjProp->GetSimPart();

        std::string strModelingObj((boost::format(FIND_MODELING_OBJ_PATTERN_NAME) % oObjName.c_str()).str());

        ModelingObjectPropertyTable *pModelingObjPT(pSimPart->ModelingObjectPropertyTables()->FindObject(strModelingObj));

        PropertyTable *pPropTab = pModelingObjPT->PropertyTable();

        BOOST_FOREACH(OutputRequestItem oReqItem, outputReqItems)
        {
            pPropTab->SetBooleanPropertyValue(oReqItem.get<0>().c_str(), oReqItem.get<1>());
        }
    }

    void SolveSettings::SetTimeStep()
    {
        BaseProjectProperty *pPrjProp  = Project::Instance()->GetProperty();
        CAE::SimPart        *pSimPart  = pPrjProp->GetSimPart();

        std::string strModelingObj((boost::format(FIND_MODELING_OBJ_PATTERN_NAME) % TIME_STEP_OUTPUT_OBJECT_NAME).str());

        ModelingObjectPropertyTable *pModelingObjPT(pSimPart->ModelingObjectPropertyTables()->FindObject(strModelingObj));

        PropertyTable *pPropTab = pModelingObjPT->PropertyTable();

        Expression *pExp = pSimPart->Expressions()->FindObject(NUM_OF_TIME_STEPS_EXP_NAME);

        pPropTab->SetIntegerPropertyValue("Number of Time Steps", numeric_cast<int>(pExp->Value()));
    }
}

//#pragma warning(pop)
