#include "syntax.h"

/**
 * Helper functions
 */
ASTNode* GetNextNode(Scope* scope){
	int loc = scope->nodeSpot++;
	if (loc >= scope->nodeLength) return NULL;
	return &scope->nodes[loc];
}

ASTList* GetNextASTList(Scope* scope){
	int loc = scope->paramsSpot++;
	if (loc >= scope->paramsLength) return NULL;
	return &scope->params[loc];
}

ASTListItem* GetNextASTListItem(Scope* scope){
	int loc = scope->paramItemsSpot++;
	if (loc >= scope->paramItemsLength) return NULL;
	return &scope->paramItems[loc];
}

int GetCurrentScopeId(Scope* scope){
	int loc = scope->scopeSpot - 1;
	return loc;
}

/**
 * Syntax:
 * 	for ([var], [expr], [inc]) {...}
 */
ASTNode* ParseFor(Scope* scope, Lexer* lexer){
	DEBUG_PRINT_SYNTAX("For");
	TRACK();
	ASTNode* forExpr = GetNextNode(scope);
	forExpr->isStmt = true;
	forExpr->type = FOR;

	Token tok = GetNextToken(lexer);
	EXPECT_TOKEN(tok, LPAREN, lexer);
	tok = GetNextToken(lexer);

	/**
	 * Since this is an internal variable for a for loop,
	 * this will not be a top level statement.
	 */
	ASTNode* var = ParseVar(scope, lexer, tok);

	forExpr->meta.forExpr.var = var;
	forExpr->meta.forExpr.condition = ParseExpression(scope, lexer);
	forExpr->meta.forExpr.inc = ParseExpression(scope, lexer);

	tok = GetNextToken(lexer);
	EXPECT_TOKEN(tok, RPAREN, lexer);
	forExpr->meta.forExpr.body = ParseBody(scope, lexer);
	return forExpr;
}

/**
 * Syntax:
 * 	if (expr) {...}
 */
ASTNode* ParseIf(Scope* scope, Lexer* lexer){
	DEBUG_PRINT_SYNTAX("If");
	TRACK();
	ASTNode* ifExpr = GetNextNode(scope);
	ifExpr->isStmt = true;
	ifExpr->type = IF;

	Token tok = GetNextToken(lexer);
	EXPECT_TOKEN(tok, LPAREN, lexer);
	ifExpr->meta.ifExpr.condition = ParseExpression(scope, lexer);
	ifExpr->meta.ifExpr.body = ParseBody(scope, lexer);
	return ifExpr;
}

/**
 * Syntax:
 * 	while ([expr]) {...}
 */
ASTNode* ParseWhile(Scope* scope, Lexer* lexer){
	DEBUG_PRINT_SYNTAX("While");
	TRACK();
	ASTNode* whileStmt = GetNextNode(scope);
	whileStmt->isStmt = true;
	whileStmt->type = WHILE;

	Token tok = GetNextToken(lexer);
	EXPECT_TOKEN(tok, LPAREN, lexer);
	whileStmt->meta.whileExpr.condition = ParseExpression(scope, lexer);
	whileStmt->meta.whileExpr.body = ParseBody(scope, lexer);
	return whileStmt;
}

/**
 * Syntax:
 * 	switch ([expr]): {...}
 */
ASTNode* ParseSwitch(Scope* scope, Lexer* lexer){
	DEBUG_PRINT_SYNTAX("Switch");
	TRACK();
	ASTNode* switchStmt = GetNextNode(scope);
	switchStmt->isStmt = true;
	switchStmt->type = SWITCH;

	Token tok = GetNextToken(lexer);
	EXPECT_TOKEN(tok, LPAREN, lexer);
	switchStmt->meta.switchExpr.condition = ParseExpression(scope, lexer);
	switchStmt->meta.whileExpr.body = ParseBody(scope, lexer);

	return switchStmt;
}

/**
 * Syntax:
 * 	case [expr]: {...}
 */
ASTNode* ParseCase(Scope* scope, Lexer* lexer){
	DEBUG_PRINT_SYNTAX("Case");
	TRACK();
	ASTNode* caseStmt = GetNextNode(scope);
	caseStmt->isStmt = true;
	caseStmt->type = CASE;

	caseStmt->meta.caseStmt.condition = ParseExpression(scope, lexer);
	Token tok = GetCurrentToken(lexer);
	EXPECT_TOKEN(tok, COLON, lexer);
	caseStmt->meta.caseStmt.body = ParseBody(scope, lexer);

	return caseStmt;
}

/**
 * Syntax:
 * 	return [expr]
 */
ASTNode* ParseReturn(Scope* scope, Lexer* lexer){
	DEBUG_PRINT_SYNTAX("Return");
	TRACK();
	ASTNode* var = GetNextNode(scope);
	var->type = RETURN;
	var->meta.returnStmt.value = ParseExpression(scope, lexer);
	var->isStmt = true;
	return var;
}

/**
 * Syntax:
 * 	break;
 */
ASTNode* ParseBreak(Scope* scope, Lexer* lexer){
	DEBUG_PRINT_SYNTAX("Break");
	TRACK();
	ASTNode* Break = GetNextNode(scope);
	Break->type = BREAK;
	Break->isStmt = true;
	return Break;
}

/**
 * Syntax:
 * 	func [name]([var], [var], ...) {...}
 */
ASTNode* ParseFunc(Scope* scope, Lexer* lexer){
	TRACK();
	ASTNode* func = GetNextNode(scope);
	func->type = FUNC;
	Token tok = GetNextToken(lexer);
	EXPECT_TOKEN(tok, IDENTIFIER, lexer);
	DEBUG_PRINT_SYNTAX2("Func", lexer->currentTokenString);

	func->meta.funcExpr.name = Allocate((sizeof(char) * strlen(lexer->currentTokenString)) + sizeof(char));
	strcpy(func->meta.funcExpr.name, lexer->currentTokenString);

	func->meta.funcExpr.params = ParseParams(scope, lexer);
	func->meta.funcExpr.body = ParseBody(scope, lexer);
	func->isStmt = true;
	return func;
}

/**
 * Syntax
 * 	func [name]([var],[var],...);
 */
ASTNode* ParseFuncCall(Scope* scope, Lexer* lexer){
	DEBUG_PRINT_SYNTAX("Func Call");
	TRACK();
	Token tok = GetCurrentToken(lexer);
	EXPECT_TOKEN(tok, IDENTIFIER, lexer);

	ASTNode* funcCall = GetNextNode(scope);
	funcCall->type = FUNC_CALL;
	funcCall->meta.funcCallExpr.func = FindSymbol(scope, lexer->currentTokenString);
	funcCall->isStmt = true;

	tok = GetNextToken(lexer);
	EXPECT_TOKEN(tok, LPAREN, lexer);
	ASTList* list = GetNextASTList(scope);

	while (tok != RPAREN){
		ASTListItem* item = GetNextASTListItem(scope);
		item->node = ParseExpression(scope, lexer);
		item->prev = list->current;

		if (list->current != NULL) list->current->next = item;

		list->current = item;
		if (list->first == NULL) list->first = item;
		tok = GetCurrentToken(lexer);
	}

	funcCall->meta.funcCallExpr.params = list;
	return funcCall;
}

ASTList* ParseParams(Scope* scope, Lexer* lexer){
	DEBUG_PRINT_SYNTAX("Func Param");
	TRACK();
	Token tok = GetNextToken(lexer);
	EXPECT_TOKEN(tok, LPAREN, lexer);
	ASTList* list = GetNextASTList(scope);
	while (tok != RPAREN){
		tok = GetNextToken(lexer);
		ASTListItem* item = GetNextASTListItem(scope);
		item->node = ParseVar(scope, lexer, tok);
		item->prev = list->current;

		if (list->current != NULL) list->current->next = item;

		list->current = item;
		if (list->first == NULL) list->first = item;
		tok = GetCurrentToken(lexer);
	}
	list->last = list->current;
	return list;
}

Token ParseStmtList(Scope* scope, Lexer* lexer, int scopeId, bool oneStmt){
	Token tok = GetNextToken(lexer);
	bool breakOut = false;
	while (tok != UNDEFINED){
		int type = (int) tok;
		ASTNode* node = NULL;
		switch (type){
			case BOOLEAN: case BYTE:
			case SHORT: case INT:
			case FLOAT: case DOUBLE:
			case LONG: case VAR: {
				node = ParseVar(scope, lexer, tok); 
				break;
			}
			case FOR: {
				node = ParseFor(scope, lexer);
				break;
			}
			case IF: {
				node = ParseIf(scope, lexer);
				break;
			}
			case WHILE: {
				node = ParseWhile(scope, lexer);
				break;
			}
			case SWITCH: {
				node = ParseSwitch(scope, lexer);
				break;
			}
			case CASE: {
				node = ParseCase(scope, lexer);
				break;
			}
			case RETURN: {
				node = ParseReturn(scope, lexer);
				breakOut = true;
				break;
			}
			case BREAK: {
				node = ParseBreak(scope, lexer);
				breakOut = true;
				break;
			}
			case FUNC: {
				node = ParseFunc(scope, lexer);
				break;
			}
			case IDENTIFIER: {
				node = ParseIdent(scope, lexer);
				break;
			}
		}
		if (node != NULL) node->scopeId = scopeId;
		if (oneStmt) break;
		if (tok == RBRACE) return tok;
		tok = GetNextToken(lexer);
	}
	return tok;
}

/**
 * All statements beginning with an identifier must be
 * part of an expression. 
 */
ASTNode* ParseIdent(Scope* scope, Lexer* lexer){
	DEBUG_PRINT_SYNTAX("Ident");
	TRACK();
	PeekedToken peeked;
	PeekNextToken(lexer, &peeked);
	if (peeked.token == LPAREN){
		return ParseFuncCall(scope, lexer);
	}
	BackOneToken(lexer);
	return ParseExpression(scope, lexer);
}

/**
 * Syntax: 
 * 	{...}
 *
 * Brackets are optional. If body contains more than one statements,
 * then the brackets are required
 */
int ParseBody(Scope* scope, Lexer* lexer){
	DEBUG_PRINT_SYNTAX("Body");
	TRACK();
	int loc = scope->scopeSpot++;
	Token tok = GetNextToken(lexer);

	/**
	 * The the body has a LBRACE, then it 
	 * could have as many statements, however if there
	 * isn't a LBRACE, only 1 statement is allowed
	 */
	bool oneStmt = tok != LBRACE;
	// if (oneStmt) tok = GetNextToken(lexer); // eat
	if (oneStmt) BackOneToken(lexer);
	tok = ParseStmtList(scope, lexer, scope->scopes[loc], oneStmt);
	if (!oneStmt){
		EXPECT_TOKEN(tok, RBRACE, lexer);
	}
	return scope->scopes[loc];
}

/**
 * Syntax:
 * 	var [name] = [expr];
 */
ASTNode* ParseVar(Scope* scope, Lexer* lexer, Token dataType){
	DEBUG_PRINT_SYNTAX(TokenToString(dataType));
	TRACK();

	// Check if next token is identifier, if it isn't, we 
	// lookup the variable name. If it is, we can assume a 
	// type was declared
	PeekedToken peeked;
	char* currentTokenString = lexer->currentTokenString;
	PeekNextToken(lexer, &peeked);
	ASTNode* var;
	Token tok;
	if (peeked.token != IDENTIFIER){
		var = FindSymbol(scope, currentTokenString);
		if (var == NULL) SYMBOL_NOT_FOUND(lexer->currentTokenString, lexer);
	}
	else{
		var = GetNextNode(scope);
		var->type = VAR;
		var->meta.varExpr.dataType = dataType;

		tok = GetNextToken(lexer);
		EXPECT_TOKEN(tok, IDENTIFIER, lexer);

		// Set the name of the variable
		char* name = lexer->currentTokenString;
		var->meta.varExpr.name = Allocate((sizeof(char) * strlen(name)) + sizeof(char));
		var->meta.varExpr.value = NULL; // mem issue prevention
		var->isStmt = true; // is statement node
		var->meta.varExpr.inc = UNDEFINED;
		strcpy(var->meta.varExpr.name, name);
		DEBUG_PRINT_SYNTAX2(TokenToString(dataType), name);
	}

	tok = GetNextToken(lexer);

	// For empty variable declarations:
	// e.g: var a; or var b;
	if (tok == SEMICOLON || tok == COMMA || tok == RPAREN) return var;

	// Only assignment operators are valid after a VAR declaration
	if (!IsAssignment(tok)) EXPECT_STRING("Assignment Operator");
	if (tok == ASSIGN){ // var a = 	
		var->meta.varExpr.value = ParseExpression(scope, lexer);
		return var;
	}
	NOT_IMPLEMENTED("Other assignment types");
	// TODO: Implement other assignment operators

	return var;
}

/**
 * This function guides to what is actually being parsed
 */
ASTNode* ParseExpression(Scope* scope, Lexer* lexer){
	DEBUG_PRINT_SYNTAX("Expression");
	Token tok = GetNextToken(lexer);
	ASTNode* result = NULL;
	char* value = lexer->currentTokenString;

	if (tok == NUMBER){
		result = GetNextNode(scope);
		// Build the ASTLiteral node
		SetNumberType(result, value);

		tok = GetNextToken(lexer);
	}
	else if (tok == STRING){
		ASTNode* str = GetNextNode(scope);
		str->type = STRING;
		str->meta.stringExpr.value = Allocate((sizeof(char) * strlen(value)) + sizeof(char));

		strcpy(str->meta.stringExpr.value, value);

		// Remove the quotes around the string.
		memmove(
			&str->meta.stringExpr.value[0], 
			&str->meta.stringExpr.value[1], 
			strlen(str->meta.stringExpr.value) - 0);
		memmove(
			&str->meta.stringExpr.value[strlen(str->meta.stringExpr.value) - 1], 
			&str->meta.stringExpr.value[strlen(str->meta.stringExpr.value)], 
			1);

		tok = GetNextToken(lexer);

		result = str;
	}
	else if (tok == IDENTIFIER){
		PeekedToken peeked;
		PeekNextToken(lexer, &peeked);
		
		if (peeked.token == LPAREN){ // Function Call
			DEBUG_PRINT_SYNTAX("Function Call");
			ASTNode* call = GetNextNode(scope);
			ASTNode* func = FindSymbol(scope, value);

			call->type = FUNC_CALL;
			call->meta.funcCallExpr.func = func;
			call->meta.funcCallExpr.params = ParseParams(scope, lexer);
			result = call;
		}	

		else {
			ASTNode* symbol = FindSymbol(scope, value);
			if (symbol == NULL){
				EXPECT_STRING("Invalid identifier");
			}
			result = symbol;
		}

		tok = GetNextToken(lexer);

		if (tok == INC || tok == DEC){
			result->meta.varExpr.inc = tok;
		}
	}
	else if (tok == TRUE_LITERAL || tok == FALSE_LITERAL){
		result = GetNextNode(scope);
		result->type = BOOLEAN;

		// Build the ASTLiteral node
		result->meta.booleanExpr.value = tok == TRUE_LITERAL;

		tok = GetNextToken(lexer);
	}

	if (IsBinaryOperator(tok) || IsBooleanOperator(tok)){
		int loc = scope->nodeSpot++;
		ASTNode* left = result; // Is this a safe assumption?
		ASTNode* binary = &scope->nodes[loc];
		binary->type = BINARY;
		binary->meta.binaryExpr.op = tok;
		binary->meta.binaryExpr.left = left;
		binary->meta.binaryExpr.right = ParseExpression(scope, lexer);
		result = binary;
	}

	return result;
}