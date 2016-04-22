%{

#include <stdio.h>

#include <stdlib.h>

#define TOKEN(t)
#define YY_USER_ACTION {if (yylineno != lineno) \
                        { col = 1;\
                          lineno = yylineno;    \
                        }}
int lineno=1, col=1;
%}

STRINGCONST \"[^(\"|\n)]*\"
COMMENT (\/\/.*)|(\/\*(([^\*]*(\*[^\/])?)*)\*\/)
KEYWORD void|int|double|bool|string|class|interface|null|this|extends|implements|for|while|if|else|return|break|New|NewArray|Print|ReadInteger|ReadLine
BOOLCONST true|false
OPERATOR \+|\-|\*|\/|\%|\<|\<\=|\>|\>\=|\=|\=\=|\!\=|\&\&|\|\||\!|\(|\)
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
{KEYWORD}      {printf("%d:%d (keyword, %s)\n", yylineno, col, yytext);  col += yyleng; TOKEN(TKEYWORD);}
{BOOLCONST}      {printf("%d:%d (const bool, %s)\n", yylineno, col, yytext);  col += yyleng; TOKEN(TBOOL);}
{OPERATOR}      {printf("%d:%d (operator, %s)\n", yylineno, col, yytext); col += yyleng; TOKEN(TOPERATOR);}
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
