// Copyright 2026 JesseTheCatLover. All Rights Reserved.

#include "Semantic/SemanticAnalyzer.h"

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

    const FFunctionDefinition* FSemanticModel::ResolveFunctionCall(const FCall& call) const
    {
        const FFunctionDefinition* function = FindFunction(call.name);

        if (function == nullptr)
            return nullptr;

        const std::size_t expectedCount = function->parameters.size();
        const std::size_t actualCount = call.arguments.size();

        if (expectedCount != actualCount)
        {
            throw std::runtime_error(
                "Semantic error at " +
                std::to_string(call.line) +
                ":" +
                std::to_string(call.column) +
                ": Function '" +
                std::string(call.name) +
                "' expects " +
                std::to_string(expectedCount) +
                (expectedCount == 1 ? " argument" : " arguments") +
                ", but received " +
                std::to_string(actualCount) +
                (actualCount == 1 ? " argument." : " arguments.")
            );
        }

        return function;
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

        // Register function definitions.

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

        // Validate calls in entity and function bodies.

        const auto validateInstructions = [&model](const std::vector<Instruction>& instructions)
        {
            for (const Instruction& instruction : instructions)
            {
                const auto* call = std::get_if<FCall>(&instruction);

                if (call == nullptr)
                    continue;

                static_cast<void>(model.ResolveFunctionCall(*call));
            }
        };

        for (const Declaration& declaration : m_File.declarations)
        {
            if (const auto* entity = std::get_if<FEntity>(&declaration))
                validateInstructions(entity->instructions);
            else if (const auto* function = std::get_if<FFunctionDefinition>(&declaration))
                validateInstructions(function->instructions);
        }

        return model;
    }
}