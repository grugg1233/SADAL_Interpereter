#include <string>
#include <map>
#include <iostream>
#include <cctype>
#include "lex.h"

using namespace std;

map<string, Token> kw_token_map = {
{"if", IF}, {"else", ELSE}, {"elsif", ELSIF}, {"put", PUT}, {"putln", PUTLN}, {"get", GET},
{"integer", INT}, {"float", FLOAT}, /*{"char", CHAR},*/ {"string", STRING}, {"boolean", BOOL},
{"procedure", PROCEDURE}, {"true", TRUE}, {"false", FALSE}, {"end", END}, {"is", IS}, {"character", CHAR},
{"begin", BEGIN}, {"then", THEN}, {"constant", CONST}, {"and", AND}, {"or", OR}, {"not", NOT}, {"putline", PUTLN},
{"mod", MOD}
};

map<char, Token> delimiters = {
{',', COMMA}, {';', SEMICOL}, {'(', LPAREN}, {')', RPAREN}, {'.', DOT}, {':', COLON}
};

map<string, Token> operators = {
{"+", PLUS}, {"-", MINUS}, {"*", MULT}, {"/", DIV}, {":=", ASSOP}, {"=", EQ}, {"/=", NEQ},
{"<", LTHAN}, {">", GTHAN}, {"<=", LTE}, {">=", GTE}, {"&", CONCAT}, {"**", EXP},/* {"mod", MOD},
{"and", AND}, {"or", OR}, {"not", NOT}, */{":", COLON}
};


LexItem getNextToken(istream& in, int& linenumber) {
    char ch;
    string lexeme = "";
    enum TokState { START, INID, INNUM, INFLOAT, INSTRING, INCHAR, INCOMMENT, INEXP, INOPERATOR, INDELIM };
    TokState lexstate = START;
    
    while(in.get(ch)) {
      switch(lexstate) {
        case START:
          if(isspace(ch)) {
            if(ch=='\n' && !(in.peek() == '\n')) {
                linenumber++;
                if(in.peek() == EOF) {
                  linenumber--; 
                  return LexItem(DONE, "", linenumber);
                }
            }
            continue;
          }
          if(ch=='-' && in.peek()=='-') {
            lexstate = INCOMMENT;
            continue;
          }
          if(isalpha(ch)) {
            lexeme = ch;
            lexstate = INID;
            continue;
          }
          if(isdigit(ch)) {
            lexeme = ch;
            lexstate = INNUM;
            continue;
          }
          if(ch=='"') {
            lexeme = "\"";
            lexstate = INSTRING;
            continue;
          }
          if(ch=='\'') {
            lexeme = "\'";
            lexstate = INCHAR;
            continue;
          }
          lexeme = ch;
          //operators 
          if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '=' || ch == '<' || ch == '>' || ch == ':' || ch == '&') {
            lexstate = INOPERATOR;
            continue;
          }
          // Delimiters
          if (ch == ',' || ch == ';' || ch == '(' || ch == ')' || ch == '.') {
              lexstate = INDELIM;
              continue;
          }
        
          return LexItem(ERR, string(1,ch), linenumber);
          
        case INCOMMENT:
          if(ch=='\n'){
            linenumber++;
            lexstate = START;
          }
          continue;
          
        case INID:
          if(isalnum(ch) || ch=='_') {

            if(ch=='_' && !lexeme.empty() && lexeme.back()=='_') {
              in.putback(ch); 
              return id_or_kw(lexeme, linenumber);
            }
            lexeme += ch;
            continue;
          } 
          else {
            in.putback(ch); 
            return id_or_kw(lexeme, linenumber);
          }
          
        case INNUM:
          if(isdigit(ch))
            lexeme += ch;
          else if(ch=='.'){
            lexeme += ch;
            lexstate = INFLOAT;
          }
          else if(tolower(ch)=='e'){
            lexeme += ch;
            lexstate = INEXP;
          }
          else {
            in.putback(ch);
            return LexItem(ICONST, lexeme, linenumber);
          }
          continue;
          
        case INFLOAT:
          if(isdigit(ch))
            lexeme += ch;
          else if(ch=='.' && isdigit(in.peek())) {
              lexeme += ch; 
              return LexItem(ERR, lexeme, linenumber);
          }
          else if(tolower(ch)=='e'){
            if(in.peek() == '+' || in.peek() == '-' || isdigit(in.peek())) {
              lexeme += ch;
              lexstate = INEXP;
            }
            else {
              in.putback(ch); 
              
              return LexItem(FCONST, lexeme, linenumber);
            }
          }
          else {
            in.putback(ch);
            return LexItem(FCONST, lexeme, linenumber);
          }
          continue;
          
          case INEXP:
          {
              if(ch == '+' || ch == '-') {
                  lexeme += ch;
                  if(in.peek() != EOF && isdigit(in.peek())) {
                      while(in.peek() != EOF && isdigit(in.peek())) {
                          in.get(ch);
                          lexeme += ch;
                      }
                      if(lexeme.find('.') != std::string::npos)
                        return LexItem(FCONST, lexeme, linenumber);
                      else return LexItem(ICONST, lexeme, linenumber); 
                  } else {
                      return LexItem(ERR, lexeme, linenumber);
                  }
              } else if(isdigit(ch)) {
                  lexeme += ch;
                  while(in.peek() != EOF && isdigit(in.peek())) {
                      in.get(ch);
                      lexeme += ch;
                  }
                  if(lexeme.find('.') == std::string::npos) return LexItem(ICONST, lexeme, linenumber);
                  else return LexItem(FCONST, lexeme, linenumber);
              } else {
                  in.putback(ch);
                  return LexItem(FCONST, lexeme, linenumber);
              }
          }
          
          
        case INSTRING:
          if(ch=='"')
            return LexItem(SCONST, lexeme, linenumber);
          else if(ch=='\n')
            return LexItem(ERR, lexeme, linenumber);
          else {
            lexeme += ch;
            continue;
          }
          
        case INCHAR: {
            char c = ch;
            lexeme += c;
            if (ch == '\n') {
              return LexItem(ERR, lexeme, linenumber);
            }
            if(in.get(ch) && ch == '\'') {
                
              return LexItem(CCONST, lexeme, linenumber);
           
                
            }   
            else {
                
                lexeme += ch; 
                return LexItem(ERR, lexeme, linenumber);
            }
        }

          
        case INOPERATOR:
          {
            if (((lexeme == "<" || lexeme == ">" || lexeme == ":" || lexeme == "/") && ch == '=') || (lexeme == "*" && ch == '*'))
            {
              lexeme += ch;
            }
            else {
              in.putback(ch);
            }
            if (operators.count(lexeme)) {
                return LexItem(operators[lexeme], lexeme, linenumber);
            }
            else {
                return LexItem(ERR, lexeme, linenumber);
            }
          }
          
          case INDELIM:
            in.putback(ch); 
            return LexItem(delimiters[lexeme[0]], lexeme, linenumber);
        
      }
    }
    return LexItem(DONE, "", linenumber);
  }
  
  LexItem id_or_kw(const string& lexeme, int linenum) {
  string lowered = lexeme;
  
  for(char &c: lowered)
    c = tolower(c);
  
  if(kw_token_map.count(lowered)) {
    if(lowered == "true" || lowered == "false")
      return LexItem(BCONST, lexeme, linenum);
    return LexItem(kw_token_map[lowered], lexeme, linenum);
  }
  return LexItem(IDENT, lowered, linenum);
}

ostream& operator<<(ostream& out, const LexItem& tok) {
  /*string lowered = tok.GetLexeme();
  
  for(char &c: lowered)
    c = tolower(c);*/
  if(tok.GetToken() == ICONST)
    out << "ICONST" << ": (" << tok.GetLexeme() << ")";
  else if(tok.GetToken() == BCONST)
    out << "BCONST" << ": (" << tok.GetLexeme() << ")";
  else if(tok.GetToken() == FCONST)
    out << "FCONST" << ": (" << tok.GetLexeme() << ")";
  else if (tok.GetToken() == PLUS) out << "PLUS";
  else if (tok.GetToken() == MINUS) out << "MINUS";
  else if (tok.GetToken() == MULT) out << "MULT";
  else if (tok.GetToken() == DIV) out << "DIV";
  else if (tok.GetToken() == ASSOP) out << "ASSOP";
  else if (tok.GetToken() == EQ) out << "EQ";
  else if (tok.GetToken() == NEQ) out << "NEQ";
  else if (tok.GetToken() == LTHAN) out << "LTHAN";
  else if (tok.GetToken() == GTHAN) out << "GTHAN";
  else if (tok.GetToken() == LTE) out << "LTE";
  else if (tok.GetToken() == GTE) out << "GTE";
  else if (tok.GetToken() == CONCAT) out << "CONCAT";
  else if (tok.GetToken() == EXP) out << "EXP";
  else if (tok.GetToken() == MOD) out << "MOD";
  else if (tok.GetToken() == AND) out << "AND";
  else if (tok.GetToken() == OR) out << "OR";
  else if (tok.GetToken() == NOT) out << "NOT";
  else if (tok.GetToken() == COMMA) out << "COMMA";
  else if (tok.GetToken() == SEMICOL) out << "SEMICOL";
  else if (tok.GetToken() == LPAREN) out << "LPAREN";
  else if (tok.GetToken() == RPAREN) out << "RPAREN";
  else if (tok.GetToken() == DOT) out << "DOT";
  else if (tok.GetToken() == COLON) out << "COLON";
  
  else if(tok.GetLexeme()=="integer") {out << "INT"; }
  else if(tok.GetLexeme()=="constant") {out << "CONST"; }
  else if(tok.GetLexeme()=="boolean") {out << "BOOL"; }
  else if(tok.GetLexeme()=="and") {out << "AND"; }
  else if(tok.GetLexeme()=="character") {out << "CHAR"; }
  else if(tok.GetLexeme()=="then") {out << "THEN"; }
  else if(tok.GetLexeme()=="end") {out << "END"; }
  else if(tok.GetLexeme()=="mod") {out << "MOD"; }
  else if(tok.GetLexeme()=="or") {out << "OR"; }
  else if(tok.GetLexeme()=="not") {out << "NOT"; }
  else if(tok.GetLexeme()=="string") {out << "STRING"; }
  else if(tok.GetLexeme()=="elsif") {out << "ELSIF"; }
  else if(tok.GetLexeme()=="procedure") {out << "PROCEDURE"; }

  //else if(tok.GetToken() == kw_token_map[lowered]) out << kw_token_map[lowered]; 
  else if(tok.GetToken() == IDENT) {
    out << "IDENT" << ": <" << tok.GetLexeme() <<">";
  } 
  else if(tok.GetToken() == SCONST)
    out << "SCONST" << ": \"" << tok.GetLexeme().substr(1) << "\"";
  else if(tok.GetToken() == CCONST)
    out << "CCONST" << ": \'" << tok.GetLexeme()[1] << "\'";
  else if(tok.GetToken() == COMMA)
    out << "COMMA";
  else if(tok.GetToken() == ERR && tok.GetLexeme()[0] == '\"')
    out << "ERR: In line " << tok.GetLinenum() << ", Error Message { Invalid string constant "<< tok.GetLexeme() << "}"; 
  else if(tok.GetToken() == ERR && tok.GetLexeme().find('\n') != string::npos)
    out << "ERR: In line " << tok.GetLinenum() << ", Error Message {New line is an invalid character constant.}";
  else if(tok.GetToken() == ERR && tok.GetLexeme()[0] == '\'')
    out << "ERR: In line " << tok.GetLinenum() << ", Error Message { Invalid character constant \'"<< tok.GetLexeme().substr(1) << "\'}"; 

  else if(tok.GetToken() == ERR)
    out << "ERR: In line " << tok.GetLinenum() << ", Error Message {" << tok.GetLexeme() << "}";
  
  out << endl;
  return out;
}
