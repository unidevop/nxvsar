// This is the main DLL file.

#pragma managed
#include "NXVsdaneBridge.hxx"
#pragma unmanaged
#include "NXVsdane.hxx"

//#include <NXOpen/Expression.hxx>
//#include <NXOpen/CAE_IFEModel.hxx>
//#include <NXOpen/CAE_CAEFace.hxx>

//using namespace NXOpen;
//using namespace NXOpen::CAE;

namespace Vsdane
{
    //void CreateSweptMesh(IFEModel *pFeModel,
    //    const std::string &meshColName, const std::string &meshName,
    //    CAEFace* pSrcFace, CAEFace *pTargetFace, Expression *pEleSize)
    //{
    //    NXVsdaneBridge::CreateSweptMesh(pFeModel, meshName, meshColName, pSrcFace, pTargetFace, pEleSize);
    //}
    //void CreateSweptMesh(void *pFeModel,
    //    const std::string &meshColName, const std::string &meshName,
    //    void* pSrcFace, void *pTargetFace, void *pEleSize)
    //{
    //    NXVsdaneBridge::CreateSweptMesh(pFeModel, meshName, meshColName, pSrcFace, pTargetFace, pEleSize);
    //}

    void CreateSweptMesh(tag_t tFeModel,
        const std::string &meshColName, const std::string &meshName,
        tag_t tSrcFace, tag_t tTargetFace, tag_t tEleSize)
    {
        NXVsdaneBridge  bridge;

        bridge.CreateSweptMesh(tFeModel, meshName, meshColName, tSrcFace, tTargetFace, tEleSize);
    }
}