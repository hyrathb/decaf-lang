%{
  #include <stdio.h>
  #include <math.h>
  #include <stdlib.h>
  #include <stdarg.h>
  #include "defs.h"
  #include "symtable.h"
  
  #define new_node (malloc(sizeof(struct semantics)))
  
  struct semantics*head;
  int yylex (void);
  void yyerror (char const *, ...);
  extern int yylineno;
  extern FILE *yyin;
%}

%token TSTRINGCONST TKVOID TKINT TKDOUBLE TKBOOL TKSTRING TKCLASS TKINTERFACE TKNULL TKTHIS TKEXTENDS
%token TKIMPLEMENTS TKFOR TKWHILE TKIF TKELSE TKRETURN TKBREAK TKNEW TKNEWARRAY TKPRINT TKREADINTEGER
%token TKREADLINE TBOOLCONST
%token TPPOINT TPSEP TPCOMMA TPLBB TPRBB TIDENT TINTCONST TDOUBLECONST

%right TOASSIGN
%left TOOR
%left TOAND
%left TOLT TOLE TOGT TOGE TOEQU TONE
%left TOPLUS TOMINUS
%left TOMUL TODIV TOIDIV
%right TONOT
%left TORSB
%right TOLSB
%left TORB
%right TOLB

%start program

%glr-parser


%% /* Grammar rules and actions follow.  */

constant:
    TINTCONST{$$=new_node; $$->type = C_ICONST; $$->i_val=strtol($1->text, NULL, 0); free($1->text); free($1);} 
    | 
    TDOUBLECONST {$$=new_node; $$->type = C_DCONST; $$->d_val=strtof($1->text, NULL); free($1->text); free($1);}
    | 
    TBOOLCONST {$$=new_node; $$->type = C_ICONST; $$->i_val= !strcmp($1->text, "true"); free($1->text); free($1);}
    | 
    TSTRINGCONST {$$=new_node; $$->type = C_SCONST; $$->s_val=$1->text; free($1);}
    | 
    TKNULL{$$ = new_node; $$->text=NULL; $$->type = C_NULL; $$->i_val=0;};


tformals :
    var {$$=new_node;$$->type=C_TFORMALS; $$->tformals = malloc(sizeof(struct tformals)); $$->tformals->var = $1; $$->tformals->next=NULL; $$->tformals->last=$$->tformals;}
    |
    tformals TPCOMMA var {$$=$1; $$->tformals->last->next= malloc(sizeof(struct tformals)); $$->tformals->last=$$->tformals->last->next; $$->tformals->last->next = NULL; $$->tformals->last->var = $3;};

formals :
    %empty {$$=new_node; $$->type=C_FORMALS; $$->formals = malloc(sizeof (struct formals)); $$->formals->tformals = NULL;}
    |
    tformals {$$=new_node; $$->type=C_FORMALS; $$->formals = malloc(sizeof (struct formals)); $$->formals->tformals = $1;};
    
type :
    TKINT {$$=new_node; $$->type=C_TYPE; $$->vtype = malloc(sizeof (struct type)); $$->vtype->is_basic = 1; $$->vtype->is_array=0; $$->vtype->btype = D_INT;}
    | 
    TKBOOL {$$=new_node; $$->type=C_TYPE; $$->vtype = malloc(sizeof (struct type)); $$->vtype->is_basic = 1; $$->vtype->is_array=0; $$->vtype->btype = D_BOOL;}
    | 
    TKDOUBLE {$$=new_node; $$->type=C_TYPE; $$->vtype = malloc(sizeof (struct type)); $$->vtype->is_basic = 1; $$->vtype->is_array=0; $$->vtype->btype = D_DOUBLE;}
    | 
    TKSTRING {$$=new_node; $$->type=C_TYPE; $$->vtype = malloc(sizeof (struct type)); $$->vtype->is_basic = 1; $$->vtype->is_array=0; $$->vtype->btype = D_STRING;}
    | 
    TIDENT {$$=new_node; $1->type_ok=1; $1->type=C_IDENT; $$->type=C_TYPE; $$->vtype = malloc(sizeof (struct type)); $$->vtype->is_basic = 0; $$->vtype->btype = D_CLASS; $$->vtype->is_array=0; $$->vtype->id = $1;}
    | 
    type TOLSB TORSB {$$=new_node; $$->type=C_TYPE; $$->vtype = malloc(sizeof (struct type)); $$->vtype->is_basic = 0; $$->vtype->is_array=0; $$->vtype->btype = D_ARRAY; $$->vtype->arr_type = $1;};

var :
    type TIDENT {$$=new_node; $2->type_ok=1; $2->type=C_IDENT; $$->type=C_VAR; $$->var = malloc(sizeof (struct var)); $$->var->type = $1; $$->var->id=$2;}; /*NEED TO CHECK TYPEOK*/

vardefine :
    var TPSEP {$$=new_node; $$->type=C_VARDEFINE; $$->vardefine = malloc(sizeof (struct vardefine)); $$->vardefine->var = $1;};

vardefines :
    %empty {$$=new_node; $$->type=C_VARDEFINES; $$->vardefines = malloc(sizeof (struct vardefines)); $$->vardefines->vardefine = NULL; $$->vardefines->next = NULL; $$->vardefines->last=$$->vardefines;}
    | 
    vardefines vardefine{$$=$1; $$->vardefines->last->next = malloc(sizeof (struct vardefines)); $$->vardefines->last=$$->vardefines->last->next; $$->vardefines->last->next = NULL; $$->vardefines->last->vardefine = $2;};

actuals :
    %empty {$$=new_node; $$->type=C_ACTUALS; $$->actuals=malloc(sizeof(struct actuals)); $$->actuals->expr_with_comma = NULL;} 
    |
    exprss {$$=new_node; $$->type=C_ACTUALS; $$->actuals=malloc(sizeof(struct actuals)); $$->actuals->expr_with_comma = $1;} ;
    
call :
    TIDENT TOLB actuals TORB {$$=new_node; $1->type_ok=1; $1->type=C_IDENT; $$->type=C_CALL; $$->call=malloc(sizeof(struct call)); $$->call->is_member=0; $$->call->lvalue = NULL; $$->call->id = $1; $$->call->actuals=$3;}
    |
    lvalue TPPOINT TIDENT TOLB actuals TORB {$$=new_node; $3->type_ok=1; $3->type=C_IDENT;  $$->type=C_CALL; $$->call=malloc(sizeof(struct call)); $$->call->is_member=1; $$->call->lvalue = $1; $$->call->id = $3; $$->call->actuals=$5;};

lvalue :
    TIDENT {$$=new_node; $1->type_ok=1; $1->type=C_IDENT; $$->type=C_LVALUE; $$->lvalue=malloc(sizeof(struct lvalue)); $$->lvalue->lvalue_type=LVAL_IDENT; $$->lvalue->id = $1; $$->lvalue->expr = NULL;}
    |
    lvalue TPPOINT TIDENT {$$=new_node; $3->type_ok=1; $3->type=C_IDENT; $$->type=C_LVALUE; $$->lvalue=malloc(sizeof(struct lvalue)); $$->lvalue->lvalue_type=LVAL_MEMBER; $$->lvalue->lvalue = $1; $$->lvalue->id = $3; $$->lvalue->expr = NULL;}
    |
    lvalue TOLSB expr TORSB {$$=new_node; $$->type=C_LVALUE; $$->lvalue=malloc(sizeof(struct lvalue)); $$->lvalue->lvalue_type=LVAL_ARRAY; $$->lvalue->id = NULL; $$->lvalue->lvalue = $1; $$->lvalue->expr=$3;};
    
expr :               /*NEED TO CHECK TYPE*/
    lvalue TOASSIGN expr {$$=new_node; $$->type=C_EXPR; $$->expr=malloc(sizeof(struct expr)); $$->expr->expr_type=EXPR_ASSIGN; $$->expr->constant= NULL; $$->expr->lvalue=$1; $$->expr->expr1=$3; $$->expr->expr2=NULL; $$->expr->call=NULL; $$->expr->id=NULL; $$->expr->type=NULL;}
    |
    constant {$$=new_node; $$->type=C_EXPR; $$->expr=malloc(sizeof(struct expr)); $$->expr->expr_type=EXPR_CONST; $$->expr->constant= $1; $$->expr->lvalue=NULL; $$->expr->expr1=NULL; $$->expr->expr2=NULL; $$->expr->call=NULL; $$->expr->id=NULL; $$->expr->type=NULL;}
    |
    lvalue {$$=new_node; $$->type=C_EXPR; $$->expr=malloc(sizeof(struct expr)); $$->expr->expr_type=EXPR_LVAL; $$->expr->constant= NULL; $$->expr->lvalue=$1; $$->expr->expr1=NULL; $$->expr->expr2=NULL; $$->expr->call=NULL; $$->expr->id=NULL; $$->expr->type=NULL;}
    |
    TKTHIS {$$=new_node; $$->type=C_EXPR; $$->expr=malloc(sizeof(struct expr)); $$->expr->expr_type=EXPR_THIS; $$->expr->constant= NULL; $$->expr->lvalue=NULL; $$->expr->expr1=NULL; $$->expr->expr2=NULL; $$->expr->call=NULL; $$->expr->id=NULL; $$->expr->type=NULL;}
    |
    call {$$=new_node; $$->type=C_EXPR; $$->expr=malloc(sizeof(struct expr)); $$->expr->expr_type=EXPR_CALL; $$->expr->constant= NULL; $$->expr->lvalue=NULL; $$->expr->expr1=NULL; $$->expr->expr2=NULL; $$->expr->call=$1; $$->expr->id=NULL; $$->expr->type=NULL;}
    |
    TOLB expr TORB {$$=new_node; $$->expr=malloc(sizeof(struct expr)); $$->type=C_EXPR; $$->expr->expr_type=EXPR_PRIORITY; $$->expr->constant= NULL; $$->expr->lvalue=NULL; $$->expr->expr1=$2; $$->expr->expr2=NULL; $$->expr->call=NULL; $$->expr->id=NULL; $$->expr->type=NULL;}
    |
    expr TOPLUS expr {$$=new_node; $$->type=C_EXPR; $$->expr=malloc(sizeof(struct expr)); $$->expr->expr_type=EXPR_PLUS; $$->expr->constant= NULL; $$->expr->lvalue=NULL; $$->expr->expr1=$1; $$->expr->expr2=$3; $$->expr->call=NULL; $$->expr->id=NULL; $$->expr->type=NULL;}
    |
    expr TOMINUS expr {$$=new_node; $$->type=C_EXPR; $$->expr=malloc(sizeof(struct expr)); $$->expr->expr_type=EXPR_MINUS; $$->expr->constant= NULL; $$->expr->lvalue=NULL; $$->expr->expr1=$1; $$->expr->expr2=$3; $$->expr->call=NULL; $$->expr->id=NULL; $$->expr->type=NULL;}
    |
    expr TOMUL expr {$$=new_node; $$->type=C_EXPR; $$->expr=malloc(sizeof(struct expr)); $$->expr->expr_type=EXPR_MUL; $$->expr->constant= NULL; $$->expr->lvalue=NULL; $$->expr->expr1=$1; $$->expr->expr2=$3; $$->expr->call=NULL; $$->expr->id=NULL; $$->expr->type=NULL;}
    |
    expr TODIV expr {$$=new_node; $$->type=C_EXPR; $$->expr=malloc(sizeof(struct expr)); $$->expr->expr_type=EXPR_DIV; $$->expr->constant= NULL; $$->expr->lvalue=NULL; $$->expr->expr1=$1; $$->expr->expr2=$3; $$->expr->call=NULL; $$->expr->id=NULL; $$->expr->type=NULL;}
    |
    expr TOIDIV expr {$$=new_node; $$->type=C_EXPR; $$->expr=malloc(sizeof(struct expr)); $$->expr->expr_type=EXPR_IDIV; $$->expr->constant= NULL; $$->expr->lvalue=NULL; $$->expr->expr1=$1; $$->expr->expr2=$3; $$->expr->call=NULL; $$->expr->id=NULL; $$->expr->type=NULL;}
    |
    expr TOLT expr {$$=new_node; $$->type=C_EXPR; $$->expr=malloc(sizeof(struct expr)); $$->expr->expr_type=EXPR_LT; $$->expr->constant= NULL; $$->expr->lvalue=NULL; $$->expr->expr1=$1; $$->expr->expr2=$3; $$->expr->call=NULL; $$->expr->id=NULL; $$->expr->type=NULL;}
    |
    expr TOLE expr {$$=new_node; $$->type=C_EXPR; $$->expr=malloc(sizeof(struct expr)); $$->expr->expr_type=EXPR_LE; $$->expr->constant= NULL; $$->expr->lvalue=NULL; $$->expr->expr1=$1; $$->expr->expr2=$3; $$->expr->call=NULL; $$->expr->id=NULL; $$->expr->type=NULL;}
    |
    expr TOGT expr {$$=new_node; $$->type=C_EXPR; $$->expr=malloc(sizeof(struct expr)); $$->expr->expr_type=EXPR_GT; $$->expr->constant= NULL; $$->expr->lvalue=NULL; $$->expr->expr1=$1; $$->expr->expr2=$3; $$->expr->call=NULL; $$->expr->id=NULL; $$->expr->type=NULL;}
    |
    expr TOGE expr {$$=new_node; $$->type=C_EXPR; $$->expr=malloc(sizeof(struct expr)); $$->expr->expr_type=EXPR_GE; $$->expr->constant= NULL; $$->expr->lvalue=NULL; $$->expr->expr1=$1; $$->expr->expr2=$3; $$->expr->call=NULL; $$->expr->id=NULL; $$->expr->type=NULL;}
    |
    expr TOEQU expr {$$=new_node; $$->type=C_EXPR; $$->expr=malloc(sizeof(struct expr)); $$->expr->expr_type=EXPR_EQU; $$->expr->constant= NULL; $$->expr->lvalue=NULL; $$->expr->expr1=$1; $$->expr->expr2=$3; $$->expr->call=NULL; $$->expr->id=NULL; $$->expr->type=NULL;}
    |
    expr TONE expr {$$=new_node; $$->type=C_EXPR; $$->expr=malloc(sizeof(struct expr)); $$->expr->expr_type=EXPR_NE; $$->expr->constant= NULL; $$->expr->lvalue=NULL; $$->expr->expr1=$1; $$->expr->expr2=$3; $$->expr->call=NULL; $$->expr->id=NULL; $$->expr->type=NULL;}
    |
    expr TOAND expr {$$=new_node; $$->type=C_EXPR; $$->expr=malloc(sizeof(struct expr)); $$->expr->expr_type=EXPR_AND; $$->expr->constant= NULL; $$->expr->lvalue=NULL; $$->expr->expr1=$1; $$->expr->expr2=$3; $$->expr->call=NULL; $$->expr->id=NULL; $$->expr->type=NULL;}
    |
    expr TOOR expr {$$=new_node; $$->type=C_EXPR; $$->expr=malloc(sizeof(struct expr)); $$->expr->expr_type=EXPR_OR; $$->expr->constant= NULL; $$->expr->lvalue=NULL; $$->expr->expr1=$1; $$->expr->expr2=$3; $$->expr->call=NULL; $$->expr->id=NULL; $$->expr->type=NULL;}
    |
    TONOT expr {$$=new_node; $$->type=C_EXPR; $$->expr=malloc(sizeof(struct expr)); $$->expr->expr_type=EXPR_NOT; $$->expr->constant= NULL; $$->expr->lvalue=NULL; $$->expr->expr1=$2; $$->expr->expr2=NULL; $$->expr->call=NULL; $$->expr->id=NULL; $$->expr->type=NULL;}
    |
    TKREADINTEGER TOLB TORB {$$=new_node; $$->type=C_EXPR; $$->expr=malloc(sizeof(struct expr)); $$->expr->expr_type=EXPR_READINTEGER; $$->expr->constant= NULL; $$->expr->lvalue=NULL; $$->expr->expr1=NULL; $$->expr->expr2=NULL; $$->expr->call=NULL; $$->expr->id=NULL; $$->expr->type=NULL;}
    |
    TKREADLINE TOLB TORB {$$=new_node; $$->type=C_EXPR; $$->expr=malloc(sizeof(struct expr)); $$->expr->expr_type=EXPR_READLINE; $$->expr->constant= NULL; $$->expr->lvalue=NULL; $$->expr->expr1=NULL; $$->expr->expr2=NULL; $$->expr->call=NULL; $$->expr->id=NULL; $$->expr->type=NULL;}
    |
    TKNEW TOLB TIDENT TORB {$$=new_node; $3->type_ok=1; $3->type=C_IDENT; $$->type=C_EXPR; $$->expr=malloc(sizeof(struct expr)); $$->expr->expr_type=EXPR_NEW; $$->expr->constant= NULL; $$->expr->lvalue=NULL; $$->expr->expr1=NULL; $$->expr->expr2=NULL; $$->expr->call=NULL; $$->expr->id=$3; $$->expr->type=NULL;}
    |
    TKNEWARRAY TOLB expr TPCOMMA type TORB{$$=new_node; $$->type=C_EXPR; $$->expr=malloc(sizeof(struct expr)); $$->expr->expr_type=EXPR_NEWARRAY; $$->expr->constant= NULL; $$->expr->lvalue=NULL; $$->expr->expr1=$3; $$->expr->expr2=NULL; $$->expr->call=NULL; $$->expr->id=NULL; $$->expr->type=$5;} ;
    
exprss :
    expr {$$=new_node; $$->type=C_EXPR_WITH_COMMA; $$->expr_with_comma = malloc(sizeof(struct expr_with_comma)); $$->expr_with_comma->expr = $1; $$->expr_with_comma->next = NULL; $$->expr_with_comma->last=$$->expr_with_comma;}
    |
    exprss TPCOMMA expr {$$=$1; $$->expr_with_comma->last->next = malloc(sizeof(struct expr_with_comma)); $$->expr_with_comma->last=$$->expr_with_comma->last->next; $$->expr_with_comma->last->expr = $3; $$->expr_with_comma->last->next = NULL;};
    
ifstmt :
    TKIF TOLB expr TORB stmt {$$=new_node; $$->type=C_IFSTM; $$->if_stm = malloc(sizeof(struct if_stm)); $$->if_stm->expr = $3; $$->if_stm->stm1 = $5; $$->if_stm->stm2=NULL;}
    |
    TKIF TOLB expr TORB stmt TKELSE stmt {$$=new_node; $$->type=C_IFSTM; $$->if_stm = malloc(sizeof(struct if_stm)); $$->if_stm->expr = $3; $$->if_stm->stm1 = $5; $$->if_stm->stm2=$7;};
    
whilestmt :
    TKWHILE TOLB expr TORB stmt {$$=new_node; $$->type=C_WHILESTM; $$->whilestm = malloc(sizeof(struct whilestm)); $$->whilestm->expr = $3; $$->whilestm->stm = $5;};

expr_or_not:
    %empty {$$=new_node; $$->type=C_EXPR_OR_NOT; $$->expr_or_not = malloc(sizeof(struct expr_or_not)); $$->expr_or_not->expr = NULL;}
    |
    expr {$$=new_node; $$->type=C_EXPR_OR_NOT; $$->expr_or_not = malloc(sizeof(struct expr_or_not)); $$->expr_or_not->expr = $1;};
    
forstmt :
    TKFOR TOLB expr_or_not TPSEP expr TPSEP expr_or_not TORB stmt{$$=new_node; $$->type=C_FORSTM; $$->forstm = malloc(sizeof(struct forstm)); $$->forstm->expr_or_not1=$3; $$->forstm->expr=$5; $$->forstm->expr_or_not2=$7; $$->forstm->stm=$9;};
    
returnstmt :
    TKRETURN expr_or_not TPSEP {$$=new_node; $$->type=C_RETSTM; $$->returnstm = malloc(sizeof(struct returnstm)); $$->returnstm->expr_or_not=$2;};

breakstmt :
    TKBREAK TPSEP {$$=new_node; $$->type=C_BREAKSTM; $$->i_val=0;};

printstmt :
    TKPRINT TOLB expr TORB TPSEP {$$=new_node; $$->type=C_PRINTSTM; $$->printstm = malloc(sizeof(struct printstm)); $$->printstm->expr=$3;};
    
stmt :
    TPSEP {$$=new_node; $$->type=C_STM; $$->stm = malloc(sizeof(struct stm)); $$->stm->stm_type=STM_EMPTY; $$->stm->s_stm=NULL;}
    |
    expr TPSEP {$$=new_node; $$->type=C_STM; $$->stm = malloc(sizeof(struct stm)); $$->stm->stm_type=STM_EXPR; $$->stm->expr=$1;}
    |
    ifstmt {$$=new_node; $$->type=C_STM; $$->stm = malloc(sizeof(struct stm)); $$->stm->stm_type=STM_IF; $$->stm->s_stm=$1;}
    |
    whilestmt {$$=new_node; $$->type=C_STM; $$->stm = malloc(sizeof(struct stm)); $$->stm->stm_type=STM_WHILE; $$->stm->s_stm=$1;}
    |
    forstmt {$$=new_node; $$->type=C_STM; $$->stm = malloc(sizeof(struct stm)); $$->stm->stm_type=STM_FOR; $$->stm->s_stm=$1;}
    |
    breakstmt {$$=new_node; $$->type=C_STM; $$->stm = malloc(sizeof(struct stm)); $$->stm->stm_type=STM_BREAK; $$->stm->s_stm=$1;}
    |
    returnstmt {$$=new_node; $$->type=C_STM; $$->stm = malloc(sizeof(struct stm)); $$->stm->stm_type=STM_RET; $$->stm->s_stm=$1;}
    |
    printstmt {$$=new_node; $$->type=C_STM; $$->stm = malloc(sizeof(struct stm)); $$->stm->stm_type=STM_PRINT; $$->stm->s_stm=$1;}
    |
    stmtblock {$$=new_node; $$->type=C_STM; $$->stm = malloc(sizeof(struct stm)); $$->stm->stm_type=STM_BLOCK; $$->stm->s_stm=$1;};
    
stmts :
    %empty {$$=new_node; $$->type=C_STMS; $$->stms = malloc(sizeof(struct stms)); $$->stms->stm=NULL; $$->stms->next=NULL; $$->stms->last=$$->stms;}
    |
    stmts stmt{$$=$1; $$->stms->last->next = malloc(sizeof(struct stms)); $$->stms->last=$$->stms->last->next; $$->stms->last->stm=$2; $$->stms->last->next=NULL;};
    
stmtblock :
    TPLBB vardefines stmts TPRBB {$$=new_node; $$->type=C_STMBLOCK; $$->stmblock = malloc(sizeof(struct stmblock)); $$->stmblock->vardefines=$2; $$->stmblock->stms=$3;};
    
funcdefine :
    type TIDENT TOLB formals TORB stmtblock {$$=new_node; $2->type_ok=1; $2->type=C_IDENT; $$->type=C_FUNCDEFINE; $$->funcdefine = malloc(sizeof(struct funcdefine)); $$->funcdefine->is_void=0; $$->funcdefine->type=$1; $$->funcdefine->id=$2; $$->funcdefine->formals=$4; $$->funcdefine->stmblock=$6;}
    |
    TKVOID TIDENT TOLB formals TORB stmtblock {$$=new_node; $2->type_ok=1; $2->type=C_IDENT; $$->type=C_FUNCDEFINE; $$->funcdefine = malloc(sizeof(struct funcdefine)); $$->funcdefine->is_void=1; $$->funcdefine->type=NULL; $$->funcdefine->id=$2; $$->funcdefine->formals=$4; $$->funcdefine->stmblock=$6;};

field :
    vardefine {$$=new_node; $$->type=C_FIELD; $$->field = malloc(sizeof(struct field)); $$->field->is_vardefine=1; $$->field->vardefine=$1;}
    |
    funcdefine {$$=new_node; $$->type=C_FIELD; $$->field = malloc(sizeof(struct field)); $$->field->is_vardefine=0; $$->field->funcdefine=$1;};

fields :
    %empty {$$=new_node; $$->type=C_FIELDS; $$->fields = malloc(sizeof(struct fields)); $$->fields->field=NULL; $$->fields->next=NULL; $$->fields->last=$$->fields;}
    |
    fields field {$$=$1; $$->fields->last->next = malloc(sizeof(struct fields)); $$->fields->last=$$->fields->last->next; $$->fields->last->field=$2; $$->fields->last->next=NULL;};

extendclause :
    %empty {$$=new_node; $$->type=C_EXTEND; $$->extend = malloc(sizeof(struct extend)); $$->extend->id = NULL;}
    |
    TKEXTENDS TIDENT {$$=new_node; $2->type_ok=1; $2->type=C_IDENT; $$->type=C_EXTEND; $$->extend = malloc(sizeof(struct extend)); $$->extend->id = $2;};

identifierss :
    TIDENT {$$=new_node; $1->type_ok=1; $1->type=C_IDENT; $$->type=C_IDENT_WITH_COMMA; $$->id_with_comma = malloc(sizeof(struct id_with_comma)); $$->id_with_comma->id = $1; $$->id_with_comma->next=NULL; $$->id_with_comma->last=$$->id_with_comma;}
    |
    identifierss TPCOMMA TIDENT {$3->type_ok=1; $3->type=C_IDENT; $$=$1; $$->id_with_comma->last->next = malloc(sizeof(struct id_with_comma)); $$->id_with_comma->last=$$->id_with_comma->last->next; $$->id_with_comma->last->id = $3; $$->id_with_comma->last->next=NULL;};
    
implementclause :
    %empty {$$=new_node; $$->type=C_IMPLEMENT; $$->implement = malloc(sizeof(struct implement)); $$->implement->id_with_comma = NULL;}
    |
    TKIMPLEMENTS identifierss {$$=new_node; $$->type=C_IMPLEMENT; $$->implement = malloc(sizeof(struct implement)); $$->implement->id_with_comma = $2;};
    
classdefine :
    TKCLASS TIDENT extendclause implementclause TPLBB fields TPRBB {$$=new_node; $2->type_ok=1; $2->type=C_IDENT; $$->type=C_CLASSDEFINE; $$->classdefine = malloc(sizeof(struct classdefine)); $$->classdefine->id = $2; $$->classdefine->extend = $3; $$->classdefine->implement = $4; $$->classdefine->fields = $6;};

protype :
    type TIDENT TOLB formals TORB TPSEP {$$=new_node; $2->type_ok=1; $2->type=C_IDENT; $$->type=C_PROTYPE; $$->protype = malloc(sizeof(struct protype)); $$->protype->is_void = 0; $$->protype->type = $1; $$->protype->id = $2; $$->protype->formals = $4;}
    |
    TKVOID TIDENT TOLB formals TORB TPSEP {$$=new_node; $2->type_ok=1; $2->type=C_IDENT; $$->type=C_PROTYPE; $$->protype = malloc(sizeof(struct protype)); $$->protype->is_void = 1; $$->protype->type = NULL; $$->protype->id = $2; $$->protype->formals = $4;};
    
protypes : 
    %empty {$$=new_node; $$->type=C_PROTYPES; $$->protypes = malloc(sizeof(struct protypes)); $$->protypes->protype=NULL; $$->protypes->next=NULL; $$->protypes->last=$$->protypes;}
    |
    protypes protype {$$=$1; $$->protypes->last->next = malloc(sizeof(struct protypes)); $$->protypes->last=$$->protypes->last->next; $$->protypes->last->protype=$2; $$->protypes->last->next=NULL;};

interfacedefine :
    TKINTERFACE TIDENT TPLBB protypes TPRBB {$$=new_node; $2->type_ok=1; $2->type=C_IDENT; $$->type=C_INTERFACEDEFINE; $$->interfacedefine = malloc(sizeof(struct interfacedefine)); $$->interfacedefine->id=$2; $$->interfacedefine->protypes=$4;};

define :
    vardefine {$$=new_node; $$->type=C_DEFINE; $$->define = malloc(sizeof(struct define)); $$->define->define_type=DEFINE_VAR; $$->define->s_define=$1;}
    |
    funcdefine {$$=new_node; $$->type=C_DEFINE; $$->define = malloc(sizeof(struct define)); $$->define->define_type=DEFINE_FUNC; $$->define->s_define=$1;}
    |
    classdefine {$$=new_node; $$->type=C_DEFINE; $$->define = malloc(sizeof(struct define)); $$->define->define_type=DEFINE_CLASS; $$->define->s_define=$1;}
    |
    interfacedefine {$$=new_node; $$->type=C_DEFINE; $$->define = malloc(sizeof(struct define)); $$->define->define_type=DEFINE_INTERFACE; $$->define->s_define=$1;};

program :
    define {$$=new_node; $$->type=C_PROGRAM; $$->program = malloc(sizeof(struct program)); $$->program->define= $1; $$->program->next=NULL; $$->program->last=$$->program; head=$$;}
     | 
     program define {$$=$1; $$->type=C_PROGRAM; $$->program->last->next = malloc(sizeof(struct program)); $$->program->last=$$->program->last->next; $$->program->last->define= $2; $$->program->last->next=NULL;};

%%
int main(int argc, char **argv)
{
    if (argc == 2)
    {
        yyin=fopen(argv[1],"r");
        #ifdef DEBUG
        yydebug=1;
        #endif
        yyparse();
        parse_program(0, head);
        printf("OK\n");
    }
    else
    {
        printf("Usage %s [FILE]\n", argv[0]);
        yyin=fopen("inheritance.decaf","r");
        #ifdef DEBUG
        yydebug=1;
        #endif
        yyparse();
        parse_program(0, head);
        printf("OK\n");
    }
    return 0;
}
   
void yyerror(const char *s, ...)
{
    extern int col;
    va_list ap;
    va_start(ap, s);
    fprintf(stderr, "[cc]Error(%d.%d): ", yylineno, col);
    vfprintf(stderr, s, ap);
    va_end(ap);
    fputc('\n', stderr);
    return;
}
