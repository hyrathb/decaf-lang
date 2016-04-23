%{
  #include <stdio.h>
  #include <math.h>
  #include <stdlib.h>
  #include <stdarg.h>
  #include "sym.h"
  
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
%right TOLSB TORSB
%left TOLB TORB

%start program

%glr-parser


%% /* Grammar rules and actions follow.  */

constant:
    TINTCONST{$$.text=NULL; $$.type_ok=1; $$.type = C_ICONST; $$.i_val=strtol($1.text, NULL, 0); free($1.text);} 
    | 
    TDOUBLECONST {$$.text=NULL; $$.type_ok=1; $$.type = C_DCONST; $$.i_val=atof($1.text); free($1.text);}
    | 
    TBOOLCONST {$$.text=NULL; $$.type_ok=1; $$.type = C_ICONST; $$.i_val= !strcmp($1.text, "true"); free($1.text);}
    | 
    TSTRINGCONST {$$.text=NULL; $$.type_ok=1; $$.type = C_SCONST; $$.s_val=$1.text;}
    | 
    TKNULL{$$.text=NULL; $$.type_ok=1; $$.type = C_NULL; $$.i_val=0;};


tformals :
    var 
    |
    tformals TPCOMMA var;

formals :
    %empty
    |
    tformals;
    
type :
    TKINT | TKBOOL | TKDOUBLE | TKSTRING | TIDENT | type TOLSB TORSB;

var :
    type TIDENT;

vardefine :
    var TPSEP;

vardefines :
    %empty | vardefines vardefine;

actuals :
    %empty | exprss;
    
call :
    TIDENT TOLB actuals TORB
    |
    expr TPPOINT TIDENT TOLB actuals TORB;

lvalue :
    TIDENT 
    |
    expr TPPOINT TIDENT
    |
    expr TOLSB expr TOLSB;
    
expr :
    lvalue TOASSIGN expr
    |
    constant
    |
    lvalue
    |
    TKTHIS
    |
    call
    |
    TOLB expr TORB
    |
    expr TOPLUS expr
    |
    expr TOMINUS expr
    |
    expr TOMUL expr
    |
    expr TODIV expr
    |
    expr TOIDIV expr
    |
    expr TOLT expr
    |
    expr TOLE expr
    |
    expr TOGT expr
    |
    expr TOGE expr
    |
    expr TOEQU expr
    |
    expr TONE expr
    |
    expr TOAND expr
    |
    expr TOOR expr
    |
    TONOT expr
    |
    TKREADINTEGER TOLB TORB
    |
    TKREADLINE TOLB TORB
    |
    TKNEW TOLB TIDENT TORB
    |
    TKNEWARRAY TOLB expr TPCOMMA type TORB; 
    
exprss :
    expr
    |
    exprss TPCOMMA expr;
    
ifstmt :
    TKIF TOLB expr TORB stmt
    |
    TKIF TOLB expr TORB stmt TKELSE stmt;
    
whilestmt :
    TKWHILE TOLB expr TORB stmt;

expr_or_not:
    %empty | expr;
    
forstmt :
    TKFOR TOLB expr_or_not TPSEP expr TPSEP expr_or_not TORB stmt;
    
returnstmt :
    TKRETURN expr_or_not TPSEP;

breakstmt :
    TKBREAK TPSEP;

printstmt :
    TKPRINT TOLB exprss TORB TPSEP;
    
stmt :
    TPSEP
    |
    expr TPSEP
    |
    ifstmt
    |
    whilestmt
    |
    forstmt
    |
    breakstmt
    |
    returnstmt
    |
    printstmt
    |
    stmtblock;
    
stmts :
    %empty | stmts stmt;
    
stmtblock :
    TPLBB vardefines stmts TPRBB;
    
funcdefine :
    type TIDENT TOLB formals TORB stmtblock
    |
    TKVOID TIDENT TOLB formals TORB stmtblock;

field :
    vardefine | funcdefine;

fields :
    %empty
    |
    fields field;

extendclause :
    %empty
    |
    TKEXTENDS TIDENT;

identifierss :
    TIDENT
    |
    identifierss TPCOMMA TIDENT;
    
implementclause :
    %empty
    |
    TKIMPLEMENTS identifierss;
    
classdefine :
    TKCLASS TIDENT extendclause implementclause TPLBB fields TPRBB;

protype :
    type TIDENT TOLB formals TORB TPSEP
    |
    TKVOID TIDENT TOLB formals TORB TPSEP;
    
protypes : 
    %empty | protypes protype;

interfacedefine :
    TKINTERFACE TIDENT TPLBB protypes TPRBB;

define :
    vardefine
    |
    funcdefine
    |
    classdefine
    |
    interfacedefine;

program :
     %empty
     | 
     program define;

%%
int main(int argc, char **argv)
{
    if (argc == 2)
    {
        yyin=fopen(argv[1],"r");
        yydebug=1;
        yyparse();
    }
    else
    {
        printf("Usage %s [FILE]\n", argv[0]);
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
