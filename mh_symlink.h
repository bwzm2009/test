/* Copyright (C) 2004 J.F.Dockes
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include "rclconfig.h"
#include "mimehandler.h"
#include "internfile.h"
#include "minwindef.h"
#include "windef.h"
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <cerrno>
#include <cstring>
#include <winioctl.h>

using namespace std;

#ifdef _WIN32

std::string read_symlink_windows(const std::string& path) {
    HANDLE hFile = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return "";
    }

    BYTE buffer[MAXIMUM_REPARSE_DATA_BUFFER_SIZE] = { 0 };
    DWORD dwBytesReturned = 0;
    if (!DeviceIoControl(hFile, FSCTL_GET_REPARSE_POINT, NULL, 0, buffer, MAXIMUM_REPARSE_DATA_BUFFER_SIZE, &dwBytesReturned, NULL)) {
        CloseHandle(hFile);
        return "";
    }

    CloseHandle(hFile);

    REPARSE_DATA_BUFFER* pbuffer = reinterpret_cast<REPARSE_DATA_BUFFER*>(buffer);
    if (pbuffer->ReparseTag != IO_REPARSE_TAG_SYMLINK) {
        return "";
    }

    WCHAR *target = pbuffer->SymbolicLinkReparseBuffer.PathBuffer;
    DWORD targetLength = pbuffer->SymbolicLinkReparseBuffer.SubstituteNameLength / sizeof(WCHAR);

    std::wstring wTargetPath(target, targetLength);
    std::string targetPath(wTargetPath.begin(), wTargetPath.end());
    return targetPath;
}

#endif

class MimeHandlerSymlink : public RecollFilter {
 public:
    MimeHandlerSymlink(RclConfig *cnf, const string &id) : RecollFilter(cnf, id) {}
	FileInterner *m_fip{nullptr};
	
	bool next_document() override {
		if (m_fip->get_fn().empty()) {
			return false;
		}
		std::string targetPath = read_symlink_windows(m_fip->get_fn().c_str());

		if (targetPath.empty()) {
			return false;
		}
		m_metaData["target"] = targetPath;
		return true;
	}

    void clear_impl() override {}
};

static std::string read_symlink_windows(const std::string &path) {
    std::string result;
    WCHAR buffer[MAX_PATH] = {0};
    size_t len = MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, nullptr, 0);
    std::wstring wpath(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, &wpath[0], len);
    DWORD attributes = GetFileAttributesW(wpath.c_str());
    if (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
        HANDLE handle = CreateFileW(wpath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, nullptr);
        if (handle != INVALID_HANDLE_VALUE) {
            DWORD resultLength = 0;
            WCHAR buffer[REPARSE_DATA_BUFFER_HEADER_SIZE + MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
            PREPARSE_DATA_BUFFER pbuffer = (PREPARSE_DATA_BUFFER)buffer;
            memset(pbuffer, 0, sizeof(buffer));
            if (DeviceIoControl(handle, FSCTL_GET_REPARSE_POINT, nullptr, 0, pbuffer, sizeof(buffer), &resultLength, nullptr)) {
                WCHAR *target = pbuffer->SymbolicLinkReparseBuffer.PathBuffer;
                DWORD targetLength = pbuffer->SymbolicLinkReparseBuffer.SubstituteNameLength / sizeof(WCHAR);
                target[targetLength] = '\0';
                int utf8_len = WideCharToMultiByte(CP_UTF8, 0, target, -1, nullptr, 0, nullptr, nullptr);
                std::string utf8_target(utf8_len, 0);
                WideCharToMultiByte(CP_UTF8, 0, target, -1, &utf8_target[0], utf8_len, nullptr, nullptr);
                result = utf8_target;
            }
            CloseHandle(handle);
        }
    }
    return result;
}
