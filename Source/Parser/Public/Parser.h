// Copyright 2026 JesseTheCatLover. All Rights Reserved.

#pragma once

#include "AST.h"

#include <Token.h>

#include <cstddef>
#include <string_view>
#include <vector>

namespace Foundry
{
    class Parser
    {
    private:
        const std::vector<Token>& m_Tokens;

        std::size_t m_Position = 0;

    public:
        explicit Parser(const std::vector<Token>& tokens);

        [[nodiscard]] std::vector<Entity> Parse();

    private:
        [[nodiscard]] const Token& Peek(std::size_t offset = 0) const;

        const Token& Advance();

        [[nodiscard]] bool IsAtEnd() const;

        bool Match(TokenType type);

        [[nodiscard]] const Token& Consume(
            TokenType type,
            std::string_view message
        );

        [[nodiscard]] Entity ParseEntity();

        [[nodiscard]] Instruction ParseInstruction();

        [[nodiscard]] Call ParseCall();

        [[noreturn]] void Error(
            const Token& token,
            std::string_view message
        ) const;
    };
}