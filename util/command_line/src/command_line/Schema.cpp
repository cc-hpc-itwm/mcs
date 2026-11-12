// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <iterator>
#include <mcs/util/command_line/Schema.hpp>
#include <mcs/util/isspace.hpp>
#include <stdexcept>
#include <string>
#include <utility>

namespace mcs::util::command_line
{
  Schema::Schema() = default;

  auto Schema::add (Schema schema) & -> Schema&
  {
    if ( std::ranges::any_of
         ( schema._embeddings
         , [this] (auto const& embedding) noexcept
           {
             return _embeddings.contains (embedding.first);
           }
         )
       )
    {
      throw std::logic_error {"invalid embedding schema"};
    }

    std::ranges::move (schema._options, std::back_inserter (_options));
    _embeddings.merge (schema._embeddings);

    return *this;
  }

  auto Schema::add (Schema schema) && -> Schema&&
  {
    add (std::move (schema));

    return std::move (*this);
  }

  auto Schema::add (option::Flag flag) & -> Schema&
  {
    _options.emplace_back (flag);

    return *this;
  }

  auto Schema::add (option::Flag flag) && -> Schema&&
  {
    add (flag);

    return std::move (*this);
  }

  auto Schema::add (option::Parameter parameter) & -> Schema&
  {
    _options.emplace_back (parameter);

    return *this;
  }

  auto Schema::add (option::Parameter parameter) && -> Schema&&
  {
    add (parameter);

    return std::move (*this);
  }

  auto Schema::embedding (std::string identifier) & -> Schema&
  {
    if (  std::ranges::any_of (identifier, isspace<char>)
       || identifier.find_first_of ("[]") != std::string::npos
       || !_embeddings.try_emplace (std::move (identifier)).second
       )
    {
      throw std::logic_error {"invalid embedding schema"};
    }

    return *this;
  }

  auto Schema::embedding (std::string identifier) && -> Schema&&
  {
    embedding (std::move (identifier));

    return std::move (*this);
  }
}
