
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
        class MeshManager;
        class MeshCollector;
        class CAEBody;
        class CAEFace;
    }
}

namespace Vsar
{
    //------------------------------------------------------------------------------
    // Declaration of global variables
    //------------------------------------------------------------------------------
    //extern NewProject *theVsar_NewProject;
    int ReportError( char *file, int line, char *call, int irc);

#ifndef UF_CALL
#define UF_CALL(X) (ReportError( __FILE__, __LINE__, #X, (X)))
#endif

    void LoadRootPart();

    std::vector<NXOpen::Body*> GetBodyByName(NXOpen::Part *pBodyPrt, const std::string &bodyName);

    std::vector<NXOpen::TaggedObject*> GetPointByAttrName( NXOpen::BasePart * pPtPrt, const std::string &ptName );

    std::vector<NXOpen::TaggedObject*> GetPointByLayer( NXOpen::BasePart * pPtPrt, int layer );

    std::vector<NXOpen::Assemblies::Component*> GetOccInCompTree(NXOpen::Assemblies::Component *pAssemTree, NXOpen::Part *pPrt);

    std::vector<NXOpen::CAE::CAEBody*> GetCaeBodies(const std::vector<NXOpen::Body*> &bodies);

    std::vector<NXOpen::CAE::CAEFace*> GetCaeFaceByName(const NXOpen::CAE::CAEBody *pCaeBody, const std::string &faceName);

    void EditSweptMeshData(NXOpen::CAE::IFEModel *pFeModel, const std::string &meshName,
                        const std::vector<NXOpen::Body*> &srcBodies);

    void CreateSweptMesh(NXOpen::CAE::MeshManager *pMeshMgr, NXOpen::CAE::MeshCollector *pMeshCol,
                         const std::string &meshName, NXOpen::CAE::CAEFace* pSrcFace, NXOpen::CAE::CAEFace *pTargetFace,
                         const std::string &eleSizeExpName);

    void CreateSweptMesh_sf(NXOpen::CAE::IFEModel *pFeModel, NXOpen::CAE::CAEBody *pSolidBody,
                         const std::string &meshColName, const std::string &meshName,
                         NXOpen::CAE::CAEFace* pSrcFace, NXOpen::Expression *pEleSize);
}

//#if defined(__MSVC_RUNTIME_CHECKS)
//#undef __MSVC_RUNTIME_CHECKS
//#endif

//#if !defined(_DEBUG)
//#define _DEBUG
//#endif

#endif //VSAR_UTILS_H_INCLUDED
