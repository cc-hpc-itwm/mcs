// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/overloaded.hpp>
#include <mcs/util/read/Read.hpp>

namespace mcs::core::storage
{
  constexpr MaxSize::MaxSize (decltype (_limit) limit) noexcept
    : _limit {limit}
  {}

  constexpr MaxSize::MaxSize (Unlimited unlimited) noexcept
    : _limit {unlimited}
  {}
  constexpr MaxSize::MaxSize (Limit limit) noexcept
    : _limit {limit}
  {}

  constexpr auto MaxSize::is_unlimited() const noexcept -> bool
  {
    return std::holds_alternative<MaxSize::Unlimited> (_limit);
  }

  constexpr auto MaxSize::limit() const -> memory::Size
  {
    return std::get<MaxSize::Limit> (_limit).size;
  }

  constexpr auto operator>
    ( memory::Size const& wanted_size
    , MaxSize const& max_size
    ) -> bool
  {
    return std::visit
      ( util::overloaded
        ( [&] (MaxSize::Unlimited) noexcept
          {
            return false;
          }
        , [&] (MaxSize::Limit limit) noexcept
          {
            return wanted_size > limit.size;
          }
        )
      , max_size._limit
      );
  }

  constexpr auto operator- (MaxSize max_size, memory::Size size) -> MaxSize
  {
    return std::visit
      ( util::overloaded
        ( [] (MaxSize::Unlimited) noexcept -> MaxSize
          {
            return MaxSize::Unlimited{};
          }
        , [&] (MaxSize::Limit limit) noexcept -> MaxSize
          {
            return MaxSize::Limit {limit.size - size};
          }
        )
      , max_size._limit
      );
  }

  constexpr auto operator+ (MaxSize lhs, MaxSize rhs) -> MaxSize
  {
    return std::visit
      ( util::overloaded
        ( [] (MaxSize::Unlimited, MaxSize::Unlimited) noexcept -> MaxSize
          {
            return MaxSize::Unlimited{};
          }
        , [] (MaxSize::Unlimited, MaxSize::Limit) noexcept -> MaxSize
          {
            return MaxSize::Unlimited{};
          }
        , [] (MaxSize::Limit, MaxSize::Unlimited) noexcept -> MaxSize
          {
            return MaxSize::Unlimited{};
          }
        , [] ( MaxSize::Limit lhs_limit
             , MaxSize::Limit rhs_limit
             ) noexcept -> MaxSize
          {
            return MaxSize::Limit {lhs_limit.size + rhs_limit.size};
          }
        )
      , lhs._limit
      , rhs._limit
      );
  }

  constexpr auto operator< (MaxSize lhs, MaxSize rhs) -> bool
  {
    return std::visit
      ( util::overloaded
        ( [&] (MaxSize::Unlimited, MaxSize::Unlimited) noexcept
          {
            return false;
          }
        , [&] (MaxSize::Unlimited, MaxSize::Limit) noexcept
          {
            return false;
          }
        , [&] (MaxSize::Limit, MaxSize::Unlimited) noexcept
          {
            return true;
          }
        , [&] (MaxSize::Limit lhs_limit, MaxSize::Limit rhs_limit) noexcept
          {
            return lhs_limit.size < rhs_limit.size;
          }
        )
      , lhs._limit
      , rhs._limit
      );
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::MaxSize>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::MaxSize>::format
      ( mcs::core::storage::MaxSize const& max_size
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return std::visit
      ( mcs::util::overloaded
      ( [&] (mcs::core::storage::MaxSize::Unlimited)
          {
            return fmt::format_to (ctx.out(), "Unlimited");
          }
        , [&] (mcs::core::storage::MaxSize::Limit limit)
          {
            return fmt::format_to (ctx.out(), "Limit {}", limit.size);
          }
        )
      , max_size._limit
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<core::storage::MaxSize>::read
      ( State<Char>& state
      ) -> core::storage::MaxSize
  {
    if (maybe_prefix (state, "Unlimited"))
    {
      return core::storage::MaxSize::Unlimited{};
    }

    prefix (state, "Limit");

    return core::storage::MaxSize::Limit
      {parse<core::memory::Size> (state)};
  }
}
