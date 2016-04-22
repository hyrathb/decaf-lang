%{

#include <stdio.h>

#include <stdlib.h>

#define TOKEN(t)
#define YY_USER_ACTION { if (yylineno != lineno) \
                        { col = 1;\
                          lineno = yylineno;    \
                        }}
int lineno=1, col=1;
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
OPLUS     <AMOP>\+
OMINUS    <AMOP>\-
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
PPOINT    \.
PSEP      \;
PCOMMA    \,
PLSB      \[
PRSB      \]
PLBB      \{
PRBB      \}
LETTER [A-Za-z]
DIGITI [0-9]
ERROR_ID [0-9]{LETTER}+
IDENTIFIER {LETTER}[\_0-9a-zA-Z]*
INTCONSTHEX ([\+\-]?0[xX][0-9a-fA-F]+)
INTCONSTDEC ([\+\-]?[0-9]+)
DOUBLECONST [\+\-]?{DIGITI}+\.{DIGITI}*([eE]{INTCONSTDEC})?

%s    AMOP

%%

[\ \t\n]          {col += yyleng;}
{STRINGCONST}  {printf("%d:%d (const string, %s)\n", yylineno, col, yytext); BEGIN(AMOP); col += yyleng; TOKEN(TSTRING);}
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
<AMOP>\+      {printf("%d:%d (operator, %s)\n", yylineno, col, yytext); BEGIN(0); col += yyleng; TOKEN(TOPLUS);}
<AMOP>\-      {printf("%d:%d (operator, %s)\n", yylineno, col, yytext); BEGIN(0); col += yyleng; TOKEN(TOMINUS);}
{OMUL}      {printf("%d:%d (operator, %s)\n", yylineno, col, yytext); BEGIN(0); col += yyleng; TOKEN(TOMUL);}
{ODIV}      {printf("%d:%d (operator, %s)\n", yylineno, col, yytext); BEGIN(0); col += yyleng; TOKEN(TODIV);}
{OIDIV}      {printf("%d:%d (operator, %s)\n", yylineno, col, yytext); BEGIN(0); col += yyleng; TOKEN(TOIDIV);}
{OLT}      {printf("%d:%d (operator, %s)\n", yylineno, col, yytext); BEGIN(0); col += yyleng; TOKEN(TOLT);}
{OLE}      {printf("%d:%d (operator, %s)\n", yylineno, col, yytext); BEGIN(0); col += yyleng; TOKEN(TOLE);}
{OGT}      {printf("%d:%d (operator, %s)\n", yylineno, col, yytext); BEGIN(0); col += yyleng; TOKEN(TOGT);}
{OGE}      {printf("%d:%d (operator, %s)\n", yylineno, col, yytext); BEGIN(0); col += yyleng; TOKEN(TOGE);}
{OEQU}      {printf("%d:%d (operator, %s)\n", yylineno, col, yytext); BEGIN(0); col += yyleng; TOKEN(TOEQU);}
{ONE}      {printf("%d:%d (operator, %s)\n", yylineno, col, yytext); BEGIN(0); col += yyleng; TOKEN(TONE);}
{OASSIGN}      {printf("%d:%d (operator, %s)\n", yylineno, col, yytext); BEGIN(0); col += yyleng; TOKEN(TOASSIGN);}
{OAND}      {printf("%d:%d (operator, %s)\n", yylineno, col, yytext); BEGIN(0); col += yyleng; TOKEN(TOAND);}
{OOR}      {printf("%d:%d (operator, %s)\n", yylineno, col, yytext); BEGIN(0); col += yyleng; TOKEN(TOOR);}
{ONOT}      {printf("%d:%d (operator, %s)\n", yylineno, col, yytext); BEGIN(0); col += yyleng; TOKEN(TONOT);}
{OLB}      {printf("%d:%d (operator, %s)\n", yylineno, col, yytext); BEGIN(0); col += yyleng; TOKEN(TOLB);}
{ORB}      {printf("%d:%d (operator, %s)\n", yylineno, col, yytext); BEGIN(AMOP); col += yyleng; TOKEN(TORB);}
{PPOINT}      {printf("%d:%d (punctuation, %s)\n", yylineno, col, yytext); BEGIN(0); col += yyleng; TOKEN(TPPOINT);}
{PSEP}      {printf("%d:%d (punctuation, %s)\n", yylineno, col, yytext); BEGIN(0); col += yyleng; TOKEN(TPSEP);}
{PCOMMA}      {printf("%d:%d (punctuation, %s)\n", yylineno, col, yytext); BEGIN(0); col += yyleng; TOKEN(TPCOMMA);}
{PLSB}      {printf("%d:%d (punctuation, %s)\n", yylineno, col, yytext); BEGIN(0); col += yyleng; TOKEN(TPLSB);}
{PRSB}      {printf("%d:%d (punctuation, %s)\n", yylineno, col, yytext); BEGIN(0); col += yyleng; TOKEN(TPRSB);}
{PLBB}      {printf("%d:%d (punctuation, %s)\n", yylineno, col, yytext); BEGIN(0); col += yyleng; TOKEN(TPLBB);}
{PRBB}      {printf("%d:%d (punctuation, %s)\n", yylineno, col, yytext); BEGIN(0); col += yyleng; TOKEN(TPRBB);}
{ERROR_ID}         {printf("near %d:%d illegal identifier %s\n", yylineno, col, yytext);}
{IDENTIFIER}      {printf("%d:%d (identifier, %s)\n", yylineno, col, yytext); BEGIN(AMOP); col += yyleng; TOKEN(TIDENT);}
<INITIAL>([\+\-]?0[xX][0-9a-fA-F]+)      {printf("%d:%d (const int, %s)\n", yylineno, col, yytext); BEGIN(AMOP); col += yyleng; TOKEN(TINT);}
<INITIAL>([\+\-]?[0-9]+)      {printf("%d:%d (const int, %s)\n", yylineno, col, yytext); BEGIN(AMOP); col += yyleng; TOKEN(TINT);}
<INITIAL>[\+\-]?{DIGITI}+\.{DIGITI}*([eE]{INTCONSTDEC})?      {printf("%d:%d (const double, %s)\n", yylineno, col, yytext); BEGIN(AMOP); col += yyleng; TOKEN(TDOUBLE);}
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
