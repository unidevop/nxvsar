// NXDotNetWrap.h

#ifndef VSDANE_NXVSDANEBRIDGE_HXX_INCLUDED
#define VSDANE_NXVSDANEBRIDGE_HXX_INCLUDED

#pragma once
#pragma unmanaged


//#using <NXOpen.dll>
//using namespace System;
#include <string>
//#include <uf_defs.h>

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

#pragma managed
#include <vcclr.h>
#include "NXVsdaneDotNet.hxx"

typedef unsigned int tag_t;

namespace Vsdane
{
    class NXVsdaneBridge
    {
    public:
        NXVsdaneBridge();

        //static void CreateSweptMesh(void *pFeModel,
        //    const std::string &meshColName, const std::string &meshName,
        //    void *pSrcFace, void *pTargetFace, void *pEleSize);

        void CreateSweptMesh(tag_t tFeModel,
            const std::string &meshColName, const std::string &meshName,
            tag_t tSrcFace, tag_t tTargetFace, tag_t tpEleSize);

        static System::String^ UnmanagedString2ManagedString(const char* pIn);

        //generic<typename T>
        //static T^ UnmanagedPtr2ManagedType(void *);
        static NXOpen::CAE::IFEModel^ UnmanagedPtr2ManagedIFEModel(void *);
        static NXOpen::CAE::CAEFace^ UnmanagedPtr2ManagedCAEFace(void *);
        static NXOpen::Expression^ UnmanagedPtr2ManagedExpression(void *);

    private:
        gcroot<NXVsdane^> m_impl;
    };
}

#endif //VSDANE_NXVSDANEBRIDGE_HXX_INCLUDED
