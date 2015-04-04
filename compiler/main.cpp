//
// Copyright (c) 2013 Alexander Shafranov shafranov@gmail.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#include <string>
#include <algorithm>
#include <stdio.h>
#include <string.h>

#include "derplanner/compiler/memory.h"
#include "derplanner/compiler/lexer.h"
#include "derplanner/compiler/parser.h"

using namespace plnnrc;

struct Buffer_Context
{
    Buffer_Context(size_t bytes)
        : data(0)
    {
        data = static_cast<char*>(plnnrc::allocate(bytes));
    }

    ~Buffer_Context()
    {
        plnnrc::deallocate(data);
    }

    char* data;
};

struct File_Context
{
    File_Context(const char* path, const char* mode)
        : fd(0)
    {
        fd = fopen(path, mode);
    }

    ~File_Context()
    {
        if (fd != 0)
        {
            fclose(fd);
        }
    }

    FILE* fd;
};

size_t file_size(const char* path)
{
    File_Context ctx(path, "rb");
    fseek(ctx.fd, 0, SEEK_END);
    size_t input_size = ftell(ctx.fd);
    fseek(ctx.fd, 0, SEEK_SET);
    return input_size;
}

// struct Error_Node_Comparator
// {
//     bool operator()(const ast::Node* a, const ast::Node* b)
//     {
//         if (a->s_expr->line == b->s_expr->line)
//         {
//             return a->s_expr->column < b->s_expr->column;
//         }

//         return a->s_expr->line < b->s_expr->line;
//     }
// };

void print_help()
{
    fprintf(stderr,
"Usage: derplannerc [options] <domain-file-path>\n"
"Options:\n"
"   --out, -o <dir>\n"
"       Set the directory for generated files.\n"
"       (default: current directory)\n"
"\n"
"   --custom-header, -c <header-name>\n"
"       Custom header.\n"
"\n"
"   --help, -h\n"
"       Print this help message and exit.\n");
}

bool parse_argument(const char* argument, std::string& name, std::string& value)
{
    // [value]
    if (argument[0] != '-')
    {
        value = std::string(argument);
        return false;
    }

    // [--long-option=value] or [--long-option value]
    if (argument[1] == '-')
    {
        const char* begin = argument + 2;
        const char* end = strchr(argument, '=');

        if (end)
        {
            name = std::string(begin, end - begin);
            value = std::string(end + 1);
            return false;
        }

        name = std::string(begin);
        return true;
    }

    // [-s value]
    name = std::string(argument + 1);
    return true;
}

std::string normalize(std::string path)
{
    for (size_t i = 0; i < path.length(); ++i)
    {
        if (path[i] == '\\')
        {
            path[i] = '/';
        }
    }

    return path;
}

std::string get_output_name(std::string path)
{
    std::string::size_type s = path.rfind("/");

    if (s != std::string::npos)
    {
        path = path.substr(s + 1);
    }

    std::string::size_type d = path.rfind(".");

    if (d != std::string::npos)
    {
        path = path.substr(0, d);
    }

    return path;
}

int main(int argc, char** argv)
{
    std::string output_dir;
    std::string custom_header;
    std::string input_path;

    for (int i = 1; i < argc; ++i)
    {
        const char* argument = argv[i];

        if (!argument[0])
        {
            fprintf(stderr, "error: received empty cmdline argument!\n");
            return 1;
        }

        std::string name;
        std::string value;

        bool expecting_value = parse_argument(argument, name, value);

        if (expecting_value)
        {
            if (name == "h" || name == "help")
            {
                print_help();
                return 1;
            }

            if (i + 1 >= argc || argv[i + 1][0] == '-')
            {
                fprintf(stderr, "error: missing value for flag: %s\n", name.c_str());
                return 1;
            }

            value = std::string(argv[++i]);
        }

        if (!name.empty())
        {
            if (name == "out" || name == "o")
            {
                if (!output_dir.empty())
                {
                    fprintf(stderr, "error: multiple values for flag: %s\n", name.c_str());
                    return 1;
                }

                output_dir = value;
                continue;
            }

            if (name == "custom-header" || name == "c")
            {
                if (!custom_header.empty())
                {
                    fprintf(stderr, "error: multiple values for flag: %s\n", name.c_str());
                    return 1;
                }

                custom_header = value;
                continue;
            }
        }
        else
        {
            if (!input_path.empty())
            {
                fprintf(stderr, "error: multiple inputs specified.\n");
                return 1;
            }

            input_path = value;
        }
    }

    if (input_path.empty())
    {
        fprintf(stderr, "error: no source file specified.\n");
        return 1;
    }

    {
        File_Context ctx(input_path.c_str(), "rb");

        if (!ctx.fd)
        {
            fprintf(stderr, "error: can't open input file: '%s'.\n", input_path.c_str());
            return 1;
        }
    }

    if (output_dir.empty())
    {
        output_dir = ".";
    }

    std::string output_name = get_output_name(normalize(input_path));

    size_t input_size = file_size(input_path.c_str());
    Buffer_Context input_buffer(input_size+1);
    {
        File_Context ctx(input_path.c_str(), "rb");
        size_t rb = fread(input_buffer.data, sizeof(char), input_size, ctx.fd);
        (void)rb;
    }

    input_buffer.data[input_size] = 0;

    {
        plnnrc::Lexer lexer;
        plnnrc::init(lexer, input_buffer.data);

        plnnrc::Token tok = plnnrc::lex(lexer);
        for (; tok.type != plnnrc::Token_Eof; tok = plnnrc::lex(lexer))
        {
            if (plnnrc::has_value(tok))
            {
                printf("%s[%.*s]\n", plnnrc::get_token_name(tok.type), (int)tok.value.length, tok.value.str);
            }
            else
            {
                printf("%s\n", plnnrc::get_token_name(tok.type));
            }
        }

        if (tok.type == plnnrc::Token_Eof)
        {
            printf("%s\n", plnnrc::get_token_name(tok.type));
        }
    }

    {
        plnnrc::Lexer lexer;
        plnnrc::init(lexer, input_buffer.data);

        plnnrc::Parser parser;
        plnnrc::init(parser, &lexer);

        parse(parser);

        printf("\n==Parsed World==\n\n");
        for (plnnrc::ast::Fact_Type* fact = parser.world->facts; fact != 0; fact = fact->next)
        {
            printf("%.*s[", (int)fact->name.length, fact->name.str);
            for (plnnrc::ast::Fact_Param* param = fact->params; param != 0; param = param->next)
            {
                printf("%s", plnnrc::get_token_name(param->type));

                if (param->next != 0)
                {
                    printf(", ");
                }
            }
            printf("]\n");
        }
    }

    return 0;
}
