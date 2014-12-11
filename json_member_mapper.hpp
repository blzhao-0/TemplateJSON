#pragma once
#ifndef __JSON_MEMBER_MAPPER_HPP__
#define __JSON_MEMBER_MAPPER_HPP__

namespace JSON {

    template<const char_t*... keys>
    struct KeyList {
        constexpr KeyList() {};
    };

    template<typename memberType, memberType member>
    struct Member {
        constexpr Member() {};

        typedef memberType type;
        constexpr static memberType value = member;
    };

    template<typename... members>
    struct MemberList {
        constexpr MemberList() {};
    };

    template<typename classType>
    constexpr void* GetPointer(classType& classOn);

    template<typename classType, typename underlyingType>
    constexpr void* ResolvePointerType(classType& classOn, underlyingType classType::* member) {
        return &(classOn.*member);
    }

    template<typename classType, typename underlyingType>
    constexpr void* ResolvePointerType(classType& classOn, underlyingType* member) {
        return (void*)member;
    }

    template<typename classType, typename memberType>
    constexpr void* GetPointer(classType& classOn) {
        return ResolvePointerType(classOn, memberType::value);
    }

    template<typename classType, typename memberType>
    struct DeclareGetPointer {
        typedef memberType type;
        typedef typename std::decay<memberType>::type underlying_type;

        constexpr static void* (*const fn)(classType&) = &JSON::GetPointer<classType, memberType>;
    };

    template<typename classType>
    struct VirtualAccessor {
        constexpr VirtualAccessor(void* (*const access)(classType&)) : GetPointer(access) {};
        void* (*const GetPointer)(classType&);
    };

    template<typename classType>
    struct MemberMap;

    namespace MapTypes {
        template<typename T>
        using maptype = std::unordered_map<JSON::stringt, VirtualAccessor<T>>;

        template<typename T>
        using value_type = typename maptype<T>::value_type;
    }

    template<typename classFor,
             const char_t* key, const char_t*... keys,
             template<const char_t*...> class K,
             typename type, typename... types,
             template<typename... M> class ML>
    constexpr static const MapTypes::maptype<classFor> CreateMap(const K<key, keys...>& k1,
                                                                 const ML<type, types...>& ml) {
        return CreateMap<classFor>(KeyList<keys...>(), MemberList<types...>(),
                         MapTypes::value_type<classFor>{
                            stringt(key),
                            VirtualAccessor<classFor>(DeclareGetPointer<classFor, type>::fn)
                        }
                );
    }

    template<typename classFor,
             const char_t* key, const char_t*... keys,
             template<const char_t*...> class K,
             typename type, typename... types,
             template<typename... M> class ML,
             typename... value_types>
    constexpr static const MapTypes::maptype<classFor> CreateMap(K<key, keys...>&& k2,
                                                                 ML<type, types...>&& ml,
                                                                 value_types&&... pairs) {
        return CreateMap<classFor>(KeyList<keys...>(), MemberList<types...>(),
                         MapTypes::value_type<classFor>{
                            stringt(key),
                            VirtualAccessor<classFor>(DeclareGetPointer<classFor, type>::fn)
                        },
                        pairs...
                );
    }

    template<typename classFor,
             template<const char_t*... T> class K,
             template<typename... M> class ML,
             typename... value_types>
    constexpr static const MapTypes::maptype<classFor> CreateMap(K<>&& k3,
                                                                 ML<>&& ml,
                                                                 value_types&&... pairs) {
        return { pairs... };
    }

#define JSON_MAKE_MEMBER_MAP(CLASS_NAME, ...)                   \
    template<>                                                  \
    struct ClassMap<CLASS_NAME> {                               \
        typedef CLASS_NAME classFor;                            \
                                                                \
        ClassMap() :                                            \
            values(                                             \
                CreateMap<CLASS_NAME>(                          \
                    JSON::KeysHolder<CLASS_NAME>::keys,         \
                    JSON::MembersHolder<CLASS_NAME>::members    \
                )                                               \
            )                                                   \
        {}                                                      \
                                                                \
        const MapTypes::maptype<classFor> values;               \
    };                                                          \
                                                                \
    template<>                                                  \
    struct MemberMap<CLASS_NAME> {                              \
        static const ClassMap<CLASS_NAME> mapping;              \
    };                                                          \
                                                                \
    const ClassMap<CLASS_NAME> MemberMap<CLASS_NAME>::mapping;

////////////////////////////////////////////////////////////////////////////////////////////////////
#define JSON_LIST_KEYS(CLASS_NAME, ...)             \
    JSON_FIRST_KEY(                                 \
        CLASS_NAME,                                 \
        BOOST_PP_VARIADIC_ELEM(0, __VA_ARGS__)      \
    )                                               \
    BOOST_PP_SEQ_FOR_EACH(                          \
        JSON_LIST_KEYS_IMPL, CLASS_NAME,            \
        BOOST_PP_SEQ_POP_FRONT(                     \
            BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)   \
        )                                           \
    )

#define JSON_LIST_KEYS_IMPL(s, CLASS_NAME, VARDATA)    \
    , KeysHolder<CLASS_NAME>:: JSON_KEY_LISTER VARDATA

#ifndef _MSC_VER
#define JSON_KEY_LISTER(...)                                      \
    BOOST_PP_OVERLOAD(JSON_KEY_LISTER, __VA_ARGS__)(__VA_ARGS__)
#else
#define JSON_KEY_LISTER(...)                                      \
    BOOST_PP_CAT(BOOST_PP_OVERLOAD(JSON_KEY_LISTER, __VA_ARGS__)(__VA_ARGS__),BOOST_PP_EMPTY())
#endif

#define JSON_KEY_LISTER2(VARNAME, JSONKEY)  \
    JSON_KEY_LISTER1(VARNAME)

#define JSON_KEY_LISTER1(VARNAME)   \
    VARNAME##__JSON_KEY

//////////////////////////////////////////////
#define JSON_FIRST_KEY(CLASS_NAME, VARDATA)     \
    KeysHolder<CLASS_NAME>:: JSON_FIRST_KEY_IMPL VARDATA

#ifndef _MSC_VER
#define JSON_FIRST_KEY_IMPL(...)                                     \
    BOOST_PP_OVERLOAD(JSON_FIRST_KEY_IMPL, __VA_ARGS__)(__VA_ARGS__)
#else
#define JSON_FIRST_KEY_IMPL(...)                                      \
    BOOST_PP_CAT(BOOST_PP_OVERLOAD(JSON_FIRST_KEY_IMPL,__VA_ARGS__)(__VA_ARGS__),BOOST_PP_EMPTY())
#endif

#define JSON_FIRST_KEY_IMPL2(VARNAME, JSONKEY)   \
    JSON_FIRST_KEY_IMPL1(VARNAME)

#define JSON_FIRST_KEY_IMPL1(VARNAME)    \
    VARNAME##__JSON_KEY

////////////////////////////////////////////////////////////////////////////////////////////////////
#define JSON_CREATE_MEMBERS(CLASS_NAME, ...)                                \
    template<>                                                              \
    struct MembersHolder<CLASS_NAME> {                                      \
    private:                                                                \
        BOOST_PP_SEQ_FOR_EACH(                                              \
            JSON_CREATE_MEMBERS_IMPL, CLASS_NAME,                           \
            BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                           \
        )                                                                   \
    public:                                                                 \
        static constexpr const auto members =                               \
            JSON::MemberList<JSON_LIST_MEMBERS(CLASS_NAME, __VA_ARGS__)>(); \
    };                                                                      \
                                                                            \
    BOOST_PP_SEQ_FOR_EACH(                                                  \
        JSON_REFERENCE_MEMBER, CLASS_NAME,                                  \
        BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                               \
    )                                                                       \
                                                                            \
    constexpr const JSON::MemberList<                                       \
        JSON_LIST_MEMBERS(CLASS_NAME, __VA_ARGS__)                          \
    > MembersHolder<CLASS_NAME>::members;

#define JSON_CREATE_MEMBERS_IMPL(s, CLASS_NAME, VARDATA)                            \
    static constexpr const auto JSON_MEMBER_NAME VARDATA =                          \
        JSON::Member<decltype(&CLASS_NAME:: JSON_VARNAME VARDATA),                  \
                     &CLASS_NAME:: JSON_VARNAME VARDATA>();

#ifndef _MSC_VER
#define JSON_MEMBER_NAME(...)                                      \
    BOOST_PP_OVERLOAD(JSON_MEMBER_NAME, __VA_ARGS__)(__VA_ARGS__)
#else
#define JSON_MEMBER_NAME(...)                                      \
    BOOST_PP_CAT(BOOST_PP_OVERLOAD(JSON_MEMBER_NAME, __VA_ARGS__)(__VA_ARGS__),BOOST_PP_EMPTY())
#endif

#define JSON_MEMBER_NAME1(VARNAME)  \
    VARNAME ## __MEMBER

#define JSON_MEMBER_NAME2(VARNAME, IGNORED)  \
    JSON_MEMBER_NAME1(VARNAME)

#ifndef _MSC_VER
#define JSON_VARNAME(...)                                      \
    BOOST_PP_OVERLOAD(JSON_VARNAME, __VA_ARGS__)(__VA_ARGS__)
#else
#define JSON_VARNAME(...)                                      \
    BOOST_PP_CAT(BOOST_PP_OVERLOAD(JSON_VARNAME, __VA_ARGS__)(__VA_ARGS__),BOOST_PP_EMPTY())
#endif

#define JSON_VARNAME1(VARNAME)  \
    VARNAME

#define JSON_VARNAME2(VARNAME, IGNORED)  \
    JSON_VARNAME1(VARNAME)

//////////////////////////////////////////////////
#define JSON_REFERENCE_MEMBER(s, CLASS_NAME, VARDATA)       \
    constexpr const JSON::Member<                           \
        decltype(&CLASS_NAME:: JSON_VARNAME VARDATA),        \
        &CLASS_NAME:: JSON_VARNAME VARDATA                  \
    > MembersHolder<CLASS_NAME>:: JSON_MEMBER_NAME VARDATA; \

};

#endif
