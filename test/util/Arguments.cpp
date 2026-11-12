// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdlib>
#include <fmt/format.h>
#include <functional>
#include <gtest/gtest.h>
#include <iterator>
#include <limits>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/STD/string.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/util/arguments/Owned.hpp>
#include <mcs/util/arguments/View.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/not_null.hpp>
#include <memory>
#include <optional>
#include <ranges>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace mcs::util
{
  template<typename ArgumentType>
    struct ArgumentsStorageTest : testing::random::Test{};

  using ArgumentStorageTypes
    = ::testing::Types<arguments::View, arguments::Owned>;

  TYPED_TEST_SUITE (ArgumentsStorageTest, ArgumentStorageTypes);

  TEST (Arguments, descriptors_are_explicit_and_constexpr)
  {
    using command_line::option::Flag;
    using command_line::option::Parameter;
    using Multiplicity = Parameter::Multiplicity;
    using Presence = Parameter::Presence;

    static_assert (!std::is_default_constructible_v<Flag>);
    static_assert (!std::is_default_constructible_v<Parameter>);
    static_assert (std::is_constructible_v<Flag, std::string_view>);
    static_assert
      ( std::is_constructible_v
        < Parameter
        , std::string_view
        , Presence::Policy
        , Multiplicity::Policy
        >
      );
    static_assert (!std::is_convertible_v<std::string_view, Flag>);
    static_assert (!std::is_convertible_v<std::string_view, Parameter>);
    static_assert (std::is_convertible_v<Flag, std::string_view>);
    static_assert (std::is_convertible_v<Parameter, std::string_view>);

    constexpr auto flag {Flag {"flag"}};
    constexpr auto parameter {Parameter {"parameter"}};
    constexpr auto repeatable
      { Parameter
        { "path"
        , Presence::Optional{}
        , Multiplicity::Repeatable{}
        }
      };
    static_assert (std::string_view {flag} == "flag");
    static_assert (std::string_view {parameter} == "parameter");
    static_assert (std::string_view {repeatable} == "path");
    static_assert (noexcept (Flag {"flag"}));
    static_assert (noexcept (Parameter {"parameter"}));
    static_assert (noexcept (std::string_view {parameter}));
    static_assert (noexcept (std::string_view {flag}));

    EXPECT_EQ (fmt::format ("--{} --{} --{}", flag, parameter, repeatable),
               "--flag --parameter --path");
  }

  TEST (Arguments, descriptors_borrow_bounded_names)
  {
    auto const parameter_name {std::string {"parameter-suffix"}};
    auto const flag_name {std::string {"flag-suffix"}};
    auto const parameter
      { command_line::option::Parameter
        { std::string_view {parameter_name.data(), 9UL}
        }
      };
    auto const flag
      { command_line::option::Flag
        { std::string_view {flag_name.data(), 4UL}
        }
      };

    EXPECT_EQ (std::string_view {parameter}.data(), parameter_name.data());
    EXPECT_EQ (std::string_view {flag}.data(), flag_name.data());
    EXPECT_EQ (fmt::format ("{:>10} {}", parameter, flag), " parameter flag");
    EXPECT_EQ (fmt::format ("{:.4}", parameter), "para");
    EXPECT_EQ (fmt::format ("{:*^8.3}", flag), "**fla***");
    EXPECT_EQ (fmt::format ("{:>{}}", flag, 6), "  flag");

    auto const schema
      { command_line::Schema{}
      . add (parameter)
      . add (flag)
      };

    auto const copied {schema};
    auto const pointers {std::array {"--parameter=value", "--flag"}};

    auto const parsed
      { command_line::Parsed {std::span<char const* const> {pointers}, copied}
      };

    EXPECT_EQ (parsed.at (parameter), "value");
    EXPECT_TRUE (parsed.flag (flag));
  }

  TEST (Arguments, schema_construction_preserves_value_categories)
  {
    using Schema = command_line::Schema;
    using Parameter = command_line::option::Parameter;
    using Flag = command_line::option::Flag;

    static_assert (std::is_default_constructible_v<Schema>);
    static_assert (std::is_copy_constructible_v<Schema>);
    static_assert (std::is_move_constructible_v<Schema>);
    static_assert
      ( std::is_same_v
        < decltype (std::declval<Schema&>().add (std::declval<Parameter>()))
        , Schema&
        >
      );
    static_assert
      ( std::is_same_v
        < decltype (std::declval<Schema&&>().add (std::declval<Parameter>()))
        , Schema&&
        >
      );
    static_assert
      ( std::is_same_v
        < decltype (std::declval<Schema&>().add (std::declval<Schema>()))
        , Schema&
        >
      );
    static_assert
      ( std::is_same_v
        < decltype (std::declval<Schema&&>().add (std::declval<Schema>()))
        , Schema&&
        >
      );
    static_assert
      ( std::is_same_v
        < decltype (std::declval<Schema&>().add (std::declval<Flag>()))
        , Schema&
        >
      );
    static_assert
      ( std::is_same_v
        < decltype (std::declval<Schema&&>().add (std::declval<Flag>()))
        , Schema&&
        >
      );
    static_assert
      ( std::is_same_v
        < decltype (std::declval<Schema&>().embedding ("FUSE"))
        , Schema&
        >
      );
    static_assert
      ( std::is_same_v
        < decltype (std::declval<Schema&&>().embedding ("FUSE"))
        , Schema&&
        >
      );
  }

  TYPED_TEST (ArgumentsStorageTest, named_schema_mutators_append_in_place)
  {
    auto schema {command_line::Schema{}};
    auto const empty {TypeParam {arguments::View {nullptr, 0UL}}};
    auto const empty_result {empty.parse (schema)};
    EXPECT_TRUE (empty_result.occurrences ("first").empty());

    auto const options
      { command_line::Schema{}
      . add (command_line::option::Parameter {"first"})
      . add (command_line::option::Flag {"group-flag"})
      . embedding ("FUSE")
      };

    EXPECT_EQ
      ( std::addressof (schema.add (options))
      , std::addressof (schema)
      );

    EXPECT_EQ
      ( std::addressof (schema.add (command_line::option::Parameter {"second"}))
      , std::addressof (schema)
      );

    schema.add (command_line::option::Flag {"flag"});
    schema.add (command_line::Schema{});

    EXPECT_EQ
      ( std::addressof (schema.embedding ("BEE"))
      , std::addressof (schema)
      );

    schema.embedding ("");

    auto const pointers
      { std::array
        { "--first=one", "--second=two", "--flag", "--group-flag"
        , "[[FUSE", "-f", "FUSE]]", "[[BEE", "-g", "BEE]]"
        , "[[", "-d", "]]"
        }
      };

    auto const arguments
      { TypeParam {arguments::View {pointers.data(), pointers.size()}}
      };
    auto const parsed {arguments.parse (schema)};

    EXPECT_EQ (parsed.at ("first"), "one");
    EXPECT_EQ (parsed.at ("second"), "two");
    EXPECT_TRUE (parsed.flag ("flag"));
    EXPECT_TRUE (parsed.flag ("group-flag"));
    EXPECT_EQ
      ( parsed.embedded ("FUSE")
      , (std::vector<std::string_view> {"-f"})
      );
    EXPECT_EQ
      ( parsed.embedded ("BEE")
      , (std::vector<std::string_view> {"-g"})
      );
    EXPECT_EQ
      ( parsed.embedded ("")
      , (std::vector<std::string_view> {"-d"})
      );

    auto const group_pointers
      { std::array {"--first=one", "--group-flag", "[[FUSE", "-f", "FUSE]]"}
      };
    auto const group_arguments
      { TypeParam
        { arguments::View {group_pointers.data(), group_pointers.size()}
        }
      };
    auto const group_parsed {group_arguments.parse (options)};

    EXPECT_EQ (group_parsed.at ("first"), "one");
    EXPECT_TRUE (group_parsed.flag ("group-flag"));
    EXPECT_EQ (group_parsed.embedded ("FUSE"), parsed.embedded ("FUSE"));
  }

  TYPED_TEST (ArgumentsStorageTest, temporary_schema_chains_append_options)
  {
    using mcs::util::command_line::option::Flag;
    using mcs::util::command_line::option::Parameter;

    auto const schema
      { command_line::Schema{}
      . add
        ( command_line::Schema{}
        . add (Parameter {"first"})
        . add (Flag {"group-flag"})
        . embedding ("FUSE")
        )
      . add (Parameter {"second"})
      . add (Flag {"flag"})
      . add (command_line::Schema{})
      . embedding ("")
      };

    auto const pointers
      { std::array
        { "--first=one", "--second=two", "--flag", "--group-flag"
        , "[[FUSE", "-f", "FUSE]]", "[[", "-d", "]]"
        }
      };

    auto const arguments
      { TypeParam {arguments::View {pointers.data(), pointers.size()}}
      };
    auto const parsed {arguments.parse (schema)};

    EXPECT_EQ (parsed.at ("first"), "one");
    EXPECT_EQ (parsed.at ("second"), "two");
    EXPECT_TRUE (parsed.flag ("flag"));
    EXPECT_TRUE (parsed.flag ("group-flag"));
    EXPECT_EQ
      ( parsed.embedded ("FUSE")
      , (std::vector<std::string_view> {"-f"})
      );
    EXPECT_EQ
      ( parsed.embedded ("")
      , (std::vector<std::string_view> {"-d"})
      );
  }

  TYPED_TEST (ArgumentsStorageTest, composition_preserves_parameter_policies)
  {
    using command_line::option::Parameter;
    using command_line::Schema;
    using Multiplicity = Parameter::Multiplicity;
    using Presence = Parameter::Presence;

    auto const group
      { Schema{}
      . add (Parameter {"first"})
      . add (Parameter {"optional", Presence::Optional{}})
      . add
        ( Parameter
          { "repeated"
          , Presence::Required{}
          , Multiplicity::Repeatable{}
          }
        )
      };
    auto const schema
      { Schema{}
      . add (group)
      . add (Parameter {"last"})
      };

    auto const empty {TypeParam {arguments::View {nullptr, 0UL}}};

    testing::require_exception
      ( [&]
        {
          std::ignore = empty.parse (schema);
        }
      , testing::assert_type_and_what<std::invalid_argument>
          ("missing required option --first")
      );

    auto const pointers
      { std::array
        { "--first=one", "--repeated=two", "--repeated=three", "--last=four"
        }
      };
    auto const arguments
      { TypeParam {arguments::View {pointers.data(), pointers.size()}}
      };
    auto const parsed {arguments.parse (schema)};

    EXPECT_EQ (parsed.at ("first"), "one");
    EXPECT_FALSE (parsed.optional ("optional").has_value());
    EXPECT_EQ
      ( parsed.occurrences ("repeated")
      , (command_line::Parsed::Occurrences {{"two"}, {"three"}})
      );
    EXPECT_EQ (parsed.at ("last"), "four");

    auto const duplicate_pointers {std::array {"--first=one", "--first=two"}};
    auto const duplicates
      { TypeParam
        { arguments::View
          { duplicate_pointers.data()
          , duplicate_pointers.size()
          }
        }
      };

    testing::require_exception
      ( [&]
        {
          std::ignore = duplicates.parse (schema);
        }
      , testing::assert_type_and_what<std::invalid_argument>
          ("duplicate option --first")
      );
  }

  TEST (Arguments, parsed_results_are_directly_constructed_and_read_only)
  {
    using mcs::util::command_line::option::Parameter;
    using Parsed = command_line::Parsed;

    static_assert
      ( std::is_constructible_v
        < Parsed
        , std::span<char const* const>
        , command_line::Schema const&
        >
      );
    static_assert
      (std::is_same_v<Parsed::Occurrence::value_type, std::string_view>);
    static_assert
      ( std::is_same_v
        < decltype (std::declval<Parsed const&>().optional (""))
        , std::optional<std::string_view>
        >
      );
    static_assert (std::is_copy_constructible_v<Parsed>);
    static_assert (std::is_move_constructible_v<Parsed>);
    static_assert
      ( std::is_same_v
        < decltype (std::declval<Parsed const&>().occurrences (""))
        , Parsed::Occurrences const&
        >
      );
    static_assert
      ( std::is_same_v
        < decltype (std::declval<Parsed&>().embedded ("FUSE"))
        , std::vector<std::string_view> const&
        >
      );
    static_assert
      ( std::is_same_v
        < decltype (std::declval<Parsed const&>().embedded ("FUSE"))
        , std::vector<std::string_view> const&
        >
      );
    static_assert (!noexcept (std::declval<Parsed const&>().embedded ("FUSE")));

    auto const pointers {std::array {"--name=value"}};
    auto const schema
      { command_line::Schema{}
      . add (Parameter {"name"})
      };
    auto const parsed
      { Parsed {std::span<char const* const> {pointers}, schema}
      };
    EXPECT_EQ (parsed.at ("name"), "value");
  }

  TYPED_TEST (ArgumentsStorageTest, parses_named_values_flags_and_repetition)
  {
    using mcs::util::command_line::option::Flag;
    using mcs::util::command_line::option::Parameter;
    using Multiplicity = Parameter::Multiplicity;
    using Presence = Parameter::Presence;

    auto const pointers
      { std::array
        { "--flag", "--name=value=more]]", "--empty", "", "--flag"
        , "--alignment=file 4096", "--alignment", "other 64"
        }
      };
    auto const arguments
      { TypeParam {arguments::View {pointers.data(), pointers.size()}}
      };
    auto const parsed
      { arguments.parse
        ( command_line::Schema{}
        . add (Flag {"flag"})
        . add (Parameter {"name"})
        . add (Parameter {"empty"})
        . add
          ( Parameter
              { "alignment"
              , Presence::Required{}
              , Multiplicity::Repeatable{}
              }
          )
        )
      };

    EXPECT_TRUE (parsed.flag ("flag"));
    EXPECT_FALSE (parsed.flag ("absent"));
    EXPECT_EQ (parsed.at ("name"), "value=more]]");
    EXPECT_EQ (parsed.at ("empty"), "");
    EXPECT_FALSE (parsed.optional ("absent"));
    EXPECT_EQ
      ( std::addressof (parsed.occurrences ("alignment"))
      , std::addressof
        ( parsed.occurrences (std::string_view {"alignment-suffix", 9})
        )
      );
    EXPECT_EQ
      ( parsed.occurrences ("alignment")
      , (command_line::Parsed::Occurrences {{"file 4096"}, {"other 64"}})
      );
  }

  TYPED_TEST (ArgumentsStorageTest, observers_accept_bounded_string_views)
  {
    using mcs::util::command_line::option::Flag;
    using mcs::util::command_line::option::Parameter;
    using Multiplicity = Parameter::Multiplicity;
    using Presence = Parameter::Presence;

    auto const pointers
      { std::array
        {"--name=value", "--flag", "--repeated=one", "--repeated=two"}
      };
    auto const arguments
      { TypeParam {arguments::View {pointers.data(), pointers.size()}}
      };
    auto const parsed
      { arguments.parse
        ( command_line::Schema{}
        . add (Parameter {"name"})
        . add (Flag {"flag"})
        . add
          ( Parameter
              { "repeated"
              , Presence::Required{}
              , Multiplicity::Repeatable{}
              }
          )
        )
      };

    auto const name {std::string_view {"name-suffix", 4UL}};
    auto const flag {std::string_view {"flag-suffix", 4UL}};
    auto const repeated {std::string_view {"repeated-suffix", 8UL}};
    auto const missing {std::string_view {"missing-suffix", 7UL}};

    EXPECT_EQ (parsed.at (name), "value");
    EXPECT_EQ (parsed.optional (name), "value");
    EXPECT_TRUE (parsed.flag (flag));

    EXPECT_EQ
      ( parsed.occurrences (repeated)
      , (command_line::Parsed::Occurrences {{"one"}, {"two"}})
      );
    EXPECT_FALSE (parsed.optional (missing));
    EXPECT_FALSE (parsed.flag (missing));
    EXPECT_TRUE (parsed.occurrences (missing).empty());
    testing::require_exception
      ( [&]
        {
          std::ignore = parsed.at (missing);
        }
      , testing::assert_type_and_what<std::invalid_argument>
          ("option --missing must have exactly one value")
      );
    testing::require_exception
      ( [&]
        {
          std::ignore = parsed.flag (name);
        }
      , testing::assert_type_and_what<std::invalid_argument>
          ("option --name is not a flag")
      );
  }

  TYPED_TEST (ArgumentsStorageTest, at_requires_exactly_one_value)
  {
    using mcs::util::command_line::option::Flag;
    using mcs::util::command_line::option::Parameter;
    using Multiplicity = Parameter::Multiplicity;
    using Presence = Parameter::Presence;

    auto const pointers
      { std::array
        { "--flag", "--flag", "--unique=one", "--empty", ""
        , "--repeated=one", "--repeated=two"
        }
      };

    auto const arguments
      { TypeParam {arguments::View {pointers.data(), pointers.size()}}
      };
    auto const parsed
      { arguments.parse
        ( command_line::Schema{}
        . add (Flag {"flag"})
        . add (Parameter {"unique", Presence::Optional{}})
        . add (Parameter {"empty"})
        . add
          ( Parameter
              { "repeated"
              , Presence::Required{}
              , Multiplicity::Repeatable{}
              }
          )
        )
      };
    static_assert
      ( std::is_same_v<decltype (parsed.at ("unique"))
      , std::string_view>
      );

    EXPECT_EQ (parsed.at ("unique"), "one");
    EXPECT_EQ (parsed.at ("empty"), "");

    std::ranges::for_each
      ( std::array {"missing", "flag", "repeated"}
      , [&] (char const* name)
        {
          testing::require_exception
            ( [&]
              {
                std::ignore = parsed.at (name);
              }
            , testing::assert_type_and_what<std::invalid_argument>
                ( fmt::format
                  ( "option --{} must have exactly one value"
                  , name
                  )
                )
            );
        }
      );
  }

  TYPED_TEST (ArgumentsStorageTest, optional_parameters_may_be_absent)
  {
    using mcs::util::command_line::option::Flag;
    using mcs::util::command_line::option::Parameter;
    using Multiplicity = Parameter::Multiplicity;
    using Presence = Parameter::Presence;

    auto const schema
      { command_line::Schema{}
      . add (Flag {"flag"})
      . add (Parameter {"unique", Presence::Optional{}})
      . add
        ( Parameter
            { "repeated"
            , Presence::Optional{}
            , Multiplicity::Repeatable{}
            }
        )
      };

    auto const empty {TypeParam {arguments::View {nullptr, 0UL}}};
    auto const absent {empty.parse (schema)};

    EXPECT_FALSE (absent.flag ("flag"));
    EXPECT_FALSE (absent.optional ("unique"));
    EXPECT_TRUE (absent.occurrences ("repeated").empty());

    auto const pointers
      { std::array {"--unique=one", "--repeated", "two", "--repeated=three"}
      };
    auto const arguments
      { TypeParam {arguments::View {pointers.data(), pointers.size()}}
      };

    auto const present {arguments.parse (schema)};

    EXPECT_EQ (present.optional ("unique"), "one");
    EXPECT_EQ
      ( present.occurrences ("repeated")
      , (command_line::Parsed::Occurrences {{"two"}, {"three"}})
      );

    auto const duplicate_pointers
      { std::array {"--unique=one", "--unique=two"}
      };
    auto const duplicates
      { TypeParam
        { arguments::View
          { duplicate_pointers.data()
          , duplicate_pointers.size()
          }
        }
      };
    testing::require_exception
      ( [&]
        {
          auto const invalid {duplicates.parse (schema)};
        }
      , testing::assert_type_and_what<std::invalid_argument>
          ("duplicate option --unique")
      );
  }

  TYPED_TEST (ArgumentsStorageTest, required_repeatable_parameter_must_appear)
  {
    auto const arguments {TypeParam {arguments::View {nullptr, 0UL}}};

    testing::require_exception
      ( [&]
        {
          using mcs::util::command_line::option::Parameter;
          using Multiplicity = Parameter::Multiplicity;
          using Presence = Parameter::Presence;

          auto const parsed
            { arguments.parse
              ( command_line::Schema{}
              . add
                ( Parameter
                    { "repeated"
                    , Presence::Required{}
                    , Multiplicity::Repeatable{}
                    }
                )
              )
            };
        }
      , testing::assert_type_and_what<std::invalid_argument>
          ("missing required option --repeated")
      );
  }

  TYPED_TEST (ArgumentsStorageTest, rejects_invalid_option_definitions)
  {
    using command_line::option::Flag;
    using command_line::option::Parameter;
    using command_line::Schema;

    auto const arguments {TypeParam {arguments::View {nullptr, 0UL}}};

    std::ranges::for_each
      ( std::array
        { Schema{}.add (Flag {""})
        , Schema{}.add (Parameter {"--name"})
        , Schema{}.add (Parameter {"a=b"})
        , Schema{}.add (Flag {"two words"})
        , Schema{}.add (Parameter {"two\twords"})
        , Schema{}.add (Flag {"two\nwords"})
        , Schema{}.add (Parameter {"two\rwords"})
        , Schema{}.add (Flag {"two\fwords"})
        , Schema{}.add (Parameter {"two\vwords"})
        , Schema{}.add (Parameter {"[[FUSE"})
        , Schema{}.add (Flag {"same"}).add (Parameter {"same"})
        , Schema{}.add (Flag {"same"})
                  .add (Schema{}.add (Parameter {"same"}))
        }
      , [&] (Schema const& schema)
        {
          testing::require_exception
            ( [&]
              {
                std::ignore = arguments.parse (schema);
              }
            , testing::assert_type_and_what<std::logic_error>
                ("invalid named-option schema")
            );
        }
      );
  }

  TYPED_TEST (ArgumentsStorageTest, rejects_invalid_embedding_definitions)
  {
    auto const arguments {TypeParam {arguments::View {nullptr, 0UL}}};

    std::ranges::for_each
      ( std::vector<std::vector<std::string>>
        { {"", ""}
        , {"FUSE", "FUSE"}
        , {"two words"}
        , {"two\twords"}
        , {"two\nwords"}
        , {"two\rwords"}
        , {"two\fwords"}
        , {"two\vwords"}
        , {"[[FUSE"}
        , {"FUSE]]"}
        }
      , [&] (auto const& identifiers)
        {
          testing::require_exception
            ( [&]
              {
                auto schema {command_line::Schema{}};

                std::ranges::for_each
                  ( identifiers
                  , [&] (std::string const& identifier)
                    {
                      schema.embedding (identifier);
                    }
                  );

                std::ignore = arguments.parse (schema);
              }
            , testing::assert_type_and_what<std::logic_error>
                ("invalid embedding schema")
            );
        }
      );
  }

  TEST (Arguments, duplicate_markers_are_rejected_on_registration)
  {
    std::ranges::for_each
      ( std::array {"FUSE", ""}
      , [] (char const* marker)
        {
          auto schema {command_line::Schema{}.embedding (marker)};

          testing::require_exception
            ( [&]
              {
                schema.embedding (marker);
              }
            , testing::assert_type_and_what<std::logic_error>
                ("invalid embedding schema")
            );

          auto const empty {arguments::View {nullptr, 0UL}};
          auto const parsed {empty.parse (schema)};

          EXPECT_TRUE (parsed.embedded (marker).empty());
        }
      );
  }

  TEST (Arguments, composition_rejects_duplicate_markers)
  {
    std::ranges::for_each
      ( std::array {"FUSE", ""}
      , [] (char const* marker)
        {
          auto schema {command_line::Schema{}.embedding (marker)};

          auto const group
            { command_line::Schema{}
            . embedding ("OTHER")
            . embedding (marker)
            };

          testing::require_exception
            ( [&]
              {
                schema.add (group);
              }
            , testing::assert_type_and_what<std::logic_error>
                ("invalid embedding schema")
            );

          auto const empty {arguments::View {nullptr, 0UL}};
          auto const parsed {empty.parse (schema)};

          EXPECT_TRUE (parsed.embedded (marker).empty());

          testing::require_exception
            ( [&]
              {
                std::ignore = parsed.embedded ("OTHER");
              }
            , testing::assert_type_and_what<command_line::error::UnknownMarker>
                ("unknown embedding marker \"OTHER\"")
            );

          auto const group_parsed {empty.parse (group)};

          EXPECT_TRUE (group_parsed.embedded (marker).empty());
          EXPECT_TRUE (group_parsed.embedded ("OTHER").empty());
        }
      );
  }

  TYPED_TEST (ArgumentsStorageTest, unused_registered_markers_return_vectors)
  {
    auto const arguments {TypeParam {arguments::View {nullptr, 0UL}}};

    auto parsed
      { std::invoke
        ( [&]
          {
            return arguments.parse
              ( command_line::Schema{}
              . embedding ("FUSE")
              . embedding ("")
              . embedding ("UNUSED-marker-with-a-long-owning-name")
              );
          }
        )
      };

    auto const copied {parsed};
    auto const moved {std::move (parsed)};

    std::ranges::for_each
      ( std::array {std::addressof (copied), std::addressof (moved)}
      , [] (command_line::Parsed const* result)
        {
          auto const& fuse {result->embedded ("FUSE")};
          EXPECT_TRUE (fuse.empty());
          EXPECT_TRUE (result->embedded ("").empty());
          EXPECT_TRUE
            ( result->embedded ("UNUSED-marker-with-a-long-owning-name")
            . empty()
            );

          auto const bounded {std::string_view {"FUSE-suffix", 4}};
          EXPECT_EQ
            ( std::addressof (fuse)
            , std::addressof (result->embedded (bounded))
            );
          EXPECT_NE
            ( std::addressof (fuse)
            , std::addressof (result->embedded (""))
            );
        }
      );
  }

  TYPED_TEST (ArgumentsStorageTest, unknown_embedding_markers_throw)
  {
    auto const arguments {TypeParam {arguments::View {nullptr, 0UL}}};
    auto const parsed
      { arguments.parse (command_line::Schema{}.embedding ("FUSE"))
      };
    auto const& fuse {parsed.embedded ("FUSE")};

    std::ranges::for_each
      ( std::array
        { std::string_view {""}
        , std::string_view {"BEE"}
        , std::string_view {"BEE-suffix", 3}
        }
      , [&] (std::string_view marker)
        {
          testing::require_exception
            ( [&]
              {
                std::ignore = parsed.embedded (marker);
              }
            , testing::assert_type_and_what<command_line::error::UnknownMarker>
                (fmt::format ("unknown embedding marker \"{}\"", marker))
            );

          EXPECT_EQ
            ( std::addressof (fuse)
            , std::addressof (parsed.embedded ("FUSE"))
            );
        }
      );
  }

  TYPED_TEST (ArgumentsStorageTest, parses_anonymous_embeddings)
  {
    using mcs::util::command_line::option::Flag;

    auto const pointers
      { std::array
        { "--flag", "[[", "-f", "", "two words", "[[BEE", "--help"
        , "BEE]]", "]]"
        , "[[FUSE", "--help", "FUSE]]", "[[", "--mode=quiet", "]]"
        , "[[", "]]"
        }
      };
    auto const arguments
      { TypeParam {arguments::View {pointers.data(), pointers.size()}}
      };
    auto const schema
      { command_line::Schema{}
      . add (Flag {"flag"})
      . embedding ("")
      . embedding ("FUSE")
      };
    auto const parsed {arguments.parse (schema)};

    EXPECT_TRUE (parsed.flag ("flag"));
    EXPECT_EQ
      ( parsed.embedded ("")
      , ( std::vector<std::string_view>
          {"-f", "", "two words", "[[BEE", "--help", "BEE]]", "--mode=quiet"}
        )
      );
    EXPECT_EQ
      ( parsed.embedded ("FUSE")
      , (std::vector<std::string_view> {"--help"})
      );

    auto const empty_pointers {std::array {"[[", "]]"}};
    auto const empty_arguments
      { TypeParam
        { arguments::View {empty_pointers.data(), empty_pointers.size()}
        }
      };

    auto const empty_parsed {empty_arguments.parse (schema)};
    EXPECT_TRUE (empty_parsed.embedded ("").empty());
  }

  TYPED_TEST (ArgumentsStorageTest, embeddings_are_opaque_and_repeatable)
  {
    using mcs::util::command_line::option::Flag;
    using mcs::util::command_line::option::Parameter;

    auto const pointers
      { std::array
        { "--flag", "[[FUSE", "-f", "--help", "[[LURK", "--what"
        , "LURK", "]]", "", "two words", "FUSE]]", "--name=value"
        , "[[BEE", "--ctl=lurk", "2712", "BEE]]"
        , "[[FUSE", "-d", "FUSE]]", "[[EMPTY", "EMPTY]]"
        }
      };
    auto const arguments
      { TypeParam {arguments::View {pointers.data(), pointers.size()}}
      };
    auto const parsed
      { arguments.parse
        ( command_line::Schema{}
        . add (Flag {"flag"})
        . add (Parameter {"name"})
        . embedding ("FUSE")
        . embedding ("BEE")
        . embedding ("EMPTY")
        )
      };

    EXPECT_TRUE (parsed.flag ("flag"));
    EXPECT_EQ (parsed.at ("name"), "value");

    EXPECT_EQ
      ( parsed.embedded ("FUSE")
      , ( std::vector<std::string_view>
          { "-f", "--help", "[[LURK", "--what", "LURK", "]]", ""
          , "two words", "-d"
          }
        )
      );

    EXPECT_EQ
      ( parsed.embedded ("BEE")
      , (std::vector<std::string_view> {"--ctl=lurk", "2712"})
      );

    EXPECT_TRUE (parsed.embedded ("EMPTY").empty());
  }

  TYPED_TEST (ArgumentsStorageTest, searches_strictly_nested_embeddings)
  {
    auto const pointers
      { std::array
        { "program", "[[FUSE", "--inside", "", "[[BEE", "--nested"
        , "BEE]]", "--after-inner", "[[FUSE", "--same-id", "FUSE]]"
        , "[[--BEE", "--hyphen-id", "--BEE]]", "FUSE]]"
        , "[[", "--anonymous", "[[", "--anonymous-nested", "]]"
        , "[[BEE", "--named-in-anonymous", "BEE]]", "]]"
        , "--outside", "[[FUSE", "--repeated", "FUSE]]"
        , "[[FUSE", "[[", "--anonymous-in-named", "]]", "FUSE]]"
        , "[[EMPTY", "EMPTY]]", "--ending=]]"
        }
      };
    auto const arguments
      { TypeParam {arguments::View {pointers.data(), pointers.size()}}
      };

    EXPECT_TRUE (arguments.contains_any_of ({"--outside"}));
    EXPECT_TRUE (arguments.contains_any_of ({"--inside", "--outside"}));
    EXPECT_TRUE (arguments.contains_any_of ({"--ending=]]"}));
    EXPECT_FALSE
      ( arguments.contains_any_of
        ( { "--inside", "--nested", "--after-inner", "--same-id"
          , "--repeated", "--hyphen-id", ""
          , "--anonymous", "--anonymous-nested", "--named-in-anonymous"
          , "--anonymous-in-named"
          }
        )
      );
    EXPECT_FALSE
      ( arguments.contains_any_of ({"[[FUSE", "FUSE]]", "[[EMPTY", "EMPTY]]"})
      );
    EXPECT_FALSE (arguments.contains_any_of ({}));
    EXPECT_FALSE (arguments.contains_any_of ({"[[", "]]"}));
  }

  TYPED_TEST (ArgumentsStorageTest, search_stops_at_first_match)
  {
    auto const pointers
      { std::array {"--match", "[[FUSE", "[[BEE", "FUSE]]"}
      };
    auto const arguments
      { TypeParam {arguments::View {pointers.data(), pointers.size()}}
      };

    EXPECT_TRUE (arguments.contains_any_of ({"--match"}));

    testing::require_exception
      ( [&]
        {
          std::ignore = arguments.contains_any_of ({"--missing"});
        }
      , testing::assert_type_and_what<std::invalid_argument>
          ("mismatched closing embedding tag 'FUSE]]'. expected 'BEE]]'")
      );

    testing::require_exception
      ( [&]
        {
          std::ignore = arguments.contains_any_of ({});
        }
      , testing::assert_type_and_what<std::invalid_argument>
          ("mismatched closing embedding tag 'FUSE]]'. expected 'BEE]]'")
      );
  }

  TYPED_TEST (ArgumentsStorageTest, rejects_invalid_named_arguments)
  {
    struct Case
    {
      std::vector<char const*> arguments;
      std::string message;
    };

    std::ranges::for_each
      ( std::vector<Case>
        { Case
          { .arguments = {"--name="}
          , .message = "missing option value for --name"
          }
        , Case
          { .arguments = {"--name"}
          , .message = "missing option value for --name"
          }
        , Case
          { .arguments = {"--name", "--flag"}
          , .message = "missing option value for --name"
          }
        , Case
          { .arguments = {"--flag=yes"}
          , .message = "flag --flag does not accept a value"
          }
        , Case
          { .arguments = {"--name=a", "--name", "b"}
          , .message = "duplicate option --name"
          }
        , Case
          { .arguments = {"--name", "file", "4096"}
          , .message = "unexpected positional argument: 4096"
          }
        , Case
          { .arguments = {"--flag="}
          , .message = "flag --flag does not accept a value"
          }
        , Case
          { .arguments = {"--unknown=x"}
          , .message = "unknown option --unknown"
          }
        , Case
          { .arguments = {"positional"}
          , .message = "unexpected positional argument: positional"
          }
        , Case
          { .arguments = {"--"}
          , .message = "unknown option --"
          }
        , Case
          { .arguments = {"[[FUSE", "-f"}
          , .message = "missing closing embedding tag 'FUSE]]'"
          }
        , Case
          { .arguments = {"FUSE]]"}
          , .message = "unmatched closing embedding tag 'FUSE]]'"
          }
        , Case
          { .arguments = {"[[BEE", "BEE]]"}
          , .message = "unknown embedding BEE"
          }
        , Case
          { .arguments = {"[[", "]]"}
          , .message = "unknown embedding \"\""
          }
        , Case
          { .arguments = {"[["}
          , .message = "missing closing embedding tag ']]'"
          }
        , Case
          { .arguments = {"]]"}
          , .message = "unmatched closing embedding tag ']]'"
          }
        , Case
          { .arguments = {"--name", "[[FUSE", "FUSE]]", "x"}
          , .message = "missing option value for --name"
          }
        , Case
          { .arguments = {}
          , .message = "missing required option --name"
          }
        }
      , [] (Case const& test)
        {
          auto const arguments
            { TypeParam
              { arguments::View {test.arguments.data(), test.arguments.size()}
              }
            };

          testing::require_exception
            ( [&]
              {
                using mcs::util::command_line::option::Flag;
                using mcs::util::command_line::option::Parameter;

                auto const parsed
                  { arguments.parse
                    ( command_line::Schema{}
                    . add (Parameter {"name"})
                    . add (Flag {"flag"})
                    . embedding ("FUSE")
                    )
                  };
              }
            , testing::assert_type_and_what<std::invalid_argument>
                (test.message)
            );
        }
      );
  }

  TYPED_TEST (ArgumentsStorageTest, parsed_results_borrow_the_input_strings)
  {
    auto const name {std::string {"--name=value"}};
    auto const separate {std::string {"separate value"}};
    auto const repeated {std::string {"--repeated=again"}};
    auto const opening {std::string {"[[FUSE"}};
    auto const embedded {std::string {"embedded value"}};
    auto const anonymous {std::string {"[["}};
    auto const empty {std::string{}};

    auto parsed
      { std::invoke
        ( [&]
          {
            using mcs::util::command_line::option::Parameter;
            using Multiplicity = Parameter::Multiplicity;
            using Presence = Parameter::Presence;

            auto const pointers
              { std::array
                { name.c_str(), "--separate", separate.c_str()
                , repeated.c_str(), opening.c_str(), embedded.c_str()
                , "FUSE]]", anonymous.c_str(), empty.c_str(), "]]"
                }
              };

            auto const arguments
              { TypeParam
                { arguments::View {pointers.data(), pointers.size()}
                }
              };

            return arguments.parse
              ( command_line::Schema{}
              . add (Parameter {"name"})
              . add (Parameter {"separate"})
              . add ( Parameter
                      { "repeated"
                      , Presence::Required{}
                      , Multiplicity::Repeatable{}
                      }
                    )
              . embedding ("FUSE")
              . embedding ("")
              );
          }
        )
      };

    auto const copied {parsed};
    auto const moved {std::move (parsed)};

    std::ranges::for_each
      ( std::array {std::addressof (copied), std::addressof (moved)}
      , [&] (command_line::Parsed const* result)
        {
          auto const value {result->at ("name")};
          EXPECT_EQ (value, "value");
          EXPECT_EQ (value.data(), name.data() + 7);
          EXPECT_EQ (value.data()[value.size()], '\0');

          auto const optional {result->optional ("separate")};
          ASSERT_TRUE (optional);
          EXPECT_EQ (optional->data(), separate.data());
          EXPECT_EQ
            ( result->occurrences ("repeated").front().front().data()
            , repeated.data() + 11
            );
          EXPECT_EQ
            ( result->embedded ("FUSE").front().data()
            , embedded.data()
            );
          EXPECT_EQ (result->embedded ("").front().data(), empty.data());
        }
      );
  }

  TEST (Arguments, main_does_not_interpret_embedded_help)
  {
    auto pointers {std::array {"program", "[[FUSE", "--help", "FUSE]]"}};
    auto called {false};

    EXPECT_EQ
      ( util::main
        ( util::cast<int> (pointers.size())
        , pointers.data()
        , [] (arguments::View) noexcept
          {
            ADD_FAILURE() << "embedded help reached MCS help";
          }
        , [&] (arguments::View) noexcept
          {
            called = true;

            return EXIT_SUCCESS;
          }
        )
      , EXIT_SUCCESS
      );

    EXPECT_TRUE (called);
  }

  TEST (Arguments, supports_constexpr_queries)
  {
    static constexpr auto pointers
      { std::array<char const*, 3> {"program", "-f", "value"}
      };
    constexpr auto arguments
      { arguments::View {pointers.data(), pointers.size()}
      };

    static_assert (arguments.size() == 3UL);
    static_assert (arguments.at (0UL) == pointers.at (0UL));
    static_assert (arguments.from (1UL).size() == 2UL);
    static_assert (arguments.from (3UL).size() == 0UL);
  }

  TYPED_TEST (ArgumentsStorageTest, help_respects_embedding_boundaries)
  {
    struct Case
    {
      std::vector<char const*> arguments;
      bool expected {false};
    };

    std::ranges::for_each
      ( std::vector<Case>
        { Case {.arguments = {}, .expected = false}
        , Case
          { .arguments = {"[[", "--help", "-h", "]]"}
          , .expected = false
          }
        , Case
          { .arguments = {"[[", "--help", "]]", "-h"}
          , .expected = true
          }
        , Case
          { .arguments = {"[[FUSE", "[[", "--help", "]]", "FUSE]]"}
          , .expected = false
          }
        , Case
          { .arguments = {"--help=false", "--helpful", "-help"}
          , .expected = false
          }
        , Case
          { .arguments =
              { "[[FUSE", "--help", "-h", "[[LURK", "--help", "LURK]]"
              , "FUSE]]", "[[BEE", "-h", "BEE]]"
              , "[[FUSE", "--help", "FUSE]]"
              }
          , .expected = false
          }
        , Case
          { .arguments = {"-h", "[[FUSE", "--help", "FUSE]]"}
          , .expected = true
          }
        , Case
          { .arguments = {"[[FUSE", "-h", "FUSE]]", "--help"}
          , .expected = true
          }
        , Case
          { .arguments =
              {"[[FUSE", "FUSE]]", "-h", "[[BEE", "--help", "BEE]]"}
          , .expected = true
          }
        , Case
          { .arguments = {"--help", "[[FUSE", "-h"}
          , .expected = true
          }
        , Case
          { .arguments = {"-h", "FUSE]]"}
          , .expected = true
          }
        , Case
          { .arguments = {"--help", "[["}
          , .expected = true
          }
        , Case
          { .arguments = {"--help", "[[FUSE", "[[BEE", "FUSE]]"}
          , .expected = true
          }
        }
      , [] (Case const& test)
        {
          auto const arguments
            { TypeParam
              { arguments::View {test.arguments.data(), test.arguments.size()}
              }
            };

          EXPECT_EQ (arguments.help_requested(), test.expected);
          EXPECT_EQ
            ( arguments.contains_any_of ({"-h", "--help"})
            , test.expected
            );
        }
      );
  }

  TYPED_TEST (ArgumentsStorageTest, unclosed_embeddings_do_not_match)
  {
    std::ranges::for_each
      ( std::vector<std::vector<char const*>>
        { {"[[FUSE"}
        , {"[[FUSE", "-h"}
        , {"[[FUSE", "--help"}
        , {"[[FUSE", "[[BEE", "--help"}
        , {"[[FUSE", "[[BEE", "BEE]]", "--help"}
        , {"[["}
        , {"[[", "--help"}
        , {"[[", "[[FUSE", "-h"}
        }
      , [] (auto const& pointers)
        {
          auto const arguments
            { TypeParam
              { arguments::View {pointers.data(), pointers.size()}
              }
            };

          EXPECT_FALSE (arguments.help_requested());
          EXPECT_FALSE (arguments.contains_any_of ({"-h", "--help"}));
          EXPECT_FALSE (arguments.contains_any_of ({}));
        }
      );
  }

  TYPED_TEST (ArgumentsStorageTest, help_rejects_malformed_embeddings)
  {
    static_assert
      ( !noexcept (std::declval<TypeParam const&>().help_requested())
      );
    static_assert
      ( !noexcept (std::declval<TypeParam const&>().contains_any_of ({}))
      );

    struct Case
    {
      std::vector<char const*> arguments;
      std::string message;
    };

    std::ranges::for_each
      ( std::vector<Case>
        { Case
          { .arguments = {"FUSE]]", "--help"}
          , .message = "unmatched closing embedding tag 'FUSE]]'"
          }
        , Case
          { .arguments = {"[[FUSE", "[[BEE", "FUSE]]", "--help"}
          , .message = "mismatched closing embedding tag 'FUSE]]'. expected 'BEE]]'"
          }
        , Case
          { .arguments = {"[[", "--help", "FUSE]]"}
          , .message = "mismatched closing embedding tag 'FUSE]]'. expected ']]'"
          }
        , Case
          { .arguments = {"[[FUSE", "]]", "--help"}
          , .message = "mismatched closing embedding tag ']]'. expected 'FUSE]]'"
          }
        , Case
          { .arguments = {"]]", "--help"}
          , .message = "unmatched closing embedding tag ']]'"
          }
        , Case
          { .arguments = {"[[FUSE", "--help", "BEE]]"}
          , .message = "mismatched closing embedding tag 'BEE]]'. expected 'FUSE]]'"
          }
        }
      , [] (Case const& test)
        {
          auto const arguments
            { TypeParam
              { arguments::View {test.arguments.data(), test.arguments.size()}
              }
            };
          testing::require_exception
            ( [&]
              {
                std::ignore = arguments.help_requested();
              }
            , testing::assert_type_and_what<std::invalid_argument>
                (test.message)
            );
          testing::require_exception
            ( [&]
              {
                std::ignore = arguments.contains_any_of ({});
              }
            , testing::assert_type_and_what<std::invalid_argument>
                (test.message)
            );
        }
      );
  }

  TYPED_TEST (ArgumentsStorageTest, detects_top_level_help_anywhere)
  {
    auto const strings
      { std::invoke
        ( [&]
          {
            auto const number_of_non_help_arguments
              { std::invoke (testing::random::value<std::size_t> {0, 10})
              };

            auto _strings {std::vector<std::string>{}};
            _strings.reserve (number_of_non_help_arguments);

            using RandomString = testing::random::value<std::string>;

            std::ranges::generate_n
              ( std::back_inserter (_strings)
              , util::cast<long> (number_of_non_help_arguments)
              , RandomString
                { RandomString::Length {.min = 1, .max = 32}
                , 'a', 'z'
                }
              );

            return _strings;
          }
        )
      };

    auto const pointers
      { std::invoke
        ( [&]
          {
            auto _pointers {std::vector<char const*>{}};
            _pointers.reserve (strings.size());

            std::ranges::transform
              ( strings
              , std::back_inserter (_pointers)
              , [] (std::string const& argument) noexcept
                {
                  return argument.c_str();
                }
              );

            return _pointers;
          }
        )
      };

    {
      auto const arguments_without_help_request
        { TypeParam {arguments::View {pointers.data(), pointers.size()}}
        };

      EXPECT_FALSE (arguments_without_help_request.help_requested());
    }

    for (auto const* help_flag : {"-h", "--help"})
    {
      std::ranges::for_each
        ( std::views::iota (std::size_t {0}, pointers.size() + 1)
        , [&] (auto position)
          {
            auto with_help {std::vector<char const*>{}};
            with_help.reserve (pointers.size() + 1);

            std::ranges::copy
              ( pointers | std::views::take (position)
              , std::back_inserter (with_help)
              );
            with_help.push_back (help_flag);
            std::ranges::copy
              ( pointers | std::views::drop (position)
              , std::back_inserter (with_help)
              );

            auto const arguments
              { TypeParam
                 { arguments::View {with_help.data(), with_help.size()}
                 }
              };

            EXPECT_EQ (arguments.size(), pointers.size() + 1);
            EXPECT_TRUE (arguments.help_requested());
          }
        );
    }
  }

  TEST (Arguments, accepts_mutable_and_const_pointer_arrays)
  {
    auto text {std::array {'v', 'a', 'l', 'u', 'e', '\0'}};
    auto mutable_pointers {std::array {text.data()}};
    auto const const_pointers {std::array<char const*, 1> {text.data()}};
    auto const mutable_arguments
      { arguments::View {mutable_pointers.data(), mutable_pointers.size()}
      };
    auto const const_arguments
      { arguments::View {const_pointers.data(), const_pointers.size()}
      };

    EXPECT_EQ (mutable_arguments.at (0UL), text.data());
    EXPECT_EQ (const_arguments.at (0UL), text.data());

    static_assert ( std::is_same_v
                      < std::ranges::range_reference_t<arguments::View>
                      , char const* const&
                      >
                  );
    static_assert ( std::is_same_v<decltype (mutable_arguments.at (0UL))
                  , char const*>
                  );
  }

  TYPED_TEST
    ( ArgumentsStorageTest
    , iterates_and_copies_without_copying_strings
    )
  {
    auto const pointers
      { std::array<char const*, 3> {"program", "-f", "value"}
      };
    auto const arguments
      { TypeParam {arguments::View {pointers.data(), pointers.size()}}
      };
    auto const copied
      { std::vector<char const*> {std::begin (arguments), std::end (arguments)}
      };

    static_assert
      ( std::is_same_v< std::ranges::range_reference_t<TypeParam>
                      , char const* const&
                      >
      );
    static_assert
      ( std::is_same_v< std::ranges::range_reference_t<TypeParam const>
                      , char const* const&
                      >
      );
    static_assert (std::ranges::contiguous_range<TypeParam>);
    static_assert (std::is_same_v<decltype (arguments.at (0UL)), char const*>);

    EXPECT_TRUE (std::ranges::equal (arguments, pointers));
    EXPECT_TRUE (std::ranges::equal (copied, pointers));
  }

  TYPED_TEST
    ( ArgumentsStorageTest
    , from_is_inclusive_and_preserves_the_original
    )
  {
    auto const pointers
      { std::array<char const*, 3> {"program", "command", "value"}
      };
    auto const arguments
      { TypeParam {arguments::View {pointers.data(), pointers.size()}}
      };
    auto const remaining {arguments.from (1UL)};
    auto const copied {remaining};

    static_assert ( std::is_same_v< decltype (remaining)
                                  , arguments::View const
                                  >
                  );
    EXPECT_EQ
      ( std::addressof (*std::begin (remaining))
      , std::addressof (*(std::begin (arguments) + 1))
      );
    EXPECT_EQ (remaining.size(), 2UL);
    EXPECT_EQ (remaining.at (0UL), pointers.at (1UL));
    EXPECT_EQ (copied.at (1UL), pointers.at (2UL));
    EXPECT_EQ (remaining.from (1UL).at (0UL), pointers.at (2UL));
    EXPECT_EQ (arguments.from (0UL).size(), arguments.size());
    EXPECT_EQ (arguments.size(), 3UL);
    EXPECT_EQ (arguments.at (0UL), pointers.at (0UL));
    EXPECT_EQ (arguments.from (arguments.size()).size(), 0UL);
  }

  TYPED_TEST (ArgumentsStorageTest, accepts_empty_views)
  {
    auto const arguments {TypeParam {arguments::View {nullptr, 0UL}}};
    auto const remaining {arguments.from (0UL)};

    EXPECT_EQ (arguments.size(), 0UL);
    EXPECT_EQ (std::begin (arguments), std::end (arguments));
    EXPECT_EQ (std::begin (remaining), std::end (remaining));
    EXPECT_FALSE (arguments.contains_any_of ({"--help"}));
    EXPECT_FALSE (arguments.contains_any_of ({}));
  }

  TYPED_TEST
    ( ArgumentsStorageTest
    , searches_contents_exactly_and_case_sensitively
    )
  {
    auto const option {std::string {"--help"}};
    auto const pointers
      { std::array<char const*, 3> {"program", option.c_str(), ""}
      };
    auto const arguments
      { TypeParam {arguments::View {pointers.data(), pointers.size()}}
      };

    EXPECT_TRUE (arguments.contains_any_of ({"--help"}));
    EXPECT_TRUE (arguments.contains_any_of ({"-h", "--help"}));
    EXPECT_TRUE (arguments.contains_any_of ({"--help", "--help"}));
    EXPECT_TRUE (arguments.contains_any_of ({""}));
    EXPECT_TRUE (arguments.contains_any_of ({"program"}));
    EXPECT_FALSE (arguments.from (1UL).contains_any_of ({"program"}));
    EXPECT_FALSE (arguments.contains_any_of ({"--Help"}));
    EXPECT_FALSE (arguments.contains_any_of ({"--hel"}));
    EXPECT_FALSE (arguments.contains_any_of ({"--help=value"}));
    EXPECT_FALSE (arguments.contains_any_of ({"-f", "-d"}));
    EXPECT_FALSE (arguments.contains_any_of ({}));
  }

  TYPED_TEST (ArgumentsStorageTest, at_reports_invalid_index_and_count)
  {
    auto const pointers {std::array<char const*, 1> {"program"}};
    auto const arguments
      { TypeParam {arguments::View {pointers.data(), pointers.size()}}
      };

    testing::require_exception
      ( [&]
        {
          std::ignore = arguments.at (1UL);
        }
      , testing::assert_type_and_what<std::out_of_range>
          ("Arguments::at: index 1 out of range for 1 arguments")
      );

    auto const empty {arguments.from (1UL)};

    testing::require_exception
      ( [&]
        {
          std::ignore = empty.at (0UL);
        }
      , testing::assert_type_and_what<std::out_of_range>
          ("Arguments::at: index 0 out of range for 0 arguments")
      );
  }

  TYPED_TEST (ArgumentsStorageTest, from_reports_invalid_offset_and_count)
  {
    auto const pointers {std::array<char const*, 1> {"program"}};
    auto const arguments
      { TypeParam {arguments::View {pointers.data(), pointers.size()}}
      };

    testing::require_exception
      ( [&]
        {
          std::ignore = arguments.from (2UL);
        }
      , testing::assert_type_and_what<std::out_of_range>
          ("Arguments::from: offset 2 out of range for 1 arguments")
      );

    auto const empty {TypeParam {arguments::View {nullptr, 0UL}}};

    testing::require_exception
      ( [&]
        {
          std::ignore = empty.from (1UL);
        }
      , testing::assert_type_and_what<std::out_of_range>
          ("Arguments::from: offset 1 out of range for 0 arguments")
      );
  }

  TYPED_TEST (ArgumentsStorageTest, rejects_maximum_index_and_offset)
  {
    auto const arguments {TypeParam {arguments::View {nullptr, 0UL}}};
    auto const maximum {std::numeric_limits<std::size_t>::max()};

    testing::require_exception
      ( [&]
        {
          std::ignore = arguments.at (maximum);
        }
      , testing::assert_type_and_what<std::out_of_range>
          ( fmt::format
            ( "Arguments::at: index {} out of range for 0 arguments"
            , maximum
            )
          )
      );

    testing::require_exception
      ( [&]
        {
          std::ignore = arguments.from (maximum);
        }
      , testing::assert_type_and_what<std::out_of_range>
          ( fmt::format
            ( "Arguments::from: offset {} out of range for 0 arguments"
            , maximum
            )
          )
      );
  }

  TEST (OwnedArguments, copies_pointers_but_borrows_strings)
  {
    auto text {std::array {'v', 'a', 'l', 'u', 'e', '\0'}};
    auto pointers {std::array<char const*, 1> {text.data()}};
    auto const owned
      { arguments::Owned {arguments::View {pointers.data(), pointers.size()}}
      };

    pointers.at (0UL) = "replacement";
    text.at (0UL) = 'V';

    EXPECT_EQ (owned.at (0UL), text.data());
    EXPECT_STREQ (owned.at (0UL), "Value");
    EXPECT_STREQ (pointers.at (0UL), "replacement");
  }

  TEST (OwnedArguments, outlives_the_source_pointer_array)
  {
    auto const owned
      { []
        {
          auto const pointers
            { std::array<char const*, 2> {"program", "input"}
            };

          return arguments::Owned
            { arguments::View {pointers.data(), pointers.size()}
            };
        }()
      };

    EXPECT_EQ (owned.size(), 2UL);
    EXPECT_STREQ (owned.at (0UL), "program");
    EXPECT_STREQ (owned.at (1UL), "input");
  }

  TEST (OwnedArguments, copies_and_moves_pointer_storage)
  {
    static_assert (!std::is_default_constructible_v<arguments::Owned>);
    static_assert (std::is_copy_assignable_v<arguments::Owned>);
    static_assert (std::is_move_assignable_v<arguments::Owned>);

    auto const pointers
      { std::array<char const*, 2> {"program", "input"}
      };
    auto original
      { arguments::Owned {arguments::View {pointers.data(), pointers.size()}}
      };
    auto copied {original};

    EXPECT_NE
      ( std::addressof (*std::begin (original))
      , std::addressof (*std::begin (copied))
      );
    original += "extra";
    copied -= "input";

    EXPECT_EQ (original.size(), 3UL);
    ASSERT_EQ (copied.size(), 1UL);
    EXPECT_EQ (copied.at (0UL), pointers.at (0UL));

    copied = original;
    original -= "extra";

    auto moved {std::move (copied)};
    EXPECT_EQ (moved.size(), 3UL);
    EXPECT_STREQ (moved.at (2UL), "extra");

    original = std::move (moved);
    EXPECT_EQ (original.size(), 3UL);
    EXPECT_STREQ (original.at (2UL), "extra");
  }

  TEST (OwnedArguments, view_borrows_owned_pointer_storage)
  {
    auto const pointers
      { std::array<char const*, 2> {"program", "input"}
      };
    auto const owned
      { arguments::Owned {arguments::View {pointers.data(), pointers.size()}}
      };
    auto const view {owned.view()};

    static_assert (std::is_same_v<decltype (view), arguments::View const>);

    EXPECT_TRUE (std::ranges::equal (view, pointers));
    EXPECT_EQ
      ( std::addressof (*std::begin (view))
      , std::addressof (*std::begin (owned))
      );
  }

  TEST (OwnedArguments, appends_borrowed_strings_and_returns_self)
  {
    auto owned {arguments::Owned {arguments::View {nullptr, 0UL}}};
    auto text {std::array {'v', 'a', 'l', 'u', 'e', '\0'}};

    EXPECT_EQ (std::addressof (owned += text.data()), std::addressof (owned));
    owned += "";
    owned += text.data();
    text.at (0UL) = 'V';

    ASSERT_EQ (owned.size(), 3UL);
    EXPECT_EQ (owned.at (0UL), text.data());
    EXPECT_STREQ (owned.at (0UL), "Value");
    EXPECT_STREQ (owned.at (1UL), "");
    EXPECT_EQ (owned.at (2UL), text.data());
  }

  TEST (OwnedArguments, removes_all_content_matches_and_preserves_order)
  {
    auto const option {std::string {"--remove"}};
    auto const pointers
      { std::array<char const*, 8>
        { "program"
        , option.c_str()
        , "input"
        , "--remove"
        , "--remove=value"
        , "--Remove"
        , ""
        , "--remove"
        }
      };
    auto owned
      { arguments::Owned {arguments::View {pointers.data(), pointers.size()}}
      };
    auto const expected
      { std::array<char const*, 5>
        { pointers.at (0UL)
        , pointers.at (2UL)
        , pointers.at (4UL)
        , pointers.at (5UL)
        , pointers.at (6UL)
        }
      };

    EXPECT_EQ (std::addressof (owned -= "--remove"), std::addressof (owned));
    EXPECT_TRUE (std::ranges::equal (owned, expected));

    owned -= "--missing";

    EXPECT_TRUE (std::ranges::equal (owned, expected));

    owned -= "";

    EXPECT_EQ (owned.size(), 4UL);
    EXPECT_FALSE (owned.contains_any_of ({""}));
    EXPECT_EQ (pointers.at (1UL), option.c_str());
  }

  TEST (OwnedArguments, removes_only_top_level_matches)
  {
    auto const pointers
      { std::array
        { "--remove", "program", "[[FUSE", "--remove", ""
        , "[[", "--remove", "]]"
        , "[[BEE", "--remove", "BEE]]", "[[FUSE", "--remove", "FUSE]]"
        , "FUSE]]", "--remove", "[[FUSE", "--remove", "FUSE]]"
        , "--remove", ""
        , "[[", "--remove", "[[BEE", "--remove", "BEE]]"
        , "[[", "--remove", "]]", "]]"
        }
      };

    auto owned
      { arguments::Owned {arguments::View {pointers.data(), pointers.size()}}
      };
    auto const expected
      { std::array
        { "program", "[[FUSE", "--remove", "", "[[", "--remove", "]]"
        , "[[BEE", "--remove"
        , "BEE]]", "[[FUSE", "--remove", "FUSE]]", "FUSE]]"
        , "[[FUSE", "--remove", "FUSE]]"
        , "[[", "--remove", "[[BEE", "--remove", "BEE]]"
        , "[[", "--remove", "]]", "]]"
        }
      };

    EXPECT_EQ (std::addressof (owned -= "--remove"), std::addressof (owned));
    owned -= "";
    owned -= "[[FUSE";
    owned -= "FUSE]]";
    owned -= "[[";
    owned -= "]]";

    EXPECT_TRUE
      ( std::ranges::equal
        ( owned
        , expected
        , [] (std::string_view left, std::string_view right) noexcept
          {
            return left == right;
          }
        )
      );
  }

  TEST (OwnedArguments, removal_rejects_invalid_markers)
  {
    struct Case
    {
      std::vector<char const*> arguments;
      std::string message;
    };

    std::ranges::for_each
      ( std::vector<Case>
        { Case
          { .arguments = {"--remove", "[[", "FUSE]]"}
          , .message = "mismatched closing embedding tag 'FUSE]]'. expected ']]'"
          }
        , Case
          { .arguments = {"--remove", "]]"}
          , .message = "unmatched closing embedding tag ']]'"
          }
        , Case
          { .arguments = {"--remove", "FUSE]]"}
          , .message = "unmatched closing embedding tag 'FUSE]]'"
          }
        , Case
          { .arguments = {"--remove", "[[FUSE", "BEE]]"}
          , .message = "mismatched closing embedding tag 'BEE]]'. expected 'FUSE]]'"
          }
        , Case
          { .arguments = {"--remove", "[[FUSE", "[[BEE", "FUSE]]"}
          , .message = "mismatched closing embedding tag 'FUSE]]'. expected 'BEE]]'"
          }
        }
      , [] (Case const& test)
        {
          auto owned
            { arguments::Owned
              { arguments::View {test.arguments.data(), test.arguments.size()}
              }
            };
          testing::require_exception
            ( [&]
              {
                owned -= "--remove";
              }
            , testing::assert_type_and_what<std::invalid_argument>
                (test.message)
            );
        }
      );
  }

  TEST (OwnedArguments, unclosed_embeddings_preserve_embedded_matches)
  {
    auto const pointers
      { std::array
        { "--remove", "program", "--remove", "[[FUSE", "--remove"
        , "[[BEE", "--remove", "[[", "--remove"
        }
      };
    auto owned
      { arguments::Owned {arguments::View {pointers.data(), pointers.size()}}
      };
    auto const expected
      { std::array
        { "program", "[[FUSE", "--remove", "[[BEE", "--remove"
        , "[[", "--remove"
        }
      };

    owned -= "--remove";

    EXPECT_TRUE
      ( std::ranges::equal
        ( owned
        , expected
        , [] (std::string_view left, std::string_view right) noexcept
          {
            return left == right;
          }
        )
      );
  }

  TEST (OwnedArguments, removes_every_argument_and_accepts_empty_input)
  {
    auto const pointers
      { std::array<char const*, 2> {"remove", "remove"}
      };
    auto owned
      { arguments::Owned {arguments::View {pointers.data(), pointers.size()}}
      };

    owned -= "remove";

    EXPECT_EQ (owned.size(), 0UL);
    EXPECT_EQ (std::begin (owned), std::end (owned));
    EXPECT_EQ (owned.view().size(), 0UL);
    EXPECT_EQ (owned.from (0UL).size(), 0UL);

    owned -= "remove";

    EXPECT_EQ (owned.size(), 0UL);

    owned += "new";

    EXPECT_STREQ (owned.at (0UL), "new");
  }

  TEST (OwnedArguments, rejects_null_operator_arguments_without_mutation)
  {
    auto const pointers {std::array<char const*, 1> {"program"}};
    auto owned
      { arguments::Owned {arguments::View {pointers.data(), pointers.size()}}
      };

    testing::require_exception
      ( [&]
        {
          owned += nullptr;
        }
      , testing::assert_type_and_what
          < util::not_null<char const>::Error::MustNotBeNull
          > ("not_null: must not be null")
      );

    testing::require_exception
      ( [&]
        {
          owned -= nullptr;
        }
      , testing::assert_type_and_what
          < util::not_null<char const>::Error::MustNotBeNull
          > ("not_null: must not be null")
      );

    EXPECT_TRUE (std::ranges::equal (owned, pointers));
  }

  TEST (Arguments, main_forwards_the_complete_argument_view)
  {
    auto pointers {std::array<char const*, 3> {"program", "-f", "value"}};

    auto const result
      { util::main
        ( 3
        , pointers.data()
        , [] (arguments::View) noexcept {}
        , [&] (arguments::View arguments) noexcept -> int
          {
            EXPECT_TRUE (std::ranges::equal (arguments, pointers));

            return EXIT_SUCCESS;
          }
        )
      };

    EXPECT_EQ (result, EXIT_SUCCESS);
  }

  TEST (Arguments, main_invokes_help_before_the_main_implementation)
  {
    auto pointers
      { std::array<char const*, 4> {"program", "--malformed", "value", "-h"}
      };
    auto help_called {false};
    auto main_called {false};

    auto const result
      { util::main
        ( util::cast<int> (pointers.size())
        , pointers.data()
        , [&] (arguments::View arguments) noexcept
          {
            help_called = true;

            EXPECT_TRUE (arguments.help_requested());
            EXPECT_EQ (arguments.size(), pointers.size());
          }
        , [&] (arguments::View) noexcept -> int
          {
            main_called = true;

            return EXIT_FAILURE;
          }
        )
      };

    EXPECT_EQ (result, EXIT_SUCCESS);
    EXPECT_TRUE (help_called);
    EXPECT_FALSE (main_called);
  }

  TEST (Arguments, main_reports_help_callback_exceptions)
  {
    auto pointers {std::array<char const*, 2> {"program", "--help"}};
    auto main_called {false};

    auto const result
      { util::main
        ( util::cast<int> (pointers.size())
        , pointers.data()
        , [] (arguments::View)
          {
            throw std::runtime_error {"help callback failed"};
          }
        , [&] (arguments::View) noexcept -> int
          {
            main_called = true;

            return EXIT_SUCCESS;
          }
        )
      };

    EXPECT_EQ (result, EXIT_FAILURE);
    EXPECT_FALSE (main_called);
  }
}
