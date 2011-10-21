
#ifndef VSARUI_BASECOMPDIALOG_H_INCLUDED
#define VSARUI_BASECOMPDIALOG_H_INCLUDED

#include <VsarUI_BaseDialog.hxx>

namespace NXOpen
{
    class Expression;
}

namespace Vsar
{
    class BaseComponent;
}

namespace VsarUI
{
    class BaseCompDialog : public BaseDialog
    {
        // class members
    public:

        //----------------------- BlockStyler Callback Prototypes ---------------------
        // The following member function prototypes define the callbacks 
        // specified in your BlockStyler dialog.  The empty implementaiton
        // of these prototypes is provided in the Vsar_BaseCompDialog.cpp file. 
        // You are REQUIRED to write the implementation for these funtions.
        //------------------------------------------------------------------------------
        virtual void InitializeCb();
        //virtual void DialogShownCb();
        virtual int ApplyCb();
        //virtual int CancelCb();
        //virtual int UpdateCb(NXOpen::BlockStyler::UIBlock* pBlock);

    protected:
        BaseCompDialog(const std::string &dialogName, Vsar::BaseComponent *pComp);
        virtual ~BaseCompDialog() = 0;

        template <typename Handler>
        void HandleExpressions(Handler hander);

        void InitBlock( NXOpen::BlockStyler::UIBlock *pBlock, NXOpen::Expression *pExpression );

        void WriteExpression( NXOpen::BlockStyler::UIBlock *pBlock, NXOpen::Expression *pExpression );

    protected:
        boost::scoped_ptr<Vsar::BaseComponent>    m_pComp;
    };
}

#endif //VSARUI_BASECOMPDIALOG_H_INCLUDED
