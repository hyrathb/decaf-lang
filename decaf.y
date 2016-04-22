%{
  #include <stdio.h>
  #include <math.h>
  int yylex (void);
  void yyerror (char const *);
  extern int yylineno;
  extern FILE *yyin;
%}

%token TSTRINGCONST TCOMMENT TKVOID TKINT TKDOUBLE TKBOOL TKSTRING TKCLASS TKINTERFACE TKNULL TKTHIS TKEXTENDS
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

%% /* Grammar rules and actions follow.  */

constant:
    TINTCONST | TDOUBLECONST | TBOOLCONST | TSTRINGCONST | TKNULL;


formals :
    var 
    |
    formals TPCOMMA var;

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
    TIDENT TOLB actuals TORB TPSEP
    |
    expr TPPOINT TIDENT TOLB actuals TORB TPSEP;

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
    expr_or_not TPSEP
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
    TKCLASS TIDENT extendclause implementclause TPLBB field TPRBB;

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

        yyparse();
        }
        else
        {
            printf("Usage %s [FILE]\n", argv[0]);
        }
    return 0;
   }
   
   void yyerror(const char *s)
   {
        printf("Error at line %d, message: %s\n", yylineno, s);
   }
