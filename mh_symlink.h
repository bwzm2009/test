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
#ifndef _MH_SYMLINK_H_INCLUDED_
#define _MH_SYMLINK_H_INCLUDED_

#include <string>
#include "safeunistd.h"
#include <errno.h>

#include "cstr.h"
#include "mimehandler.h"
#include "transcode.h"
#include "pathut.h"
#include "log.h"

/** Index symlink target 
 *
 * Not sure that this is such a good idea, so it's disabled by default in
 * the config. Add inode/symlink = internal to the index section of mimeconf 
 * to enable.
 */
class MimeHandlerSymlink : public RecollFilter {
public:
    MimeHandlerSymlink(RclConfig *cnf, const std::string& id) 
        : RecollFilter(cnf, id) {}
    virtual ~MimeHandlerSymlink() = default;
    MimeHandlerSymlink(const MimeHandlerSymlink&) = delete;
    MimeHandlerSymlink& operator=(const MimeHandlerSymlink&) = delete;
#ifdef _WIN32
#include <windows.h>

std::string read_symlink_windows(const char* path) {
    wchar_t wTargetPath[MAX_PATH];
    HANDLE hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, NULL);
    if (GetFinalPathNameByHandleW(hFile, wTargetPath, MAX_PATH, VOLUME_NAME_DOS) > 0) {
        char targetPath[MAX_PATH];
        WideCharToMultiByte(CP_UTF8, 0, wTargetPath, -1, targetPath, MAX_PATH, NULL, NULL);
        CloseHandle(hFile);
        return std::string(targetPath);
    }
    CloseHandle(hFile);
    return std::string();
}
#endif

    virtual bool next_document() {
        if (m_havedoc == false)
            return false;
        m_havedoc = false; 
        m_metaData[cstr_dj_keycontent] = cstr_null;
        char lc[1024];
        std::string targetPath = read_symlink(m_fn);
		ssize_t bytes = targetPath.length();

        if (bytes != (ssize_t)-1) {
            string slc(lc, bytes);
            transcode(path_getsimple(slc), m_metaData[cstr_dj_keycontent], 
                      m_config->getDefCharset(true), "UTF-8");
        } else {
            LOGDEB("Symlink: readlink [" << m_fn << "] failed, errno " <<
                   errno << "\n");
        }
        m_metaData[cstr_dj_keymt] = cstr_textplain;
        return true;
    }
protected:
    virtual bool set_document_file_impl(const string&, const string& fn) {
        m_fn = fn;
        return m_havedoc = true;
    }

private:
    std::string m_fn;
};

#endif /* _MH_SYMLINK_H_INCLUDED_ */

