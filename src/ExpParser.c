#include "include/ExpParser.h"


static const char* state;

int is_digit(void) {
	return *state >= '0' && *state <= '9';
}

// Get the digit value of state
int digit() {
	return *state - '0';
}

// Parses a number
double number() {
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
double parse_factor() {
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
double parse_mul() {
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
double parse_add() {
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


char* eval(const char* string) {
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
