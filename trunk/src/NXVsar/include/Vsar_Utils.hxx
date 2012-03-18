
#ifndef VSAR_UTILS_H_INCLUDED
#define VSAR_UTILS_H_INCLUDED

//------------------------------------------------------------------------------
//These includes are needed for the following template code
//------------------------------------------------------------------------------
#include <string>
#include <vector>

namespace NXOpen
{
    class BasePart;
    class Part;
    class Body;
    class TaggedObject;
    class DisplayableObject;
    class Expression;

    namespace Assemblies
    {
        class Component;
    }

    namespace CAE
    {
        class IFEModel;
        class BaseFEModel;
        class BaseFemPart;
        class MeshManager;
        class MeshCollector;
        class CAEBody;
        class CAEFace;
        class FEModelOccurrence;
        class IHierarchicalFEModel;
        class Mesh;
        class FENode;
        class CaePart;
    }
}

namespace Vsar
{
    //------------------------------------------------------------------------------
    // Declaration of global variables
    //------------------------------------------------------------------------------
    //extern NewProject *theVsar_NewProject;
    int ReportError(const std::string &file, int line, const std::string &call, int irc);

#ifndef UF_CALL
#define UF_CALL(X) (ReportError( __FILE__, __LINE__, #X, (X)))
#endif

    void LoadRootPart();

    std::vector<NXOpen::Body*> GetBodyByName(NXOpen::Part *pBodyPrt, const std::string &bodyName);

    std::vector<NXOpen::TaggedObject*> GetPointByAttrName( NXOpen::BasePart * pPtPrt, const std::string &ptName );

    std::vector<NXOpen::TaggedObject*> GetPointByLayer( NXOpen::BasePart * pPtPrt, int layer );

    std::vector<NXOpen::Assemblies::Component*> GetOccInCompTree(NXOpen::Assemblies::Component *pAssemTree, NXOpen::Part *pPrt);

    std::vector<NXOpen::CAE::CAEBody*> GetCaeBodies(const std::vector<NXOpen::Body*> &bodies);

    std::vector<NXOpen::CAE::CAEFace*> GetCaeFaceByName(NXOpen::CAE::CAEBody *pCaeBody, const std::string &faceName);

    std::vector<NXOpen::CAE::CAEFace*> GetCaeFaceByName(NXOpen::CAE::CaePart *pPrt, const std::string &faceName);

    std::vector<NXOpen::CAE::CAEBody*> GetCaeBodyByName(NXOpen::CAE::CaePart *pPrt, const std::string &bodyName);

    std::vector<NXOpen::CAE::CAEFace*> GetCaeFacesOfBodyByName(NXOpen::CAE::CaePart *pPrt, const std::string &bodyName, const std::string &faceName);

    void EditSweptMeshData(NXOpen::CAE::IFEModel *pFeModel, const std::string &meshName,
                        const std::vector<NXOpen::Body*> &srcBodies);

    void CreateSweptMesh(NXOpen::CAE::MeshManager *pMeshMgr, NXOpen::CAE::MeshCollector *pMeshCol,
                         const std::string &meshName, NXOpen::CAE::CAEFace* pSrcFace, NXOpen::CAE::CAEFace *pTargetFace,
                         const std::string &eleSizeExpName);

    void CreateSweptMesh_sf(NXOpen::CAE::IFEModel *pFeModel, NXOpen::CAE::CAEBody *pSolidBody,
                         const std::string &meshColName, const std::string &meshName,
                         NXOpen::CAE::CAEFace* pSrcFace, NXOpen::Expression *pEleSize);

    void Update1DConnection(NXOpen::CAE::BaseFEModel *pFeModel,
                            const std::vector<NXOpen::TaggedObject*> &railConnectPts,
                            const std::vector<NXOpen::TaggedObject*> &slabConnectPts,
                            const std::string &connName,
                            const std::string &connColName,
                            const std::string &meshName);

    NXOpen::CAE::FEModelOccurrence* GetFEModelOccByMeshName(NXOpen::CAE::IHierarchicalFEModel *pHieFeModel,
        const std::string &meshName);

    NXOpen::CAE::Mesh* GetMeshByName(NXOpen::CAE::IFEModel *pFEModel,
                                     const std::string &meshNamePattern, const std::string &meshName);

    void DeleteMeshesInCollector(NXOpen::CAE::IFEModel *pFeModel, const std::string &meshColName);

    std::vector<NXOpen::CAE::Mesh*> GetMeshesInCollector(NXOpen::CAE::IFEModel *pFeModel,
        const std::string &meshNamePattern, const std::string &meshColName);

    std::vector<NXOpen::CAE::FENode*> GetNodesOnFace(NXOpen::CAE::CaePart *pCaePrt, const std::vector<NXOpen::CAE::CAEFace*> &pFaces);

    int GetNodeOffset(NXOpen::CAE::FEModelOccurrence *pFeModelOcc);

    std::vector<NXOpen::CAE::FENode*> GetNodeOcc(NXOpen::CAE::FEModelOccurrence *pFeModelOcc,
                                                 int nodeOffset,
                                                 const std::vector<NXOpen::CAE::FENode*> &nodeProtos);

    NXOpen::CAE::Mesh* GetMesh(NXOpen::TaggedObject *pNode);

    NXOpen::CAE::FEModelOccurrence* GetFEModelOccOfNode(NXOpen::CAE::FEModelOccurrence *pParentFEModel, NXOpen::CAE::FENode *pNodeProto);

    std::string GetNXVersion();
}

//#if defined(__MSVC_RUNTIME_CHECKS)
//#undef __MSVC_RUNTIME_CHECKS
//#endif

//#if !defined(_DEBUG)
//#define _DEBUG
//#endif

#endif //VSAR_UTILS_H_INCLUDED
