
#ifndef VSAR_COMPONENT_H_INCLUDED
#define VSAR_COMPONENT_H_INCLUDED

#include <string>
#include <vector>
//#include <boost/scoped_ptr.hpp>

//------------------------------------------------------------------------------
// Forward declaration for Class
//------------------------------------------------------------------------------
namespace NXOpen
{
    class Body;
    class Expression;

    namespace CAE
    {
        class BaseFemPart;
        class FemPart;
        class IFEModel;
        class CAEBody;
        class Mesh;
    }
}

namespace Vsar
{

    struct CompAttrInfo
    {
        std::string  m_attrName;
        std::string  m_partName;
        std::string  m_expName;
    };

    typedef std::vector<CompAttrInfo>  StlCompAttrInfoVector;

    class BaseComponent
    {
    public:
        BaseComponent(const CompAttrInfo *pCompAttrs, int compAttrCnt);

        virtual ~BaseComponent();

        void LoadGeometryPart(bool onDemandLoad = true);

        const StlCompAttrInfoVector& GetAttrInfo() const
        {
            return m_compAttrs;
        }

        NXOpen::Expression*  GetExpression(const std::string &attrName);

        static NXOpen::Expression*  GetExpression(const std::string &partName, const std::string &expName);

        void UpdateModel();

        virtual void OnInit();

    protected:

        virtual bool HasGeometryDependency() const;

        void UpdateGeometryModel();

        void UpdateFEModel();

        template <typename UpdateCallback>
        void UpdateFECompModel(NXOpen::CAE::BaseFemPart *pFem, UpdateCallback updateCb);

        void UpdateRailSlabModel();

        void UpdateBraseModel();

        virtual bool CanUpdateRailSlabFEModel() const;

        virtual bool CanUpdateBraseFEModel() const;

        void UpdateAssembleModel();

        void UpdateSweptMesh(NXOpen::CAE::IFEModel *pFeModel, const std::vector<NXOpen::CAE::CAEBody*> &pPolygonBodies,
                            const std::string &meshColName, const std::string &meshName, const std::string &eleSizeExpName);

        void UpdateSweptMesh_sf(NXOpen::CAE::IFEModel *pFeModel, const std::vector<NXOpen::CAE::CAEBody*> &pPolygonBodies,
                            const std::string &meshColName, const std::string &meshName, NXOpen::Expression *pEleSize);

        std::vector<NXOpen::Body*> GetGeoModelOccs(NXOpen::CAE::FemPart *pFemPart,
            const std::string &bodyPrtName, const std::string &bodyName);

        void UpdateRailSlabConnection();

        void UpdateBaseSlabConnection();

        virtual bool CanUpdateRailSlabConnection() const;

        virtual bool CanUpdateBraseConnection() const;

        void MergeDuplicateNodes(const std::vector<NXOpen::CAE::Mesh*> &meshToMergeNodes);

        void SetFeGeometryData( NXOpen::CAE::FemPart * pFemPart, const std::vector<NXOpen::Body*> &bodyOccs, bool syncLines );

    protected:
        StlCompAttrInfoVector   m_compAttrs;
    };
}

//#if defined(__MSVC_RUNTIME_CHECKS)
//#undef __MSVC_RUNTIME_CHECKS
//#endif

//#if !defined(_DEBUG)
//#define _DEBUG
//#endif

#endif //VSAR_COMPONENT_H_INCLUDED
