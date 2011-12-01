
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
    class Expression;

    namespace CAE
    {
        class BaseFemPart;
        class IFEModel;
        class CAEBody;
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

        bool HasGeometryDependency() const;

        void UpdateGeometryModel();

        void UpdateFEModel();

        template <typename UpdateCallback>
        void UpdateFECompModel(NXOpen::CAE::BaseFemPart *pFem, UpdateCallback updateCb);

        virtual void UpdateRailSlabModel();

        virtual void UpdateBraseModel();

        virtual void UpdateAssembleModel();

        void UpdateSweptMesh(NXOpen::CAE::IFEModel *pFeModel, const std::vector<NXOpen::CAE::CAEBody*> &pPolygonBodies,
                            const std::string &meshColName, const std::string &meshName, const std::string &eleSizeExpName);

        void UpdateSweptMesh_sf(NXOpen::CAE::IFEModel *pFeModel, const std::vector<NXOpen::CAE::CAEBody*> &pPolygonBodies,
                            const std::string &meshColName, const std::string &meshName, NXOpen::Expression *pEleSize);

        void MergeDuplicateNodes();

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
