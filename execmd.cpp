#include <Windows.h>
#include <string>
#include <vector>

int execmd(const std::string& cmd, std::vector<std::string>& output)
{
    HANDLE hRead, hWrite;
    SECURITY_ATTRIBUTES saAttr;
    STARTUPINFO siStartInfo;
    PROCESS_INFORMATION piProcInfo;
    BOOL bSuccess;

    output.clear();

    // Set the bInheritHandle flag so pipe handles are inherited
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    // Create a pipe for the child process's stdout
    if (!CreatePipe(&hRead, &hWrite, &saAttr, 0)) {
        return -1;
    }

    // Ensure the read handle to the pipe for STDOUT is not inherited
    if (!SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0)) {
        CloseHandle(hRead);
        CloseHandle(hWrite);
        return -1;
    }

    // Set up members of the PROCESS_INFORMATION structure
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    // Set up members of the STARTUPINFO structure
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = hWrite;
    siStartInfo.hStdOutput = hWrite;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    // Create the child process
    bSuccess = CreateProcessA(NULL,
        const_cast<LPSTR>(cmd.c_str()),
        NULL,
        NULL,
        TRUE,
        0,
        NULL,
        NULL,
        &siStartInfo,
        &piProcInfo);

    // Close the write end of the pipe before reading from the read end of the pipe
    CloseHandle(hWrite);

    if (!bSuccess) {
        CloseHandle(hRead);
        return -1;
    }

    // Read output from the child process
    const int BUFSIZE = 4096;
    CHAR chBuf[BUFSIZE];
    DWORD dwRead;
    BOOL bReadSuccess;

    while (true) {
        bReadSuccess = ReadFile(hRead, chBuf, BUFSIZE, &dwRead, NULL);
        if (!bReadSuccess || dwRead == 0) {
            break;
        }
        output.emplace_back(chBuf, dwRead);
    }

    // Close the read end of the pipe
    CloseHandle(hRead);

    // Wait for the child process to exit
    WaitForSingleObject(piProcInfo.hProcess, INFINITE);

    // Close process and thread handles
    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);

    // Return the exit code of the child process
    DWORD exitCode;
    GetExitCodeProcess(piProcInfo.hProcess, &exitCode);
    return exitCode;
}
