// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_TTL_OUTPUTFORMAT_H_
#define OSM2TTL_TTL_OUTPUTFORMAT_H_

#include <string>
#include <unordered_map>

#include "osm2ttl/ttl/BlankNode.h"
#include "osm2ttl/ttl/IRI.h"
#include "osm2ttl/ttl/LangTag.h"
#include "osm2ttl/ttl/Literal.h"

namespace osm2ttl {
namespace ttl {

class OutputFormat {
 public:
  enum Value : uint8_t {
    TTL,
    NT
  };

  OutputFormat();
  OutputFormat(Value v); // NOLINT
  operator Value() const { return _value; }
  explicit operator bool() = delete;

  std::string header() const;
  std::string format(const osm2ttl::ttl::BlankNode& b) const;
  std::string format(const osm2ttl::ttl::IRI& i) const;
  std::string format(const osm2ttl::ttl::LangTag& l) const;
  std::string format(const osm2ttl::ttl::Literal& l) const;

 private:
  static std::string IRIREF(const std::string& p, const std::string& v);
  static std::string PrefixedName(const std::string& p, const std::string& v);
  static std::string STRING_LITERAL_QUOTE(const std::string& s);
  static std::string STRING_LITERAL_SINGLE_QUOTE(const std::string& s);
  static std::string ECHAR(char c);
  static uint8_t utf8Length(char c);
  static uint8_t utf8Length(const std::string& s);
  static uint32_t utf8Codepoint(const std::string& s);
  static std::string UCHAR(char c);
  static std::string UCHAR(const std::string& s);
  static std::string encodeIRIREF(const std::string& s);
  static std::string encodePN_LOCAL(const std::string& s);
  static std::string encodePERCENT(const std::string& s);
  void initPrefixes();
  std::unordered_map<std::string, std::string> _prefixes;
  Value _value;
};

}  // namespace ttl
}  // namespace osm2ttl

#endif  // OSM2TTL_TTL_OUTPUTFORMAT_H_
