%{

#include <stdio.h>

#include <stdlib.h>

#define TOKEN(t)
#define YY_USER_ACTION {if (yylineno != lineno) \
                        { col = 1;\
                          lineno = yylineno;    \
                        }}
int lineno=1, col=1;
int pdsign=0;
%}
%option yylineno
STRINGCONST \"[^(\"|\n)]*\"
COMMENT (\/\/.*)|(\/\*(([^\*]*(\*[^\/])?)*)\*\/)
KEYWORD void|int|double|bool|string|class|interface|null|this|extends|implements|for|while|if|else|return|break|New|NewArray|Print|ReadInteger|ReadLine
KVOID    void
KINT     int
KDOUBLE  double
KBOOL    bool
KSTRING  string
KCLASS  class
KINTERFACE interface
KNULL   null
KTHIS   this
KEXTENDS    extends
KIMPLEMENTS implements
KFOR    for
KWHILE  while
KIF     if
KELSE   else
KRETURN return
KBREAK  break
KNEW    New
KNEWARRAY NewArray
KPRINT  Print
KREADINTEGER ReadInteger
KREADLINE   ReadLine
BOOLCONST true|false
OPERATOR \+|\-|\*|\/|\%|\<|\<\=|\>|\>\=|\=|\=\=|\!\=|\&\&|\|\||\!|\(|\)
OPLUS     <[^+-]?[\ \t]*>\+
OMINUS    <[^+-]?[\ \t]*>\-
OMUL      \*
ODIV      \/
OIDIV     \%
OLT       \<
OLE       \<\=
OGT       \>
OGE       \>\=
OEQU      \=\=
ONE       \!\=
OASSIGN   \=
OAND      \&\&
OOR       \|\|
ONOT      \!
OLB       \(
ORB       \)
PUNCTUATION [\.\;\,\[\]\{\}]
LETTER [A-Za-z]
DIGITI [0-9]
ERROR_ID [0-9]{LETTER}+
IDENTIFIER {LETTER}[\_0-9a-zA-Z]*
INTCONSTHEX ([\+\-]?0[xX][0-9a-fA-F]+)
INTCONSTDEC ([\+\-]?[0-9]+)
DOUBLECONST [\+\-]?{DIGITI}+\.{DIGITI}*([eE]{INTCONSTDEC})?
   
%%

[\ \t\n]          {col += yyleng;}
{STRINGCONST}  {printf("%d:%d (const string, %s)\n", yylineno, col, yytext); col += yyleng; TOKEN(TSTRING);}
{COMMENT}      {printf("%d:%d (comment, %s)\n", yylineno, col, yytext); col += yyleng; TOKEN(TCOMMENT);}
{KVOID}      {printf("%d:%d (keyword, %s)\n", yylineno, col, yytext);  col += yyleng; TOKEN(TKVOID);}
{KINT}      {printf("%d:%d (keyword, %s)\n", yylineno, col, yytext);  col += yyleng; TOKEN(TKINT);}
{KDOUBLE}      {printf("%d:%d (keyword, %s)\n", yylineno, col, yytext);  col += yyleng; TOKEN(TKDOUBLE);}
{KBOOL}      {printf("%d:%d (keyword, %s)\n", yylineno, col, yytext);  col += yyleng; TOKEN(TKBOOL);}
{KSTRING}      {printf("%d:%d (keyword, %s)\n", yylineno, col, yytext);  col += yyleng; TOKEN(TKSTRING);}
{KCLASS}      {printf("%d:%d (keyword, %s)\n", yylineno, col, yytext);  col += yyleng; TOKEN(TKCLASS);}
{KINTERFACE}      {printf("%d:%d (keyword, %s)\n", yylineno, col, yytext);  col += yyleng; TOKEN(TKINTERFACE);}
{KNULL}      {printf("%d:%d (keyword, %s)\n", yylineno, col, yytext);  col += yyleng; TOKEN(TKNULL);}
{KTHIS}      {printf("%d:%d (keyword, %s)\n", yylineno, col, yytext);  col += yyleng; TOKEN(TKTHIS);}
{KEXTENDS}      {printf("%d:%d (keyword, %s)\n", yylineno, col, yytext);  col += yyleng; TOKEN(TKEXTENDS);}
{KIMPLEMENTS}      {printf("%d:%d (keyword, %s)\n", yylineno, col, yytext);  col += yyleng; TOKEN(TKIMPLEMENTS);}
{KFOR}      {printf("%d:%d (keyword, %s)\n", yylineno, col, yytext);  col += yyleng; TOKEN(TKFOR);}
{KWHILE}      {printf("%d:%d (keyword, %s)\n", yylineno, col, yytext);  col += yyleng; TOKEN(TKWHILE);}
{KIF}      {printf("%d:%d (keyword, %s)\n", yylineno, col, yytext);  col += yyleng; TOKEN(TKIF);}
{KELSE}      {printf("%d:%d (keyword, %s)\n", yylineno, col, yytext);  col += yyleng; TOKEN(TKELSE);}
{KRETURN}      {printf("%d:%d (keyword, %s)\n", yylineno, col, yytext);  col += yyleng; TOKEN(TKRETURN);}
{KBREAK}      {printf("%d:%d (keyword, %s)\n", yylineno, col, yytext);  col += yyleng; TOKEN(TKBREAK);}
{KNEW}      {printf("%d:%d (keyword, %s)\n", yylineno, col, yytext);  col += yyleng; TOKEN(TKNEW);}
{KNEWARRAY}      {printf("%d:%d (keyword, %s)\n", yylineno, col, yytext);  col += yyleng; TOKEN(TKNEWARRAY);}
{KPRINT}      {printf("%d:%d (keyword, %s)\n", yylineno, col, yytext);  col += yyleng; TOKEN(TKPRINT);}
{KREADINTEGER}      {printf("%d:%d (keyword, %s)\n", yylineno, col, yytext);  col += yyleng; TOKEN(TKREADINTEGER);}
{KREADLINE}      {printf("%d:%d (keyword, %s)\n", yylineno, col, yytext);  col += yyleng; TOKEN(TKREADLINE);}
{BOOLCONST}      {printf("%d:%d (const bool, %s)\n", yylineno, col, yytext);  col += yyleng; TOKEN(TBOOL);}
{OPLUS}      {if(pdsign) REJECT; printf("%d:%d (operator, %s)\n", yylineno, col, yytext); col += yyleng; TOKEN(OPLUS);}
{OMINUS}      {if(pdsign) REJECT; printf("%d:%d (operator, %s)\n", yylineno, col, yytext); col += yyleng; TOKEN(OMINUS);}
{OMUL}      {printf("%d:%d (operator, %s)\n", yylineno, col, yytext); col += yyleng; TOKEN(OMUL);}
{ODIV}      {printf("%d:%d (operator, %s)\n", yylineno, col, yytext); col += yyleng; TOKEN(ODIV);}
{OIDIV}      {printf("%d:%d (operator, %s)\n", yylineno, col, yytext); col += yyleng; TOKEN(OIDIV);}
{OLT}      {printf("%d:%d (operator, %s)\n", yylineno, col, yytext); col += yyleng; TOKEN(OLT);}
{OLE}      {printf("%d:%d (operator, %s)\n", yylineno, col, yytext); col += yyleng; TOKEN(OLE);}
{OGT}      {printf("%d:%d (operator, %s)\n", yylineno, col, yytext); col += yyleng; TOKEN(OGT);}
{OGE}      {printf("%d:%d (operator, %s)\n", yylineno, col, yytext); col += yyleng; TOKEN(OGE);}
{OEQU}      {printf("%d:%d (operator, %s)\n", yylineno, col, yytext); col += yyleng; TOKEN(OEQU);}
{ONE}      {printf("%d:%d (operator, %s)\n", yylineno, col, yytext); col += yyleng; TOKEN(ONE);}
{OASSIGN}      {printf("%d:%d (operator, %s)\n", yylineno, col, yytext); col += yyleng; TOKEN(OASSIGN);}
{OAND}      {printf("%d:%d (operator, %s)\n", yylineno, col, yytext); col += yyleng; TOKEN(OAND);}
{OOR}      {printf("%d:%d (operator, %s)\n", yylineno, col, yytext); col += yyleng; TOKEN(OOR);}
{ONOT}      {printf("%d:%d (operator, %s)\n", yylineno, col, yytext); col += yyleng; TOKEN(ONOT);}
{OLB}      {printf("%d:%d (operator, %s)\n", yylineno, col, yytext); col += yyleng; TOKEN(OLB);}
{ORB}      {printf("%d:%d (operator, %s)\n", yylineno, col, yytext); col += yyleng; TOKEN(ORB);}
{PUNCTUATION}      {printf("%d:%d (punctuation, %s)\n", yylineno, col, yytext); col += yyleng; TOKEN(TPUNCTUATION);}
{ERROR_ID}         {printf("near %d:%d illegal identifier %s\n", yylineno, col, yytext);}
{IDENTIFIER}      {printf("%d:%d (identifier, %s)\n", yylineno, col, yytext); col += yyleng; TOKEN(TIDENT);}
{INTCONSTHEX}      {printf("%d:%d (const int, %s)\n", yylineno, col, yytext); col += yyleng; TOKEN(TINT);}
{INTCONSTDEC}      {printf("%d:%d (const int, %s)\n", yylineno, col, yytext); col += yyleng; TOKEN(TINT);}
{DOUBLECONST}      {printf("%d:%d (const double, %s)\n", yylineno, col, yytext); col += yyleng; TOKEN(TDOUBLE);}
.                   {printf("near %d:%d illegal identifier %s\n", yylineno, col, yytext);}

%%

    int main(int argc, char **argv)
    {
        if (argc == 2)
        {
        yyin=fopen(argv[1],"r");

        yylex();
        }
        else
        {
            printf("Usage %s [FILE]\n", argv[0]);
        }
    return 0;
   }
   
   int yywrap()
   {
        return 1;
   }
