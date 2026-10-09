// Copyright 2026 JesseTheCatLover. All Rights Reserved.

#include <cstddef>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "Lexer/Lexer.h"
#include "Lexer/Token.h"
#include "Parser/Parser.h"
#include "Semantic/SemanticAnalyzer.h"

namespace
{
    void Require(bool condition, std::string_view message)
    {
        if (!condition)
            throw std::runtime_error(std::string(message));
    }

    void RequireToken(
        const std::vector<Foundry::FToken>& tokens,
        std::size_t index,
        Foundry::ETokenType type,
        std::string_view value,
        std::size_t line,
        std::size_t column
    )
    {
        Require(index < tokens.size(), "Token index is out of range.");

        const Foundry::FToken& token = tokens[index];

        Require(token.type == type, "Unexpected token type.");
        Require(token.value == value, "Unexpected token value.");
        Require(token.line == line, "Unexpected token line.");
        Require(token.column == column, "Unexpected token column.");
    }

    void TestLexerTokens()
    {
        constexpr std::string_view source =
            "# Module(Engine):\n"
            "type(static_library);\n"
            "version(0.1);";

        Foundry::Lexer lexer(source);
        const auto tokens = lexer.tokenize();

        Require(tokens.size() == 17, "Unexpected number of tokens.");

        RequireToken(tokens, 0, Foundry::ETokenType::Hash, "#", 1, 1);
        RequireToken(tokens, 1, Foundry::ETokenType::Identifier, "Module", 1, 3);
        RequireToken(tokens, 2, Foundry::ETokenType::LeftParen, "(", 1, 9);
        RequireToken(tokens, 3, Foundry::ETokenType::Identifier, "Engine", 1, 10);
        RequireToken(tokens, 4, Foundry::ETokenType::RightParen, ")", 1, 16);
        RequireToken(tokens, 5, Foundry::ETokenType::Colon, ":", 1, 17);

        RequireToken(tokens, 6, Foundry::ETokenType::Identifier, "type", 2, 1);
        RequireToken(tokens, 8, Foundry::ETokenType::Identifier, "static_library", 2, 6);

        RequireToken(tokens, 11, Foundry::ETokenType::Identifier, "version", 3, 1);
        RequireToken(tokens, 13, Foundry::ETokenType::Number, "0.1", 3, 9);
        RequireToken(tokens, 16, Foundry::ETokenType::EndOfFile, "", 3, 14);
    }

    void TestLexerCMakeBlock()
    {
        constexpr std::string_view source = R"(// Header comment
cmake
{
    message("literal } and {");
    # This comment must not close the block }
    if(ON) { inner() }
}
)";

        Foundry::Lexer lexer(source);
        const auto tokens = lexer.tokenize();

        Require(tokens.size() == 2, "CMake block should produce one token and EOF.");

        Require(
            tokens[0].type == Foundry::ETokenType::CMakeBlock,
            "Expected a CMakeBlock token."
        );

        Require(
            tokens[0].value.find("literal } and {") != std::string_view::npos,
            "CMake block should preserve quoted contents."
        );

        Require(
            tokens[0].value.find("inner()") != std::string_view::npos,
            "CMake block should preserve nested content."
        );

        Require(
            tokens[1].type == Foundry::ETokenType::EndOfFile,
            "Expected EOF after CMake block."
        );
    }

    void TestParserAST()
    {
        constexpr std::string_view source = R"(
# Module(Engine):
type(static_library);
public_dependency(glad, glfw);
enable_reflection();

$ configure(TARGET, KIND):
    set_target(TARGET, KIND);
    cmake
    {
        message("target }")
    }
)";

        Foundry::Lexer lexer(source);
        const auto tokens = lexer.tokenize();

        Foundry::Parser parser(tokens);
        const Foundry::FFoundryFile file = parser.Parse();

        Require(file.declarations.size() == 2, "Expected two declarations.");

        const auto* entity = std::get_if<Foundry::FEntity>(&file.declarations[0]);

        Require(entity != nullptr, "First declaration should be an entity.");
        Require(entity->type == "Module", "Unexpected entity type.");
        Require(entity->name == "Engine", "Unexpected entity name.");
        Require(entity->instructions.size() == 3, "Expected three entity instructions.");

        const auto* typeCall = std::get_if<Foundry::FCall>(&entity->instructions[0]);

        Require(typeCall != nullptr, "Expected a Call instruction.");
        Require(typeCall->name == "type", "Unexpected call name.");
        Require(typeCall->arguments.size() == 1, "Expected one type argument.");
        Require(typeCall->arguments[0] == "static_library", "Unexpected type argument.");

        const auto* dependencyCall = std::get_if<Foundry::FCall>(&entity->instructions[1]);

        Require(dependencyCall != nullptr, "Expected dependency Call.");
        Require(dependencyCall->name == "public_dependency", "Unexpected dependency call.");
        Require(dependencyCall->arguments.size() == 2, "Expected two dependencies.");
        Require(dependencyCall->arguments[0] == "glad", "Unexpected first dependency.");
        Require(dependencyCall->arguments[1] == "glfw", "Unexpected second dependency.");

        const auto* function = std::get_if<Foundry::FFunctionDefinition>(&file.declarations[1]);

        Require(function != nullptr, "Second declaration should be a function.");
        Require(function->name == "configure", "Unexpected function name.");
        Require(function->parameters.size() == 2, "Expected two function parameters.");
        Require(function->parameters[0] == "TARGET", "Unexpected first parameter.");
        Require(function->parameters[1] == "KIND", "Unexpected second parameter.");
        Require(function->instructions.size() == 2, "Expected two function instructions.");

        const auto* functionCall = std::get_if<Foundry::FCall>(&function->instructions[0]);

        Require(functionCall != nullptr, "Expected a function-body Call.");
        Require(functionCall->name == "set_target", "Unexpected function-body call.");

        const auto* cmakeBlock = std::get_if<Foundry::FCMakeBlock>(&function->instructions[1]);

        Require(cmakeBlock != nullptr, "Expected a CMakeBlock instruction.");
        Require(
            cmakeBlock->source.find("message") != std::string_view::npos,
            "Expected original CMake contents."
        );
    }

    void TestParserMissingSemicolon()
    {
        constexpr std::string_view source =
            "# Module(Engine):\n"
            "type(static_library)";

        Foundry::Lexer lexer(source);
        const auto tokens = lexer.tokenize();

        Foundry::Parser parser(tokens);

        try
        {
            static_cast<void>(parser.Parse());
        }
        catch (const std::runtime_error& exception)
        {
            Require(
                std::string_view(exception.what()).find(
                    "Expected ';' after instruction call."
                ) != std::string_view::npos,
                "Unexpected parser error."
            );

            return;
        }

        throw std::runtime_error("Parser accepted a call without a semicolon.");
    }

    void TestFunctionRequiresIndentation()
    {
        constexpr std::string_view source =
            "$ configure():\n"
            "set_target();";

        Foundry::Lexer lexer(source);
        const auto tokens = lexer.tokenize();

        Foundry::Parser parser(tokens);

        try
        {
            static_cast<void>(parser.Parse());
        }
        catch (const std::runtime_error& exception)
        {
            Require(
                std::string_view(exception.what()).find(
                    "Expected an indented function body."
                ) != std::string_view::npos,
                "Unexpected function indentation error."
            );

            return;
        }

        throw std::runtime_error("Parser accepted an unindented function body.");
    }

    void TestFunctionRegistry()
    {
        constexpr std::string_view source = R"(
# Module(Engine):
type(static_library);

$ configure(TARGET):
    configure_target(TARGET);

$ enable_reflection():
    cmake
    {
        message("Reflection enabled")
    }
)";

        Foundry::Lexer lexer(source);
        const auto tokens = lexer.tokenize();

        Foundry::Parser parser(tokens);
        const Foundry::FFoundryFile file = parser.Parse();

        Foundry::SemanticAnalyzer analyzer(file);
        const Foundry::FSemanticModel model = analyzer.Analyze();

        Require(model.GetFunctionCount() == 2, "Expected two registered functions.");

        const auto* configure = model.FindFunction("configure");

        Require(configure != nullptr, "Expected configure function to be registered.");
        Require(configure->parameters.size() == 1, "Expected one configure parameter.");
        Require(configure->parameters[0] == "TARGET", "Unexpected configure parameter.");
        Require(configure->instructions.size() == 1, "Expected one configure instruction.");

        const auto* reflection = model.FindFunction("enable_reflection");

        Require(reflection != nullptr, "Expected enable_reflection to be registered.");
        Require(reflection->parameters.empty(), "Expected no reflection parameters.");
        Require(reflection->instructions.size() == 1, "Expected one reflection instruction.");

        const auto* cmakeBlock = std::get_if<Foundry::FCMakeBlock>(&reflection->instructions[0]);

        Require(cmakeBlock != nullptr, "Expected the reflection body to contain a CMake block.");
        Require(
            cmakeBlock->source.find("Reflection enabled") != std::string_view::npos,
            "Expected the original function body to be preserved."
        );

        Require(
            model.FindFunction("unknown_function") == nullptr,
            "Unknown function should not resolve."
        );
    }

    void TestDuplicateFunctionDefinition()
    {
        constexpr std::string_view source = R"(
$ setup():
    initialize();

$ setup():
    configure();
)";

        Foundry::Lexer lexer(source);
        const auto tokens = lexer.tokenize();

        Foundry::Parser parser(tokens);
        const Foundry::FFoundryFile file = parser.Parse();

        Foundry::SemanticAnalyzer analyzer(file);

        try
        {
            static_cast<void>(analyzer.Analyze());
        }
        catch (const std::runtime_error& exception)
        {
            Require(
                std::string_view(exception.what()).find(
                    "Duplicate function definition 'setup'."
                ) != std::string_view::npos,
                "Unexpected duplicate function diagnostic."
            );

            Require(
                std::string_view(exception.what()).find("First definition at") != std::string_view::npos,
                "Diagnostic should identify the first definition."
            );

            return;
        }

        throw std::runtime_error("Semantic analyzer accepted duplicate function definitions.");
    }

    void RunTest(std::string_view name, void (*test)(), std::size_t& failures)
    {
        try
        {
            test();
            std::cout << "[PASS] " << name << '\n';
        }
        catch (const std::exception& exception)
        {
            ++failures;

            std::cerr
                << "[FAIL] "
                << name
                << ": "
                << exception.what()
                << '\n';
        }
    }
}

int main()
{
    constexpr std::size_t testCount = 7;
    std::size_t failures = 0;

    RunTest("Lexer tokens and locations", TestLexerTokens, failures);
    RunTest("Opaque CMake block", TestLexerCMakeBlock, failures);
    RunTest("Parser AST", TestParserAST, failures);
    RunTest("Parser rejects missing semicolon", TestParserMissingSemicolon, failures);
    RunTest("Function body requires indentation", TestFunctionRequiresIndentation, failures);
    RunTest("Semantic function registry", TestFunctionRegistry, failures);
    RunTest("Semantic duplicate function definitions", TestDuplicateFunctionDefinition, failures);

    std::cout
        << '\n'
        << "Tests passed: "
        << (testCount - failures)
        << "/"
        << testCount
        << '\n';

    return failures == 0 ? 0 : 1;
}