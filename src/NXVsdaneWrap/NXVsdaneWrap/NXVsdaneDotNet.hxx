// NXDotNetWrap.h

#ifndef VSDANE_NXVSDANEDOTNET_HXX_INCLUDED
#define VSDANE_NXVSDANEDOTNET_HXX_INCLUDED

#pragma once
#pragma managed

//#using <NXOpen.dll>
using namespace System;

namespace Vsdane
{
    public ref class NXVsdane
    {
    public:
        static void CreateSweptMeshDotNet(NXOpen::CAE::IFEModel ^pFeModel,
            String^ meshColName, String ^meshName,
            NXOpen::CAE::CAEFace ^pSrcFace, NXOpen::CAE::CAEFace ^pTargetFace, NXOpen::Expression ^pEleSize);
    };
}

#endif //VSDANE_NXVSDANEDOTNET_HXX_INCLUDED
