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
