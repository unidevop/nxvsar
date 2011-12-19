
#include <uf_defs.h>
#include <Vsar_Result.hxx>

#include <algorithm>
#include <fstream>
#include <regex>
#include <boost/filesystem.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
//#include <boost/function.hpp>
//#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/lexical_cast.hpp>

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

    class ResultProcessor
    {
    public:
        ResultProcessor()
        {
        }

        virtual ~ResultProcessor()
        {
        }

        void Read(std::ifstream &ifStream)
        {
            ReadHead(ifStream);
            ReadBody(ifStream);
        }

        virtual void Write() = 0;

        virtual XyFunctionUnit  GetXUnit() const;
        virtual XyFunctionUnit  GetYUnit() const = 0;

        virtual std::string GetBlockName() const = 0;
        virtual std::string GetKeyName() const = 0;

    protected:
        virtual void ReadHead(std::ifstream &ifStream) = 0;
        virtual void ReadBody(std::ifstream &ifStream) = 0;
    };

    XyFunctionUnit ResultProcessor::GetXUnit() const
    {
        return XyFunctionUnitTimeSec;
    }

    class DisplacementProcessor : public ResultProcessor
    {
    public:
        //struct DataItem
        //{
        //    double   m_time;
        //    Point3d  m_point;
        //    Vector3d m_rotate;
        //};

        typedef boost::tuple<double, Point3d, Vector3d> DispDataItem;
    public:
        DisplacementProcessor();
        virtual ~DisplacementProcessor();

        virtual void Write();

        virtual XyFunctionUnit  GetYUnit() const;

        virtual std::string GetBlockName() const;
        virtual std::string GetKeyName() const;

        int GetNodeLabel() const
        {
            return m_nodeLabel;
        }

    protected:
        virtual void ReadHead(std::ifstream &ifStream);
        virtual void ReadBody(std::ifstream &ifStream);
    private:
        int  m_nodeLabel;

        std::vector<DispDataItem>   m_values;
    };

    DisplacementProcessor::DisplacementProcessor() : m_nodeLabel(0), m_values()
    {
    }

    DisplacementProcessor::~DisplacementProcessor()
    {
    }

    XyFunctionUnit DisplacementProcessor::GetYUnit() const
    {
        return XyFunctionUnitDisplacementMm;
    }

    std::string DisplacementProcessor::GetBlockName() const
    {
        return "$DISPLACEMENTS";
    }

    std::string DisplacementProcessor::GetKeyName() const
    {
        return "POINT\\s*ID";
    }

    void DisplacementProcessor::ReadHead(std::ifstream &ifStream)
    {
        std::string  strRead;
        bool         found   = false;

        std::tr1::regex reg(std::string("^\\$").append(GetKeyName()).append("\\s*=\\s*(\\d+).*"));

        while(ifStream.good())
        {
            if (found)  // found
            {
                if (m_nodeLabel == 0)
                {
                    std::getline(ifStream, strRead);

                    std::tr1::match_results<std::string::const_iterator> what;
                    if(std::tr1::regex_match(strRead, what, reg) && what.size() == 2)
                    {
                        m_nodeLabel = boost::lexical_cast<int>(what[1]);
                    }
                }
                else
                {
                    if (ifStream.peek() != '$')     // read content
                        break;
                    else
                        std::getline(ifStream, strRead);
                }
            }
            else    // not found
            {
                ifStream >> strRead;
                if (strRead == GetBlockName())
                {
                    found = true;
                }
                ifStream.ignore(200, '\n');    // ignore rest of the line
            }
        }
    }

    void DisplacementProcessor::ReadBody(std::ifstream &ifStream)
    {

    }

    void DisplacementProcessor::Write()
    {
        
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

#if 0
    void ResponseResult::CreateRecords()
    {
        std::for_each(s_responseRecordItems, s_responseRecordItems + N_ELEMENTS(s_responseRecordItems),
            bind(&ResponseResult::CreateRecord, this, _1));
    }
#endif

    void ResponseResult::CreateRecords()
    {
        BaseProjectProperty *pPrjProp = Project::Instance()->GetProperty();
        std::string    strSolverResultName(pPrjProp->GetProjectName().append("_s-").append(VSDANE_SOLUTION_NAME).append(".pch"));
        std::ifstream  solverResult((filesystem::path(pPrjProp->GetProjectPath()) /
                                    strSolverResultName).string().c_str());

        DisplacementProcessor dispProc;

        dispProc.Read(solverResult);
    }

    void ResponseResult::CreateRecord(const ResponseRecordItem &recordItem)
    {
        AfuManager *pAfuMgr = Session::GetSession()->AfuManager();

        boost::scoped_ptr<AfuData>      pAfuData(pAfuMgr->CreateAfuData());

        pAfuData->SetFileName(GetResultPathName().c_str());
        pAfuData->SetRecordName(recordItem.m_recordName);
        pAfuData->SetAxisDefinition(AfuData::AbscissaTypeUneven, recordItem.m_xUnit,
                                    AfuData::OrdinateTypeReal, recordItem.m_yUnit);
        pAfuData->SetFunctionDataType(XyFunctionDataTypeTime);

        //  Read xy values from dat file
        std::vector<double>     xValues;
        std::vector<double>     yValues;

        ReadDataFromDat(recordItem, xValues, yValues);

        pAfuData->SetRealData(xValues, yValues);

        pAfuMgr->CreateRecord(pAfuData.get());
    }

    void ResponseResult::ReadDataFromDat(const ResponseRecordItem &recordItem,
                                         std::vector<double> &xValues, std::vector<double> &yValues) const
    {
        //  Get dat path name
        BaseProjectProperty *pPrjProp = Project::Instance()->GetProperty();

        std::string datResultPathName((filesystem::path(pPrjProp->GetProjectPath()) /
                                       recordItem.m_srcResultFile).string());

        if (!filesystem::exists(datResultPathName))
            NXException::Create(MSGTXT("Result file does not exist."));

        std::ifstream  ifDatResult(datResultPathName.c_str());
        std::vector<double>    srcRecLineItem(recordItem.m_columnCnt);

        while (ifDatResult.good())
        {
            std::for_each(srcRecLineItem.begin(), srcRecLineItem.end(), ifDatResult >> _1);

            xValues.push_back(srcRecLineItem[recordItem.m_idxColumns[0]]);
            yValues.push_back(srcRecLineItem[recordItem.m_idxColumns[1]]);
        }
    }
}
