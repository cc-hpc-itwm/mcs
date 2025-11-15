// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

namespace mcs::util::distribution::equal
{
  // Distribute M elements into P slots equally. The P ranges (p_{0},
  // p_{1}, ..., p_{P-1})
  //
  // ( p_{  0} = [begin (M, P,   0), end (M, P,   0))
  // , p_{  1} = [begin (M, P,   1), end (M, P,   1))
  // , ...
  // , p_{P-1} = [begin (M, P, P-1), end (M, P, P-1))
  // )
  //
  // are a partition of [0, M) such that the sizes of the ranges
  // differ pairwise by at most one.
  //
  // Also p_{i} and p_{i-1} are touching, that is end (p_{i}) == begin
  // (p_{i+1}).
  //
  // Also begin (p_{0}) == 0 and end (p_{P-1}) == M.
  //
  // Also begin (P) == M is well defined.
  //

  // Returns: The begin of range_{slot}.
  //
  // Pre: slot < P
  // Post: 0 <= begin (M, P, slot) < M
  //
  constexpr auto begin (auto M, auto P, auto slot);

  // Returns: The end of range_{slot}.
  //
  // Pre: slot < P
  // Post: 0 <= begin (M, P, slot) <= M
  //
  constexpr auto end (auto M, auto P, auto slot);

  // Returns: The size of range_{slot}.
  //
  // Pre: slot < P
  // Post: a, b < P ==> |size (M, P, a) - size (M, P, b)| <= 1
  //
  constexpr auto size (auto M, auto P, auto slot);
}

#include "detail/equal.ipp"
