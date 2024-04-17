#pragma once
#include <afxcmn.h>
#include <functional>
#include <map>

class CustomTreeCtrl : public CTreeCtrl
{
public:
	void SetHighlightTokens(const CString& tokens);
	void SetAdditionalTextCallback(std::function<void(CustomTreeCtrl*, HTREEITEM, CString&)> callback);
	void SetEmptyMessage(LPCTSTR lpszEmptyMessage);
	void SetEmptyButton(LPCTSTR lpszButtonCaption);

protected:

	void UpdateHoverItem(HTREEITEM item);
	void FullRowSelect(const CPoint& pt);
	BOOL FullRowDoubleClick(const CPoint& pt, HTREEITEM& item, bool& onItemRight);
	void UpdateButtonVisibility();
	void UpdateButtonPosition();

	virtual bool HasHighlightedChildren(HTREEITEM item);
	virtual bool HasHighlightedText(HTREEITEM item);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

private:
	CButton m_emptyButton;
	CString m_highlightTokens;
	CString m_emptyMessage;
	CString m_emptyButtonCaption;
	HTREEITEM m_hoverItem = nullptr;
	uint64_t m_highlightVer = 0;
	std::map<HTREEITEM, uint64_t> m_childrenHighligh;
	std::function<void(CustomTreeCtrl*, HTREEITEM, CString&)> m_additionalTextCallback;

public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnNMCustomdraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMouseLeave();
	afx_msg void OnButtonClicked();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFont(CFont*, BOOL);
};

