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

#include "mimehandler.h"
#include <string>
#include <windows.h>

class MimeHandlerSymlink : public RecollFilter {
public:
    MimeHandlerSymlink(RclConfig *cnf, const std::string &id)
        : RecollFilter(cnf, id) {}
    virtual ~MimeHandlerSymlink() {}
    virtual bool is_data_input_ok(DataInput input) const {
        return input == DOCUMENT_FILE_NAME;
    }
    virtual bool next_document();
    void clear() {}
 
 protected:
    std::string read_symlink_windows(const std::string &path) {
        std::string targetPath(MAX_PATH, '\0');
        DWORD nchars = GetFinalPathNameByHandleA(
            CreateFileA(path.c_str(), GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                        OPEN_EXISTING,
                        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
                        nullptr),
            &targetPath[0], MAX_PATH, FILE_NAME_NORMALIZED);
        if (nchars > 0) {
            targetPath.resize(nchars);
        } else {
            targetPath.clear();
        }
        return targetPath;
    }
};

#endif /* _MH_SYMLINK_H_INCLUDED_ */
