//==============================================================================
//  WARNING!!  This file is overwritten by the Block UI Styler while generating
//  the automation code. Any modifications to this file will be lost after
//  generating the code again.
//
//       Filename:  E:\Project\Response\VSAR\NXProject\AppRoot\application\Solve.cpp
//
//        This file was generated by the NX Block UI Styler
//        Created by: wujif
//              Version: NX 7.5
//              Date: 05-23-2011  (Format: mm-dd-yyyy)
//              Time: 20:48 (Format: hh-mm)
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
#include <VsarUI_Solve.hxx>

#include <NXOpen/UI.hxx>
#include <NXOpen/NXMessageBox.hxx>
#include <NXOpen/BlockStyler_UIBlock.hxx>
#include <NXOpen/BlockStyler_PropertyList.hxx>

#include <Vsar_SolveOperation.hxx>

using namespace NXOpen;
using namespace NXOpen::BlockStyler;
using namespace Vsar;

namespace VsarUI
{
    //------------------------------------------------------------------------------
    // Constructor for NX Styler class
    //------------------------------------------------------------------------------
    Solve::Solve() : BaseDialog("Solve.dlx")
    {
    }

    //------------------------------------------------------------------------------
    // Destructor for NX Styler class
    //------------------------------------------------------------------------------
    Solve::~Solve()
    {
    }

    //------------------------------------------------------------------------------
    //Method name: Show_Solve
    //------------------------------------------------------------------------------
    void Solve::ShowDialog()
    {
        boost::scoped_ptr<Solve> pSolveDlg(new Solve());

        try
        {
            // The following method shows the dialog immediately
            pSolveDlg->Show(BlockDialog::DialogModeEdit);
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
    void Solve::InitializeCb()
    {
        CompositeBlock *pTopBlock = m_theDialog->TopBlock();

        try
        {
            //trainSettingsGrp  = pTopBlock->FindBlock("trainSettingsGrp");
            m_trainSpeed        = pTopBlock->FindBlock("trainSpeed");
            m_timeStep          = pTopBlock->FindBlock("timeStep");
            //outputGrp         = pTopBlock->FindBlock("outputGrp");
            m_hasTimeOutput     = pTopBlock->FindBlock("hasTimeOutput");
            m_outputTime        = pTopBlock->FindBlock("outputTime");
            m_hasNodesOutput    = pTopBlock->FindBlock("hasNodesOutput");
            m_outputNode        = pTopBlock->FindBlock("outputNode");
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
    void Solve::DialogShownCb()
    {
        try
        {
            boost::scoped_ptr<PropertyList> pHasTimeOutputPropList(m_hasTimeOutput->GetProperties());
            boost::scoped_ptr<PropertyList> pHasNodesPropList(m_hasNodesOutput->GetProperties());

            pHasTimeOutputPropList->SetLogical("Value", true);
            pHasNodesPropList->SetLogical("Value", true);

            UpdateCb(m_hasTimeOutput);
            UpdateCb(m_hasNodesOutput);
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
    int Solve::ApplyCb()
    {
        int errorCode = 0;
        try
        {
            SolveOperation   solveOper;

            solveOper.Execute();
        }
        catch(std::exception& ex)
        {
            //---- Enter your exception handling code here -----
            errorCode = 1;
            theUI->NXMessageBox()->Show("Block Styler", NXMessageBox::DialogTypeError, ex.what());
        }
        return errorCode;
    }

    //------------------------------------------------------------------------------
    //Callback Name: update_cb
    //------------------------------------------------------------------------------
    int Solve::UpdateCb(UIBlock* block)
    {
        try
        {
            if(block == m_trainSpeed)
            {
                //---------Enter your code here-----------
            }
            else if(block == m_timeStep)
            {
                //---------Enter your code here-----------
            }
            else if(block == m_hasTimeOutput)
            {
                boost::scoped_ptr<PropertyList> pHasTimeOutputPropList(m_hasTimeOutput->GetProperties());
                boost::scoped_ptr<PropertyList> pOutputTimePropList(m_outputTime->GetProperties());

                pOutputTimePropList->SetLogical("Enable", pHasTimeOutputPropList->GetLogical("Value"));
            }
            else if(block == m_outputTime)
            {
                //---------Enter your code here-----------
            }
            else if(block == m_hasNodesOutput)
            {
                boost::scoped_ptr<PropertyList> pHasNodesPropList(m_hasNodesOutput->GetProperties());
                boost::scoped_ptr<PropertyList> pOutputNodesPropList(m_outputNode->GetProperties());

                pOutputNodesPropList->SetLogical("Enable", pHasNodesPropList->GetLogical("Value"));
            }
            else if(block == m_outputNode)
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
