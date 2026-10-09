// Copyright 2026 JesseTheCatLover. All Rights Reserved.

#include "Parser.h"

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

    Entity Parser::ParseEntity()
    {
        const Token& declaration = Consume(
            TokenType::Hash,
            "Expected '#' to begin entity declaration."
        );

        const Token& type = Consume(
            TokenType::Identifier,
            "Expected entity type after '#'."
        );

        Consume(
            TokenType::LeftParen,
            "Expected '(' after entity type."
        );

        const Token& name = Consume(
            TokenType::Identifier,
            "Expected entity name."
        );

        Consume(
            TokenType::RightParen,
            "Expected ')' after entity name."
        );

        Consume(
            TokenType::Colon,
            "Expected ':' after entity declaration."
        );

        Entity entity{
            type.value,
            name.value,
            {},
            declaration.line,
            declaration.column
        };

        while (!IsAtEnd() && Peek().type != TokenType::Hash)
        {
            entity.instructions.push_back(ParseInstruction());
        }

        return entity;
    }

    Instruction Parser::ParseInstruction()
    {
        const Token& token = Peek();

        if (token.type == TokenType::CMakeBlock)
        {
            const Token& block = Advance();

            return Instruction{
                CMakeBlock{
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

        return Instruction{ ParseCall() };
    }

    Call Parser::ParseCall()
    {
        const Token& name = Consume(
            TokenType::Identifier,
            "Expected instruction name."
        );

        Consume(
            TokenType::LeftParen,
            "Expected '(' after instruction name."
        );

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

                Consume(
                    TokenType::Comma,
                    "Expected ',' or ')' after argument."
                );
            }
        }

        Consume(
            TokenType::Semicolon,
            "Expected ';' after instruction call."
        );

        return Call{
            name.value,
            std::move(arguments),
            name.line,
            name.column
        };
    }

    [[noreturn]] void Parser::Error(
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

    std::vector<Entity> Parser::Parse()
    {
        std::vector<Entity> entities;

        while (!IsAtEnd())
        {
            if (Peek().type != TokenType::Hash)
            {
                Error(
                    Peek(),
                    "Expected an entity declaration."
                );
            }

            entities.push_back(ParseEntity());
        }

        return entities;
    }

}