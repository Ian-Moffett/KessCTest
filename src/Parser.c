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
};
#endif


static double parse_add(void);
static const char* state;

// Checks if state is a valid digit
static int is_digit(void) {
	return *state >= '0' && *state <= '9';
}

// Get the digit value of state
static int digit() {
	return *state - '0';
}

// Parses a number
static double number() {
	double result = 0;
	
	while (is_digit()) {
		int n = digit();
		
		result *= 10;
		result += n;
		
		++state;
	}
	
	if (*state == '.') {
		++state;
		double dec = 0.1;
		
		while (is_digit()) {
			int n = digit();
			
			result += n * dec;
			dec *= 0.1;
			
			++state;
		}
	}
	
	return result;
}

// Parses a factor (unary, parenthesis, and number)
static double parse_factor() {
	switch (*state) {
		case '+': ++state; return  parse_factor();
		case '-': ++state; return -parse_factor();
		case '(': {
			++state; // eat (
			double result = parse_add();
			++state; // eat )
			
			return result;
		}
		default: return number();
	}
}

// Parses * and /
static double parse_mul() {
	double left = parse_factor();
	
	while (*state == '*' || *state == '/') {
		char op = *state++;
		double right = parse_factor();
		
		if (op == '*')
			left *= right;
		else
			left /= right;
	}
	
	return left;
}

// Parses + and -
static double parse_add() {
	double left = parse_mul();
	
	while (*state == '+' || *state == '-') {
		char op = *state++;
		double right = parse_mul();
		
		if (op == '+')
			left += right;
		else
			left -= right;
	}
	
	return left;
}


static char* eval(const char* string) {
	state = string;
    double res = parse_add();

    char* strRes = (char*)calloc(20, sizeof(char));
    snprintf(strRes, 19, "%f", res);

    #ifdef NO_DECIMAL
    for (int i = 19; i > -1; --i) {
        if (strRes[i] == '.') {
            strRes[i] = '\0';
            break;
        }

        strRes[i] = '\0';
    }
    #endif

	return strRes;
}


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


static expression_t kc_parse_expr(parser_t* parser, bool call) {
    expression_t exp = {
        .expression = (char*)calloc(2, sizeof(char)),
        .errorflag = 0x0
    };

    int lparenCount = 0;
    int rparenCount = 0;
    
    while (parser->idx < parser->tokenlist.size) { 
        #ifdef KC_DUMP_TOKENS
        if (parser->curToken.type) {
            printf("KC_TOKEN: %s => %s\n", TOKENS_STR[parser->curToken.type], parser->curToken.tok);
        }
        #endif
        
        if (parser->curToken.type == T_LPAREN) {
            ++lparenCount;
        } else if (parser->curToken.type == T_RPAREN) {
            ++rparenCount;
        }


        if (parser->curToken.type == T_LPAREN || parser->curToken.type == T_RPAREN || parser->curToken.type == T_DIGIT || isop(parser->curToken)) {
            exp.expression = (char*)realloc(exp.expression, sizeof(char) * (strlen(parser->curToken.tok) + 5));
            strcat(exp.expression, parser->curToken.tok);
            advance(parser);
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
        #ifdef KC_DUMP_TOKENS
        if (parser->curToken.type) {
            printf("KC_TOKEN: %s => %s\n", TOKENS_STR[parser->curToken.type], parser->curToken.tok);
        }
        #endif

        if (parser->curToken.type == T_PRINT) {
            #ifdef KC_DUMP_TOKENS
            printf("KC_TOKEN: %s => %s\n", TOKENS_STR[parser->curToken.type], parser->curToken.tok);
            #endif
            advance(parser); 

            #ifdef KC_DUMP_TOKENS
            printf("KC_TOKEN: %s => %s\n", TOKENS_STR[parser->curToken.type], parser->curToken.tok);
            #endif

            advance(parser);

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

                #ifdef KC_DUMP_TOKENS
                printf("KC_EXPRESSION: %s\n", expression.expression);
                #endif

               free(expression.expression);
            } else if (parser->curToken.type == T_QUOTE) {
                advance(parser);
                ast_node_t printNode = createNode("PRINTF", parser->curToken.tok, false, lineNum);
                ast_push_node(&parser->ast, printNode);
                advance(parser);
            } else if (parser->curToken.type == T_EOL) {
                ++lineNum;
            }
        } else if (isDatatype(parser->curToken)) { 
            tokentype_t datatype = parser->curToken.type;
            advance(parser);

            #ifdef KC_DUMP_TOKENS
            printf("KC_TOKEN: %s => %s\n", TOKENS_STR[parser->curToken.type], parser->curToken.tok);
            #endif

            kc_parse_assert(parser->curToken.type == T_IDENTIFIER, parser, "SyntaxError: Expected identifier after typename.", "", lineNum);

            char* name = parser->curToken.tok;

            if (parser->error) {
                break;
            }

            advance(parser);
 
            #ifdef KC_DUMP_TOKENS
            printf("KC_TOKEN: %s => %s\n", TOKENS_STR[parser->curToken.type], parser->curToken.tok);
            #endif

            // TODO: Add assignment.
            kc_parse_assert(parser->curToken.type == T_EOL || parser->curToken.type == T_SEMI, parser, "SyntaxError: Expected assignment or nothing after identifier.", "", lineNum);

            if (parser->error) {
                break;
            }


            ast_node_t varNode = createNode("VAR", name, false, lineNum);
            
            switch (datatype) {
                case T_UINT8:
                    node_push_child(&varNode, createChild("TYPE", "uint8", false));
                    break;
            }
            
            advance(parser);
 
            // TODO: Change this when adding assignment.
            node_push_child(&varNode, createChild("NO_INIT", "null", false));
            ast_push_node(&parser->ast, varNode);
        }

        ++parser->idx;
    }
}
