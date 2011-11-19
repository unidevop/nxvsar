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

namespace NXOpen
{
    class Expression;

    namespace CAE
    {
        class IFEModel;
        class CAEFace;
    }
}

namespace Vsdane
{
    LIBNXVSDANEEXPORT void CreateSweptMesh(NXOpen::CAE::IFEModel *pFeModel,
        const std::string &meshColName, const std::string &meshName,
        NXOpen::CAE::CAEFace* pSrcFace, NXOpen::CAE::CAEFace *pTargetFace, NXOpen::Expression *pEleSize);
}

#endif //VSDANE_NXVSDANE_HXX_INCLUDED
