// This is the main DLL file.

#pragma managed
#include "NXVsdaneDotNet.hxx"
#pragma unmanaged
#include "NXVsdane.hxx"

using namespace NXOpen;
using namespace NXOpen::CAE;

namespace Vsdane
{
    extern void CreateSweptMesh(IFEModel *pFeModel,
        const std::string &meshColName, const std::string &meshName,
        CAEFace* pSrcFace, CAEFace *pTargetFace, Expression *pEleSize)
    {
        NXVsdane::CreateSweptMeshDotNet(pFeModel, meshName, meshColName, pSrcFace, pTargetFace, pEleSize);
    }
}