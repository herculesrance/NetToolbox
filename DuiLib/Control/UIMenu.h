#ifndef __UIMENU_H__
#define __UIMENU_H__

#pragma once

#include "../Utils/observer_impl_base.h"

namespace DuiLib {

	struct ContextMenuParam {
		// 1: remove all
		// 2: remove the sub menu
		WPARAM wParam;
		HWND hWnd;
	};

	struct MenuItemInfo {
		string_t szName;
		bool bChecked;
	};
	struct MenuCmd {
		string_t szName;
		string_t szUserData;
		string_t szText;
		BOOL bChecked;
	};

	enum MenuAlignment {
		eMenuAlignment_Left = 1 << 1,
		eMenuAlignment_Top = 1 << 2,
		eMenuAlignment_Right = 1 << 3,
		eMenuAlignment_Bottom = 1 << 4,
	};


	enum MenuItemDefaultInfo {
		ITEM_DEFAULT_HEIGHT = 30,		//ÿһ��item��Ĭ�ϸ߶ȣ�ֻ����״����ʱ�Զ��壩
		ITEM_DEFAULT_WIDTH = 150,		//���ڵ�Ĭ�Ͽ���

		ITEM_DEFAULT_ICON_WIDTH = 26,	//Ĭ��ͼ����ռ����
		ITEM_DEFAULT_ICON_SIZE = 16,	//Ĭ��ͼ��Ĵ�С

		ITEM_DEFAULT_EXPLAND_ICON_WIDTH = 20,	//Ĭ���¼��˵���չͼ����ռ����
		ITEM_DEFAULT_EXPLAND_ICON_SIZE = 9,		//Ĭ���¼��˵���չͼ��Ĵ�С

		DEFAULT_LINE_LEFT_INSET = ITEM_DEFAULT_ICON_WIDTH + 3,	//Ĭ�Ϸָ��ߵ���߾�
		DEFAULT_LINE_RIGHT_INSET = 7,	//Ĭ�Ϸָ��ߵ��ұ߾�
		DEFAULT_LINE_HEIGHT = 6,		//Ĭ�Ϸָ�����ռ�߶�
		DEFAULT_LINE_COLOR = 0xFFBCBFC4	//Ĭ�Ϸָ�����ɫ

	};

#define WM_MENUCLICK WM_USER + 121  //�������հ�ť��������Ϣ


	///////////////////////////////////////////////
	class MenuMenuReceiverImplBase;
	class MenuMenuObserverImplBase {
	public:
		virtual void AddReceiver (MenuMenuReceiverImplBase* receiver) = 0;
		virtual void RemoveReceiver (MenuMenuReceiverImplBase* receiver) = 0;
		virtual BOOL RBroadcast (ContextMenuParam param) = 0;
	};
	/////////////////////////////////////////////////
	class MenuMenuReceiverImplBase {
	public:
		virtual void AddObserver (MenuMenuObserverImplBase* observer) = 0;
		virtual void RemoveObserver () = 0;
		virtual BOOL Receive (ContextMenuParam param) = 0;
	};
	/////////////////////////////////////////////////

	class MenuReceiverImpl;
	class UILIB_API MenuObserverImpl: public MenuMenuObserverImplBase {
		friend class Iterator;
	public:
		MenuObserverImpl ():
			m_pMainWndPaintManager (nullptr),
			m_pMenuCheckInfo (nullptr) {
			pReceivers_ = new ReceiversVector;
		}

		virtual ~MenuObserverImpl () {
			if (pReceivers_) {
				delete pReceivers_;
				pReceivers_ = nullptr;
			}

		}

		virtual void AddReceiver (MenuMenuReceiverImplBase* receiver) {
			if (!receiver)
				return;

			pReceivers_->push_back (receiver);
			receiver->AddObserver (this);
		}

		virtual void RemoveReceiver (MenuMenuReceiverImplBase* receiver) {
			if (!receiver)
				return;

			ReceiversVector::iterator it = pReceivers_->begin ();
			for (; it != pReceivers_->end (); ++it) {
				if (*it == receiver) {
					pReceivers_->erase (it);
					break;
				}
			}
		}

		virtual BOOL RBroadcast (ContextMenuParam param) {
			ReceiversVector::reverse_iterator it = pReceivers_->rbegin ();
			for (; it != pReceivers_->rend (); ++it) {
				(*it)->Receive (param);
			}

			return BOOL ();
		}


		class Iterator {
			MenuObserverImpl & _tbl;
			DWORD index;
			MenuMenuReceiverImplBase* ptr;
		public:
			Iterator (MenuObserverImpl & table)
				: _tbl (table), index (0), ptr (nullptr) {}

			Iterator (const Iterator & v)
				: _tbl (v._tbl), index (v.index), ptr (v.ptr) {}

			MenuMenuReceiverImplBase* next () {
				if (index >= _tbl.pReceivers_->size ())
					return nullptr;

				for (; index < _tbl.pReceivers_->size (); ) {
					ptr = (*(_tbl.pReceivers_))[index++];
					if (ptr)
						return ptr;
				}
				return nullptr;
			}
		};

		virtual void SetManger (CPaintManagerUI* pManager) {
			if (pManager)
				m_pMainWndPaintManager = pManager;
		}

		virtual CPaintManagerUI* GetManager () const {
			return m_pMainWndPaintManager;
		}

		virtual void SetMenuCheckInfo (CStdStringPtrMap* pInfo) {
			if (pInfo)
				m_pMenuCheckInfo = pInfo;
			else
				m_pMenuCheckInfo = nullptr;
		}

		virtual CStdStringPtrMap* GetMenuCheckInfo () const {
			return m_pMenuCheckInfo;
		}

	protected:
		typedef std::vector<MenuMenuReceiverImplBase*> ReceiversVector;
		ReceiversVector *pReceivers_;
		CPaintManagerUI* m_pMainWndPaintManager;
		CStdStringPtrMap* m_pMenuCheckInfo;
	};

	////////////////////////////////////////////////////
	class UILIB_API MenuReceiverImpl: public MenuMenuReceiverImplBase {
	public:
		MenuReceiverImpl () {
			pObservers_ = new ObserversVector;
		}

		virtual ~MenuReceiverImpl () {
			if (pObservers_) {
				delete pObservers_;
				pObservers_ = nullptr;
			}
		}

		virtual void AddObserver (MenuMenuObserverImplBase* observer) {
			pObservers_->push_back (observer);
		}

		virtual void RemoveObserver () {
			ObserversVector::iterator it = pObservers_->begin ();
			for (; it != pObservers_->end (); ++it) {
				(*it)->RemoveReceiver (this);
			}
		}

		virtual BOOL Receive (ContextMenuParam param) {
			return BOOL ();
		}

	protected:
		typedef std::vector<MenuMenuObserverImplBase*> ObserversVector;
		ObserversVector* pObservers_;
	};

	/////////////////////////////////////////////////////////////////////////////////////
	//

	class CListUI;
	class CMenuWnd;
	class UILIB_API CMenuUI: public CListUI {
		DECLARE_DUICONTROL (CMenuUI)
	public:
		CMenuUI ();
		virtual ~CMenuUI ();
		CMenuWnd	*m_pWindow	= nullptr;
		string_view_t GetClass () const;
		LPVOID GetInterface (string_view_t pstrName);
		UINT GetListType ();

		virtual void DoEvent (TEventUI& event);

		virtual bool Add (CControlUI* pControl);
		virtual bool AddAt (CControlUI* pControl, int iIndex);

		virtual int GetItemIndex (CControlUI* pControl) const;
		virtual bool SetItemIndex (CControlUI* pControl, int iIndex);
		virtual bool Remove (CControlUI* pControl);

		SIZE EstimateSize (SIZE szAvailable);

		void SetAttribute (string_view_t pstrName, string_view_t pstrValue);
	};

	/////////////////////////////////////////////////////////////////////////////////////
	//

	class CMenuElementUI;
	class UILIB_API CMenuWnd: public CWindowWnd, public MenuReceiverImpl, public INotifyUI, public IDialogBuilderCallback {
	public:
		static MenuObserverImpl& GetGlobalContextMenuObserver () {
			static MenuObserverImpl s_context_menu_observer;
			return s_context_menu_observer;
		}
		static CMenuWnd* CreateMenu (CMenuElementUI* pOwner, std::variant<UINT, string_t> xml, POINT point,
			CPaintManagerUI* pMainPaintManager, CStdStringPtrMap* pMenuCheckInfo = nullptr,
			DWORD dwAlignment = eMenuAlignment_Left | eMenuAlignment_Top);
		static void DestroyMenu ();
		static MenuItemInfo* SetMenuItemInfo (string_view_t pstrName, bool bChecked);

	public:
		CMenuWnd ();
		virtual ~CMenuWnd ();
		void Close (UINT nRet = IDOK);
		bool isClosing;
		/*
		 	*@pOwner һ���˵���Ҫָ��������������ǲ˵��ڲ�ʹ�õ�
		 	*@xml	�˵��Ĳ����ļ�
		 	*@point	�˵������Ͻ�����
		 	*@pMainPaintManager	�˵��ĸ����������ָ��
		 	*@pMenuCheckInfo	����˵��ĵ�ѡ�͸�ѡ��Ϣ�ṹָ��
		 	*@dwAlignment		�˵��ĳ���λ�ã�Ĭ�ϳ������������²ࡣ
		 */

		void Init (CMenuElementUI* pOwner, std::variant<UINT, string_t> xml, POINT point,
			CPaintManagerUI* pMainPaintManager, CStdStringPtrMap* pMenuCheckInfo = nullptr,
			DWORD dwAlignment = eMenuAlignment_Left | eMenuAlignment_Top);
		string_view_t GetWindowClassName () const;
		void OnFinalMessage (HWND hWnd);
		void Notify (TNotifyUI& msg);
		CControlUI* CreateControl (string_view_t pstrClassName);

		LRESULT OnCreate (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnKillFocus (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnSize (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT HandleMessage (UINT uMsg, WPARAM wParam, LPARAM lParam);

		BOOL Receive (ContextMenuParam param);

		// ��ȡ���˵��ؼ������ڶ�̬�����Ӳ˵�
		CMenuUI* GetMenuUI ();

		// ���µ����˵��Ĵ�С
		void ResizeMenu ();

		// ���µ����Ӳ˵��Ĵ�С
		void ResizeSubMenu ();
		void setDPI (int DPI);

	public:

		POINT			m_BasedPoint;
		std::variant<UINT, string_t>		m_xml;
		CPaintManagerUI m_pm;
		CMenuElementUI* m_pOwner;
		CMenuUI	*m_pLayout;
		DWORD		m_dwAlignment;	//�˵����뷽ʽ
	};

	class CListContainerElementUI;
	class UILIB_API CMenuElementUI: public CListContainerElementUI {
		DECLARE_DUICONTROL (CMenuElementUI)
		friend CMenuWnd;
	public:
		CMenuElementUI ();
		virtual ~CMenuElementUI ();

		string_view_t GetClass () const;
		LPVOID GetInterface (string_view_t pstrName);
		bool DoPaint (HDC hDC, const RECT& rcPaint, CControlUI* pStopControl);
		void DrawItemText (HDC hDC, const RECT& rcItem);
		SIZE EstimateSize (SIZE szAvailable);

		void DoEvent (TEventUI& event);

		CMenuWnd* GetMenuWnd ();
		void CreateMenuWnd ();

		void SetLineType ();
		void SetLineColor (DWORD color);
		DWORD GetLineColor () const;
		void SetLinePadding (RECT rcInset);
		RECT GetLinePadding () const;
		void SetIcon (string_view_t strIcon);
		void SetIconSize (LONG cx, LONG cy);
		void DrawItemIcon (HDC hDC, const RECT& rcItem);
		void SetChecked (bool bCheck = true);
		bool GetChecked () const;
		void SetCheckItem (bool bCheckItem = false);
		bool GetCheckItem () const;

		void SetShowExplandIcon (bool bShow);
		void DrawItemExpland (HDC hDC, const RECT& rcItem);

		void SetAttribute (string_view_t pstrName, string_view_t pstrValue);

		MenuItemInfo* GetItemInfo (string_view_t pstrName);
		MenuItemInfo* SetItemInfo (string_view_t pstrName, bool bChecked);
	protected:
		CMenuWnd	*m_pWindow			= nullptr;

		bool		m_bDrawLine			= false;	//���ָ���
		DWORD		m_dwLineColor;  //�ָ�����ɫ
		RECT		m_rcLinePadding;	//�ָ��ߵ����ұ߾�

		SIZE		m_szIconSize; 	//��ͼ��
		CDuiString	m_strIcon;
		bool		m_bCheckItem		= false;

		bool		m_bShowExplandIcon	= false;
	};

} // namespace DuiLib

#endif // __UIMENU_H__