// Copyright 2026 JesseTheCatLover. All Rights Reserved.

#pragma once

#include <cstddef>
#include <string_view>
#include <variant>
#include <vector>

namespace Foundry {

    struct Call
    {
        std::string_view name;
        std::vector<std::string_view> arguments;

        std::size_t line;
        std::size_t column;
    };

    struct CMakeBlock
    {
        std::string_view source;

        std::size_t line;
        std::size_t column;
    };

    using Instruction = std::variant<Call, CMakeBlock>;

    struct Entity
    {
        std::string_view type;
        std::string_view name;

        std::vector<Instruction> instructions;

        std::size_t line;
        std::size_t column;
    };

}