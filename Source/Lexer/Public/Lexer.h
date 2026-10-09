// Copyright 2026 JesseTheCatLover. All Rights Reserved.

#pragma once

#include "Token.h"

#include <string_view>
#include <vector>

namespace Foundry
{
    class Lexer
    {
    private:
        std::string_view m_Source;

        std::size_t m_Position = 0;
        std::size_t m_Line = 1;
        std::size_t m_Column = 1;

    public:
        explicit Lexer(std::string_view source);

        [[nodiscard]] std::vector<FToken> tokenize();

    private:
        [[nodiscard]] char Peek(std::size_t offset = 0) const;

        char Advance();

        [[nodiscard]]
        bool IsAtEnd() const;

        void SkipWhitespace();
        void SkipComment();

        [[nodiscard]] FToken MakeToken(
            ETokenType type,
            std::size_t start,
            std::size_t line,
            std::size_t column
        ) const;

        [[nodiscard]] FToken Identifier();

        [[nodiscard]] FToken String();

        [[nodiscard]] FToken Number();

        [[nodiscard]] bool IsCMakeBlock() const;

        [[nodiscard]] FToken CMakeBlock();

        [[noreturn]] void Error(std::string_view message) const;
    };
}