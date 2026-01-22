// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <fmt/base.h>
#include <mcs/Error.hpp>
#include <mcs/core/memory/Offset.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/serialization/IArchive.hpp>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>
#include <mcs/util/read/Read.hpp>

namespace mcs::core::memory
{
  // [begin, end)
  struct Range
  {
    friend constexpr auto make_range (Offset, Offset) -> Range;
    friend constexpr auto make_range (Offset, Size) -> Range;

    friend constexpr auto begin (Range const&) -> Offset;
    friend constexpr auto end (Range const&) -> Offset;
    friend constexpr auto size (Range const&) -> Size;

    constexpr auto operator<=> (Range const&) const noexcept = default;

    friend constexpr auto shift (Range, Offset) -> Range;

    struct Error
    {
      struct BeginMustNotBeLargerThanEnd : public mcs::Error
      {
        [[nodiscard]] constexpr auto begin() const noexcept -> Offset;
        [[nodiscard]] constexpr auto end() const noexcept -> Offset;

        ~BeginMustNotBeLargerThanEnd() override;
        BeginMustNotBeLargerThanEnd (BeginMustNotBeLargerThanEnd const&) = default;
        BeginMustNotBeLargerThanEnd (BeginMustNotBeLargerThanEnd&&) noexcept = default;
        auto operator= (BeginMustNotBeLargerThanEnd const&) -> BeginMustNotBeLargerThanEnd& = default;
        auto operator= (BeginMustNotBeLargerThanEnd&&) noexcept  -> BeginMustNotBeLargerThanEnd& = default;

      private:
        friend struct Range;

        explicit BeginMustNotBeLargerThanEnd (Offset, Offset) noexcept;

        Offset _begin;
        Offset _end;
      };
    };

  private:
    constexpr explicit Range (Offset, Offset);

    Offset _begin;
    Offset _end;

    template<typename, typename, typename> friend struct fmt::formatter;
    template<typename> friend struct serialization::Implementation;
  };

  template<std::integral I>
    [[nodiscard]] constexpr auto make_range (I, I) -> Range;

  [[nodiscard]] constexpr auto make_range (Offset, Offset) -> Range;
  [[nodiscard]] constexpr auto make_range (Offset, Size) -> Range;

  [[nodiscard]] constexpr auto begin (Range const&) -> Offset;
  [[nodiscard]] constexpr auto end (Range const&) -> Offset;
  [[nodiscard]] constexpr auto size (Range const&) -> Size;

  [[nodiscard]] constexpr auto shift (Range, Offset) -> Range;

  template<typename T>
    [[nodiscard]] constexpr auto select (std::span<T>, Range const&);
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::memory::Range>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::memory::Range const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<core::memory::Range>
  {
    using Type = core::memory::Range;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<core::memory::Range>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> core::memory::Range
        ;
  };
}

#include "detail/Range.ipp"
