// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <iterator>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/util/cast.hpp>
#include <ostream>

namespace mcs::serialization
{
  auto OArchive::store (OArchive const& other) -> OArchive&
  {
    other.for_each_data_buffer
      ( [&] (auto const& buffer)
        {
          this->append
            ( std::span { _bytes_for_temporaries.emplace_back
                           ( std::begin (buffer), std::end (buffer)
                           )
                        }
            );
        }
      );

    return *this;
  }

  auto OArchive::bytes() const -> std::vector<std::byte>
  {
    std::vector<std::byte> bytes;

    for_each_data_buffer
      ( [&] (auto const& buffer)
        {
          std::ranges::copy (buffer, std::back_inserter (bytes));
        }
      );

    return bytes;
  }
}

namespace mcs::serialization
{
  auto operator<< (std::ostream& os, OArchive const& oa) -> std::ostream&
  {
    oa.for_each_data_buffer
      ( [&] (auto const& buffer) noexcept
        {
          os.write
            ( util::cast<char const*> (buffer.data())
            , util::cast<std::streamsize> (buffer.size())
            );
        }
      );

    return os;
  }
}
