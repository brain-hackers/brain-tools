#include <string>
#include <windows.h>

using namespace std;

#ifdef UNICODE
namespace std { typedef wstring tstring; }
#else
namespace std { typedef string tstring; }
#endif

tstring getCurrentDirectory() {
    TCHAR path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);
 
    TCHAR *p = _tcschr(path, _T('\0'));
    for ( ; p >= path; p--) {
        if (*p == _T('\\')) {
            *p = _T('\0');
            break;
        }
    }

    return path;
}

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPTSTR lpCmd, int nShow)
{
    tstring mainProcPath = getCurrentDirectory() + _T("\\usbstorage.exe"); 

    CreateProcess(mainProcPath.c_str(), mainProcPath.c_str(), NULL, NULL,
        false, 0, NULL, NULL, NULL, NULL);

    return 0;
}
