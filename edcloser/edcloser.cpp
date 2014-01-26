#include <string>
#include <windows.h>

#ifdef UNICODE
namespace std { typedef wstring tstring; }
#else
namespace std { typedef string tstring; }
#endif

using namespace std;

enum {
    IDM_POPUP_CLOSE_ED = 101,
    IDM_POPUP_RUN = 102,
    IDM_POPUP_EXIT = 111,
    IDM_POPUP_CANCEL = 121,
    AREA_MARGIN = 1,
    AREA_WIDTH = 24,
    AREA_HEIGHT = 24,
    WINDOW_WIDTH = AREA_WIDTH + AREA_MARGIN * 2,
    WINDOW_HEIGHT = AREA_HEIGHT + AREA_MARGIN * 2
};

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
static LRESULT CALLBACK editAreaProc(HWND hWnd, UINT msg, WPARAM wParam,
    LPARAM lParam);
static BOOL WINAPI findStringDlgProc(HWND hDlg, UINT msg, WPARAM wParam,
    LPARAM lParam);
static void onCreate(HWND hWnd);
static void onDestroy(HWND hWnd);
static void onClose(HWND hWnd);
static void onLButtonUp(HWND hWnd, int x, int y);
static void onActivate(HWND hWnd, int active);
static void onEraseBkgnd(HWND hWnd, HDC hDC);
static void onCloseEd(HWND hWnd);
static void onRun(HWND hWnd);
static void changeWindowLayout(HWND hWnd);

HINSTANCE g_hInstance = NULL;
HFONT g_hMainFont = NULL;

struct EdCloserData {
    HBITMAP hBitmap;
    HMENU hPopupMenu;
};

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam,
    LPARAM lParam) {

	switch (msg) {
    case WM_CREATE:
        onCreate(hWnd);
        return 0;
    case WM_DESTROY:
        onDestroy(hWnd);
		return 0;
    case WM_CLOSE:
        onClose(hWnd);
        return 0;
    case WM_LBUTTONUP:
        onLButtonUp(hWnd, LOWORD(lParam), HIWORD(lParam));
        return 0;
    case WM_COMMAND:
    {
        int id    = LOWORD(wParam); 
        int event = HIWORD(wParam);

        switch (id) {
        case IDM_POPUP_CLOSE_ED:
            onCloseEd(hWnd);
            break;
        case IDM_POPUP_RUN:
            onRun(hWnd);
            break;
        case IDM_POPUP_EXIT:
            SendMessage(hWnd, WM_CLOSE, 0, 0);
            break;
        case IDM_POPUP_CANCEL:
            break;
        }

        return 0;
    }
    case WM_ACTIVATE:
        onActivate(hWnd, wParam & 0xFFFF);
        return 0;
    case WM_ERASEBKGND:
        onEraseBkgnd(hWnd, (HDC)wParam);
        return 1;
	}

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

static void onCreate(HWND hWnd) {
    EdCloserData *data = new EdCloserData();
    SetWindowLong(hWnd, GWL_USERDATA, (long)data);

    data->hBitmap = (HBITMAP)LoadImage(g_hInstance, _T("MAIN"), IMAGE_BITMAP,
        0, 0, 0);

    HMENU hMenu = LoadMenu(g_hInstance, _T("MENU"));
    data->hPopupMenu = GetSubMenu(hMenu, 0);

    changeWindowLayout(hWnd);
}

static void onDestroy(HWND hWnd) {
    EdCloserData *data = (EdCloserData *)GetWindowLong(hWnd, GWL_USERDATA);

    PostQuitMessage(0);

    delete data;
}

static void onClose(HWND hWnd) {
    if (MessageBox(hWnd, _T("Do you really want to exit?"), _T("Confirm"),
        MB_ICONEXCLAMATION | MB_YESNO | MB_DEFBUTTON2 | MB_SETFOREGROUND) ==
        IDNO) {

        return;
    }

    DestroyWindow(hWnd);
}

static void onLButtonUp(HWND hWnd, int x, int y) {
    EdCloserData *data = (EdCloserData *)GetWindowLong(hWnd, GWL_USERDATA);

    MENUITEMINFO item = {0};
    item.cbSize = sizeof(MENUITEMINFO);
    item.fMask = MIIM_TYPE;
    item.fType = MFT_STRING;

    HWND hEdWindow = FindWindow(_T("SHARP SIM"), NULL);
    if (hEdWindow == NULL)
        item.dwTypeData = _T("Open Dictionary App");
    else
        item.dwTypeData = _T("Close Dictionary App");

    SetMenuItemInfo(data->hPopupMenu, IDM_POPUP_CLOSE_ED, false, &item);

    POINT pt = {x, y};
    ClientToScreen(hWnd, &pt);

    TrackPopupMenu(data->hPopupMenu, TPM_LEFTALIGN | TPM_BOTTOMALIGN,
		pt.x, pt.y, 0, hWnd, NULL);
}

static void onActivate(HWND hWnd, int active) {
    ShowWindow(hWnd, SW_HIDE);
    ShowWindow(hWnd, SW_SHOWNA);
}

static void onEraseBkgnd(HWND hWnd, HDC hDC) {
    EdCloserData *data = (EdCloserData *)GetWindowLong(hWnd, GWL_USERDATA);

    RECT clientRect;
    GetClientRect(hWnd, &clientRect);

    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(77, 109, 243));
    HPEN hPrevPen = (HPEN)SelectObject(hDC, hPen);

    MoveToEx(hDC, clientRect.right - 1, 0, NULL);
    LineTo(hDC, 0, 0);
    LineTo(hDC, 0, clientRect.bottom - 1);
    LineTo(hDC, clientRect.right - 1, clientRect.bottom - 1);
    LineTo(hDC, clientRect.right - 1, 0);

    SelectObject(hDC, hPrevPen);
    DeleteObject(hPen);

    HDC hOffscrDC = CreateCompatibleDC(hDC);
    HANDLE hPrevBmp = SelectObject(hOffscrDC, data->hBitmap);

    BitBlt(hDC, AREA_MARGIN, AREA_MARGIN, AREA_WIDTH, AREA_HEIGHT, hOffscrDC,
        0, 0, SRCCOPY);

    SelectObject(hOffscrDC, hPrevBmp);

    DeleteObject(hOffscrDC);
}

static void onCloseEd(HWND hWnd) {
    HWND hOpenerWindow = FindWindow(_T("ceOpener"), NULL);
    if (hOpenerWindow != NULL) {
        MessageBox(hWnd, _T("ceOpener is running."), _T("Warning"),
            MB_ICONEXCLAMATION | MB_SETFOREGROUND);

        return;
    }

    HWND hEdWindow = FindWindow(_T("SHARP SIM"), NULL);
    if (hEdWindow == NULL) {
        CreateProcess(_T("\\Windows\\wceprj.exe"), _T("1"), NULL, NULL,
            false, 0, NULL, NULL, NULL, NULL);
    }
    else
        PostMessage(hEdWindow, WM_CLOSE, NULL, NULL);
}

static void onRun(HWND hWnd) {
    HWND hTaskbar = FindWindow(_T("HHTaskBar"), NULL);
    if (hTaskbar != NULL)
        EnableWindow(hTaskbar, true);

    keybd_event(VK_LWIN, 0, 0, NULL);
    keybd_event('R', 0, 0, NULL);
    keybd_event('R', 0, KEYEVENTF_KEYUP, NULL);
    keybd_event(VK_LWIN, 0, KEYEVENTF_KEYUP, NULL);
}

static void changeWindowLayout(HWND hWnd) {
    RECT workAreaRect;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &workAreaRect, 0);

    MoveWindow(hWnd, 0, workAreaRect.bottom - WINDOW_HEIGHT, WINDOW_WIDTH,
        WINDOW_HEIGHT, false);
}

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPTSTR lpCmd, int nShow)
{
    const TCHAR *className = _T("EdCloser");

    g_hInstance = hInst;

    WNDCLASS windowClass;
    memset(&windowClass, 0, sizeof(WNDCLASS));
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WndProc;
    windowClass.hInstance = hInst;
    windowClass.hbrBackground = NULL;
    windowClass.lpszClassName = className;
    RegisterClass(&windowClass);

    HWND hWnd = CreateWindowEx(
        WS_EX_TOPMOST,
        className, _T("EdCloser"),
        WS_POPUP,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInst, NULL);

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);

    MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

    UnregisterClass(className, hInst);

    return msg.wParam;
}
