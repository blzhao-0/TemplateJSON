#pragma once
#ifndef __JSON_HPP__
#define __JSON_HPP__

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/facilities/expand.hpp>

#include <boost/preprocessor/stringize.hpp>

#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/pop_front.hpp>

#include <boost/preprocessor/variadic/to_seq.hpp>
#include <boost/preprocessor/variadic/elem.hpp>

#include "json_common_defs.hpp"
#include "json_value_parser.hpp"
#include "json_user_class_parsers.hpp"
#include "json_keys_handler.hpp"
#include "json_member_mapper.hpp"

namespace JSON {
    template<typename classFor>
    std::string ToJSON(const classFor& classFrom) {
        std::string json("{");
        json.reserve(1000);

        json.append(detail::MembersToJSON(classFrom, MembersHolder<classFor>::members()));

        json.append("}");
        return json;
    }

    template<typename classFor>
    jsonIter FromJSON(jsonIter iter, jsonIter end, classFor& classInto) {
        iter = ValidateObjectStart(iter, end);

        iter = detail::MembersFromJSON<classFor>(classInto, iter, end, MembersHolder<classFor>::members());

        return ValidateObjectEnd(iter, end);
    }

    template<typename classFor>
    classFor FromJSON(const std::string& jsonData) {
        classFor classInto;
        auto iter = jsonData.begin();
        auto end = jsonData.end();

        FromJSON(&*iter, &*end, classInto);

        return classInto;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
#define JSON_ENABLE(CLASS_NAME, ...)                                \
    namespace JSON {                                                \
        JSON_CREATE_KEYS(CLASS_NAME, __VA_ARGS__)                   \
        JSON_CREATE_MEMBERS(CLASS_NAME, __VA_ARGS__)                \
                                                                    \
        template struct detail::MemberMap<CLASS_NAME>;              \
                                                                    \
        template std::string ToJSON<CLASS_NAME>(const CLASS_NAME&);     \
        template CLASS_NAME FromJSON<CLASS_NAME>(const std::string&);   \
    }

#define JSON_ENABLE_ENUM(ENUM_NAME, ...)                            \
    namespace JSON {                                                \
        template<>                                                  \
        struct EnumValidator<ENUM_NAME> {                           \
            constexpr static EnumValueList<ENUM_NAME, __VA_ARGS__>  \
            values() {                                              \
                return EnumValueList<ENUM_NAME, __VA_ARGS__>();     \
            }                                                       \
        };                                                          \
    }

#define JSON_ENABLE_CONTIGUOUS_ENUM(ENUM_NAME, ...)                         \
    namespace JSON {                                                        \
        template<>                                                          \
        struct EnumValidator<ENUM_NAME> {                                   \
            constexpr static ContiguousEnumValueList<ENUM_NAME, __VA_ARGS__>\
            values() {                                                      \
                return ContiguousEnumValueList<ENUM_NAME, __VA_ARGS__>();   \
            }                                                               \
        };                                                                  \
    }

#endif
