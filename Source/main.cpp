// Copyright 2026 JesseTheCatLover. All Rights Reserved.

#include <iostream>
#include <string_view>

#include "Lexer/Private/Lexer.h"

int main() {

    constexpr std::string_view source = R"(
# Project(RedleafEngine):
language(c, cxx);
cppversion(c20);
version(0.1);

// Engine module

# Module(Engine):
type(static_library);

public_dependency(
    glad,
    stb,
    glfw
);

enable_reflection();

$ enable_reflection():
    cmake
    {
        target_compile_definitions(
            Engine
            PRIVATE
            HAVE_GLFW
        )
    }
)";

    Foundry::Lexer lexer(source);

    const auto tokens = lexer.tokenize();

    for (const auto& token : tokens) {
        std::cout
            << token.line
            << ":"
            << token.column
            << " "
            << Foundry::TokenTypeToString(token.type)
            << " \""
            << token.value
            << "\"\n";
    }

    return 0;
}