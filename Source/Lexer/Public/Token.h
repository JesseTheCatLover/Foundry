// Copyright 2026 JesseTheCatLover. All Rights Reserved.

#pragma once

#include <cstddef>
#include <string_view>

namespace Foundry {

    enum class ETokenType {
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

    [[nodiscard]] constexpr std::string_view TokenTypeToString(ETokenType type) {
        switch (type) {
            case ETokenType::Hash:        return "Hash";
            case ETokenType::Dollar:      return "Dollar";
            case ETokenType::LeftParen:   return "LeftParen";
            case ETokenType::RightParen:  return "RightParen";
            case ETokenType::LeftBrace:   return "LeftBrace";
            case ETokenType::RightBrace:  return "RightBrace";
            case ETokenType::Colon:       return "Colon";
            case ETokenType::Semicolon:   return "Semicolon";
            case ETokenType::Comma:       return "Comma";
            case ETokenType::Identifier:  return "Identifier";
            case ETokenType::String:      return "String";
            case ETokenType::Number:      return "Number";
            case ETokenType::CMakeBlock:  return "CMakeBlock";
            case ETokenType::EndOfFile:   return "EndOfFile";
        }

        return "Unknown";
    }

    struct FToken {
        ETokenType type;
        std::string_view value;

        std::size_t line;
        std::size_t column;
    };

}