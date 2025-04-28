#ifdef ETONIZER_HANDLE_INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#endif //ETONIZER HANDLE INCLUDES

typedef struct {
    char *token;  // The token string
    char *file;   // Pointer to the original file (string)
    size_t line;  // Line number in the file
    size_t pos;   // Position of the token in the file
} Token;

typedef struct {
    Token *tokens;    // Array of tokens
    size_t size;      // Number of tokens
    size_t capacity;  // Capacity of the token array
} TokenList;

void init_token_list(TokenList *list) {
    list->size = 0;
    list->capacity = 10;
    list->tokens = (Token *)malloc(list->capacity * sizeof(Token));
}

void add_token(TokenList *list, char *file, size_t line, size_t pos, char *token) {
    if (list->size >= list->capacity) {
        list->capacity *= 2;
        list->tokens = (Token *)realloc(list->tokens, list->capacity * sizeof(Token));
    }
    list->tokens[list->size].token = strdup(token);
    list->tokens[list->size].file = file;
    list->tokens[list->size].line = line;
    list->tokens[list->size].pos = pos;
    list->size++;
}

void free_token_list(TokenList *list) {
    for (size_t i = 0; i < list->size; i++) {
        free(list->tokens[i].token);
    }
    free(list->tokens);
}

int tokenize_string_literal(char *str, size_t *pos, size_t *row, char *token) {
    char quote = str[*pos];
    size_t start = *pos;
    (*pos)++;
    (*row)++;
    size_t i = 0;
    
    while (str[*pos] != quote && str[*pos] != '\0') {
        if (str[*pos] == '\\') {
            (*pos)++;  // Skip the escape character
            (*row)++;  // Skip the escape character
        }
        token[i++] = str[*pos];
        (*pos)++;
        (*row)++;
    }
    
    if (str[*pos] == quote) {
        token[i] = '\0';
        (*pos)++;
        (*row)++;
        return 1; // String literal successfully tokenized
    }
    
    return 0; // Error: unmatched quote
}

void tokenize(char *file, TokenList *list) {
    size_t len = strlen(file);
    size_t line = 1, pos = 0, row=0;
    char token[256];
    
    while (pos < len) {
        // Skip whitespace
        if (isspace(file[pos])) {
            if (file[pos] == '\n') { line++; row=0; }
            pos++;
            row++;
            continue;
        }
        
        // Handle string literals
        if (file[pos] == '"' || file[pos] == '\'') {
            size_t start_row = row;
            if (tokenize_string_literal(file, &pos, &row, token)) {
                add_token(list, file, line, start_row, token);
                continue;
            }
            else {
                fprintf(stderr, "Error: unmatched quote at line %zu, position %zu\n", line, pos);
                break;
            }
        }

        // Handle alphanumeric tokens (identifiers, numbers)
        if (isalnum(file[pos]) || file[pos] == '_') {
            size_t start_pos = pos;
            size_t i = 0;
            size_t start_row = row;
            while (isalnum(file[pos]) || file[pos] == '_') {
                token[i++] = file[pos];
                pos++;
                row++;
            }
            token[i] = '\0';
            add_token(list, file, line, start_row, token);
            continue;
        }

        // Handle operators (single character operators)
        else {
            size_t start_pos = pos;
            token[0] = file[pos];
            token[1] = '\0';
            add_token(list, file, line, row, token);
            pos++;
            row++;
            continue;
        }

        // If we reach here, we encountered an unknown character
        fprintf(stderr, "Error: unknown character '%c' at line %zu, position %zu\n", file[pos], line, pos);
        pos++;
        row++;
    }
}
