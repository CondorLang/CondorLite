#include "runner.h"

void InitRunner(Runner* runner, Scope* scope) {
  runner->scope = scope;
  for (int i = 0; i < runner->totalContexts; i++){
    RunnerContext* context = &runner->contexts[i];
    context->node = NULL;
    context->dataType = UNDEFINED;
    context->id = i + 1;
  }
}

RunnerContext* Run(Runner* runner, int scopeId) {
  DEBUG_PRINT_RUNNER("Scope")
  Scope* scope = runner->scope;
  for (int i = 0; i < scope->nodeLength; i++){
    ASTNode* node = &scope->nodes[i];
    if (node->isStmt && node->scopeId == scopeId){
      runner->currentNode = node;
      RunnerContext* context = RunStatement(runner);
      
      if (node->type == RETURN){
        // Reset to return node
        context->node = node;
        return context;
      }

      if (node->type != FUNC && scopeId == 1) {
        PrintContext(context);
      }
    }
  }

  return NULL;
}

void GCScope(Runner* runner, int scopeId) {
  Scope* scope = runner->scope;

  for (int i = 0; i < scope->nodeLength; i++){
    ASTNode* node = &scope->nodes[i];
    if (node->scopeId == scopeId){
      // Don't GC return contexts
      if (node->type == RETURN) {
        continue;
      }

      GCContextByNodeId(runner, node->id);
    }
  }
}

void GCAstList(Runner* runner, ASTList* list) {
  FOREACH_AST(list) {
    GCContextByNodeId(runner, item->node->id);
  }
}

void GCContextByNodeId(Runner* runner, int nodeId) {
  for (int i = 0; i < runner->totalContexts; i++) {
    RunnerContext* context = &runner->contexts[i];
    if (runner->contextUsed[i] && context->node->id == nodeId) {
      ResetRunnerContext(context);
      runner->contextUsed[i] = false;
    }
  }
}

void GCContext(Runner* runner, RunnerContext* context){
  runner->contextUsed[context->id - 1] = false;
  ResetRunnerContext(context);
}

void ResetRunnerContext(RunnerContext* context) {
  context->node = NULL;
  context->dataType = UNDEFINED;
}

RunnerContext* RunStatement(Runner* runner){
  ASTNode* node = runner->currentNode;
  int type = (int) node->type;

  switch (type) {
    case FUNC_CALL: {
      return RunFuncCall(runner);
    }
    case RETURN: {
      runner->currentNode = GET_RETURN_VALUE(node);
      return RunStatement(runner);
    }
    case BINARY: {
      return RunBinary(runner);
    }
  }

  return SetNodeValue(runner, node);
}

RunnerContext* RunBinary(Runner* runner){
  DEBUG_PRINT_RUNNER("Binary expression")
  ASTNode* binary = runner->currentNode;
  ASTNode* left = GET_BIN_LEFT(binary);
  ASTNode* right = GET_BIN_RIGHT(binary);
  Token op = GET_BIN_OP(binary);

  RunnerContext* leftContext = SetNodeValue(runner, left);
  runner->currentNode = right;
  RunnerContext* rightContext = RunStatement(runner);

  return RunMathContexts(leftContext, rightContext, op);
}

RunnerContext* RunMathContexts(RunnerContext* left, RunnerContext* right, Token op){
  if (left->dataType == STRING || right->dataType == STRING){
    NOT_IMPLEMENTED("String concatenation");
  }

  if (left->dataType == STRING || left->dataType == CHAR ||
      right->dataType == STRING || right->dataType == CHAR) {
    NOT_IMPLEMENTED("String comparisons not implemented")      
  }
  else {
    double leftVal = ContextToDouble(left);
    double rightVal = ContextToDouble(right);

    DEBUG_RUNNER("Runner: Math: %f %s %f\n", leftVal, TokenToString(op), rightVal)

    // TODO: Figure out true type
    left->value.vDouble = RunMath(leftVal, rightVal, op);
    CastToType(left);
  }

  return left;
}

RunnerContext* RunFuncCall(Runner* runner){
  ASTList* params = GET_FUNC_CALL_PARAMS(runner->currentNode);
  ASTNode* func = GET_FUNC_CALL_FUNC(runner->currentNode);
  DEBUG_RUNNER("Runner: Function Call %s()\n", GET_FUNC_NAME(func));
  return RunFuncWithArgs(runner, func, params);
}

RunnerContext* RunFuncWithArgs(Runner* runner, ASTNode* func, ASTList* args){
  DEBUG_PRINT_RUNNER("Function Scope")

  int argIdx = 0;
  FOREACH_AST(args){
    ASTNode* arg = item->node;
    int paramIdx = 0;
    RunnerContext* context = SetNodeValue(runner, arg);

    FOREACH_AST(GET_FUNC_PARAMS(func)){
      if (argIdx == paramIdx++){
        ASTNode* param = item->node;
        context->node = param;
        break;
      }
    }

    argIdx++;
  }

  RunnerContext* context = Run(runner, GET_FUNC_BODY(func));
  GCScope(runner, GET_FUNC_BODY(func));
  GCAstList(runner, GET_FUNC_PARAMS(func));
  return context;
}

RunnerContext* SetNodeValue(Runner* runner, ASTNode* node){
  CHECK(node != NULL);
  RunnerContext* context = GetContextByNodeId(runner, node->id);
  if (context != NULL){
    return context;
  }

  context = GetNextContext(runner);
  context->node = node;

  int type = (int) node->type;
  switch (type) {
    case BOOLEAN: {
      context->dataType = node->type;
      context->value.vBoolean = GET_BOOLEAN_VALUE(node);
      DEBUG_RUNNER("Runner: Set node value: %d\n", context->value.vBoolean);
      break;
    }
    case BYTE: {
      context->dataType = BYTE;
      context->value.vByte = GET_BYTE_VALUE(node);
      DEBUG_RUNNER("Runner: Set node value: %d\n", context->value.vByte);
      break;
    }
    case SHORT: {
      context->dataType = SHORT;
      context->value.vShort = GET_SHORT_VALUE(node);
      DEBUG_RUNNER("Runner: Set node value: %d\n", context->value.vShort);
      break;
    }
    case INT: {
      context->dataType = INT;
      context->value.vInt = GET_INT_VALUE(node);
      DEBUG_RUNNER("Runner: Set node value: %d\n", context->value.vInt);
      break;
    }
    case FLOAT: {
      context->dataType = FLOAT;
      context->value.vFloat = GET_FLOAT_VALUE(node);
      DEBUG_RUNNER("Runner: Set node value: %f\n", context->value.vFloat);
      break;
    }
    case DOUBLE: {
      context->dataType = DOUBLE;
      context->value.vDouble = GET_DOUBLE_VALUE(node);
      DEBUG_RUNNER("Runner: Set node value: %f\n", context->value.vDouble);
      break;
    }
    case LONG: {
      context->dataType = LONG;
      context->value.vLong = GET_LONG_VALUE(node);
      DEBUG_RUNNER("Runner: Set node value: %ld\n", context->value.vLong);
      break;
    }
    case CHAR: {
      context->dataType = CHAR;
      context->value.vChar = GET_CHAR_VALUE(node);
      DEBUG_RUNNER("Runner: Set node value: %c\n", context->value.vChar);
      break;
    }
    case STRING: {
      context->dataType = STRING;
      context->value.vString = GET_STRING_VALUE(node);
      DEBUG_RUNNER("Runner: Set node value: %s\n", context->value.vString);
      break;
    }
    case VAR: {
      RunSetVarType(runner, context, node);
      break;
    }
    case FUNC_CALL: {
      ASTNode* previousNode = runner->currentNode;
      runner->currentNode = node;
      RunnerContext* newContext = RunFuncCall(runner);
      runner->currentNode = previousNode;
      MergeContextValues(newContext, context);
      GCContext(runner, newContext);
      break;
    }
  }

  return context;
}

void MergeContextValues(RunnerContext* left, RunnerContext* right){
  right->dataType = left->dataType;
  right->value = left->value;
}

void RunSetVarType(Runner* runner, RunnerContext* context, ASTNode* node){
  int type = (int) GET_VAR_TYPE(node);
  switch (type) {
    case BOOLEAN: {
      context->dataType = BOOLEAN;
      RunnerContext* varContext = SetNodeValue(runner, GET_VAR_VALUE(node));
      context->value.vBoolean = varContext->value.vBoolean;
      break;
    }
    case BYTE: {
      context->dataType = BYTE;
      RunnerContext* varContext = SetNodeValue(runner, GET_VAR_VALUE(node));
      context->value.vByte = varContext->value.vByte;
      break;
    }
    case SHORT: {
      context->dataType = SHORT;
      RunnerContext* varContext = SetNodeValue(runner, GET_VAR_VALUE(node));
      context->value.vShort = varContext->value.vShort;
      break;
    }
    case INT: {
      context->dataType = INT;
      RunnerContext* varContext = SetNodeValue(runner, GET_VAR_VALUE(node));
      context->value.vInt = varContext->value.vInt;
      break;
    }
    case FLOAT: {
      context->dataType = FLOAT;
      RunnerContext* varContext = SetNodeValue(runner, GET_VAR_VALUE(node));
      context->value.vFloat = varContext->value.vFloat;
      break;
    }
    case DOUBLE: {
      context->dataType = DOUBLE;
      RunnerContext* varContext = SetNodeValue(runner, GET_VAR_VALUE(node));
      context->value.vDouble = varContext->value.vDouble;
      break;
    }
    case LONG: {
      context->dataType = LONG;
      RunnerContext* varContext = SetNodeValue(runner, GET_VAR_VALUE(node));
      context->value.vLong = varContext->value.vLong;
      break;
    }
    case CHAR: {
      context->dataType = CHAR;
      RunnerContext* varContext = SetNodeValue(runner, GET_VAR_VALUE(node));
      context->value.vChar = varContext->value.vChar;
      break;
    }
    case STRING: {
      context->dataType = STRING;
      RunnerContext* varContext = SetNodeValue(runner, GET_VAR_VALUE(node));
      context->value.vString = varContext->value.vString;
      break;
    }
  }
}

RunnerContext* GetContextByNodeId(Runner* runner, int nodeId){
  for (int i = 0; i < runner->totalContexts; i++){
    RunnerContext* context = &runner->contexts[i];
    if (context->node != NULL && context->node->id == nodeId){
      return &runner->contexts[i];
    }
  }

  return NULL;
}

RunnerContext* GetNextContext(Runner* runner) {
  for (int i = 0; i < runner->totalContexts; i++) {
    if (!runner->contextUsed[i]) {
      runner->contextUsed[i] = true;
      return &runner->contexts[i];  
    }
  }

  RUNTIME_ERROR("Ran out of contexts");
}

void PrintContext(RunnerContext* context){
  int type = (int) context->dataType;
  switch (type) {
    case BOOLEAN: {
      printf(">> %d\n", context->value.vBoolean);
      break;
    }
    case BYTE: {
      printf(">> %d\n", context->value.vByte);
      break;
    }
    case SHORT: {
      printf(">> %d\n", context->value.vShort);
      break;
    }
    case INT: {
      printf(">> %d\n", context->value.vInt);
      break;
    }
    case FLOAT: {
      printf(">> %f\n", context->value.vFloat);
      break;
    }
    case DOUBLE: {
      printf(">> %f\n", context->value.vDouble);
      break;
    }
    case LONG: {
      printf(">> %ld\n", context->value.vLong);
      break;
    }
    case CHAR: {
      printf(">> %d\n", context->value.vChar);
      break;
    }
    case STRING: {
      printf(">> %s\n", context->value.vString);
      break;
    }
  }
}