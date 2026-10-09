// Copyright 2026 JesseTheCatLover. All Rights Reserved.

#include <iostream>
#include <string_view>

#include <Lexer.h>
#include <Parser.h>

int main()
{
    constexpr std::string_view source = R"(
# Project(RedleafEngine):
language(c, cxx);
cppversion(c20);
version(0.1);

# Module(Engine):
type(static_library);

public_dependency(
    glad,
    stb,
    glfw,
    assimp,
    glm,
    nlohmann_json,
    nfd
);

enable_reflection();

cmake
{
    target_compile_definitions(
        Engine
        PRIVATE
        HAVE_GLFW
        HAVE_OPENGL
    )
}

# Module(Editor):
type(static_library);
public_dependency(Engine, imgui);

# ExternalDep(ThirdParty):
cmake_include("CMakeLists.txt");
)";

    Foundry::Lexer lexer(source);

    const auto tokens = lexer.tokenize();

    Foundry::Parser parser(tokens);

    const auto entities = parser.Parse();

    for (const auto& entity : entities)
    {
        std::cout
            << "Entity "
            << entity.type
            << "("
            << entity.name
            << ") at "
            << entity.line
            << ":"
            << entity.column
            << '\n';

        for (const auto& instruction : entity.instructions)
        {
            if (const auto* call =
                    std::get_if<Foundry::Call>(&instruction))
            {
                std::cout
                    << "    Call "
                    << call->name
                    << "(";

                for (std::size_t i = 0;
                     i < call->arguments.size();
                     ++i)
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
                         std::get_if<Foundry::CMakeBlock>(&instruction))
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
    }

    return 0;
}