
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

#include <NXOpen/ugmath.hxx>
#include <NXOpen/Session.hxx>
#include <NXOpen/NXException.hxx>
#include <NXOpen/CAE_AfuManager.hxx>
#include <NXOpen/CAE_AfuData.hxx>
#include <NXOpen/CAE_FTK_DataManager.hxx>

#include <Vsar_Init_Utils.hxx>
#include <Vsar_Project.hxx>
#include <Vsar_Names.hxx>

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
    BaseResult::BaseResult()
    {
    }

    BaseResult::~BaseResult()
    {
    }

    void BaseResult::Create()
    {
        CreateResultFile();

        Session::GetSession()->DataManager()->LoadFile(GetResultPathName());

        CreateRecords();
    }

    bool BaseResult::IsResultExist() const
    {
        return filesystem::exists(GetResultPathName());
    }

    void BaseResult::CreateResultFile()
    {
        Session::GetSession()->AfuManager()->CreateNewAfuFile(GetResultPathName().c_str());
    }

    std::string BaseResult::GetNastranResultPathName() const
    {
        BaseProjectProperty *pPrjProp = Project::Instance()->GetProperty();
        std::string    strSolverResultName(pPrjProp->GetProjectName().append("_s-").append(VSDANE_SOLUTION_NAME).append(".pch"));

        return  ((filesystem::path(pPrjProp->GetProjectPath()) / strSolverResultName).string());
    }

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


    NastranResult::NastranResult()
    {
    }

    NastranResult::~NastranResult()
    {
    }

#if 0
    void ResponseResult::CreateRecords()
    {
        std::for_each(s_responseRecordItems, s_responseRecordItems + N_ELEMENTS(s_responseRecordItems),
            bind(&ResponseResult::CreateRecord, this, _1));
    }
#endif

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
        std::string  nastranResultName(GetNastranResultPathName());

        StlResultBlockVector  vAllResultBlock;
        std::string resultName(GetResultPathName());

        //  remove work dir
        BOOST_SCOPE_EXIT((&vAllResultBlock)(&resultName))
        {
            if (vAllResultBlock.empty())
            {
                // DELETE AFU
                Session::GetSession()->AfuManager()->DeleteAfuFile(resultName.c_str());
            }
        }
        BOOST_SCOPE_EXIT_END

        if (!filesystem::exists(nastranResultName))
            throw NXException::Create("No solve result exists.");

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

    ResponseResult::ResponseResult()
    {
    }

    ResponseResult::~ResponseResult()
    {
    }

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

        // TODO: element stress block

        return vAllResultBlock;
    }

    NoiseIntermResult::NoiseIntermResult()
    {
    }

    NoiseIntermResult::~NoiseIntermResult()
    {
    }

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

    //void ResponseResult::CreateRecord(const ResponseRecordItem &recordItem)
    //{
    //    AfuManager *pAfuMgr = Session::GetSession()->AfuManager();

    //    boost::scoped_ptr<AfuData>      pAfuData(pAfuMgr->CreateAfuData());

    //    pAfuData->SetFileName(GetResultPathName().c_str());
    //    pAfuData->SetRecordName(recordItem.m_recordName);
    //    pAfuData->SetAxisDefinition(AfuData::AbscissaTypeUneven, recordItem.m_xUnit,
    //                                AfuData::OrdinateTypeReal, recordItem.m_yUnit);
    //    pAfuData->SetFunctionDataType(XyFunctionDataTypeTime);

    //    //  Read xy values from dat file
    //    std::vector<double>     xValues;
    //    std::vector<double>     yValues;

    //    ReadDataFromDat(recordItem, xValues, yValues);

    //    pAfuData->SetRealData(xValues, yValues);

    //    pAfuMgr->CreateRecord(pAfuData.get());
    //}

    //void ResponseResult::ReadDataFromDat(const ResponseRecordItem &recordItem,
    //                                     std::vector<double> &xValues, std::vector<double> &yValues) const
    //{
    //    //  Get dat path name
    //    BaseProjectProperty *pPrjProp = Project::Instance()->GetProperty();

    //    std::string datResultPathName((filesystem::path(pPrjProp->GetProjectPath()) /
    //                                   recordItem.m_srcResultFile).string());

    //    if (!filesystem::exists(datResultPathName))
    //        throw NXException::Create(MSGTXT("Result file does not exist."));

    //    std::ifstream  ifDatResult(datResultPathName.c_str());
    //    std::vector<double>    srcRecLineItem(recordItem.m_columnCnt);

    //    while (ifDatResult.good())
    //    {
    //        std::for_each(srcRecLineItem.begin(), srcRecLineItem.end(), ifDatResult >> _1);

    //        xValues.push_back(srcRecLineItem[recordItem.m_idxColumns[0]]);
    //        yValues.push_back(srcRecLineItem[recordItem.m_idxColumns[1]]);
    //    }
    //}
}
