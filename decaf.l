%{

#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include "defs.h"
#include "decaf.h"

#define TOKEN(t) return t
#define SAVE {yylval = malloc(sizeof(struct semantics)); yylval->text = malloc(strlen(yytext)+1); \
              strcpy(yylval->text, yytext);}
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
OLSB      \[
ORSB      \]
OLB       \(
ORB       \)
PUNCTUATION [\.\;\,\[\]\{\}]
PPOINT    \.
PSEP      \;
PCOMMA    \,
PLBB      \{
PRBB      \}
LETTER [A-Za-z]
DIGITI [0-9]
IDENTIFIER {LETTER}[\_0-9a-zA-Z]*
INTCONSTHEX ([\+\-]?0[xX][0-9a-fA-F]+)
INTCONSTDEC ([\+\-]?[0-9]+)
ERROR_ID [0-9]+[A-Za-z][0-9A-Za-z]
DOUBLECONST [\+\-]?{DIGITI}+\.{DIGITI}*([eE]{INTCONSTDEC})?

%s    AMOP

%%

[\ \t\n]          {col += yyleng;}
{STRINGCONST}  { BEGIN(AMOP); col += yyleng; SAVE;TOKEN(TSTRINGCONST);}
{COMMENT}      { col += yyleng;}
{KVOID}      {  col += yyleng; TOKEN(TKVOID);}
{KINT}      {  col += yyleng; TOKEN(TKINT);}
{KDOUBLE}      {  col += yyleng; TOKEN(TKDOUBLE);}
{KBOOL}      {  col += yyleng; TOKEN(TKBOOL);}
{KSTRING}      {  col += yyleng; TOKEN(TKSTRING);}
{KCLASS}      {  col += yyleng; TOKEN(TKCLASS);}
{KINTERFACE}      {  col += yyleng; TOKEN(TKINTERFACE);}
{KNULL}      {  col += yyleng; TOKEN(TKNULL);}
{KTHIS}      {  col += yyleng; TOKEN(TKTHIS);}
{KEXTENDS}      {  col += yyleng; TOKEN(TKEXTENDS);}
{KIMPLEMENTS}      {  col += yyleng; TOKEN(TKIMPLEMENTS);}
{KFOR}      {  col += yyleng; TOKEN(TKFOR);}
{KWHILE}      {  col += yyleng; TOKEN(TKWHILE);}
{KIF}      {  col += yyleng; TOKEN(TKIF);}
{KELSE}      {  col += yyleng; TOKEN(TKELSE);}
{KRETURN}      {  col += yyleng; TOKEN(TKRETURN);}
{KBREAK}      {  col += yyleng; TOKEN(TKBREAK);}
{KNEW}      {  col += yyleng; TOKEN(TKNEW);}
{KNEWARRAY}      {  col += yyleng; TOKEN(TKNEWARRAY);}
{KPRINT}      {  col += yyleng; TOKEN(TKPRINT);}
{KREADINTEGER}      {  col += yyleng; TOKEN(TKREADINTEGER);}
{KREADLINE}      {  col += yyleng; TOKEN(TKREADLINE);}
{BOOLCONST}      {  col += yyleng; SAVE; TOKEN(TBOOLCONST);}
<AMOP>\+      { BEGIN(0); col += yyleng; TOKEN(TOPLUS);}
<AMOP>\-      { BEGIN(0); col += yyleng; TOKEN(TOMINUS);}
{OMUL}      { BEGIN(0); col += yyleng; TOKEN(TOMUL);}
{ODIV}      { BEGIN(0); col += yyleng; TOKEN(TODIV);}
{OIDIV}      { BEGIN(0); col += yyleng; TOKEN(TOIDIV);}
{OLT}      { BEGIN(0); col += yyleng; TOKEN(TOLT);}
{OLE}      { BEGIN(0); col += yyleng; TOKEN(TOLE);}
{OGT}      { BEGIN(0); col += yyleng; TOKEN(TOGT);}
{OGE}      { BEGIN(0); col += yyleng; TOKEN(TOGE);}
{OEQU}      { BEGIN(0); col += yyleng; TOKEN(TOEQU);}
{ONE}      { BEGIN(0); col += yyleng; TOKEN(TONE);}
{OASSIGN}      { BEGIN(0); col += yyleng; TOKEN(TOASSIGN);}
{OAND}      { BEGIN(0); col += yyleng; TOKEN(TOAND);}
{OOR}      { BEGIN(0); col += yyleng; TOKEN(TOOR);}
{ONOT}      { BEGIN(0); col += yyleng; TOKEN(TONOT);}
{OLSB}      { BEGIN(0); col += yyleng; TOKEN(TOLSB);}
{ORSB}      { BEGIN(0); col += yyleng; TOKEN(TORSB);}
{OLB}      { BEGIN(0); col += yyleng; TOKEN(TOLB);}
{ORB}      { BEGIN(AMOP); col += yyleng; TOKEN(TORB);}
{PPOINT}      { BEGIN(0); col += yyleng; TOKEN(TPPOINT);}
{PSEP}      { BEGIN(0); col += yyleng; TOKEN(TPSEP);}
{PCOMMA}      { BEGIN(0); col += yyleng; TOKEN(TPCOMMA);}
{PLBB}      { BEGIN(0); col += yyleng; TOKEN(TPLBB);}
{PRBB}      { BEGIN(0); col += yyleng; TOKEN(TPRBB);}
{IDENTIFIER}      { BEGIN(AMOP); col += yyleng; SAVE; TOKEN(TIDENT);}
<INITIAL>([\+\-]?0[xX][0-9a-fA-F]+)      { BEGIN(AMOP); col += yyleng; SAVE; TOKEN(TINTCONST);}
<INITIAL>([\+\-]?[0-9]+)      { BEGIN(AMOP); col += yyleng; SAVE; TOKEN(TINTCONST);}
<INITIAL>[\+\-]?{DIGITI}+\.{DIGITI}*([eE]{INTCONSTDEC})?      { BEGIN(AMOP); col += yyleng; SAVE; TOKEN(TDOUBLECONST);}
.|{ERROR_ID}                   {}

%%


   
   int yywrap()
   {
        return 1;
   }
