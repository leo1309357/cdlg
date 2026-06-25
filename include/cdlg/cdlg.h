#ifndef CDLG_H
#define CDLG_H

// CDLG dialogue library
// Standalone parser and runtime for .cdlg dialogue scripts.
//
// Format overview (.cdlg):
//   dialogue <id>
//     entry <node_id>
//     node <node_id>
//       speaker <name>
//       line <text>
//       choice <label> -> <node_id>
//       goto <node_id>
//       set <flag> <value>
//       end
//   end

#include "Database.h"
#include "ParseError.h"
#include "Parser.h"
#include "Session.h"
#include "Types.h"

#endif
