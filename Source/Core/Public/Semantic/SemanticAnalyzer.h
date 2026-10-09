// Copyright 2026 JesseTheCatLover. All Rights Reserved.

#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_map>

#include "Parser/AST.h"

namespace Foundry
{
    class SemanticAnalyzer;

    struct FSemanticModel
    {
    private:
        std::unordered_map<std::string, FFunctionDefinition> m_Functions;

        friend class SemanticAnalyzer;

    public:
        [[nodiscard]] const FFunctionDefinition* FindFunction(std::string_view name) const;

        [[nodiscard]] std::size_t GetFunctionCount() const;
    };

    class SemanticAnalyzer
    {
    private:
        const FFoundryFile& m_File;

    public:
        explicit SemanticAnalyzer(const FFoundryFile& file);

        [[nodiscard]] FSemanticModel Analyze() const;
    };
}
