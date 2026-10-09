// Copyright 2026 JesseTheCatLover. All Rights Reserved.

#include <Parser.h>

#include <stdexcept>
#include <string>
#include <utility>

namespace Foundry
{
    Parser::Parser(const std::vector<Token>& tokens)
        : m_Tokens(tokens)
    {
    }

    const Token& Parser::Peek(std::size_t offset) const
    {
        static const Token endToken{
            TokenType::EndOfFile,
            {},
            1,
            1
        };

        const std::size_t position = m_Position + offset;

        if (m_Tokens.empty() || position >= m_Tokens.size())
            return endToken;

        return m_Tokens[position];
    }

    const Token& Parser::Advance()
    {
        const Token& token = Peek();

        if (token.type != TokenType::EndOfFile)
            ++m_Position;

        return token;
    }

    bool Parser::IsAtEnd() const
    {
        return Peek().type == TokenType::EndOfFile;
    }

    bool Parser::Match(TokenType type)
    {
        if (Peek().type != type)
            return false;

        Advance();

        return true;
    }

    const Token& Parser::Consume(
        TokenType type,
        std::string_view message
    )
    {
        if (Peek().type == type)
            return Advance();

        Error(Peek(), message);
    }

    FEntity Parser::ParseEntity()
    {
        const Token& declaration = Consume(
            TokenType::Hash,
            "Expected '#' to begin entity declaration."
        );

        const Token& type = Consume(
            TokenType::Identifier,
            "Expected entity type after '#'."
        );

        static_cast<void>(Consume(
            TokenType::LeftParen,
            "Expected '(' after entity type."
        ));

        const Token& name = Consume(
            TokenType::Identifier,
            "Expected entity name."
        );

        static_cast<void>(Consume(
            TokenType::RightParen,
            "Expected ')' after entity name."
        ));

        static_cast<void>(Consume(
            TokenType::Colon,
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
            const Token& token = Peek();

            if ((token.type == TokenType::Hash ||
                 token.type == TokenType::Dollar) &&
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
        const Token& declaration = Consume(
            TokenType::Dollar,
            "Expected '$' to begin function definition."
        );

        const Token& name = Consume(
            TokenType::Identifier,
            "Expected function name after '$'."
        );

        static_cast<void>(Consume(
            TokenType::LeftParen,
            "Expected '(' after function name."
        ));

        std::vector<std::string_view> parameters;

        if (!Match(TokenType::RightParen))
        {
            while (true)
            {
                const Token& parameter = Consume(
                    TokenType::Identifier,
                    "Expected parameter name."
                );

                parameters.push_back(parameter.value);

                if (Match(TokenType::RightParen))
                    break;

                static_cast<void>(Consume(
                    TokenType::Comma,
                    "Expected ',' or ')' after parameter."
                ));
            }
        }

        const Token& colon = Consume(
            TokenType::Colon,
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
        const Token& token = Peek();

        if (token.type == TokenType::CMakeBlock)
        {
            const Token& block = Advance();

            return Instruction{
                FCMakeBlock{
                    block.value,
                    block.line,
                    block.column
                }
            };
        }

        if (token.type != TokenType::Identifier)
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
        const Token& name = Consume(
            TokenType::Identifier,
            "Expected instruction name."
        );

        static_cast<void>(Consume(
            TokenType::LeftParen,
            "Expected '(' after instruction name."
        ));

        std::vector<std::string_view> arguments;

        if (!Match(TokenType::RightParen))
        {
            while (true)
            {
                const Token& argument = Peek();

                if (argument.type != TokenType::Identifier &&
                    argument.type != TokenType::String &&
                    argument.type != TokenType::Number)
                {
                    Error(
                        argument,
                        "Expected an identifier, string, or number as argument."
                    );
                }

                arguments.push_back(Advance().value);

                if (Match(TokenType::RightParen))
                    break;

                static_cast<void>(Consume(
                    TokenType::Comma,
                    "Expected ',' or ')' after argument."
                ));
            }
        }

        static_cast<void>(Consume(
            TokenType::Semicolon,
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
        const Token& token,
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
            if (Peek().type == TokenType::Hash)
            {
                file.declarations.push_back(
                    Declaration{ParseEntity()}
                );
            }
            else if (Peek().type == TokenType::Dollar)
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