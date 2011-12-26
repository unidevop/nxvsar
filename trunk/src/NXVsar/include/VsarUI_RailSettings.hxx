//==============================================================================
//  WARNING!!  This file is overwritten by the Block Styler while generating
//  the automation code. Any modifications to this file will be lost after
//  generating the code again.
//
//       Filename:  E:\Project\Response\VSAR\NXProject\AppRoot\application\RailSettings.hpp
//
//        This file was generated by the NX Block Styler
//        Created by: wujif
//              Version: NX 7.5
//              Date: 04-07-2011  (Format: mm-dd-yyyy)
//              Time: 22:30
//
//==============================================================================

#ifndef VSARUI_RAILSETTINGS_H_INCLUDED
#define VSARUI_RAILSETTINGS_H_INCLUDED

//------------------------------------------------------------------------------
//These includes are needed for the following template code
//------------------------------------------------------------------------------
#include <VsarUI_BaseCompDialog.hxx>

//------------------------------------------------------------------------------
// Forward declaration for Class
//------------------------------------------------------------------------------

namespace VsarUI
{
    class RailSettings : public BaseCompDialog
    {
        // class members
    public:
        RailSettings();
        ~RailSettings();

        static void ShowDialog();

        //----------------------- BlockStyler Callback Prototypes ---------------------
        // The following member function prototypes define the callbacks 
        // specified in your BlockStyler dialog.  The empty implementaiton
        // of these prototypes is provided in the RailSettings.cpp file. 
        // You are REQUIRED to write the implementation for these funtions.
        //------------------------------------------------------------------------------
        virtual void InitializeCb();
        virtual void DialogShownCb();
        //virtual int ApplyCb();
        virtual int UpdateCb(NXOpen::BlockStyler::UIBlock* block);

    private:

        //NXOpen::BlockStyler::UIBlock* m_grpGeometry;// Block type: Group
        NXOpen::BlockStyler::UIBlock* m_length;// Block type: Expression
        NXOpen::BlockStyler::UIBlock* m_width;// Block type: Expression
        NXOpen::BlockStyler::UIBlock* m_eleSize;// Block type: Expression

        //NXOpen::BlockStyler::UIBlock* m_grpMaterial;// Block type: Group
        NXOpen::BlockStyler::UIBlock* m_density;// Block type: Expression
        NXOpen::BlockStyler::UIBlock* m_elasticModulus;// Block type: Expression
        NXOpen::BlockStyler::UIBlock* m_poissonRatio;// Block type: Expression

    };
}
#endif //VSARUI_RAILSETTINGS_H_INCLUDED