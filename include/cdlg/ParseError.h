#ifndef CDLG_PARSE_ERROR_H
#define CDLG_PARSE_ERROR_H

#include <string>

namespace cdlg {

struct ParseError {
    int line = 0;
    int column = 0;
    std::string source;
    std::string message;
};

} // namespace cdlg

#endif
