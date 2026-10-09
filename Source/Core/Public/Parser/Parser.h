// Copyright 2026 JesseTheCatLover. All Rights Reserved.

#pragma once

#include "AST.h"

#include "Lexer/Token.h"

#include <cstddef>
#include <string_view>
#include <vector>

namespace Foundry
{
    class Parser
    {
    private:
        const std::vector<FToken>& m_Tokens;

        std::size_t m_Position = 0;

    public:
        explicit Parser(const std::vector<FToken>& tokens);

        [[nodiscard]] FFoundryFile Parse();

    private:
        [[nodiscard]] const FToken& Peek(std::size_t offset = 0) const;

        const FToken& Advance();

        [[nodiscard]] bool IsAtEnd() const;

        bool Match(ETokenType type);

        [[nodiscard]] const FToken& Consume(
            ETokenType type,
            std::string_view message
        );

        [[nodiscard]] FEntity ParseEntity();

        [[nodiscard]] FFunctionDefinition ParseFunction();

        [[nodiscard]] Instruction ParseInstruction();

        [[nodiscard]] FCall ParseCall();

        [[noreturn]] void Error(
            const FToken& token,
            std::string_view message
        ) const;
    };
}