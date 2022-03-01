#include "include/Parser.h"

#define KC_DUMP_TOKENS
#define NO_DECIMAL
#define UNMATCHED_PAREN 0x1


#ifdef KC_DUMP_TOKENS
static const char* const TOKENS_STR[] =  {
    "T_PRINT",
    "T_DIGIT",
    "T_LPAREN",
    "T_RPAREN",
    "T_STR",
    "T_QUOTE",
    "T_PLUS",
    "T_MINUS",
    "T_STAR",
    "T_SLASH",
    "T_EOL",
    "T_SEMI",
    "T_UINT8",
    "T_IDENTIFIER",
    "T_EQUALS",
};
#endif


static token_t peek(parser_t* parser, unsigned long long idx) {
    return parser->tokenlist.tokens[idx];
}


static void advance(parser_t* parser) {
    ++parser->idx;
    parser->curToken = parser->tokenlist.tokens[parser->idx]; 
}


static bool isop(token_t token) {
    switch (token.type) {
        case T_PLUS:
        case T_MINUS:
        case T_STAR:
        case T_SLASH:
            return true;
    }

    return false;
}


static bool isDatatype(token_t token) {
    switch (token.type) {
        case T_UINT8:
            return true;
    }

    return false;
}


typedef struct {
    char* expression;
    unsigned char errorflag : 3;
} expression_t;


static void kc_parse_assert(bool c, parser_t* parser, const char* const MSG, const char* const OFFENDING_LINE, unsigned long long line) {
    if (!(c)) {
        kc_log_err(MSG, OFFENDING_LINE, line);
        parser->error = true;
    }
}


inline static void get_token(parser_t* parser) {
     #ifdef KC_DUMP_TOKENS
    printf("KC_TOKEN: %s => %s\n", TOKENS_STR[parser->curToken.type], parser->curToken.tok);
    #endif
}


static expression_t kc_parse_expr(parser_t* parser, bool call) {
    expression_t exp = {
        .expression = (char*)calloc(2, sizeof(char)),
        .errorflag = 0x0
    };

    int lparenCount = 0;
    int rparenCount = 0;
    
    while (parser->idx < parser->tokenlist.size) {  
        if (parser->curToken.type == T_LPAREN) {
            ++lparenCount;
        } else if (parser->curToken.type == T_RPAREN) {
            ++rparenCount;
        }


        if (parser->curToken.type == T_LPAREN || parser->curToken.type == T_RPAREN || parser->curToken.type == T_DIGIT || isop(parser->curToken)) {
            exp.expression = (char*)realloc(exp.expression, sizeof(char) * (strlen(parser->curToken.tok) + 5));
            strcat(exp.expression, parser->curToken.tok);
            advance(parser);
            get_token(parser);
        } else {
            lparenCount = call ? lparenCount + 1 : lparenCount;
            // rparenCount = call ? rparenCount + 1 : rparenCount;
            #ifdef KC_DUMP_TOKENS
            printf("LPAREN:RPAREN RATIO: %d:%d\n", lparenCount, rparenCount);
            #endif

            if (lparenCount != rparenCount) {
                exp.errorflag |= UNMATCHED_PAREN;
            }

            exp.expression[strlen(exp.expression) - 1] = '\0';
            return exp;
        }
    }
}


inline void parse(parser_t* parser) {
    ast_t ast = {
        .size = 0,
        .nodes = (ast_node_t*)malloc(sizeof(ast_node_t)), 
    };

    parser->ast = ast;

    int lineNum = 1;

    while (parser->idx < parser->tokenlist.size && !(parser->error)) {
        parser->curToken = parser->tokenlist.tokens[parser->idx];

        get_token(parser);

        if (parser->curToken.type == T_PRINT) { 
            advance(parser); 
            get_token(parser);
            advance(parser);
            get_token(parser);

            if (parser->curToken.type == T_DIGIT && isop(peek(parser, parser->idx + 1))) { 
                expression_t expression = kc_parse_expr(parser, true);
                if (expression.errorflag & UNMATCHED_PAREN) {
                    kc_log_err("SyntaxError: Unmatched parenthesis.", "", lineNum);
                    parser->error = true;
                    free(expression.expression);
                    continue;
                }
               
                ast_node_t printNode = createNode("PRINTF", eval(expression.expression), true, lineNum);
                ast_push_node(&parser->ast, printNode);

               free(expression.expression);
            } else if (parser->curToken.type == T_QUOTE) {
                advance(parser);
                get_token(parser);
                ast_node_t printNode = createNode("PRINTF", parser->curToken.tok, false, lineNum);
                ast_push_node(&parser->ast, printNode);
                advance(parser);
                get_token(parser);
            } else if (parser->curToken.type == T_EOL) {
                ++lineNum;
            } else if (parser->curToken.type == T_IDENTIFIER) {
                ast_node_t printNode = createNode("PRINTF_VAR", parser->curToken.tok, false, lineNum);
                ast_push_node(&parser->ast, printNode); 
            }
        } else if (isDatatype(parser->curToken)) {  
            bool assignment = false;
            
            tokentype_t datatype = parser->curToken.type;
            advance(parser);
            get_token(parser);

            kc_parse_assert(parser->curToken.type == T_IDENTIFIER, parser, "SyntaxError: Expected identifier after typename.", "", lineNum);

            char* name = parser->curToken.tok;

            if (parser->error) {
                break;
            }

            advance(parser);

            get_token(parser);

            kc_parse_assert(parser->curToken.type == T_EOL || parser->curToken.type == T_SEMI || parser->curToken.type == T_EQUALS, parser, "SyntaxError: Expected assignment or nothing after identifier.", "", lineNum);

            if (parser->error) {
                break;
            }

            if (parser->curToken.type == T_EQUALS) {
                assignment = true;
            }
  
            
            if (assignment) {
                advance(parser);
                get_token(parser);
            }

            switch (datatype) {
                case T_UINT8:
                    kc_parse_assert(atoi(parser->curToken.tok) <= UCHAR_MAX, parser, "ValueError: Value for uint8 overflows.", "", lineNum);
                    break;
            }

            if (parser->error) {
                break;
            }

            ast_node_t varNode = createNode("VAR", name, false, lineNum);

            switch (datatype) {
                case T_UINT8:
                    node_push_child(&varNode, createChild("TYPE", "uint8", false));
                    break;
            }

            if (!(assignment)) {
                node_push_child(&varNode, createChild("NO_INIT", "null", false));
            } else {
                node_push_child(&varNode, createChild("VALUE", parser->curToken.tok, false));
            }

            ast_push_node(&parser->ast, varNode);
        }

        ++parser->idx;
    }
}
