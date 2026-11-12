// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <cstddef>
#include <fmt/format.h>
#include <map>
#include <mcs/util/command_line/Parsed.hpp>
#include <mcs/util/isspace.hpp>
#include <mcs/util/overloaded.hpp>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace
{
  struct Sections
  {
    std::vector<std::size_t> top_level;
    std::map<std::string_view, std::vector<std::string_view>> embedded;
  };

  [[nodiscard]] auto sections
    ( std::span<char const* const> arguments
    ) -> Sections
  {
    auto result {Sections{}};
    auto position {std::size_t{}};

    while (position < arguments.size())
    {
      auto const text {std::string_view {arguments[position]}};

      if (text.starts_with ("[["))
      {
        auto const identifier {text.substr (2)};
        auto const is_closing
          { [identifier] (std::string_view token) noexcept
            {
              return token.ends_with ("]]")
                && token.substr (0, token.size() - 2) == identifier
                ;
            }
          };

        auto& contents {result.embedded[identifier]};

        ++position;

        while (position < arguments.size() && !is_closing (arguments[position]))
        {
          contents.emplace_back (arguments[position++]);
        }

        if (position == arguments.size())
        {
          throw std::invalid_argument
            { fmt::format ("missing closing embedding tag '{}]]'", identifier)
            };
        }

        ++position;
      }
      else
      {
        if (!text.starts_with ("--") && text.ends_with ("]]"))
        {
          throw std::invalid_argument
            { fmt::format ("unmatched closing embedding tag '{}'", text)
            };
        }

        result.top_level.emplace_back (position++);
      }
    }

    return result;
  }
}

namespace mcs::util::command_line
{
  auto Parsed::occurrences (std::string_view name) const -> Occurrences const&
  {
    static auto const empty {Occurrences{}};

    auto const position {_options.find (name)};

    return position == std::end (_options) ? empty : position->second;
  }

  auto Parsed::at (std::string_view name) const -> std::string_view
  {
    auto const& found {occurrences (name)};

    if (found.size() != 1 || found.front().size() != 1)
    {
      throw std::invalid_argument
        { fmt::format ("option --{} must have exactly one value", name)
        };
    }

    return found.front().front();
  }

  auto Parsed::optional
    ( std::string_view name
    ) const -> std::optional<std::string_view>
  {
    if (occurrences (name).empty())
    {
      return {};
    }

    return at (name);
  }

  auto Parsed::flag (std::string_view name) const -> bool
  {
    auto const& found {occurrences (name)};

    if ( ! std::ranges::all_of
           ( found
           , [] (auto const& occurrence) noexcept
             {
               return occurrence.empty();
             }
           )
        )
    {
      throw std::invalid_argument
        { fmt::format ("option --{} is not a flag", name)
        };
    }

    return !found.empty();
  }

  auto Parsed::embedded
    ( std::string_view marker
    ) const -> std::vector<std::string_view> const&
  {
    if ( auto const position {_embedded.find (marker)}
       ; position != std::end (_embedded)
       )
    {
      return position->second;
    }

    throw error::UnknownMarker
      { fmt::format ("unknown embedding marker \"{}\"", marker)
      };
  }
}

namespace mcs::util::command_line
{
  Parsed::Parsed
    ( std::span<char const* const> arguments
    , Schema const& schema
    )
    : _embedded {schema._embeddings}
  {
    auto definitions {std::map<std::string_view, Schema::Definition const*>{}};

    std::ranges::for_each
      ( schema._options
      , [&] (Schema::Definition const& option)
        {
          std::visit
            ( [&] (auto const& definition)
              {
                auto const& name {definition._name};

                if (  name.empty()
                   || name.starts_with ('-')
                   || std::ranges::any_of (name, isspace<char>)
                   || name.find_first_of ("=[]") != std::string::npos
                   || !definitions.emplace (name, &option).second
                   )
                {
                  throw std::logic_error {"invalid named-option schema"};
                }
              }
            , option
            );
        }
      );

    auto scanned {sections (arguments)};

    std::ranges::for_each
      ( scanned.embedded
      , [&] (auto& section)
        {
          if ( auto const position {_embedded.find (section.first)}
             ; position != std::end (_embedded)
             )
          {
            position->second = std::move (section.second);
          }
          else
          {
            throw std::invalid_argument
              { fmt::format
                ( "unknown embedding {}"
                , section.first.empty() ? std::string_view {"\"\""}
                                        : section.first
                )
              };
          }
        }
      );

    auto position {std::size_t{}};

    while (position < scanned.top_level.size())
    {
      auto const index {scanned.top_level[position++]};
      auto const token {std::string_view {arguments[index]}};
      auto const equal {token.find ('=')};
      auto const name
        { token.substr (0, equal).substr (token.starts_with ("--") ? 2 : 0)
        };
      auto const definition {definitions.find (name)};

      if (!token.starts_with ("--") || definition == definitions.end())
      {
        throw std::invalid_argument
          { token.starts_with ("--")
            ? fmt::format ("unknown option --{}", name)
            : fmt::format ("unexpected positional argument: {}", token)
          };
      }

      std::visit
        ( util::overloaded
          { [&] (option::Flag const&)
            {
              if (equal != std::string_view::npos)
              {
                throw std::invalid_argument
                  { fmt::format ("flag --{} does not accept a value", name)
                  };
              }
              _options[name].emplace_back();
            }
          , [&] (option::Parameter const& parameter)
            {
              auto& occurrences {_options[name]};

              std::visit
                ( util::overloaded
                  { [&] (option::Parameter::Multiplicity::Unique const&)
                    {
                      if (!occurrences.empty())
                      {
                        throw std::invalid_argument
                          { fmt::format ("duplicate option --{}", name)
                          };
                      }
                    }
                  , [] (option::Parameter::Multiplicity::Repeatable const&) noexcept
                    {
                      // do nothing
                    }
                  }
                , parameter._multiplicity
                );

              auto values {Parsed::Occurrence{}};

              if (equal != std::string_view::npos)
              {
                if (equal + 1 == token.size())
                {
                  throw std::invalid_argument
                    { fmt::format ("missing option value for --{}", name)
                    };
                }

                values.emplace_back (token.substr (equal + 1));
              }
              else
              {
                if (  position == scanned.top_level.size()
                   || scanned.top_level[position] != index + 1
                   || std::string_view
                      { arguments[scanned.top_level[position]]
                      }.starts_with ("--")
                   )
                {
                  throw std::invalid_argument
                    { fmt::format ("missing option value for --{}", name)
                    };
                }

                values.emplace_back (arguments[scanned.top_level[position++]]);
              }

              occurrences.emplace_back (std::move (values));
            }
          }
        , *definition->second
        );
    }

    std::ranges::for_each
      ( schema._options
      , [&] (Schema::Definition const& option)
        {
          std::visit
            ( util::overloaded
              { [] (option::Flag const&) noexcept
                {
                  // do nothing
                }
              , [&] (option::Parameter const& parameter)
                {
                  std::visit
                    ( util::overloaded
                      { [] (option::Parameter::Presence::Optional const&) noexcept
                        {
                          // do nothing
                        }
                      , [&] (option::Parameter::Presence::Required const&)
                        {
                          if (occurrences (parameter._name).empty())
                          {
                            throw std::invalid_argument
                              { fmt::format
                                ( "missing required option --{}"
                                , parameter._name
                                )
                              };
                          }
                        }
                      }
                    , parameter._presence
                    );
                }
              }
            , option
            );
        }
      );
  }
}
