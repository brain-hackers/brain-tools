#include <vector>
#include <string>
#include <windows.h>
#include <winioctl.h>
#include <kncecomm.h>

#ifdef UNICODE
namespace std { typedef wstring tstring; }
#else
namespace std { typedef string tstring; }
#endif

using namespace std;

enum {
    IDC_USB_IMAGE = 101,
    IDC_NAND3_BUTTON = 111,
    IDC_SD_CARD_BUTTON = 112
};

#define UFN_CLIENT_NAME_MAX_CHARS 128

struct UFN_CLIENT_NAME {
    TCHAR szName[UFN_CLIENT_NAME_MAX_CHARS];
};

#define _UFN_ACCESS_CTL_CODE(_Function) \
    CTL_CODE(FILE_DEVICE_UNKNOWN, _Function, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_UFN_CHANGE_CURRENT_CLIENT _UFN_ACCESS_CTL_CODE(4)
#define IOCTL_UFN_CHANGE_DECIVE _UFN_ACCESS_CTL_CODE(0x067)

#define _MRS_ACCESS_CTL_CODE(_Function) \
    CTL_CODE(FILE_DEVICE_UNKNOWN, _Function, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_MRS_MR_SENSOR_ENABLE _MRS_ACCESS_CTL_CODE(0x801)
#define IOCTL_MRS_MR_SENSOR_DISABLE _MRS_ACCESS_CTL_CODE(0x802)
#define IOCTL_MRS_MR_SENSOR_GET_STATE _MRS_ACCESS_CTL_CODE(0x803)

void registerConnectionWindowClass();
void unregisterConnectionWindowClass();
void showConnectionWindow(HWND hMainDialog);

BOOL WINAPI MainDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static void onInitDialog(HWND hDlg);
static void onClose(HWND hDlg);
static void onNand3(HWND hDlg);
static void onSdCard(HWND hDlg);
static void connect(HWND hDlg, int devId);
static bool changeUsbFunction(int devId);
static void enablePowerOffKey(bool isEnabled);
static void enablePowerOff(bool isEnabled);
static void enableMrSensor(bool isEnabled);
int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPTSTR lpCmd,
    int nShow);

HINSTANCE g_hInstance = NULL;
HFONT g_hMainFont = NULL;

struct UsbStorageData {
    HBITMAP hBannerBitmap;
    HWND hConnectionWindow;
    bool deviceAttached;
};

BOOL WINAPI MainDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
    case WM_INITDIALOG:
        onInitDialog(hDlg);
        return true;
    case WM_CLOSE:
        onClose(hDlg);
        return true;
    case WM_COMMAND:
    {
        int id    = LOWORD(wParam); 
        int event = HIWORD(wParam);

        switch (id) {
        case IDOK:
        case IDCANCEL:
            PostMessage(hDlg, WM_CLOSE, 0, 0);
            break;
        case IDC_NAND3_BUTTON:
            onNand3(hDlg);
            break;
        case IDC_SD_CARD_BUTTON:
            onSdCard(hDlg);
            break;
        default:
            return false;
        }

        return true;
    }
	}

	return false;
}

static void onInitDialog(HWND hDlg) {
    UsbStorageData *data = new UsbStorageData();
    SetWindowLong(hDlg, GWL_USERDATA, (long)data);

    data->hConnectionWindow = NULL;
    data->deviceAttached = false;

    g_hMainFont = knceCreateDefaultFont(1, 100, 0, 0);
    knceSetDialogFont(hDlg, g_hMainFont);

    HWND hUsbImage = GetDlgItem(hDlg, IDC_USB_IMAGE);

    data->hBannerBitmap = (HBITMAP)LoadImage(g_hInstance, _T("USB"),
        IMAGE_BITMAP, 0, 0, 0);
    SendMessage(hUsbImage, STM_SETIMAGE, IMAGE_BITMAP,
        (LPARAM)data->hBannerBitmap);

    HKEY hKey = NULL;
    DWORD disp = 0;

    RegCreateKeyEx(HKEY_CURRENT_USER, _T("Software\\knatech\\usbstorage"), 0,
        NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &disp);

    RegCloseKey(hKey);

    ShowWindow(hDlg, SW_SHOW);
}

static void onClose(HWND hDlg) {
    UsbStorageData *data = (UsbStorageData *)GetWindowLong(hDlg, GWL_USERDATA);

    HKEY hKey = NULL;
    DWORD disp = 0;

    RegCreateKeyEx(HKEY_CURRENT_USER, _T("Software\\knatech\\usbstorage"), 0,
        NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &disp);

    RegCloseKey(hKey);

    DeleteObject(data->hBannerBitmap);

    DeleteObject(g_hMainFont);

    EndDialog(hDlg, IDOK);

    delete (UsbStorageData *)GetWindowLong(hDlg, GWL_USERDATA);
}

static void onNand3(HWND hDlg) {
    connect(hDlg, 0);
}

static void onSdCard(HWND hDlg) {
    connect(hDlg, 1);
}

static void connect(HWND hDlg, int devId) {
    UsbStorageData *data = (UsbStorageData *)GetWindowLong(hDlg, GWL_USERDATA);

    if (!changeUsbFunction(devId)) {
        MessageBox(hDlg, _T("Cannot change usb function."), _T("Error"),
            MB_ICONEXCLAMATION);
        return;
    }

    data->deviceAttached = false;

    enablePowerOffKey(false);
    enablePowerOff(false);
    enableMrSensor(false);

    showConnectionWindow(hDlg);

    SetForegroundWindow(hDlg);

    if (!changeUsbFunction(-1)) {
        MessageBox(hDlg, _T("Cannot change usb function."), _T("Error"),
            MB_ICONEXCLAMATION);
        return;
    }

    data->deviceAttached = false;

    enablePowerOffKey(true);
    enablePowerOff(true);
    enableMrSensor(true);
}

static bool changeUsbFunction(int devId) {
    HANDLE hUsbFunc = CreateFile(_T("UFN1:"), GENERIC_READ | GENERIC_WRITE |
        FILE_WRITE_ATTRIBUTES, 0, 0, OPEN_EXISTING, 0, 0);

    if (hUsbFunc == INVALID_HANDLE_VALUE)
        return false;

    if (devId != -1) {
        if (!DeviceIoControl(hUsbFunc, IOCTL_UFN_CHANGE_DECIVE, &devId,
            sizeof(int), NULL, 0, NULL, NULL)) {

            return false;
        }
    }

	UFN_CLIENT_NAME usbFuncClientName = {0};
    if (devId != -1)
        _tcscpy(usbFuncClientName.szName, _T("Mass_Storage_Class"));

	if (!DeviceIoControl(hUsbFunc, IOCTL_UFN_CHANGE_CURRENT_CLIENT,
        &usbFuncClientName, sizeof(UFN_CLIENT_NAME), NULL, 0, NULL, NULL)) {

        return false;
    }

    CloseHandle(hUsbFunc);

    return true;
}

void enablePowerOffKey(bool isEnabled) {
    HANDLE hEvent = OpenEvent(EVENT_ALL_ACCESS, false,
        _T("Keyboard_Event_PowerOffKey_Disable"));
    if (hEvent == NULL)
        return;

    if (isEnabled)
        ResetEvent(hEvent);
    else
        SetEvent(hEvent);

    CloseHandle(hEvent);
}

void enablePowerOff(bool isEnabled) {
    HANDLE hEvent = OpenEvent(EVENT_ALL_ACCESS, false,
        _T("PowerManager/EdDisablePowerOff"));
    if (hEvent == NULL)
        return;

    if (isEnabled)
        ResetEvent(hEvent);
    else
        SetEvent(hEvent);

    CloseHandle(hEvent);
}

void enableMrSensor(bool isEnabled) {
    HANDLE hMrSensor = CreateFile(_T("MRS1:"), GENERIC_READ | GENERIC_WRITE |
        FILE_WRITE_ATTRIBUTES, 0, 0, OPEN_EXISTING, 0, 0);

    if (hMrSensor == INVALID_HANDLE_VALUE)
        return;

    DWORD ctrlCode = isEnabled ? IOCTL_MRS_MR_SENSOR_ENABLE :
        IOCTL_MRS_MR_SENSOR_DISABLE;

    if (!DeviceIoControl(hMrSensor, ctrlCode, NULL, 0, NULL, 0, NULL, NULL))
        return;

    CloseHandle(hMrSensor);
}

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPTSTR lpCmd, int nShow)
{
    g_hInstance = hInst;

	DialogBox(hInst, _T("MAIN"), NULL, (DLGPROC)MainDlgProc);

	return 0;
}
