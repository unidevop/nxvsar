
#include <uf_defs.h>
#include <VsarUI_BaseCompDialog.hxx>

#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>

//#include <NXOpen/Session.hxx>
#include <NXOpen/UI.hxx>
#include <NXOpen/NXMessageBox.hxx>
//#include <NXOpen/Callback.hxx>
#include <NXOpen/NXException.hxx>
#include <NXOpen/Expression.hxx>
#include <NXOpen/BlockStyler_PropertyList.hxx>

#include <Vsar_Component.hxx>
#include <Vsar_Utils.hxx>

using namespace boost;
using namespace NXOpen;
using namespace NXOpen::BlockStyler;
using namespace Vsar;

//------------------------------------------------------------------------------
// Declaration of global variables
//------------------------------------------------------------------------------
namespace VsarUI
{
    //------------------------------------------------------------------------------
    // Constructor for NX Styler class
    //------------------------------------------------------------------------------
    BaseCompDialog::BaseCompDialog(const std::string &dialogName, Vsar::BaseComponent *pComp)
        : BaseDialog(dialogName), m_pComp(pComp)
    {
        try
        {
        }
        catch(std::exception&)
        {
            //---- Enter your exception handling code here -----
            throw;
        }
    }

    //------------------------------------------------------------------------------
    // Destructor for NX Styler class
    //------------------------------------------------------------------------------
    BaseCompDialog::~BaseCompDialog()
    {
    }

    //------------------------------------------------------------------------------
    //---------------------Block UI Styler Callback Functions--------------------------
    //------------------------------------------------------------------------------
    void BaseCompDialog::InitializeCb()
    {
        try
        {
            m_pComp->LoadGeometryPart();
            HandleExpressions(boost::bind(&BaseCompDialog::InitBlock, this, _1, _2));
            m_pComp->OnInit();
        }
        catch(NXException&)
        {
            //---- Enter your exception handling code here -----
            s_theUI->NXMessageBox()->Show("Initilize", NXMessageBox::DialogTypeError,
                "The model is corrupted. Maybe the part has been modified by hand. \n"
                "Please recreate a new project to continue.");
            throw;
        }
    }

    //------------------------------------------------------------------------------
    //Callback Name: dialogShown_cb
    //This callback is executed just before the dialog launch. Thus any value set 
    //here will take precedence and dialog will be launched showing that value. 
    //------------------------------------------------------------------------------
    //void BaseCompDialog::DialogShownCb()
    //{
    //    try
    //    {

    //    }
    //    catch(std::exception& ex)
    //    {
    //        //---- Enter your exception handling code here -----
    //        theUI->NXMessageBox()->Show("Block Styler", NXMessageBox::DialogTypeError, ex.what());
    //    }
    //}

    //------------------------------------------------------------------------------
    //Callback Name: apply_cb
    //------------------------------------------------------------------------------
    int BaseCompDialog::ApplyCb()
    {
        int errorCode = 0;

        Session::UndoMarkId  undoMark;

        try
        {
            if (m_pComp)
            {
                undoMark = s_theSession->SetUndoMark(Session::MarkVisibilityVisible, "Setting Vsar Component");

                HandleExpressions(boost::bind(&BaseCompDialog::WriteExpression, this, _1, _2));

                m_pComp->UpdateModel();
            }
        }
        catch(std::exception&)
        {
            //---- Enter your exception handling code here -----
            errorCode = 1;
            s_theUI->NXMessageBox()->Show("Update Model", NXMessageBox::DialogTypeError,
                "Failed to update model. The input parameter is incorrect or the model is corrupted.\n"
                " Maybe the part has been modified by hand. \n");

            s_theSession->UndoToLastVisibleMark();
        }
        return errorCode;
    }

    template <typename Handler>
    void BaseCompDialog::HandleExpressions(Handler hander)
    {
        if (m_pComp)
        {
            const StlCompAttrInfoVector           &attrInfos(m_pComp->GetAttrInfo());
            StlCompAttrInfoVector::const_iterator  iter;
            Expression                            *pExpression = NULL;
            CompositeBlock                        *pTopBlock   = m_theDialog->TopBlock();
            UIBlock                               *pBlock      = NULL;

            for (iter = attrInfos.begin(); iter != attrInfos.end(); ++iter)
            {
                pExpression = m_pComp->GetExpression(iter->m_partName, iter->m_expName);

                pBlock      = pTopBlock->FindBlock(iter->m_attrName.c_str());

                //(this->*hander)(pBlock, pExpression);
                hander(pBlock, pExpression);
            }
        }
    }

    void BaseCompDialog::InitBlock( UIBlock * pBlock, Expression * pExpression )
    {
        if (pBlock)
        {
            boost::scoped_ptr<PropertyList>  pPropList(pBlock->GetProperties());
            std::string                      blockTypeName(pBlock->Type().GetUTF8Text());

            if (blockTypeName == "Expression")
                pPropList->SetTaggedObject("ExpressionObject", pExpression);
            else if (blockTypeName == "Enumeration")
            {
                pPropList->SetEnumAsString("Value",
                    boost::lexical_cast<std::string>(pExpression->IntegerValue()).c_str());
            }
            else if (blockTypeName == "Integer")
                pPropList->SetInteger("Value", pExpression->IntegerValue());
        }
    }

    void BaseCompDialog::WriteExpression( UIBlock * pBlock, Expression * pExpression )
    {
        if (pBlock)
        {
            boost::scoped_ptr<PropertyList>  pPropList(pBlock->GetProperties());
            std::string                      blockTypeName(pBlock->Type().GetUTF8Text());

            if (blockTypeName == "Enumeration")
            {
                pExpression->SetRightHandSide(pPropList->GetEnumAsString("Value").GetUTF8Text());
            }
            else if (blockTypeName == "Integer")
            {
                int  ctrlValue = pPropList->GetInteger("Value");

                pExpression->SetValue(ctrlValue);
            }
        }
    }

    //int BaseCompDialog::CancelCb()
    //{
    //    int errorCode = 0;
    //    try
    //    {
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
    //int BaseCompDialog::UpdateCb(UIBlock* block)
    //{
    //    try
    //    {

    //    }
    //    catch(std::exception& ex)
    //    {
    //        //---- Enter your exception handling code here -----
    //        theUI->NXMessageBox()->Show("Block Styler", NXMessageBox::DialogTypeError, ex.what());
    //    }
    //    return 0;
    //}

}
