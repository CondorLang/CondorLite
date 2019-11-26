// Copyright 2016 Chase Willden and The CondorLang Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef TYPECHECKER_H_
#define TYPECHECKER_H_

#include "../ast/scope.h"
#include "../ast/ast.h"
#include "../token/token.h"
#include "../../utils/assert.h"
#include "../../utils/debug.h"



Token GetBinaryType(ASTNode* node);

#ifdef IS_TEST
	void Test_GetBinaryType();
#endif

#endif // TYPECHECKER_H_
