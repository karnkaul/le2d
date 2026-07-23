#pragma once
#include "cards/catalog/catalog.hpp"
#include "djson/json.hpp"

namespace cards {
class Catalog::Parser {
  public:
	explicit Parser(Catalog& catalog) : m_catalog(catalog) {}

	void parse(dj::Json const& json);

  private:
	void parse_images(dj::Json const& json);
	void parse_suit(dj::Json const& json, Suit suit);
	void parse_fonts(dj::Json const& json);
	void parse_config(dj::Json const& json);

	Catalog& m_catalog;
};

inline void parse_catalog(Catalog& out, dj::Json const& json) { Catalog::Parser{out}.parse(json); }
} // namespace cards
