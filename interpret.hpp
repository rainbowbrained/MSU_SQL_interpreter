#ifndef _SQL_H_
#define _SQL_H_

#include "dbms.hpp"
#include <algorithm>
#include <cctype>
#include <cstring>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include <unistd.h>

enum sql_exception_code
{
    XW_like_in,
    XW_like_str,
    XW_log_rel,
    XW_log_expr,
    XW_in,
    XW_long_br,
    XW_long_lex,
    XW_long_stol,
    XW_long_expr,
    XW_in_br,
    XW_empty_lexer,
    XW_lexer_nexist,
    SE_nexist,
    SE_expr,
    CR_open,
    CR_close,
    CR_expr,
    IS_open,
    IS_close,
    IS_expr,
    IS_range,
    DR_expr,
    DL_expr,
    UP_expr,
    UP_nexist,
    UP_eq,
    UP_info,
    UP_range,
    expr
};

class SQL_Xception
{
public:
    std::string Message;
    void report();
    SQL_Xception(sql_exception_code);
    SQL_Xception(const std::string &aMessage) : Message(aMessage){};
    SQL_Xception(const SQL_Xception &xception) : Message(xception.Message){};
    ~SQL_Xception() {}
};

void SQL_Xception::report()
{
    std::cerr << Message << std::endl;
    return;
};

SQL_Xception::SQL_Xception(sql_exception_code errcode)
{
    switch (errcode)
    {
    case SE_nexist:
        Message = "Error in the select sentenece: no such field";
        break;
    case SE_expr:
        Message = "Error in the select sentenece: wrong expression";
        break;
    case CR_open:
        Message = "Error in the create sentenece: expected opening bracket";
        break;
    case CR_close:
        Message = "Error in the create sentenece: expected opening bracket";
        break;
    case CR_expr:
        Message = "Error in the create sentenece: wrong expression";
        break;
    case IS_open:
        Message = "Error in the insert sentenece: expected opening bracket";
        break;
    case IS_close:
        Message = "Error in the insert sentenece: expected opening bracket";
        break;
    case IS_expr:
        Message = "Error in the insert sentenece: wrong expression";
        break;
    case IS_range:
        Message = "Error in the insert sentenece: text is too big to be inserted";
        break;
    case DR_expr:
        Message = "Error in the drop sentenece: wrong expression";
        break;
    case DL_expr:
        Message = "Error in the delete sentenece: wrong expression";
        break;
    case UP_expr:
        Message = "Error in the update sentenece: wrong expression";
        break;
    case UP_nexist:
        Message = "Error in the update sentenece: such field does not exist";
        break;
    case UP_eq:
        Message = "Error in the update sentenece: wrong expression, expected '='";
        break;
    case UP_info:
        Message = "Error in the update sentenece: can not get information about the field";
        break;
    case UP_range:
        Message = "Error in the update sentenece: text is too big to be inserted";
        break;
    case XW_like_in:
        Message = "Error in where clause: expected 'LIKE' or 'IN' word";
        break;
    case XW_like_str:
        Message = "Error in where clause: expected string";
        break;
    case XW_log_rel:
        Message = "Error in where clause: expected relation sign";
        break;
    case XW_log_expr:
        Message = "Error in where clause: unknown logic expression";
        break;
    case XW_in:
        Message = "Error in where clause: expected 'IN' word";
        break;
    case XW_long_br:
        Message = "Error in where clause: expected closing bracket in the long expression";
        break;
    case XW_long_lex:
        Message = "Error in where clause: unknown lexeme in the long expression";
        break;
    case XW_long_stol:
        Message = "Error: error while converting string to long";
        break;
    case XW_long_expr:
        Message = "Error in where clause: unknown expression";
        break;
    case XW_in_br:
        Message = "Error in where clause: expected closing bracket in the IN-clause";
        break;
    case XW_empty_lexer:
        Message = "Error in where clause (lexer where): empty string";
        break;
    case XW_lexer_nexist:
        Message = "Error in where clause (lexer where): no such title in the table";
        break;
    case expr:
        Message = "Error: unkown statement";
        break;
    default:
        break;
    }
};

// Interpreter --- SQL-interpreter class
class Interpreter
{
private:
    void select_sentence(std::string &);
    void insert_sentence(std::string &);
    void update_sensence(std::string &);
    void delete_sentence(std::string &);
    void create_sentence(std::string &);
    void drop_sentence(std::string &);
    void field_description(std::string &);
    std::vector<size_t> where_clause(std::string &);
    ITable bd_table;

public:
    Interpreter(std::string &);
    ~Interpreter() {}
};

/*-----------------------------------------------------------------------------------------------*/
// read one word and one symbol from line
std::string read_word(std::string &str, char separator = ' ')
{
    std::string word;
    int i = 0;
    while (isspace(str.c_str()[i]) && !(str.empty()))
    {
        str.erase(0, 1);
    }
    while (!isspace(str.c_str()[i]) && !(str.empty()) && (str.c_str()[i] != separator))
    {
        word = word + str.c_str()[i];
        str.erase(0, 1);
    }
    while (isspace(str.c_str()[i]) && !(str.empty()))
    {
        str.erase(0, 1);
    }
    return word;
}

// read one symbol from line
std::string read_symb(std::string &str)
{
    std::string word;
    word += str.c_str()[0];
    str.erase(0, 1);
    return word;
}

// change all constructions in string like "from" to constructions "to" (for tokenization)
void replace_all(std::string &str, const std::string &from, const std::string &to)
{
    if (from.empty())
    {
        return;
    }
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
    return;
}

/*---------------------------------------- LONG_EXPR --------------------------------------------*/
/* lexical parser for long-expressions

LONG_EXPR   = LONG_SUM { S_OP LONG_SUM }
S_OP        = + | -
LONG_SUM    = LONG_MULT { M_OP LONG_MULT }
M_OP        = * | \ | %
LONG_MULT   = LONG_VAL | (LONG_EXPR)
LONG_VAL    = id | longint
*/

enum long_type_t
{
    START,
    PLUS,   // +
    MINUS,  // -
    MULT,   // *
    DIV,    // /
    MOD,    // %
    OPEN,   // (
    CLOSE,  // )
    NUMBER, // set of numbers
    L_NAME, // name of the field with type LONG
    END
};

namespace lexer_long_expr
{
    enum long_type_t cur_lex_type; // type of a current lexem
    std::string cur_lex_text;      // previous lexem
    std::string c;                 // current lexeme

    // read first word, set current lexem type to START
    void init(std::string &str)
    {
        c = read_word(str);
        cur_lex_type = START;
    }

    // read next words and set new lexem type
    void next(std::string &str)
    {
        cur_lex_text.resize(0);
        long_type_t state = START;
        while (state != END)
        {
            switch (state)
            {
            case START:
                if (c == "+")
                {
                    state = PLUS; // plus
                }
                else if (c == "-")
                {
                    state = MINUS; // mius
                }
                else if (c == "*")
                {
                    state = MULT; // multiply
                }
                else if (c == "/")
                {
                    state = DIV; // div
                }
                else if (c == "%")
                {
                    state = MOD; // mod
                }
                else if (c == "(")
                {
                    state = OPEN; // open bracket
                }
                else if (c == ")")
                {
                    state = CLOSE; // close bracket
                }
                // words, that can follow long-expression
                else if ((c == "NOT") || (c == "IN") ||
                         (c == "WHERE") || (c == "=") ||
                         (c == ">") || (c == "<") ||
                         (c == ">=") || (c == "<=") ||
                         (c == "!=") || (c == "AND") ||
                         (c == "OR") || c.empty())
                {
                    cur_lex_type = END;
                    state = END;
                }
                else
                {
                    unsigned int i = 0;
                    state = NUMBER;
                    while ((i < c.length()))
                    {
                        if (!isdigit(c[i]))
                        {
                            // not long integer, maybe it is a long_name
                            i = c.length();
                            state = L_NAME;
                        }
                        i++;
                    }
                }
                break;

            case PLUS:
                cur_lex_type = PLUS;
                state = END;
                break;

            case MINUS:
                cur_lex_type = MINUS;
                state = END;
                break;

            case MULT:
                cur_lex_type = MULT;
                state = END;
                break;

            case DIV:
                cur_lex_type = DIV;
                state = END;
                break;

            case MOD:
                cur_lex_type = MOD;
                state = END;
                break;

            case OPEN:
                cur_lex_type = OPEN;
                state = END;
                break;

            case CLOSE:
                cur_lex_type = CLOSE;
                state = END;
                break;

            case NUMBER:
                cur_lex_type = NUMBER;
                state = END;
                break;

            case L_NAME:
                cur_lex_type = L_NAME;
                state = END;
                break;

            case END:
                break;

            default:
                break;
            }

            if (state != END)
            {
                if (cur_lex_type != END)
                {
                    cur_lex_text = c;
                }
                c = read_word(str);
            }
        }
    }
}

// syntactic parser for long-expressions
namespace parser_long_expr
{
    // the number of the row in the table from which to insert the value in the expression
    size_t line_to_check;

    // read first word, set current lexem type to START
    void init(std::string &str)
    {
        lexer_long_expr ::init(str);
        lexer_long_expr ::next(str);
    }

    // functions for syntactic parser
    std::string A(std::string &); // processes + | - (S_OP)
    std::string B(std::string &); // processes * | / | % (M_OP) and LONG_SUM
    std::string C(std::string &); // LONG_MULT, longint, LONG id and brackets (calls A())

    // functions to calculate the value for each row in the table
    long Al(std::string &, ITable &);
    long Bl(std::string &, ITable &);
    long Cl(std::string &, ITable &);

    std::string A(std::string &str)
    {
        std::string s = B(str);
        while ((lexer_long_expr::cur_lex_type == PLUS) || (lexer_long_expr::cur_lex_type == MINUS))
        {
            // LONG_SUM { S_OP LONG_SUM }
            if (lexer_long_expr::cur_lex_type == PLUS)
            {
                lexer_long_expr::next(str);
                s = s + "+ " + B(str);
            }
            else if (lexer_long_expr::cur_lex_type == MINUS)
            {
                lexer_long_expr::next(str);
                s = s + "- " + B(str);
            }
        }
        return s;
    }

    std::string B(std::string &str)
    {
        std::string s = C(str);
        while ((lexer_long_expr::cur_lex_type == MULT) ||
               (lexer_long_expr::cur_lex_type == DIV) ||
               (lexer_long_expr::cur_lex_type == MOD))
        {
            // LONG_MULT { M_OP LONG_MULT }
            if (lexer_long_expr::cur_lex_type == MULT)
            {
                lexer_long_expr::next(str);
                s = s + "* " + C(str);
            }
            else if (lexer_long_expr::cur_lex_type == DIV)
            {
                lexer_long_expr::next(str);
                s = s + "/ " + C(str);
            }
            else if (lexer_long_expr::cur_lex_type == MOD)
            {
                lexer_long_expr::next(str);
                s = s + "% " + C(str);
            }
        }
        return s;
    }

    std::string C(std::string &str)
    {
        std::string s;

        if (lexer_long_expr::cur_lex_type == OPEN) // LONG_MULT =  '(' LONG_EXPR ')'
        {
            lexer_long_expr::next(str);
            s = "( " + A(str);
            if (lexer_long_expr::cur_lex_type != CLOSE)
            {
                throw SQL_Xception(XW_long_br);
            }
            s = s + ") ";
            lexer_long_expr::next(str);
        }
        else if (lexer_long_expr::cur_lex_type == NUMBER) // LONG_MULT = longint
        {
            s = lexer_long_expr::cur_lex_text + " ";
            lexer_long_expr::next(str);
        }
        else if (lexer_long_expr::cur_lex_type == L_NAME) // LONG_MULT = id
        {
            s = lexer_long_expr::cur_lex_text + " ";
            lexer_long_expr::next(str);
        }
        else
        {
            throw SQL_Xception(XW_long_lex);
        }
        return s;
    }

    long Al(std::string &str, ITable &line)
    {
        long num = parser_long_expr::Bl(str, line);
        while ((lexer_long_expr::cur_lex_type == PLUS) ||
               (lexer_long_expr::cur_lex_type == MINUS))
        {
            if (lexer_long_expr::cur_lex_type == PLUS)
            {
                lexer_long_expr::next(str);
                num = num + parser_long_expr::Bl(str, line);
            }
            else if (lexer_long_expr::cur_lex_type == MINUS)
            {
                lexer_long_expr::next(str);
                num = num - parser_long_expr::Bl(str, line);
            }
        }
        return num;
    }

    long Bl(std::string &str, ITable &line)
    {
        long num = parser_long_expr::Cl(str, line);
        while ((lexer_long_expr::cur_lex_type == MULT) ||
               (lexer_long_expr::cur_lex_type == DIV) ||
               (lexer_long_expr::cur_lex_type == MOD))
        {
            if (lexer_long_expr::cur_lex_type == MULT)
            {
                lexer_long_expr::next(str);
                num = num * parser_long_expr::Cl(str, line);
            }
            else if (lexer_long_expr::cur_lex_type == DIV)
            {
                lexer_long_expr::next(str);
                num = num / parser_long_expr::Cl(str, line);
            }
            else if (lexer_long_expr::cur_lex_type == MOD)
            {
                lexer_long_expr::next(str);
                num = num % parser_long_expr::Cl(str, line);
            }
        }
        return num;
    }

    long Cl(std::string &str, ITable &line)
    {
        long num = 0;
        if (lexer_long_expr::cur_lex_type == OPEN)
        {
            lexer_long_expr::next(str);
            num = parser_long_expr::Al(str, line);
            if (lexer_long_expr::cur_lex_type != CLOSE)
            {
                throw SQL_Xception(XW_long_br);
            }
            lexer_long_expr::next(str);
        }
        else if (lexer_long_expr::cur_lex_type == NUMBER)
        {
            try
            {
                // convert string to long
                num = stol(lexer_long_expr::cur_lex_text);
            }
            catch (...)
            {
                throw SQL_Xception(XW_long_stol);
            }
            lexer_long_expr::next(str);
        }
        else if (lexer_long_expr::cur_lex_type == L_NAME)
        {
            // get value from the table
            num = line.GetLongField(lexer_long_expr::cur_lex_text, line_to_check);
            lexer_long_expr::next(str);
        }
        else
        {
            throw SQL_Xception(XW_long_lex);
        }
        return num;
    }
}

/*--------------------------------------- WHERE-CLAUSE ------------------------------------------*/
/* lexical parser for where-clause
WHERE = 'WHERE' TEXT_ID ['NOT'] 'LIKE' STR | EXPR ['NOT'] 'IN' (CONSTS) | 'WHERE' LOG | 'WHERE ALL'
*/

enum where_type_t
{
    START_w,
    PLUS_w,   // +
    MINUS_w,  // -
    OR_w,     // OR
    MULT_w,   // *
    DIV_w,    // /
    MOD_w,    // %
    AND_w,    // AND
    NOT_w,    // NOT
    OPEN_w,   // (
    CLOSE_w,  // )
    REL_w,    // =, >, <, !=, >=, <=
    NUMBER_w, // set of numbers
    L_NAME_w, // name of the field with the type LONG
    T_NAME_w, // name of the field with the type TEXT
    STR_w,    // line
    LIKE_w,   // LIKE
    IN_w,     // IN
    COM_w,    // ,
    ALL_w,    // ALL
    END_w
};

namespace lexer_where
{
    enum where_type_t cur_lex_type_w;
    std::string cur_lex_text_w;
    std::string c_w;

    // read first word, set current lexem type to START_w
    void init(std::string &str)
    {
        c_w = read_word(str);
        cur_lex_type_w = START_w;
    }

    void next(std::string &str, ITable bd)
    {
        cur_lex_text_w.resize(0);
        where_type_t state_w = START_w;
        while (state_w != END_w)
        {
            switch (state_w)
            {
            case START_w:
                if (c_w == "+")
                {
                    state_w = PLUS_w;
                }
                else if (c_w == "-")
                {
                    state_w = MINUS_w;
                }
                else if (c_w == "OR")
                {
                    state_w = OR_w;
                }
                else if (c_w == "*")
                {
                    state_w = MULT_w;
                }
                else if (c_w == "/")
                {
                    state_w = DIV_w;
                }
                else if (c_w == "%")
                {
                    state_w = MOD_w;
                }
                else if (c_w == "AND")
                {
                    state_w = AND_w;
                }
                else if (c_w == "NOT")
                {
                    state_w = NOT_w;
                }
                else if (c_w == "(")
                {
                    state_w = OPEN_w;
                }
                else if (c_w == ")")
                {
                    state_w = CLOSE_w;
                }
                else if ((c_w == "=") || (c_w == "!=") ||
                         (c_w == ">") || (c_w == ">=") ||
                         (c_w == "<") || (c_w == "<="))
                {
                    state_w = REL_w;
                }
                else if (c_w == "LIKE")
                {
                    state_w = LIKE_w;
                }
                else if (c_w == "IN")
                {
                    state_w = IN_w;
                }
                else if (c_w == ",")
                {
                    state_w = COM_w;
                }
                else if (c_w == "ALL")
                {
                    state_w = ALL_w;
                }
                else if (c_w.empty())
                {
                    cur_lex_type_w = END_w;
                    state_w = END_w;
                }
                else // number, text field's title or long field's title
                {
                    unsigned int i = 0;
                    bool flag_long = 1;
                    while ((i < c_w.length()) && flag_long)
                    {
                        if (!isdigit(c_w[i]))
                            flag_long = 0; // not long
                        i++;
                    }
                    if (flag_long)
                        state_w = NUMBER_w;

                    else if (c_w[0] == '\'') // line
                    {
                        state_w = STR_w;
                        if (c_w[c_w.length() - 1] == '\'')
                        {
                            cur_lex_type_w = STR_w;
                            cur_lex_text_w = cur_lex_text_w + c_w + " ";
                            c_w = read_word(str);
                            state_w = END_w;
                        }
                    }
                    else
                    {
                        // check if the word is the field and its type
                        Type t;
                        try
                        {
                            t = bd.GetFieldType(c_w);
                        }
                        catch (...)
                        {
                            throw SQL_Xception(XW_lexer_nexist);
                        }
                        if (t == TEXT)
                            state_w = T_NAME_w;
                        else
                            state_w = L_NAME_w;
                    }
                }
                break;

            case PLUS_w:
                cur_lex_type_w = PLUS_w;
                state_w = END_w;
                break;

            case MINUS_w:
                cur_lex_type_w = MINUS_w;
                state_w = END_w;
                break;

            case OR_w:
                cur_lex_type_w = OR_w;
                state_w = END_w;
                break;

            case MULT_w:
                cur_lex_type_w = MULT_w;
                state_w = END_w;
                break;

            case DIV_w:
                cur_lex_type_w = DIV_w;
                state_w = END_w;
                break;

            case MOD_w:
                cur_lex_type_w = MOD_w;
                state_w = END_w;
                break;

            case AND_w:
                cur_lex_type_w = AND_w;
                state_w = END_w;
                break;

            case NOT_w:
                cur_lex_type_w = NOT_w;
                state_w = END_w;
                break;

            case OPEN_w:
                cur_lex_type_w = OPEN_w;
                state_w = END_w;
                break;

            case CLOSE_w:
                cur_lex_type_w = CLOSE_w;
                state_w = END_w;
                break;

            case REL_w:
                cur_lex_type_w = REL_w;
                state_w = END_w;
                break;

            case NUMBER_w:
                cur_lex_type_w = NUMBER_w;
                state_w = END_w;
                break;

            case L_NAME_w:
                cur_lex_type_w = L_NAME_w;
                state_w = END_w;
                break;

            case T_NAME_w:
                cur_lex_type_w = T_NAME_w;
                state_w = END_w;
                break;

            case STR_w:
                // searching fot the line end
                if (c_w.empty())
                {
                    throw SQL_Xception(XW_empty_lexer);
                }
                if (c_w[c_w.length() - 1] == '\'') // otherwise stay in START_w
                {
                    cur_lex_type_w = STR_w;
                    cur_lex_text_w = cur_lex_text_w + c_w + " ";
                    c_w = read_word(str);
                    state_w = END_w;
                }
                break;

            case LIKE_w:
                cur_lex_type_w = LIKE_w;
                state_w = END_w;
                break;

            case IN_w:
                cur_lex_type_w = IN_w;
                state_w = END_w;
                break;

            case COM_w:
                cur_lex_type_w = COM_w;
                state_w = END_w;
                break;

            case ALL_w:
                cur_lex_type_w = ALL_w;
                state_w = END_w;
                break;

            case END_w:
                break;

            default:
                break;
            }

            if (state_w != END_w)
            {
                // read a new word for the future analyses
                if (cur_lex_type_w != END_w)
                {
                    cur_lex_text_w = cur_lex_text_w + c_w + " ";
                }
                c_w = read_word(str);
            }
        }
    }
}

// modes of where_clause
enum mode_type
{
    LIKE_alt, // 'WHERE' TEXT_ID ['NOT'] 'LIKE' STR |
    IN_alt_T, // 'WHERE' TEXT_EXPR ['NOT'] 'IN' (CONSTS)
    IN_alt_L, // 'WHERE' LONG_EXPR ['NOT'] 'IN' (CONSTS)
    LOG_alt,  // 'WHERE' LOG
    ALL_alt   // 'WHERE' 'ALL'
};

// syntactic parser for where-clause
namespace parser_where
{
    size_t line_to_check;
    bool flag_log = 0;
    bool flag_expr = 0;
    enum mode_type mode;
    std::multiset<long> mst_l;        // list of long constants
    std::multiset<std::string> mst_s; // list of string constants

    void init(std::string &str, ITable bd)
    {
        lexer_where ::init(str); //  c_w = read_word(str); cur_lex_type_w = START_w;
        lexer_where ::next(str, bd);
    }

    // functions for sintactic parser
    std::string W0(std::string &, ITable &); // beginning of where-clause
    std::string W1(std::string &, ITable &); // LIKE-alternative
    std::string W2(std::string &, ITable &); // IN-alternative
    std::string W3(std::string &, ITable &); // start processing long or logic expressions, call W4
    std::string W4(std::string &, ITable &); // long or logic expressions, call W5 or W7
    std::string W5(std::string &, ITable &); // long or logic expressions, call W3 (if in brackets), W6 or W7
    std::string W6(std::string &, ITable &); // parsing text expression and relations for logic-expr
    std::string W7(std::string &, ITable &); // brackets, call W3
    void W8(std::string &, ITable &);        // list of constants

    // functions to calculate the value of logic-expression
    long W31(std::string &, ITable &); // start funstion
    long W41(std::string &, ITable &);
    long W51(std::string &, ITable &);
    long W71(std::string &, ITable &); // brackets

    std::string W0(std::string &str, ITable &bd)
    {
        std::string s;
        if (lexer_where::cur_lex_type_w == ALL_w)
        {
            s = "ALL";
            mode = ALL_alt;
            lexer_where::next(str, bd);
        }
        else if (lexer_where::cur_lex_type_w == T_NAME_w)
        {
            // if the first word is name of the TEXT field, it can be LIKE, LOG or IN-alternative
            s = lexer_where::cur_lex_text_w;
            lexer_where::next(str, bd);

            if (lexer_where::cur_lex_type_w == NOT_w) // if there is the word "NOT"
            {
                s = s + lexer_where::cur_lex_text_w;
                lexer_where::next(str, bd);
            }
            if (lexer_where::cur_lex_type_w == LIKE_w)
            {
                s = s + lexer_where::cur_lex_text_w;
                lexer_where::next(str, bd);
                // processing of LIKE-alternative
                s = s + W1(str, bd);
            }
            else if (lexer_where::cur_lex_type_w == IN_w)
            {
                s = s + lexer_where::cur_lex_text_w;
                mode = IN_alt_T;
                // processing of IN-alternative
                s = s + W2(str, bd);
            }
            else
            {
                throw SQL_Xception(XW_like_in);
            }
        }

        else if (lexer_where::cur_lex_type_w == STR_w)
        {
            // the first word is a line, maybe IN-alternative (it is the text-expr)
            s = lexer_where::cur_lex_text_w;
            lexer_where::next(str, bd);

            if (lexer_where::cur_lex_type_w == NOT_w) // if there is the word "NOT"
            {
                s = s + lexer_where::cur_lex_text_w;
                lexer_where::next(str, bd);
            }
            if (lexer_where::cur_lex_type_w == IN_w)
            {
                s = s + lexer_where::cur_lex_text_w;
                mode = IN_alt_T;
                // processing of IN-alternative
                s = s + W2(str, bd);
            }
            else
            {
                throw SQL_Xception(XW_in);
            }
        }

        else if ((lexer_where::cur_lex_type_w == L_NAME_w) || (lexer_where::cur_lex_type_w == NUMBER_w))
        {
            // the first word is a set of numbers or name of the field with type LONG,
            // it can be IN-alternative (the beginning of long-expr)
            str.insert(0, lexer_where::cur_lex_text_w);
            lexer_where::c_w += " ";
            str.insert(lexer_where::cur_lex_text_w.length(), lexer_where::c_w);
            // processing of long-expression
            parser_long_expr::init(str);
            s = parser_long_expr::A(str);

            // if the expression is not right
            if (lexer_long_expr::cur_lex_type != END)
            {
                throw SQL_Xception(XW_long_expr);
            }
            s = s + " ";
            lexer_where::c_w = lexer_long_expr::c;
            lexer_where::next(str, bd);

            if (lexer_where::cur_lex_type_w == NOT_w)
            {
                s = s + lexer_where::cur_lex_text_w + " ";
                lexer_where::next(str, bd);
            }
            if (lexer_where::cur_lex_type_w == IN_w)
            {
                s = s + lexer_where::cur_lex_text_w + " ";
                mode = IN_alt_L;
                // processing of IN-alternative
                s = s + W2(str, bd);
            }
            else
            {
                throw SQL_Xception(XW_long_expr);
            }
        }
        else if ((lexer_where::cur_lex_type_w == NOT_w) ||
                 (lexer_where::cur_lex_type_w == OPEN_w))
        {
            // "NOT" and "(" can lead to long-expression or to logic-expression
            s = W3(str, bd);

            // if it is long-expression
            if (!flag_log)
            {
                if (lexer_where::cur_lex_type_w == NOT_w)
                {
                    s = s + "NOT ";
                    lexer_where::next(str, bd);
                }
                if (lexer_where::cur_lex_type_w == IN_w)
                {
                    s = s + "IN ";
                    mode = IN_alt_L;
                    // processing of IN-alternative
                    s = s + W2(str, bd);
                }
            }
            // if it is logic-expression
            else
            {
                mode = LOG_alt;
            }
        }
        else
        {
            throw SQL_Xception(XW_long_expr);
        }
        return s;
    }

    // processing of LIKE-alternative
    std::string W1(std::string &str, ITable &bd)
    {
        std::string s;
        // saving sample string
        if (lexer_where::cur_lex_type_w == STR_w)
        {
            s = lexer_where::cur_lex_text_w;
            lexer_where::next(str, bd);
        }
        else
        {
            throw SQL_Xception(XW_like_str);
        }
        mode = LIKE_alt;
        return s;
    }

    // processing of IN-alternative
    std::string W2(std::string &str, ITable &bd)
    {
        lexer_where::next(str, bd);
        std::string s = " ";
        if (lexer_where::cur_lex_type_w == OPEN_w)
        {
            lexer_where::next(str, bd);
            // list of constants
            W8(str, bd);
            if (lexer_where::cur_lex_type_w != CLOSE_w)
            {
                throw SQL_Xception(XW_in_br);
            }
            lexer_where::next(str, bd);
        }
        return s;
    }

    // the beginning og long- or logic-expression
    // flag_log == 1 => there is logic operators
    // flag_expr == 1 => processing long-expr

    std::string W3(std::string &str, ITable &bd)
    {
        std::string s;
        s = W4(str, bd);
        while ((lexer_where::cur_lex_type_w == PLUS_w) ||
               (lexer_where::cur_lex_type_w == MINUS_w) ||
               (lexer_where::cur_lex_type_w == OR_w))
        {
            // processing long-expression
            if (flag_expr)
            {
                if (lexer_where::cur_lex_type_w == PLUS_w)
                {
                    lexer_where::next(str, bd);
                    s = s + "+ ";
                }
                else if (lexer_where::cur_lex_type_w == MINUS_w)
                {
                    lexer_where::next(str, bd);
                    s = s + "- ";
                }
            }
            // processing logic-expression
            else
            {
                if (lexer_where::cur_lex_type_w == OR_w)
                {
                    lexer_where::next(str, bd);
                    s = s + "OR ";
                }
            }
            s = s + W4(str, bd);
        }
        return s;
    }

    std::string W4(std::string &str, ITable &bd)
    {
        std::string s;
        s = W5(str, bd);
        while ((lexer_where::cur_lex_type_w == MULT_w) ||
               (lexer_where::cur_lex_type_w == DIV_w) ||
               (lexer_where::cur_lex_type_w == MOD_w) ||
               (lexer_where::cur_lex_type_w == AND_w))
        {
            // processing long-expression now
            if (flag_expr)
            {
                if (lexer_where::cur_lex_type_w == MULT_w)
                {
                    lexer_where::next(str, bd);
                    s = s + "* ";
                }
                else if (lexer_where::cur_lex_type_w == DIV_w)
                {
                    lexer_where::next(str, bd);
                    s = s + "/ ";
                }
                else if (lexer_where::cur_lex_type_w == MOD_w)
                {
                    lexer_where::next(str, bd);
                    s = s + "% ";
                }
                s = s + W5(str, bd);
            }
            // processing logic-expression now
            else
            {
                if (lexer_where::cur_lex_type_w == AND_w)
                {
                    lexer_where::next(str, bd);
                    s = s + "AND ";
                }
                // (..) after logic operators
                s = s + W7(str, bd);
            }
        }
        return s;
    }

    std::string W5(std::string &str, ITable &bd)
    {
        std::string s;
        if (lexer_where::cur_lex_type_w == OPEN_w)
        {
            lexer_where::next(str, bd);
            s = "( " + W3(str, bd);
            if (lexer_where::cur_lex_type_w != CLOSE_w)
            {
                throw SQL_Xception(XW_long_br);
            }
            s = s + ") ";
            lexer_where::next(str, bd);
        }
        else if (lexer_where::cur_lex_type_w == NOT_w)
        {
            s = W7(str, bd);
        }
        else if (lexer_where::cur_lex_type_w == NUMBER_w)
        {
            flag_expr = 1; // processing long-expression, starting with a number
            s = lexer_where::cur_lex_text_w + " ";
            lexer_where::next(str, bd);
            if (lexer_where::cur_lex_type_w == REL_w)
            {
                flag_log = 1;
                s = s + lexer_where::cur_lex_text_w + " ";
                lexer_where::next(str, bd);
                s = s + W3(str, bd);
                flag_expr = 0; // end of processing long-expression
            }
        }
        else if (lexer_where::cur_lex_type_w == L_NAME_w)
        {
            flag_expr = 1; // processing long-expression, starting with a long field's name
            s = lexer_where::cur_lex_text_w + " ";
            lexer_where::next(str, bd);
            if (lexer_where::cur_lex_type_w == REL_w)
            {
                flag_log = 1;
                s = s + lexer_where::cur_lex_text_w + " ";
                lexer_where::next(str, bd);
                s = s + W3(str, bd);
                flag_expr = 0; // end of processing long-expression
            }
        }

        else if ((lexer_where::cur_lex_type_w == T_NAME_w) ||
                 (lexer_where::cur_lex_type_w == STR_w))
        {
            // logic expression
            flag_log = 1;
            s = lexer_where::cur_lex_text_w + " ";
            lexer_where::next(str, bd);
            s = s + W6(str, bd);
        }
        else
        {
            throw SQL_Xception(XW_long_expr);
        }
        return s;
    }

    std::string W6(std::string &str, ITable &bd)
    {
        std::string s;
        // processing text-expression for logic-expression
        if (lexer_where::cur_lex_type_w == REL_w)
        {
            s = lexer_where::cur_lex_text_w;
            s = s + " ";
            lexer_where::next(str, bd);
        }
        else
        {
            throw SQL_Xception(XW_log_rel);
        }
        if ((lexer_where::cur_lex_type_w == T_NAME_w) ||
            (lexer_where::cur_lex_type_w == STR_w))
        {
            s = s + lexer_where::cur_lex_text_w;
            s = s + " ";
            lexer_where::next(str, bd);
        }
        else
        {
            throw SQL_Xception(XW_log_expr);
        }
        return s;
    }

    std::string W7(std::string &str, ITable &bd)
    {
        std::string s;
        // (...) after logic operators
        if (lexer_where::cur_lex_type_w == OPEN_w)
        {
            lexer_where::next(str, bd);
            s = "( ";
            s = s + W3(str, bd);
            if ((lexer_where::cur_lex_type_w != CLOSE_w) || flag_expr)
            {
                throw SQL_Xception(XW_log_expr);
            }
            s = s + ") ";
            lexer_where::next(str, bd);
        }
        else if (lexer_where::cur_lex_type_w == NOT_w)
        {
            flag_log = 1;
            s = "NOT ";
            lexer_where::next(str, bd);
            s = s + W7(str, bd);
        }
        else
        {
            throw SQL_Xception(XW_log_expr);
        }
        return s;
    }

    // list of constants for IN-alternative
    void W8(std::string &str, ITable &bd)
    {
        // list of strings
        if (lexer_where::cur_lex_type_w == STR_w)
        {
            lexer_where::cur_lex_text_w.pop_back();  // " "
            lexer_where::cur_lex_text_w.pop_back();  // "'"
            lexer_where::cur_lex_text_w.erase(0, 1); // "'"
            mst_s.insert(lexer_where::cur_lex_text_w);
            lexer_where::next(str, bd);

            while (lexer_where::cur_lex_type_w == COM_w)
            {
                lexer_where::next(str, bd);
                if (lexer_where::cur_lex_type_w == STR_w)
                {
                    lexer_where::cur_lex_text_w.pop_back();  // " "
                    lexer_where::cur_lex_text_w.pop_back();  // "'"
                    lexer_where::cur_lex_text_w.erase(0, 1); // "'"
                    mst_s.insert(lexer_where::cur_lex_text_w);
                    lexer_where::next(str, bd);
                }
                else
                {
                    throw SQL_Xception(XW_long_lex);
                }
            }
        }
        // list of numbers
        else if (lexer_where::cur_lex_type_w == NUMBER_w)
        {
            lexer_where::cur_lex_text_w.pop_back();
            long num = 0;
            try
            {
                // convert string to long
                num = stol(lexer_where::cur_lex_text_w);
            }
            catch (...)
            {
                throw SQL_Xception(XW_long_stol);
            }
            mst_l.insert(num);
            lexer_where::next(str, bd);
            while (lexer_where::cur_lex_type_w == COM_w)
            {
                lexer_where::next(str, bd);
                if (lexer_where::cur_lex_type_w == NUMBER_w)
                {
                    lexer_where::cur_lex_text_w.pop_back();
                    try
                    {
                        num = stol(lexer_where::cur_lex_text_w);
                    }
                    catch (...)
                    {
                        throw SQL_Xception(XW_long_stol);
                    }
                    mst_l.insert(num);
                    lexer_where::next(str, bd);
                }
                else
                {
                    throw SQL_Xception(XW_long_lex);
                }
            }
        }
        else
        {
            throw SQL_Xception(XW_long_lex);
        }
    }

    long W31(std::string &str, ITable &bd)
    {
        long res;
        res = W41(str, bd);

        while ((lexer_where::cur_lex_type_w == PLUS_w) ||
               (lexer_where::cur_lex_type_w == MINUS_w) ||
               (lexer_where::cur_lex_type_w == OR_w))
        {
            // long-expression
            if (flag_expr)
            {
                if (lexer_where::cur_lex_type_w == PLUS_w)
                {
                    lexer_where::next(str, bd);
                    res = res + W41(str, bd);
                }
                else if (lexer_where::cur_lex_type_w == MINUS_w)
                {
                    lexer_where::next(str, bd);
                    res = res - W41(str, bd);
                }
            }
            // logic-expression
            else
            {
                if (lexer_where::cur_lex_type_w == OR_w)
                {
                    lexer_where::next(str, bd);
                    if (res || W41(str, bd))
                    {
                        res = 1;
                    }
                    else
                    {
                        res = 0;
                    }
                }
            }
        }
        return res;
    }

    long W41(std::string &str, ITable &bd)
    {
        long res;
        res = W51(str, bd);
        while ((lexer_where::cur_lex_type_w == MULT_w) ||
               (lexer_where::cur_lex_type_w == DIV_w) ||
               (lexer_where::cur_lex_type_w == MOD_w) ||
               (lexer_where::cur_lex_type_w == AND_w))
        {
            // long-expression
            if (flag_expr)
            {
                if (lexer_where::cur_lex_type_w == MULT_w)
                {
                    lexer_where::next(str, bd);
                    res = res * W51(str, bd);
                }
                else if (lexer_where::cur_lex_type_w == DIV_w)
                {
                    lexer_where::next(str, bd);
                    res = res / W51(str, bd);
                }
                else if (lexer_where::cur_lex_type_w == MOD_w)
                {
                    lexer_where::next(str, bd);
                    res = res % W51(str, bd);
                }
            }
            // logic-expression
            else
            {
                if (lexer_where::cur_lex_type_w == AND_w)
                {
                    lexer_where::next(str, bd);
                    if (res && W71(str, bd))
                    {
                        res = 1;
                    }
                    else
                    {
                        res = 0;
                    }
                }
            }
        }
        return res;
    }

    long W51(std::string &str, ITable &bd)
    {
        long res = 0;
        if (lexer_where::cur_lex_type_w == OPEN_w)
        {
            //expression in brackets, call start function W31
            lexer_where::next(str, bd);
            res = W31(str, bd);
            if (lexer_where::cur_lex_type_w != CLOSE_w)
            {
                throw SQL_Xception(XW_log_expr);
            }
            lexer_where::next(str, bd);
        }
        else if (lexer_where::cur_lex_type_w == NOT_w)
        {
            res = W71(str, bd);
        }
        else if ((lexer_where::cur_lex_type_w == L_NAME_w) ||
                 (lexer_where::cur_lex_type_w == NUMBER_w))
        {
            // processing long-expression
            flag_expr = 1;
            long res1 = 0;
            long res2 = 0;
            std::string op;
            str.insert(0, lexer_where::cur_lex_text_w);
            lexer_where::c_w = lexer_where::c_w + " ";
            str.insert(lexer_where::cur_lex_text_w.length(), lexer_where::c_w);
            parser_long_expr::init(str);
            res1 = parser_long_expr::Al(str, bd);
            lexer_where::c_w = lexer_long_expr::c;

            // logic operator
            lexer_where::next(str, bd);
            op = lexer_where::cur_lex_text_w;
            lexer_where::next(str, bd);

            lexer_where::c_w = lexer_where::c_w + " ";
            str.insert(0, lexer_where::c_w);
            str.insert(0, lexer_where::cur_lex_text_w);
            // processing long-expression now
            parser_long_expr::init(str);
            res2 = parser_long_expr::Al(str, bd);
            lexer_where::c_w = lexer_where::c_w + " ";
            str.insert(0, lexer_where::c_w);
            str.insert(0, ") ");
            lexer_where::c_w = ")";
            lexer_where::next(str, bd);

            if (op == "= ")
            {
                if (res1 == res2)
                {
                    res = 1;
                }
                else
                {
                    res = 0;
                }
            }

            else if (op == "< ")
            {
                if (res1 < res2)
                {
                    res = 1;
                }
                else
                {
                    res = 0;
                }
            }

            else if (op == "> ")
            {
                if (res1 > res2)
                {
                    res = 1;
                }
                else
                {
                    res = 0;
                }
            }

            else if (op == "!= ")
            {
                if (res1 != res2)
                {
                    res = 1;
                }
                else
                {
                    res = 0;
                }
            }

            else if (op == "<= ")
            {
                if (res1 <= res2)
                {
                    res = 1;
                }
                else
                {
                    res = 0;
                }
            }

            else if (op == ">= ")
            {
                if (res1 >= res2)
                {
                    res = 1;
                }
                else
                {
                    res = 0;
                }
            }
            flag_expr = 0;
        }
        else if ((lexer_where::cur_lex_type_w == T_NAME_w) ||
                 (lexer_where::cur_lex_type_w == STR_w))
        {
            // text-expression
            std::string f1 = "";
            std::string f2 = "";
            std::string op = "";
            if (lexer_where::cur_lex_type_w == T_NAME_w)
            {
                lexer_where::cur_lex_text_w.pop_back();
                f1 = bd.GetTextField(lexer_where::cur_lex_text_w, line_to_check);
            }
            else if (lexer_where::cur_lex_type_w == STR_w)
            {
                lexer_where::cur_lex_text_w.pop_back();
                lexer_where::cur_lex_text_w.pop_back();
                lexer_where::cur_lex_text_w.erase(0, 1);
                f1 = lexer_where::cur_lex_text_w;
            }
            lexer_where::next(str, bd);
            if (lexer_where::cur_lex_type_w == REL_w)
            {
                lexer_where::cur_lex_text_w.pop_back();
                op = lexer_where::cur_lex_text_w;
                lexer_where::next(str, bd);
            }
            if (lexer_where::cur_lex_type_w == T_NAME_w)
            {
                lexer_where::cur_lex_text_w.pop_back();
                f2 = bd.GetTextField(lexer_where::cur_lex_text_w, line_to_check);
            }
            else if (lexer_where::cur_lex_type_w == STR_w)
            {
                lexer_where::cur_lex_text_w.pop_back();
                lexer_where::cur_lex_text_w.pop_back();
                lexer_where::cur_lex_text_w.erase(0, 1);
                f2 = lexer_where::cur_lex_text_w;
            }
            lexer_where::next(str, bd);
            if (op == "=")
            {
                if (f1 == f2)
                {
                    res = 1;
                }
                else
                {
                    res = 0;
                }
            }

            else if (op == "<")
            {
                if (f1 < f2)
                {
                    res = 1;
                }
                else
                {
                    res = 0;
                }
            }

            else if (op == ">")
            {
                if (f1 > f2)
                {
                    res = 1;
                }
                else
                {
                    res = 0;
                }
            }

            else if (op == "!=")
            {
                if (f1 != f2)
                {
                    res = 1;
                }
                else
                {
                    res = 0;
                }
            }

            else if (op == "<=")
            {
                if (f1 <= f2)
                {
                    res = 1;
                }
                else
                {
                    res = 0;
                }
            }

            else if (op == ">=")
            {
                if (f1 >= f2)
                {
                    res = 1;
                }
                else
                {
                    res = 0;
                }
            }
        }
        else
        {
            throw SQL_Xception(XW_log_expr);
        }
        return res;
    }

    long W71(std::string &str, ITable &bd)
    {
        // (...) for logic operators
        long res;
        if (lexer_where::cur_lex_type_w == OPEN_w)
        {
            lexer_where::next(str, bd);
            res = W31(str, bd);
            if ((lexer_where::cur_lex_type_w != CLOSE_w) || flag_expr)
            {
                throw SQL_Xception(XW_log_expr);
            }
            lexer_where::next(str, bd);
        }
        else if (lexer_where::cur_lex_type_w == NOT_w)
        {
            lexer_where::next(str, bd);
            if (W71(str, bd))
            {
                res = 0;
            }
            else
            {
                res = 1;
            }
        }
        else
        {
            throw SQL_Xception(XW_log_expr);
        }
        return res;
    }
}

/*------------------------------------- LIKE-ALTERMATIVE ----------------------------------------*/
/* lexical and syntactic parsers for regular expressions in LIKE-alternative where-clause (regex)

%   - any sequence of >= symbols
_   - any symbol
[]  - any symbol from brackets: [abcdef] of [a-f]
[^] - any symbol not from brackets: [^abcdef] of [^a-f]
*/

namespace myregex
{

    enum reg_type
    {
        START,
        SYMB,     // check if the symbol in reference equals symbol in check
        ANY_SEQ,  // % sign, expected any sequence of symbols
        SET,      // [] or [^]
        SET_S,    // [] sign, set of symbols or sequence
        SET_SYMB, // [] sign, expected any symbol, that is present in brackets: [abc],
        SET_SEQ_S,
        SET_SEQ_SYMB, // [] sign, case of [a-z]
        SET_SUCCESS,
        NOT_S,
        NOT_SET,          // [^] sign, set of symbols or sequence
        NOT_SET_SYMB,     // [^] sign, expected any symbol, that is not present in brackets: [^abc]
        NOT_SET_SEQ_SYMB, // [^] sign, case of [^a-z]
        NOT_SET_SEQ_SYMB_,
        END,
        ERROR
    };

    enum reg_type mode;
    std::string ref;
    std::string ch;
    char c, escape_any_seq_mode, not_set, set;

    void next()
    {
        if (ref.size() == 0)
        {
            if (ch.size() != 0)
                mode = ERROR;
            else
                mode = END;
            return;
        }
        if (ch.size() == 0)
        {
            if ((ref[0] == ']') || (ref[0] == '%'))
                mode = END;
            else
                mode = ERROR;
            return;
        }
        if (mode != ANY_SEQ)
        {
            c = ref[0];
            ref.erase(0, 1);
        }

        switch (mode)
        {
        case (START):
            if (c == '_')
            {
                mode = START;
                ch.erase(0, 1);
            }
            else if (c == '%')
            {
                mode = ANY_SEQ;
                if (ref.empty())
                    mode = END;
                else
                {
                    c = ref[0];
                    ref.erase(0, 1);
                }
            }
            else if (c == '[')
                mode = SET;
            else // symbol
            {
                if (c == ch[0])
                {
                    mode = SYMB;
                    ch.erase(0, 1);
                }
                else
                    mode = ERROR;
            }
            break;

        case (SYMB):
            if (c == '_')
            {
                mode = START;
                ch.erase(0, 1);
            }
            else if (c == '%')
            {
                mode = ANY_SEQ;
                if (ref.empty())
                    mode = END;
                else
                {
                    c = ref[0];
                    ref.erase(0, 1);
                }
            }
            else if (c == '[')
                mode = SET;
            else // else stay in the SYMB mode
            {
                if (c == ch[0])
                {
                    ch.erase(0, 1);
                }
                else
                    mode = ERROR;
            }
            break;

        case (ANY_SEQ): // % sign
            if (c == ch[0])
            {
                ch.erase(0, 1);
                mode = START;
            }
            else
                ch.erase(0, 1);
            break;

        // ----------------- Processing SET --------------------
        case (SET): // NOT_S, EMPTY os SET_S
            if (c == '^')
                mode = NOT_S;
            else if (c == ']')
                mode = START;
            else if (c == '-') // [-
                mode = ERROR;
            else
            {
                if (c == ch[0])
                {
                    ch.erase(0, 1);
                    mode = SET_SUCCESS;
                }
                else
                {
                    set = c;
                    mode = SET_S;
                }
            }
            break;

        case (SET_S): // SET start, [abc] or [a-c] mode
            if (c == '-')
            {
                if (ch[0] > set)
                    mode = SET_SEQ_S;
                else
                    mode = ERROR;
            }
            else if (c == ']')
            {
                mode = START; // 2 symbols in the set
            }
            else if (ch[0] == c)
            {
                ch.erase(0, 1);
                mode = SET_SUCCESS;
            }
            else
                mode = SET_SYMB;
            break;

        case (SET_SYMB):
            if (c == ']')
                mode = ERROR; // symbol is not in the set
            else if (ch[0] == c)
            {
                ch.erase(0, 1);
                mode = SET_SUCCESS;
            }
            break;

        case (SET_SEQ_S):
            if (c == ']')
                mode = ERROR; // case [a-]
            else              // case [a-c]
                if (c >= ch[0])
            {
                mode = SET_SUCCESS;
                ch.erase(0, 1);
            }
            else
                mode = ERROR;
            break;

        case (SET_SEQ_SYMB):
            if (c == ']')
                mode = ERROR; // case [a-]
            else              // case [a-c]
                if (c <= ch[0])
            {
                mode = SET_SUCCESS;
                ch.erase(0, 1);
            }
            else
                mode = ERROR; // symbol is not in the set
            break;

        case (SET_SUCCESS):
            // go to the end of a set
            if (c == ']')
                mode = START;
            break;

            // -------------------- Processing SET with ^ --------------------
        case (NOT_S): // [^a
            if ((c == '-') || (c == ch[0]))
                mode = ERROR;
            else if (c == ']')
                mode = START;
            else
            {
                mode = NOT_SET;
                not_set = c; // remember the first symbol
            }
            break;

        case (NOT_SET): // sequence or symbols
            if (c == '-')
                mode = NOT_SET_SEQ_SYMB;
            else if (c == ch[0])
                mode = ERROR;
            else
                mode = NOT_SET_SYMB;
            // else stay in the mode;
            break;

        case (NOT_SET_SYMB):
            if (c == ']')
            {
                mode = START;
                ch.erase(0, 1);
            }
            else if (c == ch[0])
                mode = ERROR;
            // else stay in the mode
            break;

        case (NOT_SET_SEQ_SYMB):
            if (c == ']')
                mode = ERROR; // case [^a-]
            else if ((ch[0] >= not_set) && (ch[0] <= c))
                mode = ERROR;
            else
                mode = NOT_SET_SEQ_SYMB_;
            break;

        case (NOT_SET_SEQ_SYMB_):
            if (c == ']')
            {
                ch.erase(0, 1);
                mode = START;
            }
            else
                mode = ERROR; // case [^a-bc]
            break;

        default:
            break;
        }
        return;
    }

    bool my_regex_match(std::string reference, std::string check)
    {
        mode = START;
        ref = reference;
        ch = check;
        while ((mode != END) && (mode != ERROR))
        {
            next();
        }
        if (mode == ERROR)
            return false;
        else
            return true;
    }
}

/*--------------------------------------- INTERPRETER ----------------------------------------*/
Interpreter ::Interpreter(std::string &str)
{
    std::cout << str << std::endl;
    std::string cur_word;
    cur_word = read_word(str); // operation
    if (cur_word == "SELECT")
    {
        select_sentence(str);
    }
    else if (cur_word == "INSERT")
    {
        insert_sentence(str);
    }
    else if (cur_word == "UPDATE")
    {
        update_sensence(str);
    }
    else if (cur_word == "DELETE")
    {
        delete_sentence(str);
    }
    else if (cur_word == "CREATE")
    {
        create_sentence(str);
    }
    else if (cur_word == "DROP")
    {
        drop_sentence(str);
    }
    else
    {
        throw SQL_Xception(expr);
    }
}

// SEL = 'SELECT' FIELDS 'FROM' id WHERE
// FIELDS  = id {, id} | *
void Interpreter ::select_sentence(std::string &str)
{
    replace_all(str, "(", " ( ");
    replace_all(str, ")", " ) ");
    replace_all(str, ",", " , ");
    std::vector<std::string> vect;              // vector with fields' id
    std::string cur_word = read_word(str, ','); // first field_name
    bool all = 0;
    if (cur_word == "*")
    {
        // all fields
        all = 1;
        cur_word = read_word(str, ',');
    }
    else // field_name {, field_name}
    {
        vect.push_back(cur_word);
        cur_word = read_word(str);
        while (cur_word == ",")
        {
            cur_word = read_word(str, ','); // next field_name
            vect.push_back(cur_word);
            cur_word = read_word(str); // "," or not
        }
    }
    if (cur_word != "FROM")
    {
        throw SQL_Xception(SE_expr);
    }
    cur_word = read_word(str); // table_name
    bd_table = ITable(cur_word + ".txt");

    for (size_t i = 0; i < vect.size(); i++)
    {
        if (!bd_table.isTitle(vect[i]))
        {
            throw SQL_Xception(SE_nexist);
        }
    }

    cur_word = read_word(str);
    if (cur_word != "WHERE")
    {
        throw SQL_Xception(SE_expr);
    }
    std::vector<size_t> v_where = where_clause(str); // where-clause

    if (all)
    {
        std::cout << bd_table.GetName() << std::endl;
        bd_table.Print_field_names();
        for (size_t i = 0; i < v_where.size(); i++)
        {
            bd_table.Print_line(v_where[i]);
        }
    }
    else
    {
        std::cout << bd_table.GetName() << std::endl
                  << "\\ ";
        for (size_t i = 0; i < vect.size(); i++)
        {
            std::cout << vect[i] << "\t | ";
        }
        std::cout << std::endl;
        for (size_t i = 0; i < v_where.size(); i++)
        {
            bd_table.Print_line(vect, v_where[i]);
        }
    }
}

// INS = 'INSERT INTO' id (F_VAL {, F_VAL})
void Interpreter ::insert_sentence(std::string &str)
{
    std::string cur_word = read_word(str);
    if (cur_word != "INTO")
    {
        throw SQL_Xception(IS_expr);
    }
    replace_all(str, "(", " ( ");
    replace_all(str, ")", " ) ");
    replace_all(str, ",", " , ");
    std::string table_name = read_word(str);
    bd_table = ITable(table_name + ".txt"); // open necessary table
    cur_word = read_symb(str);
    while ((cur_word != "(") && (isspace(cur_word[0])))
        cur_word = read_symb(str);
    if (cur_word != "(")
    {
        throw SQL_Xception(IS_open);
    }
    //std::cout << "bd_table.GetNumFields() = " << bd_table.GetNumFields() << std::endl;
    for (size_t i = 0; i < bd_table.GetNumFields(); i++)
    {
        // if the field with type TEXT
        if (bd_table.GetFieldType(i) == TEXT)
        {
            std::string t_str;
            if (cur_word.empty())
            {
                throw SQL_Xception(IS_expr);
            }
            do
            {
                cur_word = read_word(str, ',');
                t_str = t_str + cur_word + ' ';
            } while ((cur_word[cur_word.length() - 1] != '\'') && (!cur_word.empty()));
            if (t_str[0] != '\'')
            {
                throw SQL_Xception(IS_expr);
            }
            t_str.erase(0, 1);
            t_str.erase((t_str.length() - 2), 2);
            if (t_str.length() > bd_table.GetTextSize(i))
            {
                throw SQL_Xception(IS_range);
            }
            bd_table.AddText(t_str, i);
        }
        // if the field with type LONG
        else
        {
            cur_word = read_word(str, ',');
            long num;
            try
            {
                num = stol(cur_word); // convert string to long
            }
            catch (...)
            {
                throw SQL_Xception(XW_long_stol);
            }
            bd_table.AddLong(num, i);
        }
        cur_word = read_word(str);
    }
    if (cur_word != ")")
    {
        throw SQL_Xception(IS_close);
    }
    cur_word = read_word(str);
    // check if it is the end of the comand
    if (!cur_word.empty())
    {
        throw SQL_Xception(IS_expr);
    }
    bd_table.Add_line();
    bd_table.ToFile(table_name + ".txt");
    bd_table.Print_table();
}

// UPD = 'UPDATE' id 'SET' id '=' EXPR WHERE 
void Interpreter ::update_sensence(std::string &str) //OK
{
    replace_all(str, "(", " ( ");
    replace_all(str, ")", " ) ");
    replace_all(str, ",", " , ");
    replace_all(str, "=", " = ");
    std::string file_name = read_word(str); // table_name
    bd_table = ITable(file_name + ".txt");  // open necessary table
    std::string cur_word = read_word(str);
    if (cur_word != "SET")
    {
        throw SQL_Xception(UP_expr);
    }

    std::string title = read_word(str); // field_name
    // check if the field is present in the table
    if (!bd_table.isTitle(title))
    {
        throw SQL_Xception(UP_nexist);
    }
    std::string expr_num;
    std::string expr_text = ""; // empty if the reference is a field name
    std::string title_ref;

    if (bd_table.GetFieldType(title) == TEXT)
    {
        // processing text-expression
        cur_word = read_word(str);
        if (cur_word != "=")
        {
            throw SQL_Xception(UP_eq);
        }

        cur_word = read_word(str);
        if (cur_word[0] != '\'') // it is a field name
        {
            if (!bd_table.isTitle(cur_word))
            {
                throw SQL_Xception(UP_nexist);
            }
            title_ref = cur_word;
        }
        else // it is a string reference
        {
            std::string t_str;
            t_str = t_str + cur_word + ' ';
            while (cur_word[cur_word.length() - 1] != '\'')
            {
                cur_word = read_word(str);
                if (cur_word.empty())
                {
                    throw SQL_Xception(UP_expr);
                }
                t_str = t_str + cur_word + ' ';
            }
            t_str.erase(0, 1);
            t_str.erase((t_str.length() - 2), 2);
            if (t_str.length() > bd_table.GetTextSize(title))
            {
                throw SQL_Xception(UP_range);
            }
            expr_text = t_str;
        }
        cur_word = read_word(str);
    }
    // processing long-expression
    else
    {
        if (read_symb(str) != "=")
        {
            throw SQL_Xception(UP_eq);
        };
        parser_long_expr::init(str);
        expr_num = parser_long_expr::A(str);
        if (lexer_long_expr::cur_lex_type != END)
        {
            throw SQL_Xception(UP_expr);
        }
        cur_word = lexer_long_expr::c;
    }
    if (cur_word != "WHERE")
    {
        throw SQL_Xception(UP_expr);
    }

    std::vector<size_t> v_where;
    v_where = where_clause(str); // where-clause

    for (size_t i = 0; i < v_where.size(); i++)
    {
        if (bd_table.GetFieldType(title) == TEXT)
        {
            if (!expr_text.empty())
            {
                bd_table.GetTextField(title, v_where[i]) = expr_text;
            }
            else
            {
                bd_table.GetTextField(title, v_where[i]) = bd_table.GetTextField(title_ref, v_where[i]);
            }
        }
        else
        {
            std::string tmp = expr_num + "WHERE";
            parser_long_expr::line_to_check = v_where[i];
            parser_long_expr::init(tmp);
            bd_table.GetLongField(title, v_where[i]) = parser_long_expr::Al(tmp, bd_table);
        }
    }
    bd_table.Print_table();
    bd_table.ToFile(file_name);
}

// DEL  = 'DELETE FROM' id WHERE
void Interpreter ::delete_sentence(std::string &str) // OK
{
    replace_all(str, "(", " ( ");
    replace_all(str, ")", " ) ");
    replace_all(str, ",", " , ");
    std::string cur_word = read_word(str);
    if (cur_word != "FROM")
    {
        throw SQL_Xception(DL_expr);
    }
    std::string file_name = read_word(str); // table_name
    bd_table = ITable(file_name + ".txt");
    cur_word = read_word(str);
    if (cur_word != "WHERE")
    {
        throw SQL_Xception(DL_expr);
    }
    std::vector<size_t> v_where;
    v_where = where_clause(str); // where-clause

    for (size_t i = v_where.size(); i > 0; i--)
    {
        bd_table.Delete_line(i);
    }
    bd_table.Print_table();
    bd_table.ToFile(file_name + ".txt");
}

// CR = 'CREATE TABLE' id (DESCR) 
void Interpreter ::create_sentence(std::string &str) //OK
{
    replace_all(str, "(", " ( ");
    replace_all(str, ")", " ) ");
    replace_all(str, ",", " , ");
    std::string cur_word = read_word(str);
    if (cur_word != "TABLE")
    {
        throw SQL_Xception(CR_expr);
    }
    std::string table_name = read_word(str);
    cur_word = read_symb(str);
    while ((cur_word != "(") && (isspace(cur_word[0])))
        cur_word = read_symb(str);
    if (cur_word != "(")
    {
        throw SQL_Xception(CR_open);
    }
    bd_table.SetName(table_name);

    field_description(str);    // first field
    cur_word = read_symb(str); // "," or not
    while (cur_word == ",")
    {
        field_description(str);    // next field
        cur_word = read_word(str); // "," or not
    }
    if (cur_word != ")")
    {
        throw SQL_Xception(CR_close);
    }
    cur_word = read_word(str);
    // check if it is the end of the comand
    if (!cur_word.empty())
    {
        throw SQL_Xception(CR_expr);
    }
    bd_table.Print_table();
    bd_table.ToFile(table_name + ".txt");
}

// DR = 'DROP TABLE' id 
void Interpreter ::drop_sentence(std::string &str) //OK
{
    std::string cur_word;
    std::string t_name;
    cur_word = read_word(str);
    if (cur_word != "TABLE")
    {
        throw SQL_Xception(DR_expr);
    }
    t_name = read_word(str); // table_name
    cur_word = read_word(str);
    // check if it is the end of the comand
    if (!cur_word.empty())
    {
        throw SQL_Xception(DR_expr);
    }
    bd_table.Delete_Table(t_name);
}

void Interpreter ::field_description(std::string &str)
{
    std::string f_name = read_word(str);      // field_name
    std::string f_type = read_word(str, ')'); // field_type, may be the last in the sequence
    if (f_type == "TEXT")
    {
        std::string cur_word = read_word(str);
        while ((cur_word != "(") && (isspace(cur_word[0])))
            cur_word = read_word(str);
        if (cur_word != "(")
        {
            throw SQL_Xception(CR_open);
        }
        cur_word = read_word(str, ')');
        unsigned long num = 0;
        try
        {
            num = stoul(cur_word); // convert string to u long
        }
        catch (...)
        {
            throw SQL_Xception(XW_long_stol);
        }
        cur_word = read_word(str);
        while ((cur_word != ")") && (isspace(cur_word[0]) && !(str.empty())))
            cur_word = read_word(str);
        if (cur_word != ")")
        {
            throw SQL_Xception(CR_close);
        }
        bd_table.AddTitle(TEXT, f_name, num);
    }
    else if (f_type == "LONG")
    {
        bd_table.AddTitle(LONG, f_name, 0);
    }
    else
    {
        throw SQL_Xception(CR_expr);
    }
}

// list of record numbers need to be treated
std::vector<size_t> Interpreter ::where_clause(std::string &str)
{
    std::vector<size_t> vect;
    // analisys of WHERE-clause
    parser_where ::init(str, bd_table); // read s word, change state
    std::string s = parser_where ::W0(str, bd_table);
    if (lexer_where::cur_lex_type_w != END_w)
    {
        throw SQL_Xception(XW_long_lex);
    }
    std::string f_name;
    std::string w;
    std::string s_log;
    // processing necesssary mode
    switch (parser_where::mode)
    {
    case LIKE_alt:
        f_name = read_word(s);
        w = read_word(s);
        if (w == "NOT")
        {
            read_word(s);
            w = read_word(s);
            w.pop_back();
            w.erase(0, 1);
            // filling in the list
            // if NOT LIKE
            for (size_t i = 0; i < bd_table.GetNumLines(); i++)
            {
                if (!myregex::my_regex_match(w, bd_table.GetTextField(f_name, i)))
                {
                    vect.push_back(i);
                }
            }
        }
        else if (w == "LIKE")
        {
            w = read_word(s);
            w.pop_back();
            w.erase(0, 1);
            // filling in the list
            // if LIKE
            for (size_t i = 0; i < bd_table.GetNumLines(); i++)
            {
                if (myregex::my_regex_match(w, bd_table.GetTextField(f_name, i)))
                {
                    vect.push_back(i);
                }
            }
        }
        break;

    case IN_alt_L:
        for (size_t i = 0; i < bd_table.GetNumLines(); i++)
        {
            std::string s_tmp = s;
            long num;
            // calculating the value of long-expression
            parser_long_expr::init(s_tmp);
            parser_long_expr::line_to_check = i;
            num = parser_long_expr::Al(s_tmp, bd_table);
            // filling in the list
            // if IN
            if (lexer_long_expr::c == "IN")
            {
                if (parser_where::mst_l.count(num))
                {
                    vect.push_back(i);
                }
            }
            // if NOT IN
            else if (lexer_long_expr::c == "NOT")
            {
                if (!parser_where::mst_l.count(num))
                {
                    vect.push_back(i);
                }
            }
        }
        break;

    case IN_alt_T:
        // if text-expression is name of the field with type TEXT
        if (s[0] != '\'')
        {
            f_name = read_word(s);
            w = read_word(s);
            for (size_t i = 0; i < bd_table.GetNumLines(); i++)
            {
                std::string f = bd_table.GetTextField(f_name, i);
                // filling in the list
                // if IN
                if (w == "IN")
                {
                    if (parser_where::mst_s.count(f))
                    {
                        vect.push_back(i);
                    }
                }
                // if NOT IN
                else if (w == "NOT")
                {
                    if (!parser_where::mst_s.count(f))
                    {
                        vect.push_back(i);
                    }
                }
            }
        }
        // if text-expression is string
        else
        {
            std::string t_str;
            w = read_word(s);
            t_str = w;
            while (w[w.length() - 1] != '\'')
            {
                w = read_word(s);
                t_str = t_str + ' ' + w;
            }
            t_str.pop_back();
            w = read_word(s);
            // filling in the list
            // if IN
            if (w == "IN")
            {
                if (parser_where::mst_s.count(t_str))
                {
                    for (size_t i = 0; i < bd_table.GetNumLines(); i++)
                    {
                        vect.push_back(i);
                    }
                }
            }
            // filling in the list
            // if NOT IN
            else if (w == "NOT")
            {
                if (!parser_where::mst_s.count(t_str))
                {
                    for (size_t i = 0; i < bd_table.GetNumLines(); i++)
                    {
                        vect.push_back(i);
                    }
                }
            }
        }
        break;

    case LOG_alt:
        long num;
        // filling in the list
        for (size_t i = 0; i < bd_table.GetNumLines(); i++)
        {
            s_log = s;
            parser_where::line_to_check = i;
            parser_long_expr::line_to_check = i;
            // calculating the value of logic-expression
            parser_where ::init(s_log, bd_table);
            num = parser_where ::W31(s_log, bd_table);
            if (num)
                vect.push_back(i);
        }
        break;

    case ALL_alt:
        // all records
        for (size_t i = 0; i < bd_table.GetNumLines(); i++)
        {
            vect.push_back(i);
        }
        break;
    default:
        break;
    }
    sort(vect.begin(), vect.end());
    vect.erase(unique(vect.begin(), vect.end()), vect.end());
    return vect;
}

#endif