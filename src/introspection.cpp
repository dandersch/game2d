#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// #define INTROSPECT(params)

char* read_file_into_memory_null_terminated(char* filename)
{
    char* buffer = NULL;

    FILE* fd = fopen(filename, "r");
    if (!fd) return buffer;

    fseek(fd, 0, SEEK_END);
    size_t file_size = ftell(fd);
    fseek(fd, 0, SEEK_SET);

    buffer = (char*) malloc(file_size + 1);
    fread(buffer, sizeof(*buffer), file_size, fd);
    buffer[file_size] = '\0';

    fclose(fd);

    return buffer;
}

enum token_type_e
{
    TOKEN_TYPE_INVALID,

    TOKEN_TYPE_OPEN_PAREN,
    TOKEN_TYPE_CLOSE_PAREN,
    TOKEN_TYPE_COLON,
    TOKEN_TYPE_SEMICOLON,
    TOKEN_TYPE_ASTERISK,
    TOKEN_TYPE_OPEN_BRACKET,
    TOKEN_TYPE_CLOSE_BRACKET,
    TOKEN_TYPE_OPEN_BRACE,
    TOKEN_TYPE_CLOSE_BRACE,

    TOKEN_TYPE_IDENTIFIER,
    TOKEN_TYPE_STRING,

    TOKEN_TYPE_EOF,
};

struct token_t
{
    token_type_e type;
    size_t       text_len;
    char*        text;
};

struct tokenizer_t
{
    char* at;
};


#define IS_WHITESPACE(character) ((character == ' ')  || \
                                  (character == '\t') || \
                                  (character == '\n') || \
                                  (character == '\r'))

void skip_whitespaces_and_comments(tokenizer_t* tokenizer)
{
    while(IS_WHITESPACE(tokenizer->at[0]))
    {
        ++tokenizer->at;
    }

    // TODO skip // & /**/ style comments
}

token_t get_token(tokenizer_t* tokenizer)
{
    skip_whitespaces_and_comments(tokenizer);

    token_t token = { TOKEN_TYPE_INVALID, 1, tokenizer->at };

    switch (tokenizer->at[0])
    {
        case '\0': { token.type = TOKEN_TYPE_EOF; } break;

        case '(': { token.type = TOKEN_TYPE_OPEN_PAREN;    } break;
        case ')': { token.type = TOKEN_TYPE_CLOSE_PAREN;   } break;
        case ':': { token.type = TOKEN_TYPE_COLON;         } break;
        case ';': { token.type = TOKEN_TYPE_SEMICOLON;     } break;
        case '*': { token.type = TOKEN_TYPE_ASTERISK;      } break;
        case '[': { token.type = TOKEN_TYPE_OPEN_BRACKET;  } break;
        case ']': { token.type = TOKEN_TYPE_CLOSE_BRACKET; } break;
        case '{': { token.type = TOKEN_TYPE_OPEN_BRACE;    } break;
        case '}': { token.type = TOKEN_TYPE_CLOSE_BRACE;   } break;

        default:
        {
            // if (tokenizer->at[0] == '"' ...)
            // {
            //     TOKEN_TYPE_STRING;
            // }

            //TOKEN_TYPE_IDENTIFIER;
        } break;
    }

    return token;
}

int main()
{
    // open file in 'src/'
    const char* filename = "camera.h";
    const char* src_folder = "src/";
    char* filepath = (char*) malloc(sizeof(char) * (strlen(src_folder) + strlen(filename)));
    strcpy(filepath, src_folder);
    strcpy(&filepath[strlen(src_folder)], filename);
    char* file_buf = read_file_into_memory_null_terminated(filepath);

    tokenizer_t tokenizer = {file_buf};

    bool parsing = true;
    while (parsing)
    {
        printf("%s\n", file_buf);
        parsing = false;

        //token_t token = get_token();
        token_t token;
        switch (token.type)
        {
            // ...
            default:
            {
                //printf("%d: %.*s\n", token.type, token.text_len, token.text);
            } break;

            case TOKEN_TYPE_EOF:
            {
                parsing = false;
            } break;
        }
    }

    // cleanup
    free(file_buf);
    free(filepath);
}
