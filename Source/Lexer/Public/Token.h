// Copyright 2026 JesseTheCatLover. All Rights Reserved.

#pragma once

#include <cstddef>
#include <string_view>

namespace Foundry {

    enum class TokenType {
        Hash,
        Dollar,

        LeftParen,
        RightParen,

        LeftBrace,
        RightBrace,

        Colon,
        Semicolon,
        Comma,

        Identifier,
        String,
        Number,

        CMakeBlock,

        EndOfFile
    };

    [[nodiscard]] constexpr std::string_view TokenTypeToString(TokenType type) {
        switch (type) {
            case TokenType::Hash:        return "Hash";
            case TokenType::Dollar:      return "Dollar";
            case TokenType::LeftParen:   return "LeftParen";
            case TokenType::RightParen:  return "RightParen";
            case TokenType::LeftBrace:   return "LeftBrace";
            case TokenType::RightBrace:  return "RightBrace";
            case TokenType::Colon:       return "Colon";
            case TokenType::Semicolon:   return "Semicolon";
            case TokenType::Comma:       return "Comma";
            case TokenType::Identifier:  return "Identifier";
            case TokenType::String:      return "String";
            case TokenType::Number:      return "Number";
            case TokenType::CMakeBlock:  return "CMakeBlock";
            case TokenType::EndOfFile:   return "EndOfFile";
        }

        return "Unknown";
    }

    struct Token {
        TokenType type;
        std::string_view value;

        std::size_t line;
        std::size_t column;
    };

}