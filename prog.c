//
//   cl /nologo prog.c RstrtMgr.lib
//

#include <windows.h>
#include <RestartManager.h>
#include <stdio.h>

int __cdecl wmain(int argc, WCHAR **argv) {

   if (argc < 1) {
       wprintf(L"specify filename\n");
       return 1;
   }

   DWORD rmSes;
   WCHAR rmSesKey[CCH_RM_SESSION_KEY+1] = { 0 };

   DWORD rc = RmStartSession(&rmSes, 0, rmSesKey);

   if (rc != ERROR_SUCCESS) {
       wprintf(L"RmStartSession failed and returned %d", rc);
       return 2;
   }

   PCWSTR pszFile = argv[1];
   rc = RmRegisterResources(rmSes, 1, &pszFile, 0, NULL, 0, NULL);

   if (rc != ERROR_SUCCESS) {
      wprintf(L"RmRegisterResources(%ls) failed and returned %d\n", pszFile, rc);
      return 3;
   }

   DWORD dwReason;
   UINT  nProcInfoNeeded;
   UINT  nProcInfo = 10;

   RM_PROCESS_INFO rgpi[10];

   rc = RmGetList(rmSes, &nProcInfoNeeded, &nProcInfo, rgpi, &dwReason);

   if (rc != ERROR_SUCCESS) {
      wprintf(L"RmGetList returned failed and returned %d\n", rc);
      return 4;
   }

   for (UINT procCnt = 0; procCnt < nProcInfo; procCnt++) {

      HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, rgpi[procCnt].Process.dwProcessId);

      WCHAR* appType;
      WCHAR* appName;
      WCHAR  sz[MAX_PATH];

      if (hProcess) {

         switch (rgpi[procCnt].ApplicationType) {
            case RmUnknownApp  : appType = L"Unknown"  ; break;
            case RmMainWindow  : appType = L"MainWin"  ; break;
            case RmOtherWindow : appType = L"OtherWin" ; break;
            case RmService     : appType = L"Service"  ; break;
            case RmExplorer    : appType = L"Explorer" ; break;
            case RmConsole     : appType = L"Console"  ; break;
            case RmCritical    : appType = L"Critical" ; break;
                                 appType = L"?";
         }

         appName = rgpi[procCnt].strAppName;

         FILETIME ftCreate, ftExit, ftKernel, ftUser;

         if (GetProcessTimes(hProcess, &ftCreate, &ftExit, &ftKernel, &ftUser) &&
             CompareFileTime(&rgpi[procCnt].Process.ProcessStartTime, &ftCreate) == 0) {

            DWORD cch = MAX_PATH;

            if (! (QueryFullProcessImageNameW(hProcess, 0, sz, &cch) && cch <= MAX_PATH) ) {
               sz[0] = 0;
            }
         }

         CloseHandle(hProcess);
      }
      else {
         appType = L"";
         appName = L"";
         sz[0]   =  0;
      }
      wprintf(L"%5d %-8s %-90s %-20s\n", rgpi[procCnt].Process.dwProcessId, appType, sz, appName);
   }
   RmEndSession(rmSes);

   return 0;
}
