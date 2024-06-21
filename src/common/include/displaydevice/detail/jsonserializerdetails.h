#pragma once

#ifdef DD_JSON_DETAIL
  // system includes
  #include <nlohmann/json.hpp>

  // Special versions of the NLOHMANN definitions to remove the "m_" prefix in string form ('cause I like it that way ;P)
  #define DD_JSON_TO(v1) nlohmann_json_j[#v1] = nlohmann_json_t.m_##v1;
  #define DD_JSON_FROM(v1) nlohmann_json_j.at(#v1).get_to(nlohmann_json_t.m_##v1);

  // Coverage has trouble with inlined functions when they are included in different units,
  // therefore the usual macro was split into declaration and definition
  #define DD_JSON_DECLARE_SERIALIZE_TYPE(Type)                                  \
    void to_json(nlohmann::json &nlohmann_json_j, const Type &nlohmann_json_t); \
    void from_json(const nlohmann::json &nlohmann_json_j, Type &nlohmann_json_t);

  #define DD_JSON_DEFINE_SERIALIZE_STRUCT(Type, ...)                               \
    void to_json(nlohmann::json &nlohmann_json_j, const Type &nlohmann_json_t) {   \
      NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(DD_JSON_TO, __VA_ARGS__))           \
    }                                                                              \
                                                                                   \
    void from_json(const nlohmann::json &nlohmann_json_j, Type &nlohmann_json_t) { \
      NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(DD_JSON_FROM, __VA_ARGS__))         \
    }

  // Coverage has trouble with getEnumMap() function since it has a lot of "fallthrough"
  // branches when creating a map, therefore the macro has baked in pattern to disable branch coverage
  // in GCOVR
  #define DD_JSON_DEFINE_SERIALIZE_ENUM_GCOVR_EXCL_BR_LINE(Type, ...)                                                                                                   \
    const std::map<Type, nlohmann::json> &                                                                                                                              \
    getEnumMap(const Type &) {                                                                                                                                          \
      static_assert(std::is_enum<Type>::value, #Type " must be an enum!");                                                                                              \
      static const std::map<Type, nlohmann::json> map = __VA_ARGS__;                                                                                                    \
      return map;                                                                                                                                                       \
    }                                                                                                                                                                   \
                                                                                                                                                                        \
    void to_json(nlohmann::json &nlohmann_json_j, const Type &nlohmann_json_t) {                                                                                        \
      nlohmann_json_j = findInEnumMap<Type>(#Type " is missing enum mapping!", [nlohmann_json_t](const auto &pair) { return pair.first == nlohmann_json_t; })->second;  \
    }                                                                                                                                                                   \
                                                                                                                                                                        \
    void from_json(const nlohmann::json &nlohmann_json_j, Type &nlohmann_json_t) {                                                                                      \
      nlohmann_json_t = findInEnumMap<Type>(#Type " is missing enum mapping!", [&nlohmann_json_j](const auto &pair) { return pair.second == nlohmann_json_j; })->first; \
    }

namespace display_device {
  // A shared function for enums to find values in the map. Extracted here for UTs + coverage
  template <class T, class Predicate>
  std::map<T, nlohmann::json>::const_iterator
  findInEnumMap(const char *error_msg, Predicate predicate) {
    const auto &map { getEnumMap(T {}) };
    auto it { std::find_if(std::begin(map), std::end(map), predicate) };
    if (it == std::end(map)) {  // GCOVR_EXCL_BR_LINE for fallthrough branch
      throw std::runtime_error(error_msg);  // GCOVR_EXCL_BR_LINE for fallthrough branch
    }
    return it;
  }
}  // namespace display_device

namespace nlohmann {
  // Specialization for optional types until they actually implement it.
  template <typename T>
  struct adl_serializer<std::optional<T>> {
    static void
    to_json(json &nlohmann_json_j, const std::optional<T> &nlohmann_json_t) {
      if (nlohmann_json_t == std::nullopt) {
        nlohmann_json_j = nullptr;
      }
      else {
        nlohmann_json_j = *nlohmann_json_t;
      }
    }

    static void
    from_json(const json &nlohmann_json_j, std::optional<T> &nlohmann_json_t) {
      if (nlohmann_json_j.is_null()) {
        nlohmann_json_t = std::nullopt;
      }
      else {
        nlohmann_json_t = nlohmann_json_j.template get<T>();
      }
    }
  };
}  // namespace nlohmann
#endif
