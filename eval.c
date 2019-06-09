#include <stdio.h>
#include <stdlib.h>
#include "include/eval.h"
#include "include/symbol.h"

function_t *functions;
symbol_t *sym_global, *sym_local;

ast_tree_t *main_procedure;

void runtime_error(char* msg) {
  fprintf(stderr, "runtime error: %s\n", msg);
  exit(0);
}

void eval(program_t* program, symbol_t* global) {
  if (program->function)
    functions = program->function; // local symbol은 evaluation할 때 함수 진입시 할당

  sym_global = global;
  main_procedure = program->statement;

  fprintf(stderr, "main procedure: start\n");
  while (main_procedure != NULL) {
    evaluation(main_procedure);
    main_procedure = main_procedure->stmtlist;
  }
  fprintf(stderr, "main procedure: end\n");
}

rvalue_t evaluation(ast_tree_t *node) {
  rvalue_t retval;

  if (node->token.type == 258) {
    rvalue_t nodeend = { .type = RVAL_INT, .value.ival = 0 };
    return nodeend;
  }

  token_data_t token_data = node->token;
  fprintf(stderr, "\nprocessing token %d\n", token_data.type);
  fprintf(stderr, "left: %p, right: %p\n", node->left, node->right);

  if (token_data.type == TT_INT) {
    if (EVAL_DEBUG)
      fprintf(stderr, "[i] eval type: int\n");

    retval.type = RVAL_INT;
    retval.value.ival = token_data.value.intval;
  }
  else if (token_data.type == TT_FLOAT) {
    if (EVAL_DEBUG)
      fprintf(stderr, "[i] eval type: float\n");

    retval.type = RVAL_REAL;
    retval.value.dval = token_data.value.doubleval;
  }
  else if (token_data.type == TT_ARITH_PLUS) {
    if (EVAL_DEBUG)
      fprintf(stderr,"[i] eval type: plus\n");

    rvalue_t lval = evaluation(node->left);
    rvalue_t rval = evaluation(node->right);

    if (lval.type == RVAL_INT && rval.type == RVAL_REAL) {
      retval.type = RVAL_REAL;
      retval.value.dval = (double)lval.value.ival + rval.value.dval;
    }
    else if (lval.type == RVAL_INT && rval.type == RVAL_INT) {
      retval.type = RVAL_INT;
      retval.value.ival = lval.value.ival + rval.value.ival;
    }
    else if (lval.type == RVAL_REAL && rval.type == RVAL_REAL) {
      retval.type = RVAL_REAL;
      retval.value.dval = lval.value.dval + rval.value.dval;
    }
    else if (lval.type == RVAL_REAL && rval.type == RVAL_INT) {
      retval.type = RVAL_REAL;
      retval.value.dval = lval.value.dval + (double)rval.value.ival;
    }
    else runtime_error("undefined return type value");
  }
  else if (token_data.type == TT_ARITH_MINUS) {
    if (EVAL_DEBUG)
      fprintf(stderr,"[i] eval type: minus\n");

    rvalue_t lval = evaluation(node->left);
    rvalue_t rval = evaluation(node->right);
    
    if (lval.type == RVAL_INT && rval.type == RVAL_REAL) {
      retval.type = RVAL_REAL;
      retval.value.dval = (double)lval.value.ival - rval.value.dval;
    }
    else if (lval.type == RVAL_INT && rval.type == RVAL_INT) {
      retval.type = RVAL_INT;
      retval.value.ival = lval.value.ival - rval.value.ival;
    }
    else if (lval.type == RVAL_REAL && rval.type == RVAL_REAL) {
      retval.type = RVAL_REAL;
      retval.value.dval = lval.value.dval - rval.value.dval;
    }
    else if (lval.type == RVAL_REAL && rval.type == RVAL_INT) {
      retval.type = RVAL_REAL;
      retval.value.dval = lval.value.dval - (double)rval.value.ival;
    }
    else runtime_error("undefined return type value");
  }
  else if (token_data.type == TT_ARITH_MULTI) {
    if (EVAL_DEBUG)
      fprintf(stderr,"[i] eval type: multi\n");

    rvalue_t lval = evaluation(node->left);
    rvalue_t rval = evaluation(node->right);
    
    if (lval.type == RVAL_INT && rval.type == RVAL_REAL) {
      retval.type = RVAL_REAL;
      retval.value.dval = (double)lval.value.ival * rval.value.dval;
    }
    else if (lval.type == RVAL_INT && rval.type == RVAL_INT) {
      retval.type = RVAL_INT;
      retval.value.ival = lval.value.ival * rval.value.ival;
    }
    else if (lval.type == RVAL_REAL && rval.type == RVAL_REAL) {
      retval.type = RVAL_REAL;
      retval.value.dval = lval.value.dval * rval.value.dval;
    }
    else if (lval.type == RVAL_REAL && rval.type == RVAL_INT) {
      retval.type = RVAL_REAL;
      retval.value.dval = lval.value.dval * (double)rval.value.ival;
    }
    else runtime_error("undefined return type value");
  }
  else if (token_data.type == TT_ARITH_DIVIDE) {
    if (EVAL_DEBUG)
      fprintf(stderr,"[i] eval type: divide\n");

    rvalue_t lval = evaluation(node->left);
    rvalue_t rval = evaluation(node->right);

    if (rval.type == RVAL_INT) {                // divide by zero
      if (rval.value.ival == 0)
        runtime_error("divide by zero");
    }
    else {
      if (rval.value.dval == 0)
        runtime_error("divide by zero");
    }
    
    if (lval.type == RVAL_INT && rval.type == RVAL_REAL) {
      retval.type = RVAL_REAL;
      retval.value.dval = (double)lval.value.ival / rval.value.dval;
    }
    else if (lval.type == RVAL_INT && rval.type == RVAL_INT) {
      if (lval.value.ival % rval.value.ival != 0) { // 소수점이 발생하는지 비교, 발생시 double로 변경
        retval.type = RVAL_REAL;
        retval.value.dval = (double)lval.value.ival / (double)rval.value.ival;  
      }
      else { // 발생하지 않으면 integer
        retval.type = RVAL_INT;
        retval.value.ival = lval.value.ival / rval.value.ival;
      }
    }
    else if (lval.type == RVAL_REAL && rval.type == RVAL_REAL) {
      retval.type = RVAL_REAL;
      retval.value.dval = lval.value.dval / rval.value.dval;
    }
    else if (lval.type == RVAL_REAL && rval.type == RVAL_INT) {
      retval.type = RVAL_REAL;
      retval.value.dval = lval.value.dval / (double)rval.value.ival;
    }
    else runtime_error("undefined return type value");
  }
  else if (token_data.type == TT_ARITH_UMINUS) {
    if (EVAL_DEBUG)
      fprintf(stderr, "[i] eval type: uminus\n");

    rvalue_t val = evaluation(node->left);

    if (val.type == RVAL_INT) {
      retval.type = RVAL_INT;
      retval.value.ival = -val.value.ival;
    }
    else {
      retval.type = RVAL_REAL;
      retval.value.dval = -val.value.dval;
    }
  }
  else if (token_data.type == TT_ASSIGN) {
    if (EVAL_DEBUG)
      fprintf(stderr, "[i] eval type: assign\n");

    int lval_symid = node->left->token.value.symid;
    fprintf(stderr, "[i] symid: %d\n", lval_symid);
    rvalue_t rval = evaluation(node->right);

    if (stack_isempty()) { // 스택이 비어있으면 (main procedure)
      fprintf(stderr, "[i] global assigning, symtableTypeDef: %d\n", sym_global[lval_symid].type);
      symbol_t* sym = get_symbol(lval_symid, sym_global);

      if (rval.type == RVAL_REAL) {
        sym->type = TYPE_VARIABLE_DOUBLE;
        sym->value.real_constant = rval.value.dval;

        retval.type = RVAL_REAL;
        retval.value.dval = rval.value.dval;
      }
      else if (rval.type == RVAL_INT) {
        sym->type = TYPE_VARIABLE_INT;
        sym->value.integer_constant = rval.value.ival;

        retval.type = RVAL_INT;
        retval.value.ival = rval.value.ival;
      }

      fprintf(stderr, "[i] global assigning, after symtableTypeDef: %d\n", sym_global[lval_symid].type);

      sym->assigned = true;
    }
    else { // subroutine 내부일 경우
      fprintf(stderr, "[i] local assigning\n");
      if (!symbol_exists_on_table(lval_symid, sym_local)) { // 심볼id가 null이면 (할당 안됐으면)
        symbol_t* sym = get_symbol(lval_symid, sym_global);

        if (rval.type == RVAL_REAL) {
          sym->type = TYPE_VARIABLE_DOUBLE;
          sym->value.real_constant = rval.value.dval; 

          retval.type = RVAL_REAL;
          retval.value.dval = rval.value.dval;
        }
        else if (rval.type == RVAL_INT) {
          sym->type = TYPE_VARIABLE_INT;
          sym->value.integer_constant = rval.value.ival;

          retval.type = RVAL_INT;
          retval.value.ival = rval.value.ival;
        }

        sym->assigned = true;
      }
      else { // 로컬에 할당
        symbol_t* sym = get_symbol(lval_symid, sym_local);

        sym->assigned = true;

        if (rval.type == RVAL_REAL) {
          sym->type = TYPE_VARIABLE_DOUBLE;
          sym->value.real_constant = rval.value.dval;

          retval.type = RVAL_REAL;
          retval.value.dval = rval.value.dval;
        }
        else if (rval.type == RVAL_INT) {
          sym->type = TYPE_VARIABLE_INT;
          sym->value.integer_constant = rval.value.ival;

          retval.type = RVAL_INT;
          retval.value.ival = rval.value.ival;
        }
      }
    }

    print_symboltable(sym_global);
  }
  else if (token_data.type == TT_COMP_LGT) {
    if (EVAL_DEBUG)
      fprintf(stderr, "[i] eval type: comp_lgt\n");

    rvalue_t lval = evaluation(node->left);
    rvalue_t rval = evaluation(node->right);
    
    retval.type = RVAL_INT;

    if (lval.type == RVAL_INT && rval.type == RVAL_REAL)
      retval.value.ival = lval.value.ival > rval.value.dval;
    else if (lval.type == RVAL_INT && rval.type == RVAL_INT)
      retval.value.ival = lval.value.ival > rval.value.ival;
    else if (lval.type == RVAL_REAL && rval.type == RVAL_REAL)
      retval.value.ival = lval.value.dval > rval.value.dval;
    else if (lval.type == RVAL_REAL && rval.type == RVAL_INT)
      retval.value.ival = lval.value.dval > rval.value.ival;
    else runtime_error("undefined return type value");
  }
  else if (token_data.type == TT_COMP_LEGT) {
    if (EVAL_DEBUG)
      fprintf(stderr, "[i] eval type: comp_legt\n");

    rvalue_t lval = evaluation(node->left);
    rvalue_t rval = evaluation(node->right);
    
    retval.type = RVAL_INT;

    if (lval.type == RVAL_INT && rval.type == RVAL_REAL)
      retval.value.ival = lval.value.ival >= rval.value.dval;
    else if (lval.type == RVAL_INT && rval.type == RVAL_INT)
      retval.value.ival = lval.value.ival >= rval.value.ival;
    else if (lval.type == RVAL_REAL && rval.type == RVAL_REAL)
      retval.value.ival = lval.value.dval >= rval.value.dval;
    else if (lval.type == RVAL_REAL && rval.type == RVAL_INT)
      retval.value.ival = lval.value.dval >= rval.value.ival;
    else runtime_error("undefined return type value");
  }
  else if (token_data.type == TT_COMP_RGT) {
    if (EVAL_DEBUG)
      fprintf(stderr, "[i] eval type: comp_rgt\n");

    rvalue_t lval = evaluation(node->left);
    rvalue_t rval = evaluation(node->right);
    
    retval.type = RVAL_INT;

    if (lval.type == RVAL_INT && rval.type == RVAL_REAL)
      retval.value.ival = lval.value.ival < rval.value.dval;
    else if (lval.type == RVAL_INT && rval.type == RVAL_INT)
      retval.value.ival = lval.value.ival < rval.value.ival;
    else if (lval.type == RVAL_REAL && rval.type == RVAL_REAL)
      retval.value.ival = lval.value.dval < rval.value.dval;
    else if (lval.type == RVAL_REAL && rval.type == RVAL_INT)
      retval.value.ival = lval.value.dval < rval.value.ival;
    else runtime_error("undefined return type value");
  }
  else if (token_data.type == TT_COMP_REGT) {
    if (EVAL_DEBUG)
      fprintf(stderr, "[i] eval type: comp_regt\n");

    rvalue_t lval = evaluation(node->left);
    rvalue_t rval = evaluation(node->right);
    
    retval.type = RVAL_INT;

    if (lval.type == RVAL_INT && rval.type == RVAL_REAL)
      retval.value.ival = lval.value.ival <= rval.value.dval;
    else if (lval.type == RVAL_INT && rval.type == RVAL_INT)
      retval.value.ival = lval.value.ival <= rval.value.ival;
    else if (lval.type == RVAL_REAL && rval.type == RVAL_REAL)
      retval.value.ival = lval.value.dval <= rval.value.dval;
    else if (lval.type == RVAL_REAL && rval.type == RVAL_INT)
      retval.value.ival = lval.value.dval <= rval.value.ival;
    else runtime_error("undefined return type value");
  }
  else if (token_data.type == TT_COMP_EQUALS) {
    if (EVAL_DEBUG)
      fprintf(stderr, "[i] eval type: comp_equals\n");

    rvalue_t lval = evaluation(node->left);
    rvalue_t rval = evaluation(node->right);
    
    retval.type = RVAL_INT;

    if (lval.type == RVAL_INT && rval.type == RVAL_REAL)
      retval.value.ival = lval.value.ival == rval.value.dval;
    else if (lval.type == RVAL_INT && rval.type == RVAL_INT)
      retval.value.ival = lval.value.ival == rval.value.ival;
    else if (lval.type == RVAL_REAL && rval.type == RVAL_REAL)
      retval.value.ival = lval.value.dval == rval.value.dval;
    else if (lval.type == RVAL_REAL && rval.type == RVAL_INT)
      retval.value.ival = lval.value.dval == rval.value.ival;
    else runtime_error("undefined return type value");
  }
  else if (token_data.type == TT_COMP_NOTEQUALS) {
    if (EVAL_DEBUG)
      fprintf(stderr, "[i] eval type: comp_notequals\n");

    rvalue_t lval = evaluation(node->left);
    rvalue_t rval = evaluation(node->right);
    
    retval.type = RVAL_INT;

    if (lval.type == RVAL_INT && rval.type == RVAL_REAL)
      retval.value.ival = lval.value.ival != rval.value.dval;
    else if (lval.type == RVAL_INT && rval.type == RVAL_INT)
      retval.value.ival = lval.value.ival != rval.value.ival;
    else if (lval.type == RVAL_REAL && rval.type == RVAL_REAL)
      retval.value.ival = lval.value.dval != rval.value.dval;
    else if (lval.type == RVAL_REAL && rval.type == RVAL_INT)
      retval.value.ival = lval.value.dval != rval.value.ival;
    else runtime_error("undefined return type value");
  }
  else if (token_data.type == TT_PROCEDURE_CALL) {
    if (EVAL_DEBUG)
      fprintf(stderr, "[i] eval type: pcall\n");

    // 기존 로컬 심볼테이블을 stack에 넣기 (루틴 개선해야함, 첫번째 NULL일 경우)
    push_symboltable(sym_local);

    function_t *func;
    ast_tree_t *arglist, *req_arglist;

    func = get_function(token_data.value.funstr, functions);
    
    if (func == NULL) runtime_error("tried to call non-exist function");
    
    fprintf(stderr, "[!] functionstr: %s\n", token_data.value.funstr);

    sym_local = func->ast->localvar;
    req_arglist = func->arguments;
    arglist = node->arglist;

    fprintf(stderr, "[pcall] func %p, localarg %p, arglist %p, sym_local %p\n", func, req_arglist, arglist, sym_local);

    if (func->defined) {
      fprintf(stderr, "  defined function %s\n", func->name);
      // arguments에 있는 변수명 순서에 따라 들어가는 symbol을 해당 이름으로 로컬에 카피함
      while (arglist != NULL && req_arglist != NULL) {
        // 기존 symbol table에서 해당 expression 값을 가져옴 (스택 비었으면 글로벌)
        fprintf(stderr, "[pcall] on arglist\n");

        int sidx;
        token_data_t td;
        rvalue_t val;

        sidx = insert_symbol(req_arglist->token.value.argval, &sym_local); // 로컬에 심볼 자리 만들고
        val = evaluation(arglist); // 현재 args를 evaluation함

        if (val.type == RVAL_INT) {
          td.type = TYPE_VARIABLE_INT;
          td.value.intval = val.value.ival;
          fprintf(stderr, "[pcall] argval: %d\n", val.value.ival);
        }
        else {
          td.type = TYPE_VARIABLE_DOUBLE;
          td.value.doubleval = val.value.dval;
          fprintf(stderr, "[pcall] argval: %f\n", val.value.dval);
        }

        insert_symbol_value(sidx, td, &sym_local);

        print_symboltable(sym_local);

        req_arglist = req_arglist->arglist;
        arglist = arglist->arglist;
      }

      rvalue_t fval = evaluation(func->ast); // run subroutine

      if (fval.type == RVAL_INT) {
        retval.type = RVAL_INT;
        retval.value.ival = fval.value.ival;
      }
      else {
        retval.type = RVAL_REAL;
        retval.value.dval = fval.value.dval;
      }
    }
    else runtime_error("cannot use undefined function"); // 정의되지 않은 함수 사용

    if (arglist != NULL || req_arglist != NULL)
      runtime_error("argument does not match");

    if (!stack_isempty())
      sym_local = pop_symboltable(); // 스택이 차있다면 1-level 내려오기
    else sym_local = NULL; // 아니면 NULL
  }
  else if (token_data.type == TT_PRINT) {
    if (EVAL_DEBUG)
      fprintf(stderr, "[i] eval type: print\n");

    rvalue_t val = evaluation(node->left);

    if (val.type == RVAL_INT)
      printf("%d\n", val.value.ival);
    else
      printf("%f\n", val.value.dval);
  }
  else if (token_data.type == TT_IF || token_data.type == TT_IFELSE) {
    if (EVAL_DEBUG)
      fprintf(stderr, "[i] eval type: if\n");

    // condition을 추출해서 if로 돌려버리기 (1 = true, 0 = false)
    ast_tree_t* condition = node->condition;
    ast_tree_t* if_true = node->left;
    ast_tree_t* if_false = node->right;

    fprintf(stderr, "[branch] cond %p, true %p, false %p\n", condition, if_true, if_false);

    int cond_result = evaluation(condition).value.ival;

    // if들어가면 node->left실행
    if (cond_result > 0 || cond_result < 0) {
      fprintf(stderr, "[branch] condition:true, goto %p\n", if_true);
      evaluation(if_true); // left만 실행하고 나와야함
    }
    else { // 안들어가면 node->right 실행
      if (if_false != NULL) { // node->right가 null이면 if만 있는걸로 처리
        fprintf(stderr, "[branch] condition:false, goto %p\n", if_false);
        evaluation(if_false);
      }
      else fprintf(stderr, "[branch] condition:false, no else\n");
    }

    fprintf(stderr, "[branch] out\n");
  }
  else if (token_data.type == TT_WHILE) {
    if (EVAL_DEBUG)
      fprintf(stderr, "[i] eval type: while\n");

    ast_tree_t* condition = node->condition;
    ast_tree_t* body = node->left;

    fprintf(stderr, "[loop] cond %p, body %p\n", condition, body);
    
    while (true) {
      int cond_result = evaluation(condition).value.ival;
      fprintf(stderr, "[loop] condition result: %d\n", cond_result);

      if (cond_result > 0 || cond_result < 0)
        evaluation(body);
      else break;
    }

    fprintf(stderr, "[loop] out");
  }
  else if (token_data.type == TT_RETURN) {
    if (EVAL_DEBUG)
      fprintf(stderr, "[i] eval type: return, notimpl\n");

    rvalue_t val = evaluation(node->left);

    if (val.type == RVAL_INT) {
      retval.type = RVAL_INT;
      retval.value.ival = val.value.ival;
    }
    else {
      retval.type = RVAL_REAL;
      retval.value.dval = val.value.dval;
    }
  }
  else if (token_data.type == TT_ID) {
    if (EVAL_DEBUG)
      fprintf(stderr, "[i] eval type: id\n");

    int symid = token_data.value.symid;

    // local stack이 차있으면 로컬에서 찾고
    if (!stack_isempty()) {
      if (symbol_exists_on_table(symid, sym_local)) { // 로컬에 심볼이 있음
        symbol_t* sym = get_symbol(symid, sym_local);
        
        if (sym->type == TYPE_VARIABLE_INT) {
          retval.type = RVAL_INT;
          retval.value.ival = sym->value.integer_constant;
        }
        else {
          retval.type = RVAL_REAL;
          retval.value.dval = sym->value.real_constant;
        }
      }
      else { // 로컬에 심볼이 없음: 글로벌에서 찾기
        symbol_t* sym = get_symbol(symid, sym_global);

        if (sym->type == TYPE_VARIABLE_INT) {
          retval.type = RVAL_INT;
          retval.value.ival = sym->value.integer_constant;
        }
        else {
          retval.type = RVAL_REAL;
          retval.value.dval = sym->value.real_constant;
        }
      }
    }
    else { // stack이 비어있으면 글로벌에서 찾기
      symbol_t* sym = get_symbol(symid, sym_global);

      if (sym->type == TYPE_VARIABLE_INT) {
        retval.type = RVAL_INT;
        retval.value.ival = sym->value.integer_constant;
      }
      else {
        retval.type = RVAL_REAL;
        retval.value.dval = sym->value.real_constant;
      }
    }
  }
  else
    runtime_error("unknown data type, failed to evaluation");

  return retval;
}