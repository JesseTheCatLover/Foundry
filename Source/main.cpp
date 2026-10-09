// Copyright 2026 JesseTheCatLover. All Rights Reserved.

#include <cstddef>
#include <exception>
#include <iostream>
#include <string_view>
#include <variant>

#include <Lexer.h>
#include <Parser.h>

void PrintInstruction(const Foundry::Instruction& instruction)
{
    if (const auto* call = std::get_if<Foundry::FCall>(&instruction))
    {
        std::cout
            << "    Call "
            << call->name
            << "(";

        for (std::size_t i = 0; i < call->arguments.size(); ++i)
        {
            if (i > 0)
                std::cout << ", ";

            std::cout << call->arguments[i];
        }

        std::cout
            << "); at "
            << call->line
            << ":"
            << call->column
            << '\n';
    }
    else if (const auto* block =
                 std::get_if<Foundry::FCMakeBlock>(&instruction))
    {
        std::cout
            << "    CMakeBlock at "
            << block->line
            << ":"
            << block->column
            << " ("
            << block->source.size()
            << " characters)\n";
    }
}

int main()
{
    constexpr std::string_view source = R"(
# Project(RedleafEngine):
language(c, cxx);
cppversion(c20);
version(0.1);

# Module(Engine):
type(static_library);
public_dependency(glad, stb, glfw);
enable_reflection();

# ExternalDep(ThirdParty):
cmake_include("CMakeLists.txt");

$ enable_reflection():
    cmake
    {
        target_compile_definitions(
            Engine
            PRIVATE
            REFLECTION_ENABLED
        )
    }
)";

    try
    {
        Foundry::Lexer lexer(source);
        const auto tokens = lexer.tokenize();

        Foundry::Parser parser(tokens);
        const Foundry::FFoundryFile syntaxTree = parser.Parse();

        for (const auto& declaration : syntaxTree.declarations)
        {
            if (const auto* entity =
                    std::get_if<Foundry::FEntity>(&declaration))
            {
                std::cout
                    << "Entity "
                    << entity->type
                    << "("
                    << entity->name
                    << ") at "
                    << entity->line
                    << ":"
                    << entity->column
                    << '\n';

                for (const auto& instruction : entity->instructions)
                    PrintInstruction(instruction);
            }
            else if (const auto* function =
                         std::get_if<Foundry::FFunctionDefinition>(&declaration))
            {
                std::cout
                    << "Function "
                    << function->name
                    << "(";

                for (std::size_t i = 0; i < function->parameters.size(); ++i)
                {
                    if (i > 0)
                        std::cout << ", ";

                    std::cout << function->parameters[i];
                }

                std::cout
                    << ") at "
                    << function->line
                    << ":"
                    << function->column
                    << '\n';

                for (const auto& instruction : function->instructions)
                    PrintInstruction(instruction);
            }
        }
    }
    catch (const std::exception& exception)
    {
        std::cerr << exception.what() << '\n';
        return 1;
    }

    return 0;
}