#include <vector>
#include <string>
#include <windows.h>
#include <commctrl.h>
#include <kncecomm.h>

#ifdef UNICODE
namespace std { typedef wstring tstring; }
#else
namespace std { typedef string tstring; }
#endif

using namespace std;

enum {
    IDC_STATUS_LABEL = 101,
    IDC_CENTER_IMAGE = 102,
    IDC_DISCONNECT_BUTTON = 103,
    DISCONNECT_BUTTON_WIDTH = 100,
    DISCONNECT_BUTTON_HEIGHT = 50
};

BOOL WINAPI connectionDlgProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);
static void onUsbAttach(HWND hDlg);
static void onUsbDetach(HWND hDlg);
static void onInitDialog(HWND hDlg);
static void onDestroy(HWND hDlg);
static void onDisconnect(HWND hDlg);
static void onEraseBkgnd(HWND hWnd, HDC hDC);
static HBRUSH onCtlColorStatic(HWND hWnd, HDC hDC, HWND hStatic);
static void changeWindowLayout(HWND hDlg);

extern HINSTANCE g_hInstance;

struct ConnectionWindowData {
    HBITMAP hConnectionBitmap;
    bool deviceAttached;
};

void showConnectionWindow(HWND hMainDialog) {
	DialogBox(g_hInstance, _T("CONNECTION"), NULL, (DLGPROC)connectionDlgProc);
}

BOOL WINAPI connectionDlgProc(HWND hDlg, UINT msg, WPARAM wParam,
    LPARAM lParam) {

    if (msg == RegisterWindowMessage(_T("USBAttach"))) {
        onUsbAttach(hDlg);
        return true;
    }
    else if (msg == RegisterWindowMessage(_T("USBDetach"))) {
        onUsbDetach(hDlg);
        return true;
    }

	switch (msg) {
    case WM_INITDIALOG:
        onInitDialog(hDlg);
        return true;
    case WM_DESTROY:
        onDestroy(hDlg);
		return true;
    case WM_CLOSE:
        DestroyWindow(hDlg);
        return true;
    case WM_COMMAND:
    {
        int id    = LOWORD(wParam); 
        int event = HIWORD(wParam);

        switch (id) {
        case IDOK:
            onDisconnect(hDlg);
            break;
        default:
            return false;
        }

        return true;
    }
    case WM_ERASEBKGND:
        onEraseBkgnd(hDlg, (HDC)wParam);
        return 1;
    case WM_CTLCOLORSTATIC:
    {
        HBRUSH hBrush = onCtlColorStatic(hDlg, (HDC)wParam, (HWND)lParam);
        if (hBrush != NULL)
            return (BOOL)hBrush;
        break;
    }
	}

	return false;
}

static void onUsbAttach(HWND hDlg) {
    ConnectionWindowData *data = (ConnectionWindowData *)GetWindowLong(hDlg,
        GWL_USERDATA);

    HWND hStatusLabel = GetDlgItem(hDlg, IDC_STATUS_LABEL);
    SetWindowText(hStatusLabel, _T("Connecting..."));

    data->deviceAttached = true;
}

static void onUsbDetach(HWND hDlg) {
    ConnectionWindowData *data = (ConnectionWindowData *)GetWindowLong(hDlg,
        GWL_USERDATA);

    if (data->deviceAttached)
        SendMessage(hDlg, WM_CLOSE, 0, 0);
}

static void onInitDialog(HWND hDlg) {
    ConnectionWindowData *data = new ConnectionWindowData();
    SetWindowLong(hDlg, GWL_USERDATA, (long)data);

    HWND hCenterImage = GetDlgItem(hDlg, IDC_CENTER_IMAGE);

    data->hConnectionBitmap = (HBITMAP)LoadImage(g_hInstance, _T("CONNECTING"),
        IMAGE_BITMAP, 0, 0, 0);
    SendMessage(hCenterImage, STM_SETIMAGE, IMAGE_BITMAP,
        (LPARAM)data->hConnectionBitmap);

    data->deviceAttached = false;

    changeWindowLayout(hDlg);
}

static void onDestroy(HWND hDlg) {
    ConnectionWindowData *data = (ConnectionWindowData *)GetWindowLong(hDlg,
        GWL_USERDATA);

    DeleteObject(data->hConnectionBitmap);

    delete (ConnectionWindowData *)GetWindowLong(hDlg, GWL_USERDATA);
}

static void onDisconnect(HWND hDlg) {
    int ret = MessageBox(hDlg, _T("Disconnect?"), _T("Confirm"),
        MB_YESNO | MB_ICONQUESTION);

    if (ret == IDYES)
        SendMessage(hDlg, WM_CLOSE, 0, 0);
}

static void onEraseBkgnd(HWND hDlg, HDC hDC) {
    RECT rect;
    GetClipBox(hDC, &rect);

    FillRect(hDC, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
}

static HBRUSH onCtlColorStatic(HWND hDlg, HDC hDC, HWND hStatic) {
    if (hStatic != GetDlgItem(hDlg, IDC_STATUS_LABEL))
        return NULL;

    SetTextColor(hDC, RGB(0, 0, 0));
    SetBkColor(hDC, RGB(255, 255, 255));

    return (HBRUSH)GetStockObject(WHITE_BRUSH);
}

static void changeWindowLayout(HWND hDlg) {
    ConnectionWindowData *data = (ConnectionWindowData *)GetWindowLong(hDlg,
        GWL_USERDATA);

    RECT desktopRect;
    HWND hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktopRect); 

    MoveWindow(hDlg, 0, 0, desktopRect.right, desktopRect.bottom, false);

    SetWindowPos(hDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
}
