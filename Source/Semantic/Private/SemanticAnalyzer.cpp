// Copyright 2026 JesseTheCatLover. All Rights Reserved.

#include "SemanticAnalyzer.h"

#include <stdexcept>
#include <string>

namespace Foundry
{
    const FFunctionDefinition* FSemanticModel::FindFunction(std::string_view name) const
    {
        const auto iterator = m_Functions.find(std::string(name));

        if (iterator == m_Functions.end())
            return nullptr;

        return &iterator->second;
    }

    std::size_t FSemanticModel::GetFunctionCount() const
    {
        return m_Functions.size();
    }

    SemanticAnalyzer::SemanticAnalyzer(const FFoundryFile& file)
        : m_File(file)
    {
    }

    FSemanticModel SemanticAnalyzer::Analyze() const
    {
        FSemanticModel model;

        for (const Declaration& declaration : m_File.declarations)
        {
            const auto* function = std::get_if<FFunctionDefinition>(&declaration);

            if (function == nullptr)
                continue;

            const auto [iterator, inserted] = model.m_Functions.emplace(
                std::string(function->name),
                *function
            );

            if (inserted)
                continue;

            const FFunctionDefinition& previous = iterator->second;

            throw std::runtime_error(
                "Semantic error at " +
                std::to_string(function->line) +
                ":" +
                std::to_string(function->column) +
                ": Duplicate function definition '" +
                std::string(function->name) +
                "'. First definition at " +
                std::to_string(previous.line) +
                ":" +
                std::to_string(previous.column) +
                "."
            );
        }

        return model;
    }
}