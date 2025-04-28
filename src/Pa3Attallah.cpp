/* Implementation of Interpreter
	for the Simple Ada-Like (SADAL) Language
 * parser.cpp
 * Programming Assignment 3
 * Spring 2025
 * George Attallah Pa3 
*/
#include <iostream>
#include <vector>
#include <sstream>
#include "parserInterp.h"


map<string, bool> defVar;
map<string, Token> SymTable;
map<string, Value> TempsResults; //Container of temporary locations of Value objects for results of expressions, variables values and constants 


static vector<string> Id_List;

static string currProcName;

namespace Parser {
	bool pushed_back = false;
	LexItem	pushed_token;

	static LexItem GetNextToken(istream& in, int& line) {
		if( pushed_back ) {
			pushed_back = false;
			return pushed_token;
		}
		return getNextToken(in, line);
	}

	static void PushBackToken(LexItem & t) {
		if( pushed_back ) {
			abort();
		}
		pushed_back = true;
		pushed_token = t;	
	}

}

static int error_count = 0;

int ErrCount()
{
    return error_count;
}

void ParseError(int line, string msg)
{
	++error_count;
	cout << line << ": " << msg << endl;
}



//Prog ::= PROCEDURE ProcName IS ProcBody

bool Prog(istream& in, int& line)
{
    defVar.clear();
    SymTable.clear();
    TempsResults.clear();

    LexItem tok = Parser::GetNextToken(in, line);
    if (tok != PROCEDURE) {
        ParseError(line, "Program must start with PROCEDURE.");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != IDENT) {
        ParseError(line, "Missing Procedure Name.");
        return false;
    }
    // remember for matching in ProcBody
    currProcName = tok.GetLexeme();
    defVar[currProcName] = true;

    tok = Parser::GetNextToken(in, line);
    if (tok != IS) {
        ParseError(line, "Incorrect Procedure Header Format.");
        return false;
    }

    if (!ProcBody(in, line)) {
        ParseError(line, "Incorrect Procedure Definition.");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != DONE) {
        ParseError(line, "Incorrect compilation file.");
        return false;
    }
    cout << '\n' << "(DONE)" << endl; 
    return true;
}//end of prog
//ProcBody ::= DeclPart BEGIN StmtList END ProcName ;
bool ProcBody(istream& in, int& line)
{
    if (!DeclPart(in, line)) {
        ParseError(line, "Incorrect Declaration Part.");
        return false;
    }

    LexItem tok = Parser::GetNextToken(in, line);
    if (tok != BEGIN) {
        ParseError(line, "Missing BEGIN of procedure body.");
        return false;
    }

    if (!StmtList(in, line)) {
        ParseError(line, "Incorrect Procedure Body.");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != END) {
        ParseError(line, "Missing END of Procedure Keyword.");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != IDENT) {
        ParseError(line, "Missing END of procedure name.");
        return false;
    }
    if (tok.GetLexeme() != currProcName) {
        ParseError(line, "Run-Time Error: Procedure name mismatch");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != SEMICOL) {
        ParseError(line, "Missing end of procedure semicolon.");
        return false;
    }

    return true;
}

//end of ProcBody

//StmtList ::= Stmt { Stmt }
bool StmtList(istream& in, int& line)
{
    bool status = Stmt(in, line);
    LexItem tok = Parser::GetNextToken(in, line);
    while (status && tok != END && tok != ELSIF && tok != ELSE) {
        Parser::PushBackToken(tok);
        status = Stmt(in, line);
        tok = Parser::GetNextToken(in, line);
    }
    if (!status) {
        ParseError(line, "Syntactic error in statement list.");
        return false;
    }
    Parser::PushBackToken(tok);
    return true;
}
//End of StmtList

//DeclPart ::= DeclStmt { DeclStmt }
bool DeclPart(istream& in, int& line) {
    if (!DeclStmt(in, line)) {
        ParseError(line, "Non-recognizable Declaration Part.");
        return false;
    }
    LexItem tok = Parser::GetNextToken(in, line);
    if (tok == BEGIN) {
        Parser::PushBackToken(tok);
        return true;
    }
    Parser::PushBackToken(tok);
    return DeclPart(in, line);
}
//end of DeclPart function

//DeclStmt ::= IDENT {, IDENT } : Type [:= Expr]
bool DeclStmt(istream& in, int& line)
{
    if (!IdentList(in, line)) {
        ParseError(line, "Incorrect identifiers list in Declaration Statement.");
        return false;
    }
    vector<string> names = Id_List;
    Id_List.clear();

    LexItem t = Parser::GetNextToken(in, line);
    if (t != COLON) {
        Parser::PushBackToken(t);
        ParseError(line, "Incorrect Declaration Statement Syntax.");
        return false;
    }

    t = Parser::GetNextToken(in, line);
    if (t == CONST) {
        t = Parser::GetNextToken(in, line);
    }

    Token typeTok = t.GetToken();
    if (typeTok != INT && typeTok != FLOAT && typeTok != STRING && typeTok != CHAR && typeTok != BOOL) {
        ParseError(line, "Incorrect Declaration Type.");
        return false;
    }
    for (auto & name : names) {
        SymTable[name] = typeTok;
    }

    t = Parser::GetNextToken(in, line);
    if (t == LPAREN) {
        Value lo, hi;
        if (!Range(in, line, lo, hi)) {
            ParseError(line, "Run-Time Error: Invalid range definition");
            return false;
        }
        t = Parser::GetNextToken(in, line);
        if (t != RPAREN) {
            ParseError(line, "Incorrect syntax for a range in declaration statement");
            return false;
        }
        t = Parser::GetNextToken(in, line);
    }

    if (t == ASSOP) {
        Value initVal;
        if (!Expr(in, line, initVal)) {
            ParseError(line, "Incorrect initialization expression.");
            return false;
        }
        bool typeMismatch = (typeTok == INT    && !initVal.IsInt())
                         || (typeTok == FLOAT  && !initVal.IsReal())
                         || (typeTok == STRING && !initVal.IsString())
                         || (typeTok == CHAR   && !initVal.IsChar())
                         || (typeTok == BOOL   && !initVal.IsBool());
        if (typeMismatch) {
            ParseError(line, "Run-Time Error: Illegal initialization type");
            return false;
        }
        for (auto & name : names) {
            TempsResults[name] = initVal;
        }
        t = Parser::GetNextToken(in, line);
    } else {
        Parser::PushBackToken(t);
        t = Parser::GetNextToken(in, line);
    }

    if (t == SEMICOL) {
        return true;
    }
    --line;
    ParseError(line, "Missing semicolon at end of statement");
    return false;
}

//End of DeclStmt

//IdList:= IDENT {,IDENT}
bool IdentList(std::istream& in, int& line) {
    LexItem tok = Parser::GetNextToken(in, line);
    if (tok == IDENT) {
        std::string identstr = tok.GetLexeme();
        // use count() instead of find()->second
        if (defVar.count(identstr) == 0) {
            defVar[identstr] = true;
            Id_List.push_back(identstr);
        }
        else {
            ParseError(line, "Variable Redefinition");
            return false;
        }
    }
    else {
        Parser::PushBackToken(tok);
        return true;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok == COMMA) {
        return IdentList(in, line);
    }
    else if (tok == COLON) {
        Parser::PushBackToken(tok);
        return true;
    }
    else if (tok == IDENT) {
        ParseError(line, "Missing comma in declaration statement.");
        return false;
    }
    else {
        ParseError(line, "Invalid name for an identifier:");
        std::cout << "(" << tok.GetLexeme() << ")" << std::endl;
        return false;
    }
} //End of IdentList
	

//Stmt ::= AssignStmt | PrintStmts | GetStmt | IfStmt
bool Stmt(istream& in, int& line) {
	bool status = false;
	
	LexItem t = Parser::GetNextToken(in, line);
	
	switch( t.GetToken() ) {
	case IDENT:
		Parser::PushBackToken(t);
		status = AssignStmt(in, line);
		
		if(!status)
		{
			ParseError(line, "Invalid assignment statement.");
			return false;
		}
		break;
		
	case IF: 
		Parser::PushBackToken(t);
		status = IfStmt(in, line);
		if(!status)
		{
			ParseError(line, "Invalid If statement.");
			return false;
		}
		
		break;
		
	case PUT: case PUTLN:
		Parser::PushBackToken(t);
		status = PrintStmts(in, line);
		if(!status)
		{
			ParseError(line, "Invalid put statement.");
			return false;
		}
		break;
		
	case GET:
		Parser::PushBackToken(t);
		status = GetStmt(in, line);
		if(!status)
		{
			ParseError(line, "Invalid get statement.");
			return false;
		}
		break;
		
	default:
		Parser::PushBackToken(t);
		return false;
	}

	return status;
}//End of Stmt
	
//PrintStmts ::= (PutLine | Put) ( Expr) ; 
bool PrintStmts(istream& in, int& line)
{
    LexItem t = Parser::GetNextToken(in, line);
    bool isPutLine = (t == PUTLN);
    if (t != PUT && t != PUTLN) {
        ParseError(line, "Missing Put or PutLine Keyword");
        return false;
    }

    t = Parser::GetNextToken(in, line);
    if (t != LPAREN) {
        ParseError(line, "Missing Left Parenthesis");
        return false;
    }

    Value val;
    if (!Expr(in, line, val)) {
        ParseError(line, "Missing expression for an output statement");
        return false;
    }

    t = Parser::GetNextToken(in, line);
    if (t != RPAREN) {
        ParseError(line, "Missing Right Parenthesis");
        return false;
    }

    t = Parser::GetNextToken(in, line);
    if (t != SEMICOL) {
        --line;
        ParseError(line, "Missing semicolon at end of statement");
        return false;
    }
    cout << val;
    if (isPutLine)
        cout << '\n';

    return true;
}
//End of PrintStmts





// IfStmt ::= IF Expr THEN StmtList { ELSIF Expr THEN StmtList } [ ELSE StmtList ] END IF ;
bool IfStmt(istream& in, int& line)
{
    LexItem tok = Parser::GetNextToken(in, line);
    if (tok != IF) {
        ParseError(line, "Missing IF Keyword");
        return false;
    }

    
    Value cond;
    if (!Expr(in, line, cond)) {
        ParseError(line, "Missing if statement condition");
        return false;
    }
    if (!cond.IsBool()) {
        ParseError(line, "Run-Time Error: Illegal expression type for If condition");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != THEN) {
        ParseError(line, "If-Stmt Syntax Error");
        return false;
    }

    
    bool taken = false;

    
    if (cond.GetBool()) {
        taken = true;
        if (!StmtList(in, line)) {
            ParseError(line, "Missing Statement for If-Stmt Then-clause");
            return false;
        }
    }
    else {
        
        LexItem t2;
        do {
            t2 = Parser::GetNextToken(in, line);
        } while (t2 != ELSIF && t2 != ELSE && t2 != END);
        Parser::PushBackToken(t2);
    }

   
    tok = Parser::GetNextToken(in, line);
    while (tok == ELSIF) {
        
        Value cond2;
        if (!Expr(in, line, cond2)) {
            ParseError(line, "Missing Elsif statement condition");
            return false;
        }
        if (!cond2.IsBool()) {
            ParseError(line, "Run-Time Error: Illegal expression type for Elsif condition");
            return false;
        }

        tok = Parser::GetNextToken(in, line);
        if (tok != THEN) {
            ParseError(line, "Elsif-Stmt Syntax Error");
            return false;
        }

        if (!taken && cond2.GetBool()) {
            
            taken = true;
            if (!StmtList(in, line)) {
                ParseError(line, "Missing Statement for If-Stmt Else-If-clause");
                return false;
            }
        }
        else {
            
            LexItem t3;
            do {
                t3 = Parser::GetNextToken(in, line);
            } while (t3 != ELSIF && t3 != ELSE && t3 != END);
            Parser::PushBackToken(t3);
        }
        tok = Parser::GetNextToken(in, line);
    }

    // optional ELSE
    if (tok == ELSE) {
        if (!taken) {
            
            if (!StmtList(in, line)) {
                ParseError(line, "Missing Statement for If-Stmt Else-clause");
                return false;
            }
        } else {
            
            LexItem t4;
            do {
                t4 = Parser::GetNextToken(in, line);
            } while (t4 != END);
            Parser::PushBackToken(t4);
        }
        tok = Parser::GetNextToken(in, line);
    }

    if (tok == END) {
        tok = Parser::GetNextToken(in, line);
        if (tok == IF) {
            tok = Parser::GetNextToken(in, line);
            if (tok != SEMICOL) {
                --line;
                ParseError(line, "Missing semicolon at end of statement");
                return false;
            }
            return true;
        }
    }

    ParseError(line, "Missing closing END IF for If-statement.");
    return false;
}//End of IfStmt function

//GetStmt := Get (Var) ;
bool GetStmt(istream& in, int& line)
{
    LexItem t = Parser::GetNextToken(in, line);
    if (t != GET) {
        ParseError(line, "Missing Get Keyword");
        return false;
    }

    t = Parser::GetNextToken(in, line);
    if (t != LPAREN) {
        ParseError(line, "Missing Left Parenthesis");
        return false;
    }

    LexItem idtok;
    if (!Var(in, line, idtok)) {
        ParseError(line, "Missing a variable for an input statement");
        return false;
    }
    string name = idtok.GetLexeme();

    t = Parser::GetNextToken(in, line);
    if (t != RPAREN) {
        ParseError(line, "Missing Right Parenthesis");
        return false;
    }

    t = Parser::GetNextToken(in, line);
    if (t != SEMICOL) {
        line--;
        ParseError(line, "Missing semicolon at end of statement");
        return false;
    }

    auto sit = SymTable.find(name);
    if (sit == SymTable.end()) {
        ParseError(line, "Run-Time Error: Undeclared Variable");
        return false;
    }

    Value inputVal;
    switch (sit->second) {
        case INT: {
            int v; cin >> v;
            inputVal = Value(v);
            break;
        }
        case FLOAT: {
            double d; cin >> d;
            inputVal = Value(d);
            break;
        }
        case STRING: {
            string s; cin >> s;
            inputVal = Value(s);
            break;
        }
        case CHAR: {
            char c; cin >> c;
            inputVal = Value(c);
            break;
        }
        case BOOL: {
            string bs; cin >> bs;
            bool b = (bs == "true");
            inputVal = Value(b);
            break;
        }
        default:
            ParseError(line, "Run-Time Error: Cannot read into this type");
            return false;
    }

    TempsResults[name] = inputVal;
    return true;
}//end of GetStmt

//Var ::= ident
bool Var(std::istream& in, int& line, LexItem& idtok)
{
    idtok = Parser::GetNextToken(in, line);
    if (idtok == IDENT) {
        const string& name = idtok.GetLexeme();
        if (defVar.find(name) == defVar.end()) {
            ParseError(line, "Run-Time Error: Undeclared Variable");
            return false;
        }
        return true;
    }
    if (idtok.GetToken() == ERR) {
        ParseError(line, "Run-Time Error: Unrecognized Input Pattern");
        cout << "(" << idtok.GetLexeme() << ")" << endl;
        return false;
    }
    Parser::PushBackToken(idtok);
    return false;
}
//End of Var

//AssignStmt:= Var := Expr
bool AssignStmt(istream& in, int& line)
{
    LexItem idtok;
    if (!Var(in, line, idtok)) {
        ParseError(line, "Missing Left-Hand Side Variable in Assignment statement");
        return false;
    }
    string name = idtok.GetLexeme();

    LexItem t = Parser::GetNextToken(in, line);
    if (t != ASSOP) {
        if (t == ERR) {
            ParseError(line, "Unrecognized Input Pattern");
            cout << "(" << t.GetLexeme() << ")" << endl;
            return false;
        }
        ParseError(line, "Missing Assignment Operator");
        return false;
    }

    Value rhs;
    if (!Expr(in, line, rhs)) {
        ParseError(line, "Missing Expression in Assignment Statement");
        return false;
    }

    Token expected = SymTable[name];
    bool okay =
        (expected == INT    && rhs.IsInt())    ||
        (expected == FLOAT  && rhs.IsReal())   ||
        (expected == STRING && rhs.IsString()) ||
        (expected == CHAR   && rhs.IsChar())   ||
        (expected == BOOL   && rhs.IsBool());
    if (!okay) {
        ParseError(line, "Run-Time Error: Illegal Assignment Operation");
        return false;
    }

    TempsResults[name] = rhs;

    t = Parser::GetNextToken(in, line);
    if (t != SEMICOL) {
        ParseError(line, "Missing semicolon at end of statement");
        return false;
    }
    return true;
}
//End of AssignStmt

//Expr ::= Relation {(AND | OR) Relation }
bool Expr(std::istream& in, int& line, Value& retVal)
{
    Value lhs;
    if (!Relation(in, line, lhs))
        return false;

    retVal = lhs;
    LexItem tok = Parser::GetNextToken(in, line);
    if (tok.GetToken() == ERR) {
        ParseError(line, "Unrecognized Input Pattern");
        std::cout << "(" << tok.GetLexeme() << ")" << std::endl;
        return false;
    }
    while (tok == OR || tok == AND) {
        Value rhs;
        if (!Relation(in, line, rhs)) {
            ParseError(line, "Missing operand after operator");
            return false;
        }
        if (tok == OR)  retVal = retVal || rhs;
        else            retVal = retVal && rhs;

        tok = Parser::GetNextToken(in, line);
        if (tok.GetToken() == ERR) {
            ParseError(line, "Unrecognized Input Pattern");
            std::cout << "(" << tok.GetLexeme() << ")" << std::endl;
            return false;
        }
    }
    Parser::PushBackToken(tok);
    return true;
}
//End of Expr

//Relation ::= SimpleExpr [  ( = | /= | < | <= | > | >= )  SimpleExpr ]
bool Relation(std::istream& in, int& line, Value& retVal)
{
    Value lhs;
    if (!SimpleExpr(in, line, lhs))
        return false;

    LexItem tok = Parser::GetNextToken(in, line);
    if (tok.GetToken() == ERR) {
        ParseError(line, "Unrecognized Input Pattern");
        std::cout << "(" << tok.GetLexeme() << ")" << std::endl;
        return false;
    }

    if (tok == EQ || tok == NEQ || tok == LTHAN || tok == GTHAN || tok == LTE || tok == GTE) {
        Value rhs;
        if (!SimpleExpr(in, line, rhs)) {
            ParseError(line, "Missing operand after operator");
            return false;
        }
        switch (tok.GetToken()) {
            case EQ:   retVal = lhs == rhs; break;
            case NEQ:  retVal = lhs != rhs; break;
            case LTHAN:retVal = lhs < rhs;  break;
            case GTHAN:retVal = lhs > rhs;  break;
            case LTE:  retVal = lhs <= rhs; break;
            case GTE:  retVal = lhs >= rhs; break;
            default:   break;
        }
        tok = Parser::GetNextToken(in, line);
        if (tok.GetToken() == ERR) {
            ParseError(line, "Unrecognized Input Pattern");
            std::cout << "(" << tok.GetLexeme() << ")" << std::endl;
            return false;
        }
    }
    else {
        retVal = lhs;
    }

    Parser::PushBackToken(tok);
    return true;
}//End of Relation

//SimpleExpr ::= STerm {  ( + | - | & ) STerm }
bool SimpleExpr(std::istream& in, int& line, Value& retVal)
{
    if (!STerm(in, line, retVal))
        return false;

    LexItem tok = Parser::GetNextToken(in, line);
    while (tok == PLUS || tok == MINUS || tok == CONCAT) {
        Value rhs;
        if (!STerm(in, line, rhs)) {
            ParseError(line, "Missing operand after operator");
            return false;
        }
        if (tok == PLUS)       retVal = retVal + rhs;
        else if (tok == MINUS) retVal = retVal - rhs;
        else                   retVal = retVal.Concat(rhs);

        tok = Parser::GetNextToken(in, line);
    }

    if (tok.GetToken() == ERR) {
        ParseError(line, "Unrecognized Input Pattern");
        std::cout << "(" << tok.GetLexeme() << ")" << std::endl;
        return false;
    }

    Parser::PushBackToken(tok);
    return true;
}
//End of SimpleExpr

//STerm ::= [( - | + | NOT )] Term
bool STerm(istream& in, int& line,  Value& retVal)
{
	LexItem t = Parser::GetNextToken(in, line);
	bool status;
	int sign = 0;
	if(t == MINUS )
	{
		sign = -1;
	}
	else if(t == PLUS)
	{
		sign = 1;
	}
	else
		Parser::PushBackToken(t);
		
	status = Term(in, line, sign, retVal);
	return status;
}//End of STerm

//Term:= Factor {( * | / | MOD) Factor}
bool Term(std::istream& in, int& line, int sign, Value& retVal)
{
    if (!Factor(in, line, sign, retVal))
        return false;

    LexItem tok = Parser::GetNextToken(in, line);
    while (tok == MULT || tok == DIV || tok == MOD) {
        Value rhs;
        if (!Factor(in, line, 0, rhs)) {
            ParseError(line, "Missing operand after operator");
            return false;
        }
        if (tok == MULT)      retVal = retVal * rhs;
        else if (tok == DIV)  retVal = retVal / rhs;
        else                   retVal = retVal % rhs;

        tok = Parser::GetNextToken(in, line);
    }

    if (tok.GetToken() == ERR) {
        ParseError(line, "Unrecognized Input Pattern");
        std::cout << "(" << tok.GetLexeme() << ")" << std::endl;
        return false;
    }

    Parser::PushBackToken(tok);
    return true;
}
//End of Term

//Factor ::= Primary [** Primary ] | NOT Primary
bool Factor(std::istream& in, int& line, int sign, Value& retVal)
{
    LexItem tok = Parser::GetNextToken(in, line);
    if (tok == NOT) {
        if (!Primary(in, line, 0, retVal)) {
            ParseError(line, "Incorrect operand for NOT operator");
            return false;
        }
        if (!retVal.IsBool()) {
            ParseError(line, "Run-Time Error: Illegal operand for NOT");
            return false;
        }
        retVal = Value(!retVal.GetBool());
        return true;
    }
    Parser::PushBackToken(tok);
    if (!Primary(in, line, sign, retVal)) {
        ParseError(line, "Incorrect operand");
        return false;
    }
    tok = Parser::GetNextToken(in, line);
    if (tok == EXP) {
        Value exponent;
        if (!Primary(in, line, 0, exponent)) {
            ParseError(line, "Missing raised power for exponent operator");
            return false;
        }
        retVal = retVal.Exp(exponent);
        return true;
    }
    Parser::PushBackToken(tok);
    return true;
}//End of Factor

//Primary ::= Name | ICONST | FCONST | SCONST | BCONST | CCONST | (Expr)
bool Primary(std::istream& in, int& line, int sign, Value& retVal) {
    LexItem tok = Parser::GetNextToken(in, line);

    if (tok == IDENT) {    
        Parser::PushBackToken(tok);
        if (!Name(in, line, sign, retVal)) {
            ParseError(line, "Invalid reference to a variable.");
            return false;
        }
        return true;
    }
    else if (tok == ICONST) {
        int v = std::stoi(tok.GetLexeme());
        retVal = Value(v);
        if (sign < 0) retVal = Value(-v);
        return true;
    }
    else if (tok == SCONST) {
        retVal = Value(tok.GetLexeme());
        return true;
    }
    else if (tok == FCONST) {
        double v = std::stod(tok.GetLexeme());
        retVal = Value(v);
        if (sign < 0) retVal = Value(-v);
        return true;
    }
    else if (tok == BCONST) {
        bool b = (tok.GetLexeme() == "true");
        retVal = Value(b);
        return true;
    }
    else if (tok == CCONST) {
        char c = tok.GetLexeme()[0];
        retVal = Value(c);
        return true;
    }
    else if (tok == LPAREN) {
        if (!Expr(in, line, retVal)) {
            ParseError(line, "Invalid expression after left parenthesis");
            return false;
        }
        tok = Parser::GetNextToken(in, line);
        if (tok != RPAREN) {
            ParseError(line, "Missing right parenthesis");
            return false;
        }
        if (sign < 0) {
            if      (retVal.IsInt())  retVal = Value(-retVal.GetInt());
            else if (retVal.IsReal()) retVal = Value(-retVal.GetReal());
            else {
                ParseError(line, "Run-Time Error: Invalid operand for unary sign");
                return false;
            }
        }
        return true;
    }
    else if (tok.GetToken() == ERR) {
        ParseError(line, "Unrecognized Input Pattern");
        std::cout << "(" << tok.GetLexeme() << ")" << std::endl;
        return false;
    }

    Parser::PushBackToken(tok);
    ParseError(line, "Invalid Expression");
    return false;
}//End of Primary


//Name ::= IDENT [ ( Range ) ]
bool Name(std::istream& in, int& line, int sign, Value& retVal) {
    LexItem tok = Parser::GetNextToken(in, line);
    if (tok != IDENT) {
        Parser::PushBackToken(tok);
        ParseError(line, "Expected Identifier");
        return false;
    }
    std::string lexeme = tok.GetLexeme();

    // declaration check without inserting new keys
    if (defVar.find(lexeme) == defVar.end()) {
        ParseError(line, "Run-Time Error: Using undefined variable");
        return false;
    }

    auto it = TempsResults.find(lexeme);
    if (it == TempsResults.end()) {
        ParseError(line, "Run-Time Error: Using uninitialized variable");
        return false;
    }

    retVal = it->second;

    if (sign != 0) {
        if      (retVal.IsInt())  retVal = Value(sign * retVal.GetInt());
        else if (retVal.IsReal()) retVal = Value(sign * retVal.GetReal());
        else {
            ParseError(line, "Run-Time Error: Invalid operand for unary sign");
            return false;
        }
    }

    tok = Parser::GetNextToken(in, line);
    if (tok == LPAREN) {
        Value retVal1, retVal2;
        if (!Range(in, line, retVal1, retVal2)) {
            ParseError(line, "Run-Time Error: Invalid range definition");
            return false;
        }
        tok = Parser::GetNextToken(in, line);
        if (tok != RPAREN) {
            ParseError(line, "Run-Time Error: Missing closing ')'");
            return false;
        }

        if (!retVal.IsString()) {
            ParseError(line, "Run-Time Error: Cannot index non-string");
            return false;
        }
        const std::string& s = retVal.GetString();
        int i1 = retVal1.GetInt(), i2 = retVal2.GetInt();
        if (i1 < 0 || i2 >= (int)s.size() || i1 > i2) {
            ParseError(line, "Run-Time Error: Substring bounds error");
            return false;
        }
        if (i1 == i2) {
            retVal = Value(s[i1]);            
        } else {
            retVal = Value(s.substr(i1, i2 - i1 + 1));  
        }

    }

    else {
        Parser::PushBackToken(tok);
    }
    return true;
}//End of Name


// Range ::= SimpleExpr [ .. SimpleExpr ]
bool Range(std::istream& in, int& line, Value& retVal1, Value& retVal2) {
    if (!SimpleExpr(in, line, retVal1)) {
        ParseError(line, "Run-Time Error: Invalid lower bound in range");
        return false;
    }

    LexItem tok = Parser::GetNextToken(in, line);
    if (tok == DOT) {
        tok = Parser::GetNextToken(in, line);
        if (tok != DOT) {
            ParseError(line, "Run-Time Error: Invalid range syntax; expected '..'");
            return false;
        }
        if (!SimpleExpr(in, line, retVal2)) {
            ParseError(line, "Run-Time Error: Invalid upper bound in range");
            return false;
        }
    }
    else {
        Parser::PushBackToken(tok);
        retVal2 = retVal1;
    }
    return true;
}//End of Range
