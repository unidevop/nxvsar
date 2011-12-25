//==============================================================================
//  WARNING!!  This file is overwritten by the Block UI Styler while generating
//  the automation code. Any modifications to this file will be lost after
//  generating the code again.
//
//       Filename:  E:\Project\CAE\Response\VSAR-DEV\NXProject\AppRoot\application\BridgeSettings.cpp
//
//        This file was generated by the NX Block UI Styler
//        Created by: Joseph
//              Version: NX 7.5
//              Date: 05-08-2011  (Format: mm-dd-yyyy)
//              Time: 14:03 (Format: hh-mm)
//
//==============================================================================

//==============================================================================
//  Purpose:  This TEMPLATE file contains C++ source to guide you in the
//  construction of your Block application dialog. The generation of your
//  dialog file (.dlx extension) is the first step towards dialog construction
//  within NX.  You must now create a NX Open application that
//  utilizes this file (.dlx).
//
//  The information in this file provides you with the following:
//
//  1.  Help on how to load and display your Block UI Styler dialog in NX
//      using APIs provided in NXOpen.BlockStyler namespace
//  2.  The empty callback methods (stubs) associated with your dialog items
//      have also been placed in this file. These empty methods have been
//      created simply to start you along with your coding requirements.
//      The method name, argument list and possible return values have already
//      been provided for you.
//==============================================================================

//------------------------------------------------------------------------------
//These includes are needed for the following template code
//------------------------------------------------------------------------------
#include <uf_defs.h>
#include <VsarUI_BridgeSettings.hxx>

#include <NXOpen/UI.hxx>
#include <NXOpen/NXMessageBox.hxx>

#include <Vsar_Component.hxx>
#include <Vsar_Init_Utils.hxx>

#include <Vsar_Names.hxx>
#include <Vsar_Bridge.hxx>

using namespace NXOpen;
using namespace NXOpen::BlockStyler;
using namespace Vsar;

namespace VsarUI
{
    //static CompAttrInfo attrExpInfo[] =
    //{
    //    {WIDTH_ID_NAME,                 BEAM_PRT_PART_NAME,     WIDTH_EXP_NAME},
    //    {HEIGHT_ID_NAME,                BEAM_PRT_PART_NAME,     HEIGHT_EXP_NAME},
    //    {SPAN_COUNT_ID_NAME,            BRIDGE_PRT_PART_NAME,   SPAN_COUNT_EXP_NAME},

    //    {ELASTIC_MODULUS_ID_NAME,       BRIDGE_FEM_PART_NAME,   BRIDGE_ELASTIC_MODULUS_EXP_NAME},
    //    {UNIT_WEIGHT_ID_NAME,           BRIDGE_FEM_PART_NAME,   BRIDGE_UNIT_WEIGHT_EXP_NAME},
    //    {POISSON_RATIO_ID_NAME,         BRIDGE_FEM_PART_NAME,   BRIDGE_POISSON_RATIO_EXP_NAME}
    //};

    //------------------------------------------------------------------------------
    // Constructor for NX Styler class
    //------------------------------------------------------------------------------
    BridgeSettings::BridgeSettings() : BaseCompDialog("BridgeSettings.dlx",
        new Bridge())
    {
    }

    //------------------------------------------------------------------------------
    // Destructor for NX Styler class
    //------------------------------------------------------------------------------
    BridgeSettings::~BridgeSettings()
    {
    }

    void BridgeSettings::ShowDialog()
    {
        boost::scoped_ptr<BridgeSettings> pRailSettingDlg(new BridgeSettings());

        try
        {
            // The following method shows the dialog immediately
            pRailSettingDlg->Show(BlockDialog::DialogModeEdit);
        }
        catch(std::exception& ex)
        {
            //---- Enter your exception handling code here -----
            theUI->NXMessageBox()->Show("Block Styler", NXMessageBox::DialogTypeError, ex.what());
        }
    }

    //------------------------------------------------------------------------------
    //---------------------Block UI Styler Callback Functions--------------------------
    //------------------------------------------------------------------------------

    //------------------------------------------------------------------------------
    //Callback Name: initialize_cb
    //------------------------------------------------------------------------------
    void BridgeSettings::InitializeCb()
    {
        try
        {
            BaseCompDialog::InitializeCb();

            CompositeBlock *pTopBlock = m_theDialog->TopBlock();

            //grpGeometric = (pTopBlock->FindBlock("grpGeometric"));
            m_width = pTopBlock->FindBlock(WIDTH_ID_NAME);
            m_height = pTopBlock->FindBlock(HEIGHT_ID_NAME);
            m_spanCnt = pTopBlock->FindBlock(SPAN_COUNT_ID_NAME);

            //grpMaterial = pTopBlock->FindBlock("grpMaterial");
            m_elasticModulus = pTopBlock->FindBlock(ELASTIC_MODULUS_ID_NAME);
            m_massDensity = pTopBlock->FindBlock(BRIDGE_MASS_DENSITY_ID_NAME);
            m_poissonRatio = pTopBlock->FindBlock(POISSON_RATIO_ID_NAME);
        }
        catch(std::exception& ex)
        {
            //---- Enter your exception handling code here -----
            theUI->NXMessageBox()->Show("Block Styler", NXMessageBox::DialogTypeError, ex.what());
        }
    }

    //------------------------------------------------------------------------------
    //Callback Name: dialogShown_cb
    //This callback is executed just before the dialog launch. Thus any value set 
    //here will take precedence and dialog will be launched showing that value. 
    //------------------------------------------------------------------------------
    void BridgeSettings::DialogShownCb()
    {
        try
        {
            //---- Enter your callback code here -----
        }
        catch(std::exception& ex)
        {
            //---- Enter your exception handling code here -----
            theUI->NXMessageBox()->Show("Block Styler", NXMessageBox::DialogTypeError, ex.what());
        }
    }

    //------------------------------------------------------------------------------
    //Callback Name: apply_cb
    //------------------------------------------------------------------------------
    //int BridgeSettings::ApplyCb()
    //{
    //    int errorCode = 0;
    //    try
    //    {
    //        //---- Enter your callback code here -----
    //    }
    //    catch(std::exception& ex)
    //    {
    //        //---- Enter your exception handling code here -----
    //        errorCode = 1;
    //        theUI->NXMessageBox()->Show("Block Styler", NXMessageBox::DialogTypeError, ex.what());
    //    }
    //    return errorCode;
    //}

    //------------------------------------------------------------------------------
    //Callback Name: update_cb
    //------------------------------------------------------------------------------
    int BridgeSettings::UpdateCb(UIBlock* block)
    {
        try
        {
            if(block == m_width)
            {
                //---------Enter your code here-----------
            }
            else if(block == m_height)
            {
                //---------Enter your code here-----------
            }
            else if(block == m_spanCnt)
            {
                //---------Enter your code here-----------
            }
            else if(block == m_elasticModulus)
            {
                //---------Enter your code here-----------
            }
            else if(block == m_massDensity)
            {
                //---------Enter your code here-----------
            }
            else if(block == m_poissonRatio)
            {
                //---------Enter your code here-----------
            }
        }
        catch(std::exception& ex)
        {
            //---- Enter your exception handling code here -----
            theUI->NXMessageBox()->Show("Block Styler", NXMessageBox::DialogTypeError, ex.what());
        }
        return 0;
    }
}
