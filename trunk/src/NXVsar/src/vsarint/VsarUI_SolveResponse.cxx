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
#include <VsarUI_SolveResponse.hxx>

#include <NXOpen/UI.hxx>
#include <NXOpen/NXMessageBox.hxx>
#include <NXOpen/BlockStyler_UIBlock.hxx>
#include <NXOpen/BlockStyler_PropertyList.hxx>

#include <Vsar_SolveOperation.hxx>
#include <Vsar_Names.hxx>
#include <Vsar_Solution.hxx>

using namespace NXOpen;
using namespace NXOpen::BlockStyler;
using namespace Vsar;

namespace VsarUI
{
    //------------------------------------------------------------------------------
    // Constructor for NX Styler class
    //------------------------------------------------------------------------------
    SolveResponse::SolveResponse() : BaseCompDialog("SolveResponse.dlx", new ResponseSolution())
    {
    }

    //------------------------------------------------------------------------------
    // Destructor for NX Styler class
    //------------------------------------------------------------------------------
    SolveResponse::~SolveResponse()
    {
    }

    //------------------------------------------------------------------------------
    //Method name: Show_Solve
    //------------------------------------------------------------------------------
    void SolveResponse::ShowDialog()
    {
        boost::scoped_ptr<SolveResponse> pSolveDlg(new SolveResponse());

        try
        {
            // The following method shows the dialog immediately
            pSolveDlg->Show(BlockDialog::DialogModeEdit);
        }
        catch(std::exception& ex)
        {
            //---- Enter your exception handling code here -----
            s_theUI->NXMessageBox()->Show("Block Styler", NXMessageBox::DialogTypeError, ex.what());
        }
    }

    //------------------------------------------------------------------------------
    //---------------------Block UI Styler Callback Functions--------------------------
    //------------------------------------------------------------------------------

    //------------------------------------------------------------------------------
    //Callback Name: initialize_cb
    //------------------------------------------------------------------------------
    void SolveResponse::InitializeCb()
    {
        BaseCompDialog::InitializeCb();

        try
        {
            CompositeBlock *pTopBlock = m_theDialog->TopBlock();

            //trainSettingsGrp  = pTopBlock->FindBlock("trainSettingsGrp");
            m_trainSpeed        = pTopBlock->FindBlock(TRAIN_SPEED_ID_NAME);
            m_timeStep          = pTopBlock->FindBlock(COMPUTE_TIME_STEP_ID_NAME);
            //outputGrp         = pTopBlock->FindBlock("outputGrp");
            //m_hasTimeOutput     = pTopBlock->FindBlock("hasTimeOutput");
            //m_outputTime        = pTopBlock->FindBlock("outputTime");

            m_selOutputType      = pTopBlock->FindBlock("selOutputType");
            m_hasElemsOutput     = pTopBlock->FindBlock("hasElemsOutput");
            m_outputElements     = pTopBlock->FindBlock("outputElements");
            m_hasNodesOutput     = pTopBlock->FindBlock("hasNodesOutput");
            m_outputNodes         = pTopBlock->FindBlock("outputNode");

            m_hasNoiseNodeOutput = pTopBlock->FindBlock("hasNoiseNodeOutput");

            m_loadBtn            = pTopBlock->FindBlock("loadBtn");
        }
        catch(std::exception& ex)
        {
            //---- Enter your exception handling code here -----
            s_theUI->NXMessageBox()->Show("Block Styler", NXMessageBox::DialogTypeError, ex.what());
        }
    }

    //------------------------------------------------------------------------------
    //Callback Name: dialogShown_cb
    //This callback is executed just before the dialog launch. Thus any value set 
    //here will take precedence and dialog will be launched showing that value. 
    //------------------------------------------------------------------------------
    void SolveResponse::DialogShownCb()
    {
        try
        {
            //boost::scoped_ptr<PropertyList> pHasTimeOutputPropList(m_selOutputType->GetProperties());
            //boost::scoped_ptr<PropertyList> pHasNodesPropList(m_outputElements->GetProperties());
            //boost::scoped_ptr<PropertyList> pHasNodesPropList(m_outputNode->GetProperties());

            //pHasTimeOutputPropList->SetLogical("Value", true);
            //pHasNodesPropList->SetLogical("Value", true);

            UpdateCb(m_hasElemsOutput);
            UpdateCb(m_hasNodesOutput);
        }
        catch(std::exception& ex)
        {
            //---- Enter your exception handling code here -----
            s_theUI->NXMessageBox()->Show("Block Styler", NXMessageBox::DialogTypeError, ex.what());
        }
    }

    //------------------------------------------------------------------------------
    //Callback Name: apply_cb
    //------------------------------------------------------------------------------
    int SolveResponse::ApplyCb()
    {
        int errorCode = 0;
        try
        {
            SolveSettings solveSettings(CanOutputElements(), GetOutputElements(),
                CanOutputNodes(), GetOutputNodes(), CanOutputNodesForNoise());

            solveSettings.Apply();

            SolveResponseOperation   solveOper;

            solveOper.Execute();
        }
        catch(std::exception& ex)
        {
            //---- Enter your exception handling code here -----
            errorCode = 1;
            s_theUI->NXMessageBox()->Show("Block Styler", NXMessageBox::DialogTypeError, ex.what());
        }
        return errorCode;
    }

    //------------------------------------------------------------------------------
    //Callback Name: update_cb
    //------------------------------------------------------------------------------
    int SolveResponse::UpdateCb(UIBlock* block)
    {
        try
        {
            if(block == m_selOutputType)
            {
                boost::scoped_ptr<PropertyList> pselOutputTypePropList(m_selOutputType->GetProperties());
                boost::scoped_ptr<PropertyList> pOutputElemsPropList(m_outputElements->GetProperties());
                boost::scoped_ptr<PropertyList> pOutputNodesPropList(m_outputNodes->GetProperties());

                SelectionOutputType selOutputType = static_cast<SelectionOutputType>(pselOutputTypePropList->GetEnum("Value"));

                pOutputElemsPropList->SetLogical("Show", selOutputType == Selection_Output_Elements);
                if (selOutputType == Selection_Output_Elements)
                    m_outputElements->Focus();

                pOutputNodesPropList->SetLogical("Show", selOutputType == Selection_Output_Nodes);
                if (selOutputType == Selection_Output_Nodes)
                    m_outputNodes->Focus();
            }
            else if(block == m_hasElemsOutput)
            {
                boost::scoped_ptr<PropertyList> pOutputElemsPropList(m_outputElements->GetProperties());

                pOutputElemsPropList->SetLogical("Enable", CanOutputElements());
            }
            else if(block == m_hasNodesOutput)
            {
                boost::scoped_ptr<PropertyList> pOutputNodesPropList(m_outputNodes->GetProperties());

                pOutputNodesPropList->SetLogical("Enable", CanOutputNodes());
            }
            else if(block == m_loadBtn)
            {
                SolveResponseOperation   solveOper;

                solveOper.LoadResult();
            }
        }
        catch(std::exception& ex)
        {
            //---- Enter your exception handling code here -----
            s_theUI->NXMessageBox()->Show("Block Styler", NXMessageBox::DialogTypeError, ex.what());
        }
        return 0;
    }

    bool SolveResponse::CanOutputElements() const
    {
        boost::scoped_ptr<PropertyList> pHasElemsPropList(m_hasElemsOutput->GetProperties());

        return pHasElemsPropList->GetLogical("Value");
    }

    std::vector<TaggedObject*> SolveResponse::GetOutputElements() const
    {
        boost::scoped_ptr<PropertyList> pOutputElemsPropList(m_outputElements->GetProperties());

        return pOutputElemsPropList->GetTaggedObjectVector("SelectedObjects");
    }

    bool SolveResponse::CanOutputNodes() const
    {
        boost::scoped_ptr<PropertyList> pHasNodesPropList(m_hasNodesOutput->GetProperties());

        return pHasNodesPropList->GetLogical("Value");
    }

    std::vector<TaggedObject*> SolveResponse::GetOutputNodes() const
    {
        boost::scoped_ptr<PropertyList> pOutputNodesPropList(m_outputNodes->GetProperties());

        return pOutputNodesPropList->GetTaggedObjectVector("SelectedObjects");
    }

    bool SolveResponse::CanOutputNodesForNoise() const
    {
        boost::scoped_ptr<PropertyList> pHasNoiseNodesPropList(m_hasNoiseNodeOutput->GetProperties());

        return pHasNoiseNodesPropList->GetLogical("Value");
    }
}
