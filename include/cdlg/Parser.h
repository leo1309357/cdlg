#ifndef CDLG_PARSER_H
#define CDLG_PARSER_H

#include "Database.h"
#include "ParseError.h"

#include <string>

namespace cdlg {

struct ParseResult {
    Database database;
    std::vector<ParseError> errors;

    bool success() const;
};

ParseResult parseString(const std::string& source, const std::string& sourceName = "<string>");
ParseResult parseFile(const std::string& path);

} // namespace cdlg

#endif
