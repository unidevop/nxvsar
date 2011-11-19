// NXDotNetWrap.h
#ifndef VSDANE_NXVSDANE_HXX_INCLUDED
#define VSDANE_NXVSDANE_HXX_INCLUDED

#pragma once
#pragma unmanaged

#ifdef LIBNXVSDANE
#define LIBNXVSDANEEXPORT __declspec(dllexport)
#else
#define LIBNXVSDANEEXPORT __declspec(dllimport)
#endif


#include <string>
#include <uf_defs.h>

//namespace NXOpen
//{
//    class Expression;
//
//    namespace CAE
//    {
//        class IFEModel;
//        class CAEFace;
//    }
//}

namespace Vsdane
{
    //LIBNXVSDANEEXPORT void CreateSweptMesh(NXOpen::CAE::IFEModel *pFeModel,
    //    const std::string &meshColName, const std::string &meshName,
    //    NXOpen::CAE::CAEFace* pSrcFace, NXOpen::CAE::CAEFace *pTargetFace, NXOpen::Expression *pEleSize);
    //LIBNXVSDANEEXPORT void CreateSweptMesh(void *pFeModel,
    //    const std::string &meshColName, const std::string &meshName,
    //    void* pSrcFace, void *pTargetFace, void *pEleSize);

    LIBNXVSDANEEXPORT void CreateSweptMesh(tag_t tFeModel,
        const std::string &meshColName, const std::string &meshName,
        tag_t tSrcFace, tag_t tTargetFace, tag_t tpEleSize);
}

#endif //VSDANE_NXVSDANE_HXX_INCLUDED
