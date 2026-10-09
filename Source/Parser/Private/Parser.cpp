// Copyright 2026 JesseTheCatLover. All Rights Reserved.

#include <Parser.h>

#include <stdexcept>
#include <string>
#include <utility>

namespace Foundry
{
    Parser::Parser(const std::vector<FToken>& tokens)
        : m_Tokens(tokens)
    {
    }

    const FToken& Parser::Peek(std::size_t offset) const
    {
        static const FToken endToken{
            ETokenType::EndOfFile,
            {},
            1,
            1
        };

        const std::size_t position = m_Position + offset;

        if (m_Tokens.empty() || position >= m_Tokens.size())
            return endToken;

        return m_Tokens[position];
    }

    const FToken& Parser::Advance()
    {
        const FToken& token = Peek();

        if (token.type != ETokenType::EndOfFile)
            ++m_Position;

        return token;
    }

    bool Parser::IsAtEnd() const
    {
        return Peek().type == ETokenType::EndOfFile;
    }

    bool Parser::Match(ETokenType type)
    {
        if (Peek().type != type)
            return false;

        Advance();

        return true;
    }

    const FToken& Parser::Consume(
        ETokenType type,
        std::string_view message
    )
    {
        if (Peek().type == type)
            return Advance();

        Error(Peek(), message);
    }

    FEntity Parser::ParseEntity()
    {
        const FToken& declaration = Consume(
            ETokenType::Hash,
            "Expected '#' to begin entity declaration."
        );

        const FToken& type = Consume(
            ETokenType::Identifier,
            "Expected entity type after '#'."
        );

        static_cast<void>(Consume(
            ETokenType::LeftParen,
            "Expected '(' after entity type."
        ));

        const FToken& name = Consume(
            ETokenType::Identifier,
            "Expected entity name."
        );

        static_cast<void>(Consume(
            ETokenType::RightParen,
            "Expected ')' after entity name."
        ));

        static_cast<void>(Consume(
            ETokenType::Colon,
            "Expected ':' after entity declaration."
        ));

        FEntity entity{
            type.value,
            name.value,
            {},
            declaration.line,
            declaration.column
        };

        while (!IsAtEnd())
        {
            const FToken& token = Peek();

            if ((token.type == ETokenType::Hash ||
                 token.type == ETokenType::Dollar) &&
                token.column <= declaration.column)
            {
                break;
            }

            entity.instructions.push_back(ParseInstruction());
        }

        return entity;
    }

    FFunctionDefinition Parser::ParseFunction()
    {
        const FToken& declaration = Consume(
            ETokenType::Dollar,
            "Expected '$' to begin function definition."
        );

        const FToken& name = Consume(
            ETokenType::Identifier,
            "Expected function name after '$'."
        );

        static_cast<void>(Consume(
            ETokenType::LeftParen,
            "Expected '(' after function name."
        ));

        std::vector<std::string_view> parameters;

        if (!Match(ETokenType::RightParen))
        {
            while (true)
            {
                const FToken& parameter = Consume(
                    ETokenType::Identifier,
                    "Expected parameter name."
                );

                parameters.push_back(parameter.value);

                if (Match(ETokenType::RightParen))
                    break;

                static_cast<void>(Consume(
                    ETokenType::Comma,
                    "Expected ',' or ')' after parameter."
                ));
            }
        }

        const FToken& colon = Consume(
            ETokenType::Colon,
            "Expected ':' after function signature."
        );

        if (IsAtEnd() ||
            Peek().line <= colon.line ||
            Peek().column <= declaration.column)
        {
            Error(Peek(), "Expected an indented function body.");
        }

        FFunctionDefinition function{
            name.value,
            std::move(parameters),
            {},
            declaration.line,
            declaration.column
        };

        while (!IsAtEnd() &&
               Peek().column > declaration.column)
        {
            function.instructions.push_back(ParseInstruction());
        }

        return function;
    }

    Instruction Parser::ParseInstruction()
    {
        const FToken& token = Peek();

        if (token.type == ETokenType::CMakeBlock)
        {
            const FToken& block = Advance();

            return Instruction{
                FCMakeBlock{
                    block.value,
                    block.line,
                    block.column
                }
            };
        }

        if (token.type != ETokenType::Identifier)
        {
            Error(
                token,
                "Expected an instruction call or CMake block."
            );
        }

        return Instruction{ParseCall()};
    }

    FCall Parser::ParseCall()
    {
        const FToken& name = Consume(
            ETokenType::Identifier,
            "Expected instruction name."
        );

        static_cast<void>(Consume(
            ETokenType::LeftParen,
            "Expected '(' after instruction name."
        ));

        std::vector<std::string_view> arguments;

        if (!Match(ETokenType::RightParen))
        {
            while (true)
            {
                const FToken& argument = Peek();

                if (argument.type != ETokenType::Identifier &&
                    argument.type != ETokenType::String &&
                    argument.type != ETokenType::Number)
                {
                    Error(
                        argument,
                        "Expected an identifier, string, or number as argument."
                    );
                }

                arguments.push_back(Advance().value);

                if (Match(ETokenType::RightParen))
                    break;

                static_cast<void>(Consume(
                    ETokenType::Comma,
                    "Expected ',' or ')' after argument."
                ));
            }
        }

        static_cast<void>(Consume(
            ETokenType::Semicolon,
            "Expected ';' after instruction call."
        ));

        return FCall{
            name.value,
            std::move(arguments),
            name.line,
            name.column
        };
    }

    [[noreturn]]
    void Parser::Error(
        const FToken& token,
        std::string_view message
    ) const
    {
        throw std::runtime_error(
            "Parser error at " +
            std::to_string(token.line) +
            ":" +
            std::to_string(token.column) +
            ": " +
            std::string(message)
        );
    }

    FFoundryFile Parser::Parse()
    {
        FFoundryFile file;

        while (!IsAtEnd())
        {
            if (Peek().type == ETokenType::Hash)
            {
                file.declarations.push_back(
                    Declaration{ParseEntity()}
                );
            }
            else if (Peek().type == ETokenType::Dollar)
            {
                file.declarations.push_back(
                    Declaration{ParseFunction()}
                );
            }
            else
            {
                Error(
                    Peek(),
                    "Expected an entity declaration or function definition."
                );
            }
        }

        return file;
    }
}