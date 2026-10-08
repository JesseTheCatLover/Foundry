// Copyright 2026 JesseTheCatLover. All Rights Reserved.

#include "Lexer.h"

#include <cctype>
#include <stdexcept>
#include <string>

namespace Foundry {

    Lexer::Lexer(std::string_view source)
        : m_Source(source)
    {
    }

    char Lexer::Peek(std::size_t offset) const
    {
        const std::size_t position = m_Position + offset;

        if (position >= m_Source.size())
            return '\0';

        return m_Source[position];
    }

    char Lexer::Advance()
    {
        if (IsAtEnd())
            return '\0';

        const char character = m_Source[m_Position++];

        if (character == '\n')
        {
            ++m_Line;
            m_Column = 1;
        }
        else
        {
            ++m_Column;
        }

        return character;
    }

    bool Lexer::IsAtEnd() const
    {
        return m_Position >= m_Source.size();
    }

    void Lexer::SkipWhitespace()
    {
        while (!IsAtEnd())
        {
            if (!std::isspace(
                static_cast<unsigned char>(Peek())))
            {
                break;
            }

            Advance();
        }
    }

    void Lexer::SkipComment()
    {
        Advance(); // /
        Advance(); // /

        while (!IsAtEnd() && Peek() != '\n')
            Advance();
    }

    Token Lexer::MakeToken(
        TokenType type,
        std::size_t start,
        std::size_t line,
        std::size_t column
    ) const
    {
        return {
            type,
            m_Source.substr(start, m_Position - start),
            line,
            column
        };
    }

    Token Lexer::Identifier()
    {
        const std::size_t start = m_Position;
        const std::size_t line = m_Line;
        const std::size_t column = m_Column;

        while (!IsAtEnd())
        {
            const char character = Peek();

            if (!std::isalnum(
                    static_cast<unsigned char>(character)) &&
                character != '_')
            {
                break;
            }

            Advance();
        }

        return MakeToken(
            TokenType::Identifier,
            start,
            line,
            column
        );
    }

    Token Lexer::String()
    {
        const std::size_t start = m_Position;
        const std::size_t line = m_Line;
        const std::size_t column = m_Column;

        Advance(); // Opening "

        while (!IsAtEnd())
        {
            const char character = Peek();

            if (character == '\\')
            {
                Advance();

                if (!IsAtEnd())
                    Advance();

                continue;
            }

            if (character == '"')
            {
                Advance();

                return MakeToken(
                    TokenType::String,
                    start,
                    line,
                    column
                );
            }

            Advance();
        }

        Error("Unterminated string literal.");
    }

    Token Lexer::Number()
    {
        const std::size_t start = m_Position;
        const std::size_t line = m_Line;
        const std::size_t column = m_Column;

        bool hasDecimal = false;

        while (!IsAtEnd())
        {
            const char character = Peek();

            if (std::isdigit(
                    static_cast<unsigned char>(character)))
            {
                Advance();
                continue;
            }

            if (character == '.' && !hasDecimal)
            {
                hasDecimal = true;
                Advance();

                if (IsAtEnd() ||
                    !std::isdigit(
                        static_cast<unsigned char>(Peek())))
                {
                    Error("Expected digits after decimal point.");
                }

                continue;
            }

            break;
        }

        return MakeToken(
            TokenType::Number,
            start,
            line,
            column
        );
    }

    bool Lexer::IsCMakeBlock() const
    {
        constexpr std::string_view keyword = "cmake";

        if (m_Source.substr(
                m_Position,
                keyword.size()
            ) != keyword)
        {
            return false;
        }

        const std::size_t afterKeyword =
            m_Position + keyword.size();

        if (afterKeyword < m_Source.size())
        {
            const char character = m_Source[afterKeyword];

            if (std::isalnum(
                    static_cast<unsigned char>(character)) ||
                character == '_')
            {
                return false;
            }
        }

        std::size_t position = afterKeyword;

        while (position < m_Source.size() &&
               std::isspace(
                   static_cast<unsigned char>(
                       m_Source[position])))
        {
            ++position;
        }

        if (position >= m_Source.size())
            return false;

        return m_Source[position] == '{';
    }

    Token Lexer::CMakeBlock()
    {
        const std::size_t start = m_Position;
        const std::size_t line = m_Line;
        const std::size_t column = m_Column;

        while (!IsAtEnd() && Peek() != '{')
            Advance();

        if (IsAtEnd())
            Error("Expected '{' after cmake.");

        Advance(); // {

        std::size_t depth = 1;

        while (!IsAtEnd() && depth > 0)
        {
            const char character = Peek();

            if (character == '"')
            {
                Advance();

                while (!IsAtEnd())
                {
                    const char stringCharacter = Peek();

                    if (stringCharacter == '\\')
                    {
                        Advance();

                        if (!IsAtEnd())
                            Advance();

                        continue;
                    }

                    Advance();

                    if (stringCharacter == '"')
                        break;
                }

                continue;
            }

            if (character == '#')
            {
                while (!IsAtEnd() && Peek() != '\n')
                    Advance();

                continue;
            }

            if (character == '{')
            {
                ++depth;
                Advance();
                continue;
            }

            if (character == '}')
            {
                --depth;
                Advance();
                continue;
            }

            Advance();
        }

        if (depth != 0)
            Error("Unterminated cmake block.");

        return MakeToken(
            TokenType::CMakeBlock,
            start,
            line,
            column
        );
    }

    [[noreturn]]
    void Lexer::Error(std::string_view message) const
    {
        throw std::runtime_error(
            "Lexer error at " +
            std::to_string(m_Line) +
            ":" +
            std::to_string(m_Column) +
            ": " +
            std::string(message)
        );
    }

    std::vector<Token> Lexer::tokenize()
    {
        std::vector<Token> tokens;

        while (!IsAtEnd())
        {
            SkipWhitespace();

            if (IsAtEnd())
                break;

            if (Peek() == '/' && Peek(1) == '/')
            {
                SkipComment();
                continue;
            }

            const std::size_t line = m_Line;
            const std::size_t column = m_Column;
            const std::size_t start = m_Position;

            if (IsCMakeBlock())
            {
                tokens.push_back(CMakeBlock());
                continue;
            }

            switch (Peek())
            {
                case '#':
                    Advance();
                    tokens.push_back(
                        MakeToken(
                            TokenType::Hash,
                            start,
                            line,
                            column
                        )
                    );
                    break;

                case '$':
                    Advance();
                    tokens.push_back(
                        MakeToken(
                            TokenType::Dollar,
                            start,
                            line,
                            column
                        )
                    );
                    break;

                case '(':
                    Advance();
                    tokens.push_back(
                        MakeToken(
                            TokenType::LeftParen,
                            start,
                            line,
                            column
                        )
                    );
                    break;

                case ')':
                    Advance();
                    tokens.push_back(
                        MakeToken(
                            TokenType::RightParen,
                            start,
                            line,
                            column
                        )
                    );
                    break;

                case '{':
                    Advance();
                    tokens.push_back(
                        MakeToken(
                            TokenType::LeftBrace,
                            start,
                            line,
                            column
                        )
                    );
                    break;

                case '}':
                    Advance();
                    tokens.push_back(
                        MakeToken(
                            TokenType::RightBrace,
                            start,
                            line,
                            column
                        )
                    );
                    break;

                case ':':
                    Advance();
                    tokens.push_back(
                        MakeToken(
                            TokenType::Colon,
                            start,
                            line,
                            column
                        )
                    );
                    break;

                case ';':
                    Advance();
                    tokens.push_back(
                        MakeToken(
                            TokenType::Semicolon,
                            start,
                            line,
                            column
                        )
                    );
                    break;

                case ',':
                    Advance();
                    tokens.push_back(
                        MakeToken(
                            TokenType::Comma,
                            start,
                            line,
                            column
                        )
                    );
                    break;

                case '"':
                    tokens.push_back(String());
                    break;

                default:
                    if (std::isalpha(
                            static_cast<unsigned char>(Peek())) ||
                        Peek() == '_')
                    {
                        tokens.push_back(Identifier());
                    }
                    else if (std::isdigit(
                            static_cast<unsigned char>(Peek())))
                    {
                        tokens.push_back(Number());
                    }
                    else
                    {
                        Error(
                            "Unexpected character '" +
                            std::string(1, Peek()) +
                            "'."
                        );
                    }

                    break;
            }
        }

        tokens.push_back({
            TokenType::EndOfFile,
            {},
            m_Line,
            m_Column
        });

        return tokens;
    }

}