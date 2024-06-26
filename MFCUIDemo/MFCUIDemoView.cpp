
// MFCUIDemoView.cpp : implementation of the CMFCUIDemoView class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "MFCUIDemo.h"
#endif

#include "MFCUIDemoDoc.h"
#include "MFCUIDemoView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMFCUIDemoView

IMPLEMENT_DYNCREATE(CMFCUIDemoView, CView)

BEGIN_MESSAGE_MAP(CMFCUIDemoView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CMFCUIDemoView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CMFCUIDemoView construction/destruction

CMFCUIDemoView::CMFCUIDemoView() noexcept
{
	// TODO: add construction code here

}

CMFCUIDemoView::~CMFCUIDemoView()
{
}

BOOL CMFCUIDemoView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CMFCUIDemoView drawing

void CMFCUIDemoView::OnDraw(CDC* /*pDC*/)
{
	CMFCUIDemoDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CMFCUIDemoView printing


void CMFCUIDemoView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CMFCUIDemoView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CMFCUIDemoView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CMFCUIDemoView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CMFCUIDemoView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CMFCUIDemoView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CMFCUIDemoView diagnostics

#ifdef _DEBUG
void CMFCUIDemoView::AssertValid() const
{
	CView::AssertValid();
}

void CMFCUIDemoView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMFCUIDemoDoc* CMFCUIDemoView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMFCUIDemoDoc)));
	return (CMFCUIDemoDoc*)m_pDocument;
}
#endif //_DEBUG


// CMFCUIDemoView message handlers
