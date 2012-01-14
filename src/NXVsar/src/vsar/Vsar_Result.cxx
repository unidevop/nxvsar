
#include <uf_defs.h>
#include <Vsar_Result.hxx>

#include <algorithm>
#include <fstream>
#include <regex>
#include <boost/filesystem.hpp>
#include <boost/scope_exit.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
//#include <boost/function.hpp>
//#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/foreach.hpp>

#include <uf_ui.h>

#include <NXOpen/ugmath.hxx>
#include <NXOpen/Session.hxx>
#include <NXOpen/NXException.hxx>
#include <NXOpen/Point.hxx>
#include <NXOpen/CAE_AfuManager.hxx>
#include <NXOpen/CAE_AfuData.hxx>
#include <NXOpen/CAE_AfuDataConvertor.hxx>
#include <NXOpen/CAE_FTK_DataManager.hxx>

#include <Vsar_Init_Utils.hxx>
#include <Vsar_Project.hxx>
#include <Vsar_Names.hxx>
#include <Vsar_SolveOperation.hxx>

using namespace boost;
using namespace boost::lambda;
using namespace NXOpen;
using namespace NXOpen::CAE;
using namespace Vsar;

//------------------------------------------------------------------------------
// Declaration of global variables
//------------------------------------------------------------------------------
namespace Vsar
{
    BaseResult::~BaseResult()
    {
    }

    bool BaseResult::IsResultExist() const
    {
        return filesystem::exists(GetResultPathName());
    }

    bool BaseResult::Load() const
    {
        bool success = false;

        if (IsResultExist())
        {
            try
            {
                FTK::DataManager *pDataMgr = Session::GetSession()->DataManager();

                pDataMgr->LoadFile(GetResultPathName().c_str());
            }
            catch (std::exception &)
            {
            }
            success = true;
        }
        else
            success = false;

        return success;
    }

    std::string ResponseResult::GetResultPathName() const
    {
        //  Get result path name
        BaseProjectProperty *pPrjProp = Project::Instance()->GetProperty();

        return (filesystem::path(pPrjProp->GetProjectPath()) /
            pPrjProp->GetResponseResultName()).string();
    }

    void BaseAfuResult::Create()
    {
        CreateResultFile();

        Session::GetSession()->DataManager()->LoadFile(GetResultPathName());

        CreateRecords();
    }

    void BaseAfuResult::CreateResultFile()
    {
        Session::GetSession()->AfuManager()->CreateNewAfuFile(GetResultPathName().c_str());
    }

    std::string BaseAfuResult::GetNastranResultPathName() const
    {
        BaseProjectProperty *pPrjProp = Project::Instance()->GetProperty();

        std::string    strSolverResultName((boost::format(NASTRAN_PCH_RESULT_FILE_PATTERN_NAME) %
            pPrjProp->GetProjectName() % VSDANE_SOLUTION_NAME).str());

        return  ((filesystem::path(pPrjProp->GetProjectPath()) / strSolverResultName).string());
    }

#if 0
    struct ResponseRecordItem
    {
        std::string         m_recordName;
        std::string         m_srcResultFile;
        unsigned int        m_idxColumns[2];
        unsigned int        m_columnCnt;
        XyFunctionUnit      m_xUnit;
        XyFunctionUnit      m_yUnit;
    };

    static ResponseRecordItem s_responseRecordItems[] =
    {
        {"Beam-time-acceleration",          "beam_out.dat",     0, 3, 4, XyFunctionUnitTimeSec, XyFunctionUnitAccelerationM},
        {"Vehicle-time-acceleration",       "vehicle_out.dat",  0, 3, 4, XyFunctionUnitTimeSec, XyFunctionUnitAccelerationM},
        {"Rail-Midspan-time-acceleration",  "rail_out.dat",     0, 3, 4, XyFunctionUnitTimeSec, XyFunctionUnitAccelerationM},
        {"Slab-time-acceleration",          "fslab_out.dat",    0, 3, 4, XyFunctionUnitTimeSec, XyFunctionUnitAccelerationM},
        {"Sometime-Rail-time-acceleration", "rail_timeout.dat", 0, 3, 4, XyFunctionUnitTimeSec, XyFunctionUnitAccelerationM},
        {"Sometime-Beam-time-acceleration", "beam_timeout.dat", 0, 3, 4, XyFunctionUnitTimeSec, XyFunctionUnitAccelerationM},
        {"Sometime-Rail-displacement",      "rail_timeout.dat", 0, 1, 4, XyFunctionUnitTimeSec, XyFunctionUnitDisplacementM },
        {"Sometime-Beam-displacement",      "beam_timeout.dat", 0, 1, 4, XyFunctionUnitTimeSec, XyFunctionUnitDisplacementM },
        {"Rail-Midspan-displacement",       "rail_out.dat",     0, 1, 4, XyFunctionUnitTimeSec, XyFunctionUnitDisplacementM },
        {"Slab-time-displacement",          "fslab_out.dat",    0, 1, 4, XyFunctionUnitTimeSec, XyFunctionUnitDisplacementM },
        {"Beam-time-displacement",          "beam_out.dat",     0, 1, 4, XyFunctionUnitTimeSec, XyFunctionUnitDisplacementM }
    };
#endif

    class ResultBlock
    {
    public:
        ResultBlock() : m_label(0)
        {
        }

        virtual ~ResultBlock()
        {
        }

        bool Read(std::ifstream &ifStream)
        {
            if (!ReadHead(ifStream))
                return false;

            ReadBody(ifStream);

            return true;
        }

        virtual void Write(const std::string &afuFile) = 0;

        virtual XyFunctionUnit  GetXUnit() const;
        virtual XyFunctionUnit  GetYUnit() const = 0;

        virtual std::string GetBlockName() const = 0;
        virtual std::string GetKeyName() const = 0;

    protected:
        virtual bool ReadHead(std::ifstream &ifStream);
        virtual void ReadHeadInfo(const std::string &strHeadLine);
        virtual void ReadBody(std::ifstream &ifStream) = 0;
    protected:
        int  m_label;
    };

    XyFunctionUnit ResultBlock::GetXUnit() const
    {
        return XyFunctionUnitTimeSec;
    }

    bool ResultBlock::ReadHead(std::ifstream &ifStream)
    {
        std::string  strRead;
        bool         found   = false;
        std::string  strBlockId(std::string("$").append(GetBlockName()));

        std::tr1::regex reg(std::string("^\\$").append(GetKeyName()).append("\\s*=\\s*(\\d+).*"));

        while(!ifStream.eof())
        {
            if (found)  // found
            {
                if (m_label == 0)
                {
                    std::getline(ifStream, strRead);

                    std::tr1::smatch what;
                    if(std::tr1::regex_match(strRead, what, reg) && what.size() == 2)
                    {
                        m_label = boost::lexical_cast<int>(what[1]);
                    }
                }
                else
                {
                    if (ifStream.peek() != '$')     // read content
                        break;
                    else
                    {
                        std::getline(ifStream, strRead);
                        ReadHeadInfo(strRead);
                    }
                }
            }
            else    // not found
            {
                ifStream >> strRead;
                if (strRead == strBlockId)
                {
                    found = true;
                }
                ifStream.ignore(200, '\n');    // ignore rest of the line
            }
        }

        return found;
    }

    void ResultBlock::ReadHeadInfo(const std::string &strHeadLine)
    {
    }

    class NodeResultBlock : public ResultBlock
    {
    public:
        typedef boost::tuple<double, Point3d, Vector3d> NodeDataItem;

    public:
        NodeResultBlock();
        virtual ~NodeResultBlock();

        virtual void Write(const std::string &afuFile);

        virtual std::string GetKeyName() const;

    protected:
        virtual void ReadBody(std::ifstream &ifStream);

        void WriteRecord(const std::string &afuFile, const std::string &recordName,
            const std::vector<double> &xValues, const std::vector<double> &yValues);
    private:

        std::vector<NodeDataItem>   m_values;
    };

    NodeResultBlock::NodeResultBlock() : m_values()
    {
    }

    NodeResultBlock::~NodeResultBlock()
    {
    }

    std::string NodeResultBlock::GetKeyName() const
    {
        return "POINT\\s*ID";
    }

    void NodeResultBlock::ReadBody(std::ifstream &ifStream)
    {
        std::string  strRead;
        char         aStr[10];
        double       time;
        Point3d      coord;
        Vector3d     rotate;

        while(!ifStream.eof() && (ifStream.peek() != '$'))
        {
            ifStream >> time >> aStr >> coord.X >> coord.Y >> coord.Z;
            ifStream.ignore(200, '\n');    // ignore rest of the line
            ifStream >> aStr >> rotate.X >> rotate.Y >> rotate.Z;
            ifStream.ignore(200, '\n');    // ignore rest of the line

            m_values.push_back(NodeDataItem(time, coord, rotate));
        }
    }

    void NodeResultBlock::WriteRecord(const std::string &afuFile, const std::string &recordName,
                                        const std::vector<double> &xValues, const std::vector<double> &yValues)
    {
        AfuManager *pAfuMgr = Session::GetSession()->AfuManager();

        boost::scoped_ptr<AfuData>      pAfuData(pAfuMgr->CreateAfuData());

        pAfuData->SetFileName(afuFile.c_str());
        pAfuData->SetRecordName(recordName.c_str());
        pAfuData->SetAxisDefinition(AfuData::AbscissaTypeUneven, GetXUnit(),
                                    AfuData::OrdinateTypeReal, GetYUnit());
        pAfuData->SetFunctionDataType(XyFunctionDataTypeTime);

        pAfuData->SetRealData(xValues, yValues);

        pAfuMgr->CreateRecord(pAfuData.get());
    }

    void NodeResultBlock::Write(const std::string &afuFile)
    {
        std::vector<double> times;
        std::vector<double> xCoords;
        std::vector<double> yCoords;
        std::vector<double> zCoords;

        times.reserve(m_values.size());
        xCoords.reserve(m_values.size());
        yCoords.reserve(m_values.size());
        zCoords.reserve(m_values.size());

        BOOST_FOREACH(NodeDataItem dispItem, m_values)
        {
            times.push_back(dispItem.get<0>());
            xCoords.push_back(dispItem.get<1>().X);
            yCoords.push_back(dispItem.get<1>().Y);
            zCoords.push_back(dispItem.get<1>().Z);
        }

        boost::format  recordName(boost::format(RESPONSE_RESULT_RECORD_PATTERN_NAME) % GetBlockName() % m_label);

        WriteRecord(afuFile, (boost::format(recordName) % "X").str(), times, xCoords);
        WriteRecord(afuFile, (boost::format(recordName) % "Y").str(), times, yCoords);
        WriteRecord(afuFile, (boost::format(recordName) % "Z").str(), times, zCoords);
    }

    class DisplacementBlock : public NodeResultBlock
    {
    public:
        DisplacementBlock();
        virtual ~DisplacementBlock();

        virtual XyFunctionUnit  GetYUnit() const;

        virtual std::string GetBlockName() const;
    };

    DisplacementBlock::DisplacementBlock() : NodeResultBlock()
    {
    }

    DisplacementBlock::~DisplacementBlock()
    {
    }

    XyFunctionUnit DisplacementBlock::GetYUnit() const
    {
        return XyFunctionUnitDisplacementMm;
    }

    std::string DisplacementBlock::GetBlockName() const
    {
        return "DISPLACEMENTS";
    }

    class AccelerationBlock : public NodeResultBlock
    {
    public:
        AccelerationBlock();
        virtual ~AccelerationBlock();

        virtual XyFunctionUnit  GetYUnit() const;

        virtual std::string GetBlockName() const;
    };

    AccelerationBlock::AccelerationBlock() : NodeResultBlock()
    {
    }

    AccelerationBlock::~AccelerationBlock()
    {
    }

    XyFunctionUnit AccelerationBlock::GetYUnit() const
    {
        return XyFunctionUnitAccelerationMm;
    }

    std::string AccelerationBlock::GetBlockName() const
    {
        return "ACCELERATION";
    }

    class VelocityBlock : public NodeResultBlock
    {
    public:
        VelocityBlock();
        virtual ~VelocityBlock();

        virtual XyFunctionUnit  GetYUnit() const;

        virtual std::string GetBlockName() const;
    };

    VelocityBlock::VelocityBlock() : NodeResultBlock()
    {
    }

    VelocityBlock::~VelocityBlock()
    {
    }

    XyFunctionUnit VelocityBlock::GetYUnit() const
    {
        return XyFunctionUnitVelocityMm;
    }

    std::string VelocityBlock::GetBlockName() const
    {
        return "VELOCITY";
    }

    template<typename BlockType>
    StlResultBlockVector NastranResult::ReadDataBlock(std::ifstream &ifStream)
    {
        StlResultBlockVector vResultBlock;

        //  Handle displacement result
        ifStream.clear();
        ifStream.seekg(0, std::ios::beg);  // goto file head

        while (!ifStream.eof())
        {
            boost::shared_ptr<ResultBlock> pResultBlock(new BlockType());

            if (pResultBlock->Read(ifStream))
                vResultBlock.push_back(pResultBlock);
        }

        return vResultBlock;
    }

    void NastranResult::CreateRecords()
    {
        std::string resultName(GetResultPathName());

        //  remove work dir
        BOOST_SCOPE_EXIT((&resultName))
        {
            AfuManager *pAfuMgr = Session::GetSession()->AfuManager();

            // DELETE AFU
            if (pAfuMgr->GetRecordIndexes(resultName.c_str()).empty())
                pAfuMgr->DeleteAfuFile(resultName.c_str());
        }
        BOOST_SCOPE_EXIT_END

        std::string  nastranResultName(GetNastranResultPathName());

        if (!filesystem::exists(nastranResultName))
            throw NXException::Create("No solve result exists.");

        StlResultBlockVector  vAllResultBlock;

        std::ifstream  solverResult(nastranResultName.c_str());

        vAllResultBlock = ExtractContent(solverResult);

        if (!vAllResultBlock.empty())
        {
            BOOST_FOREACH(StlResultBlockVector::value_type pResultBlock, vAllResultBlock)
            {
                pResultBlock->Write(resultName);
            }
        }
    }

#if 0
    std::string ResponseResult::GetResultPathName() const
    {
        //  Get result path name
        BaseProjectProperty *pPrjProp = Project::Instance()->GetProperty();

        return (filesystem::path(pPrjProp->GetProjectPath()) /
                                 pPrjProp->GetResponseResultName()).string();
    }

    StlResultBlockVector ResponseResult::ExtractContent(std::ifstream &solverResult)
    {
        StlResultBlockVector  vResultBlock;
        StlResultBlockVector vAllResultBlock;

        vResultBlock = ReadDataBlock<DisplacementBlock>(solverResult);
        vAllResultBlock.insert(vAllResultBlock.end(), vResultBlock.begin(), vResultBlock.end());

        vResultBlock = ReadDataBlock<AccelerationBlock>(solverResult);
        vAllResultBlock.insert(vAllResultBlock.end(), vResultBlock.begin(), vResultBlock.end());

        // element stress block

        return vAllResultBlock;
    }
#endif

    //////////////////////////////////////////////////////////////////////////
    //  NoiseIntermResult
    std::string NoiseIntermResult::GetResultPathName() const
    {
        //  Get result path name
        BaseProjectProperty *pPrjProp = Project::Instance()->GetProperty();

        return (filesystem::path(pPrjProp->GetProjectPath()) /
                                 pPrjProp->GetNoiseIntermediateResultName()).string();
    }

    StlResultBlockVector NoiseIntermResult::ExtractContent(std::ifstream &solverResult)
    {
        return ReadDataBlock<VelocityBlock>(solverResult);
    }

    //////////////////////////////////////////////////////////////////////////
    //  NoiseResult
    std::string NoiseResult::GetResultPathName() const
    {
        //  Get result path name
        BaseProjectProperty *pPrjProp = Project::Instance()->GetProperty();

        return (filesystem::path(pPrjProp->GetProjectPath()) /
                                 pPrjProp->GetNoiseResultName()).string();
    }

    void NoiseResult::CreateRecords()
    {
        std::string resultName(GetResultPathName());

        //  remove work dir
        BOOST_SCOPE_EXIT((&resultName))
        {
            AfuManager *pAfuMgr = Session::GetSession()->AfuManager();

                // DELETE AFU
            if (pAfuMgr->GetRecordIndexes(resultName.c_str()).empty())
                pAfuMgr->DeleteAfuFile(resultName.c_str());
        }
        BOOST_SCOPE_EXIT_END

        for (unsigned int idx = 0; idx < m_outputPoints.size(); idx++)
        {
            std::vector<double>     xValues;
            std::vector<double>     yValues;

            OutputTimeRecord(idx, xValues, yValues);

            OutputFreqRecord(idx, xValues, yValues);
        }
    }

    void NoiseResult::OutputTimeRecord(int idxRecord, std::vector<double> &xValues, std::vector<double> &yValues)
    {
        //  output time-sound pressure
        Point3d   coord(m_outputPoints[idxRecord]->Coordinates());

        std::string noiseOutputName;
        std::string recordName;

        noiseOutputName = (boost::format(NOISE_TIME_OUTPUT_FILE_NAME) % (idxRecord+1)).str();
        recordName      = (boost::format(NOISE_RESULT_TIME_RECORD_PATTERN_NAME) %
            coord.X % coord.Y % coord.Z).str();

        //  Read xy values from dat file
        ReadDataFromDat(noiseOutputName, xValues, yValues);

        WriteRecord(recordName, XyFunctionDataTypeTime,
            XyFunctionUnitTimeSec, XyFunctionUnitUnknown, xValues, yValues);
    }

    void NoiseResult::OutputFreqRecord(int idxRecord, const std::vector<double> &xValues, const std::vector<double> &yValues)
    {
        //  output frequency-sound pressure
        Point3d   coord(m_outputPoints[idxRecord]->Coordinates());

        //std::string noiseOutputName;
        std::string recordName;

        //noiseOutputName = (boost::format(NOISE_FREQ_OUTPUT_FILE_NAME) % (idxRecord+1)).str();
        recordName      = (boost::format(NOISE_RESULT_FREQ_RECORD_PATTERN_NAME) %
            coord.X % coord.Y % coord.Z).str();

#if 0
        WriteRecord(noiseOutputName, recordName, XyFunctionDataTypeSpectrum,
            XyFunctionUnitFrequencyHz, XyFunctionUnitStressPa);
#endif

        //  FFT Time-Sound data
        AfuManager *pAfuMgr = Session::GetSession()->AfuManager();

        AfuDataConvertor *pAfuConvert = pAfuMgr->AfuDataConvertor();

        std::vector<double> freqVals, yReals, yImags;

        yImags = pAfuConvert->GetFftFrequencyData(xValues, yValues, freqVals, yReals);

        WriteRecord(recordName, XyFunctionDataTypeGeneral,
            XyFunctionUnitFrequencyHz, XyFunctionUnitUnknown, freqVals, yReals, yImags);
    }

    void NoiseResult::WriteRecord(const std::string &recordName, XyFunctionDataType funcType,
                                  XyFunctionUnit xUnit, XyFunctionUnit yUnit,
                                  const std::vector<double> &xValues, const std::vector<double> &yValues)
    {
        AfuManager *pAfuMgr = Session::GetSession()->AfuManager();

        boost::scoped_ptr<AfuData>      pAfuData(pAfuMgr->CreateAfuData());

        pAfuData->SetFileName(GetResultPathName().c_str());
        pAfuData->SetRecordName(recordName.c_str());
        pAfuData->SetAxisDefinition(AfuData::AbscissaTypeUneven, xUnit,
                                    AfuData::OrdinateTypeReal, yUnit);
        pAfuData->SetFunctionDataType(funcType);

        // The following API is not available
        // Modify Y Axis Label to "Sound Pressure"
        // Modify Y Unit Label to "decibal"

        pAfuData->SetRealData(xValues, yValues);

        pAfuMgr->CreateRecord(pAfuData.get());
    }

    void NoiseResult::WriteRecord(const std::string &recordName, XyFunctionDataType funcType,
                                  XyFunctionUnit xUnit, XyFunctionUnit yUnit,
                                  const std::vector<double> &xValues,
                                  const std::vector<double> &yRealValues, const std::vector<double> &yImagValues)
    {
        AfuManager *pAfuMgr = Session::GetSession()->AfuManager();

        boost::scoped_ptr<AfuData>      pAfuData(pAfuMgr->CreateAfuData());

        pAfuData->SetFileName(GetResultPathName().c_str());
        pAfuData->SetRecordName(recordName.c_str());
        pAfuData->SetAxisDefinition(AfuData::AbscissaTypeUneven, xUnit,
                                    AfuData::OrdinateTypeRealImaginary, yUnit);
        pAfuData->SetFunctionDataType(funcType);

        pAfuData->SetComplexData(xValues, yRealValues, yImagValues);

        pAfuMgr->CreateRecord(pAfuData.get());
    }

    void NoiseResult::ReadDataFromDat(const std::string &noiseOutputName,
                                      std::vector<double> &xValues, std::vector<double> &yValues) const
    {
        //  Get dat path name
        std::string datResultPathName((m_srcDir / noiseOutputName).string());

        if (!filesystem::exists(datResultPathName))
            throw NXException::Create((boost::format(MSGTXT("Result file %1% does not exist.")) % noiseOutputName).str().c_str());

        std::ifstream  ifDatResult(datResultPathName.c_str());

        // omit first line
        std::string  strLine;

        std::getline(ifDatResult, strLine);

        double          xVal = 0.0;
        double          yVal = 0.0;

        while (!ifDatResult.eof())
        {
            ifDatResult >> xVal >> yVal;

            xValues.push_back(xVal);
            yValues.push_back(yVal);
        }
    }

    void ResultsLoader::operator() ()
    {
        bool             respResultLoaded  = false;
        bool             noiseResultLoaded = false;

        SolveResponseOperation  solveRespOper;

        solveRespOper.LoadResult();

        NoiseResult noiseResult(filesystem::path(""), std::vector<Point*>());  // for result query only

        noiseResultLoaded = noiseResult.Load();

        std::string strRespStatus = (respResultLoaded) ?
            "Successfully loaded response result. " :
            "Failed to load response result, please solve first. ";

        std::string strNoiseStatus = (noiseResultLoaded) ?
                "Successfully loaded noise result." :
                "Failed to load noise result, please solve first.";

        UF_UI_set_status(const_cast<char*>((strRespStatus + strNoiseStatus).c_str()));
    }
}
