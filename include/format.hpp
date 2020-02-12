#ifndef LRSTD_FORMAT_HPP
#define LRSTD_FORMAT_HPP

#include <array>
#include <charconv>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>

namespace lrstd {

#define LRSTD_UNREACHABLE() __builtin_unreachable()

// clang-format off
/*
namespace std {
  // [format.error], class format_error
  class format_error;

  // [format.formatter], formatter
  template<class charT> class basic_format_parse_context;
  using format_parse_context = basic_format_parse_context<char>;
  using wformat_parse_context = basic_format_parse_context<wchar_t>;
  
  template<class Out, class charT> class basic_format_context;
  using format_context = basic_format_context<unspecified, char>;
  using wformat_context = basic_format_context<unspecified, wchar_t>;

  template<class T, class charT = char> struct formatter;
  
  // [format.arguments], arguments
  template<class Context> class basic_format_arg;

  template<class Visitor, class Context>
    see below visit_format_arg(Visitor&& vis, basic_format_arg<Context> arg);

  template<class Context, class... Args> struct format-arg-store; // exposition only

  template<class Context> class basic_format_args;
  using format_args = basic_format_args<format_context>;
  using wformat_args = basic_format_args<wformat_context>;

  template<class Out, class charT>
    using format_args_t = basic_format_args<basic_format_context<Out, charT>>;

  template<class Context = format_context, class... Args>
    format-arg-store<Context, Args...>
      make_format_args(const Args&... args);
  template<class... Args>
    format-arg-store<wformat_context, Args...>
      make_wformat_args(const Args&... args);

  // [format.functions], formatting functions
  template<class... Args>
    string format(string_view fmt, const Args&... args);
  template<class... Args>
    wstring format(wstring_view fmt, const Args&... args);

  string vformat(string_view fmt, format_args args);
  wstring vformat(wstring_view fmt, wformat_args args);

  template<class Out, class... Args>
    Out format_to(Out out, string_view fmt, const Args&... args);
  template<class Out, class... Args>
    Out format_to(Out out, wstring_view fmt, const Args&... args);

  template<class Out>
    Out vformat_to(Out out, string_view fmt, format_args_t<Out, char> args);
  template<class Out>
    Out vformat_to(Out out, wstring_view fmt, format_args_t<Out, wchar_t> args);

  template<class Out>
    struct format_to_n_result {
      Out out;
      iter_difference_t<Out> size;
    };
  
  template<class Out, class... Args>
    format_to_n_result<Out> format_to_n(Out out, iter_difference_t<Out> n,
                                        string_view fmt, const Args&... args);
  template<class Out, class... Args>
    format_to_n_result<Out> format_to_n(Out out, iter_difference_t<Out> n,
                                        wstring_view fmt, const Args&... args);

  template<class... Args>
    size_t formatted_size(string_view fmt, const Args&... args);
  template<class... Args>
    size_t formatted_size(wstring_view fmt, const Args&... args);
}
*/
// clang-format on

class format_error : public std::runtime_error {
   public:
    explicit format_error(const std::string& w) : std::runtime_error{w} {}
    explicit format_error(const char* w) : std::runtime_error{w} {}
};

template <typename CharT>
using basic_string_view = std::basic_string_view<CharT>;

template <class Out, class CharT>
class basic_format_context;
using format_context =
      basic_format_context<std::back_insert_iterator<std::string>, char>;
using wformat_context =
      basic_format_context<std::back_insert_iterator<std::wstring>, wchar_t>;

template <class CharT>
class basic_format_parse_context;
using format_parse_context = basic_format_parse_context<char>;
using wformat_parse_context = basic_format_parse_context<wchar_t>;

template <class Context>
class basic_format_args;
using format_args = basic_format_args<format_context>;
using wformat_args = basic_format_args<wformat_context>;
template <class Out, class CharT>
using format_args_t = basic_format_args<basic_format_context<Out, CharT>>;

template <class T, class CharT>
struct formatter;

namespace detail {
template <class CharT, class Out>
Out vformat_to_impl(Out out,
                    basic_string_view<CharT> fmt,
                    format_args_t<Out, CharT>& args);
}  // namespace detail

template <class CharT>
class basic_format_parse_context {
   public:
    using char_type = CharT;
    using const_iterator = typename basic_string_view<CharT>::const_iterator;
    using iterator = const_iterator;

   private:
    iterator _begin;
    iterator _end;
    enum class indexing : char { unknown, manual, automatic };
    indexing _indexing;
    std::size_t _next_arg_id;
    std::size_t _num_args;

    void ncc() {}

    template <class C, class O>
    friend O detail::vformat_to_impl(O out,
                                     basic_string_view<C> fmt,
                                     format_args_t<O, C>& args);

   public:
    explicit constexpr basic_format_parse_context(basic_string_view<CharT> fmt,
                                                  size_t num_args = 0) noexcept
        : _begin{fmt.begin()}
        , _end{fmt.end()}
        , _indexing{indexing::unknown}
        , _next_arg_id{0}
        , _num_args{num_args} {}

    basic_format_parse_context(const basic_format_parse_context&) = delete;
    basic_format_parse_context& operator=(const basic_format_parse_context&) =
          delete;

    constexpr const_iterator begin() const noexcept { return _begin; }
    constexpr const_iterator end() const noexcept { return _end; }
    constexpr void advance_to(const_iterator it) noexcept { _begin = it; }

    constexpr std::size_t next_arg_id() {
        if (_indexing != indexing::manual) {
            _indexing = indexing::automatic;
            return _next_arg_id++;
        }
        throw format_error{"mixing of automatic and manual argument indexing"};
    }
    constexpr void check_arg_id(std::size_t id_) {
        if (id_ >= _num_args)
            ncc();
        if (_indexing != indexing::automatic)
            _indexing = indexing::manual;
        else
            throw format_error{
                  "mixing of automatic and manual argument indexing"};
    }
};

namespace detail {
template <typename T>
inline constexpr bool is_signed_integer_v =
      std::is_integral_v<T>&& std::is_signed_v<T>;
template <typename T>
inline constexpr bool is_unsigned_integer_v =
      std::is_integral_v<T> && !std::is_signed_v<T>;
}  // namespace detail

namespace detail {
template <class Context, class... Args>
struct args_storage;
}  // namespace detail

template <class Context>
struct basic_format_arg {
   public:
    class handle {
        using char_type = typename Context::char_type;
        const void* _ptr;
        void (*_format)(basic_format_parse_context<char_type>&,
                        Context&,
                        const void*);

        template <class T>
        explicit handle(const T& val) noexcept
            : _ptr{std::addressof(val)}
            , _format{[](basic_format_parse_context<char_type>& pc,
                         Context& fc,
                         const void* ptr) {
                typename Context::template formatter_type<T> f;
                pc.advance_to(f.parse(pc));
                fc.advance_to(f.format(*static_cast<const T*>(ptr), fc));
            }} {}

       public:
        void format(basic_format_parse_context<char_type>& pc,
                    Context& fc) const {
            _format(pc, fc, _ptr);
        }
    };

   private:
    using char_type = typename Context::char_type;

    std::variant<std::monostate,
                 bool,
                 char_type,
                 int,
                 unsigned int,
                 long long int,
                 unsigned long long int,
                 double,
                 long double,
                 const char_type*,
                 basic_string_view<char_type>,
                 const void*,
                 handle>
          value;

    template <class T,
              typename = std::enable_if_t<std::is_same_v<
                    decltype(typename Context::template formatter_type<T>()
                                   .format(std::declval<const T&>(),
                                           std::declval<Context&>())),
                    typename Context::iterator>>>
    explicit basic_format_arg(const T& v) noexcept
        : value{[&] {
            if constexpr (std::is_same_v<T, bool> ||
                          std::is_same_v<T, char_type>) {
                return v;
            } else if constexpr (std::is_same_v<T, char> &&
                                 std::is_same_v<char_type, wchar_t>) {
                return static_cast<wchar_t>(v);
            } else if constexpr (detail::is_signed_integer_v<T> &&
                                 sizeof(T) <= sizeof(int)) {
                return static_cast<int>(v);
            } else if constexpr (detail::is_unsigned_integer_v<T> &&
                                 sizeof(T) <= sizeof(unsigned int)) {
                return static_cast<unsigned int>(v);
            } else if constexpr (detail::is_signed_integer_v<T> &&
                                 sizeof(T) <= sizeof(long long int)) {
                return static_cast<long long int>(v);
            } else if constexpr (detail::is_unsigned_integer_v<T> &&
                                 sizeof(T) <= sizeof(unsigned long long int)) {
                return static_cast<unsigned long long int>(v);
            } else {
                return handle(v);
            }
        }()} {}

    explicit basic_format_arg(float n) noexcept
        : value{static_cast<double>(n)} {}
    explicit basic_format_arg(double n) noexcept : value{n} {}
    explicit basic_format_arg(long double n) noexcept : value{n} {}
    explicit basic_format_arg(const char_type* s) : value{s} {}

    template <class Traits>
    explicit basic_format_arg(
          std::basic_string_view<char_type, Traits> s) noexcept
        : value{s} {}
    template <class Traits, class Alloc>
    explicit basic_format_arg(
          const std::basic_string<char_type, Traits, Alloc>& s)
        : value{basic_string_view<char_type>{s.data(), s.size()}} {}

    explicit basic_format_arg(std::nullptr_t)
        : value{static_cast<const void*>(nullptr)} {}

    template <class T, class = std::enable_if_t<std::is_void_v<T>>>
    explicit basic_format_arg(T* p) : value{p} {}

    template <class Ctx, class... Args>
    friend detail::args_storage<Ctx, Args...> make_format_args(const Args&...);

    template <class Visitor, class Ctx>
    friend auto visit_format_arg(Visitor&&, basic_format_arg<Ctx>);

   public:
    basic_format_arg() noexcept = default;

    explicit operator bool() const noexcept {
        return std::holds_alternative<std::monostate>(value);
    }
};

template <class Visitor, class Context>
auto visit_format_arg(Visitor&& visitor, basic_format_arg<Context> arg) {
    // TODO
    if (auto* val = std::get_if<typename basic_format_arg<Context>::handle>(
              &arg.value))
        return visitor(*val);
    if (auto* val = std::get_if<typename basic_format_arg<Context>::char_type>(
              &arg.value))
        return visitor(*val);
    if (auto* val = std::get_if<std::monostate>(&arg.value))
        return visitor(*val);
}

namespace detail {
template <class Context, class... Args>
struct args_storage {
    std::array<basic_format_arg<Context>, sizeof...(Args)> args;
};
}  // namespace detail

template <class Context>
class basic_format_args {
    std::size_t _size;
    const basic_format_arg<Context>* _data;

    template <class, class>
    friend class basic_format_context;

   public:
    constexpr basic_format_args() noexcept : _size{0}, _data{nullptr} {}

    template <class... Args>
    basic_format_args(
          const detail::args_storage<Context, Args...>& storage) noexcept
        : _size{storage.args.size()}, _data{storage.args.data()} {}

    constexpr basic_format_arg<Context> get(std::size_t i) const noexcept {
        return i < _size ? _data[i] : basic_format_arg<Context>();
    }
};

template <class Out, class CharT>
class basic_format_context {
    basic_format_args<basic_format_context> _args;
    Out _out;

    basic_format_context(const basic_format_args<basic_format_context>& args,
                         Out out)
        : _args{args}, _out{out} {}

    template <class C, class O>
    friend O detail::vformat_to_impl(O out,
                                     basic_string_view<C> fmt,
                                     format_args_t<O, C>& args);

    constexpr std::size_t args_size() const { return _args._size; }

   public:
    using iterator = Out;
    using char_type = CharT;
    template <class T>
    using formatter_type = formatter<T, CharT>;

    basic_format_arg<basic_format_context> arg(std::size_t id_) const {
        return _args.get(id_);
    }

    iterator out() noexcept { return _out; }
    void advance_to(iterator it) { _out = it; }
};

template <class Context = format_context, class... Args>
detail::args_storage<Context, Args...> make_format_args(const Args&... args) {
    return {basic_format_arg<Context>(args)...};
}

template <class... Args>
detail::args_storage<wformat_context, Args...> make_wformat_args(
      const Args&... args) {
    return make_format_args<wformat_context>(args...);
}

namespace detail {

template <class T>
inline constexpr bool is_char_or_wchar_v =
      std::is_same_v<T, char> || std::is_same_v<T, wchar_t>;

template <class T, class CharT, bool Enable = is_char_or_wchar_v<CharT>>
struct formatter_impl {
    formatter_impl() = delete;
    formatter_impl(const formatter_impl&) = delete;
    formatter_impl& operator=(const formatter_impl&) = delete;
};

template <class CharT>
struct formatter_impl<CharT, CharT, true> {
    typename basic_format_parse_context<CharT>::iterator parse(
          basic_format_parse_context<CharT>& pc) {
        return pc.begin();
    }

    template <typename Out>
    typename basic_format_context<Out, CharT>::iterator format(
          CharT c,
          basic_format_context<Out, CharT>& fc) {
        fc.out() = c;
        return fc.out();
    }
};

// template <class CharT>
// struct formatter_impl<int, CharT, true> {
//     template <t

//     template <typename Out>
//     typename basic_format_context<Out, CharT>::iterator format(
//           int i,
//           basic_format_context<Out, CharT>& fc);
// };

}  // namespace detail

template <typename T, typename Char>
struct formatter : public detail::formatter_impl<T, Char> {};

namespace detail {

template <typename... F>
struct overloaded : public F... {
    using F::operator()...;
};
template <typename... F>
overloaded(F...)->overloaded<F...>;

namespace parse_fmt_str {
using end = std::monostate;
struct text {};
struct escaped_lbrace {};
struct escaped_rbrace {};
struct error {};

template <class CharT>
struct replacement_field {
    using arg_id_t = unsigned;
    arg_id_t arg_id;
    basic_string_view<CharT> format_spec;

    static constexpr arg_id_t auto_val() noexcept {
        return std::numeric_limits<arg_id_t>::max();
    }
    constexpr bool is_auto() const noexcept { return arg_id == auto_val(); }
};

template <class CharT>
using result = std::variant<end,
                            text,
                            escaped_lbrace,
                            escaped_rbrace,
                            replacement_field<CharT>,
                            error>;

template <class CharT>
constexpr bool consume(basic_string_view<CharT>& fmt,
                       basic_string_view<CharT> prefix) {
    using Traits = typename basic_string_view<CharT>::traits_type;
    if (fmt.size() >= prefix.size() &&
        Traits::compare(fmt.begin(), prefix.begin(), prefix.size()) == 0) {
        fmt.remove_prefix(prefix.size());
        return true;
    }
    return false;
}
template <class CharT>
constexpr bool consume(basic_string_view<CharT>& fmt, CharT c) {
    if (*fmt.begin() == c) {
        fmt.remove_prefix(1);
        return true;
    }
    return false;
}

template <class CharT>
void advance_to(basic_string_view<CharT>& s,
                typename basic_string_view<CharT>::iterator pos) {
    s.remove_prefix(std::distance(s.begin(), pos));
}

constexpr std::variant<replacement_field<char>::arg_id_t,
                       std::nullopt_t,
                       error> inline parse_arg_id(basic_string_view<char>&
                                                        fmt) {
    if (consume(fmt, '0'))
        return replacement_field<char>::arg_id_t(0);
    replacement_field<char>::arg_id_t val = 0;
    const auto result = std::from_chars(fmt.begin(), fmt.end(), val, 10);
    if (result.ptr == fmt.begin())
        return std::nullopt;
    if (result.ec != std::errc())
        return error{};
    advance_to(fmt, result.ptr);
    return val;
}

inline std::variant<replacement_field<wchar_t>::arg_id_t, std::nullopt_t, error>
parse_arg_id(basic_string_view<wchar_t>& ctx) {
    throw "not yet implemented";
}

template <class CharT>
struct get_replacement_field {
    constexpr std::optional<replacement_field<CharT>> operator()(
          typename replacement_field<CharT>::arg_id_t id_) const noexcept {
        return replacement_field<CharT>{id_};
    }
    constexpr std::optional<replacement_field<CharT>> operator()(
          std::nullopt_t) const noexcept {
        return replacement_field<CharT>{replacement_field<CharT>::auto_val()};
    }
    constexpr std::optional<replacement_field<CharT>> operator()(error) const
          noexcept {
        return std::nullopt;
    }
};

template <class CharT>
constexpr std::optional<replacement_field<CharT>> parse_replacement_field(
      basic_string_view<CharT>& fmt) {
    auto field = std::visit(get_replacement_field<CharT>{}, parse_arg_id(fmt));
    if (!field)
        return std::nullopt;
    if (consume(fmt, static_cast<CharT>(':'))) {
    }
    return field;
}

template <class CharT>
struct double_lbrace;
template <>
struct double_lbrace<char> {
    static constexpr std::string_view value = "{{";
};
template <>
struct double_lbrace<wchar_t> {
    static constexpr std::wstring_view value = L"{{";
};

template <class CharT>
struct double_rbrace;
template <>
struct double_rbrace<char> {
    static constexpr std::string_view value = "}}";
};
template <>
struct double_rbrace<wchar_t> {
    static constexpr std::wstring_view value = L"}}";
};

template <class CharT>
constexpr result<CharT> parse_next(basic_string_view<CharT>& fmt) {
    if (fmt.empty())
        return end{};

    if (consume(fmt, double_lbrace<CharT>::value))
        return escaped_lbrace{};
    if (consume(fmt, double_rbrace<CharT>::value))
        return escaped_rbrace{};

    using Traits = typename basic_string_view<CharT>::traits_type;
    using Iterator = typename basic_string_view<CharT>::const_iterator;
    static_assert(std::is_same_v<Iterator, const CharT*>, "this won't work");

    auto fmt_find = [&](CharT c) -> Iterator {
        const CharT* result = Traits::find(fmt.begin(), fmt.size(), c);
        return result ? result : fmt.end();
    };

    const auto next_lbrace_it = fmt_find(static_cast<CharT>('{'));
    if (next_lbrace_it != fmt.begin()) {
        advance_to(fmt, next_lbrace_it);
        return text{};
    }

    consume(fmt, static_cast<CharT>('{'));
    const auto result = parse_replacement_field(fmt);
    if (!result || !consume(fmt, static_cast<CharT>('}')))
        return error{};
    return *result;
}

}  // namespace parse_fmt_str

namespace fmt_out {
template <class CharT, class Out>
void text_out(basic_string_view<CharT> text, Out out) {
    for (auto& c : text)
        *++out = c;
}

template <typename T, typename Ret = void>
struct throw_uninitialized_format_arg {
    Ret operator()(const T&) const {
        throw format_error("uninitialized format argument");
    }
};

template <class Context, class Out, class CharT>
void arg_out(Context& fc,
             basic_format_parse_context<CharT>& pc,
             const basic_format_arg<Context>& arg,
             Out out) {
    visit_format_arg(
          overloaded{
                throw_uninitialized_format_arg<std::monostate>{},
                [&](const typename basic_format_arg<Context>::handle& handle) {
                    handle.format(pc, fc);
                },
                [&](const auto& val) {
                    using T = std::remove_cv_t<
                          std::remove_reference_t<decltype(val)>>;
                    typename Context::template formatter_type<T> f;
                    pc.advance_to(f.parse(pc));
                    fc.advance_to(f.format(val, fc));
                }},
          arg);
}
}  // namespace fmt_out

template <typename T, typename Ret = void>
struct unreachable {
    Ret operator()(const T&) const { LRSTD_UNREACHABLE(); }
};

template <class CharT, class Out>
Out vformat_to_impl(Out out,
                    basic_string_view<CharT> fmt,
                    format_args_t<Out, CharT>& args) {
    basic_format_context<Out, CharT> context(args, out);
    unsigned current_arg_id = 0;

    namespace p = parse_fmt_str;
    while (true) {
        auto old_begin = fmt.begin();
        p::result<CharT> r = p::parse_next(fmt);
        if (std::holds_alternative<p::end>(r))
            return out;
        if (std::holds_alternative<p::error>(r))
            throw format_error("bad format string");
        std::visit(
              overloaded{
                    unreachable<p::end>{}, unreachable<p::error>{},
                    [&](p::text) {
                        fmt_out::text_out(
                              basic_string_view<CharT>{
                                    old_begin,
                                    static_cast<std::size_t>(std::distance(
                                          old_begin, fmt.begin()))},
                              out);
                    },
                    [&](p::escaped_lbrace) {
                        const CharT c = static_cast<CharT>('{');
                        fmt_out::text_out(basic_string_view<CharT>{&c, 1}, out);
                    },
                    [&](p::escaped_rbrace) {
                        const CharT c = static_cast<CharT>('}');
                        fmt_out::text_out(basic_string_view<CharT>{&c, 1}, out);
                    },
                    [&](p::replacement_field<CharT> field) {
                        basic_format_parse_context<CharT> parse_context(
                              field.format_spec, context.args_size());
                        parse_context._next_arg_id = current_arg_id;
                        fmt_out::arg_out(
                              context, parse_context,
                              context.arg(field.is_auto()
                                                ? parse_context.next_arg_id()
                                                : field.arg_id),
                              out);
                        current_arg_id = parse_context.next_arg_id();
                    }},
              r);
    }
}  // namespace detail

template <class CharT>
std::basic_string<CharT> vformat_impl(
      basic_string_view<CharT> fmt,
      format_args_t<std::back_insert_iterator<std::basic_string<CharT>>, CharT>
            args) {
    std::basic_string<CharT> ret;
    vformat_to_impl(std::back_inserter(ret), fmt, args);
    return ret;
}

}  // namespace detail

template <class Out>
Out vformat_to(Out out, std::string_view fmt, format_args_t<Out, char> args) {
    return detail::vformat_to_impl(out, fmt, args);
}

template <class Out>
Out vformat_to(Out out,
               std::wstring_view fmt,
               format_args_t<Out, wchar_t> args) {
    return detail::vformat_to_impl(out, fmt, args);
}

template <class Out, class... Args>
Out format_to(Out out, std::string_view fmt, const Args&... args) {
    using Context = basic_format_context<
          Out, format_args_t<Out, std::string_view::value_type>>;
    return vformat_to(out, fmt, {make_format_args<Context>(args...)});
}

template <class Out, class... Args>
Out format_to(Out out, std::wstring_view fmt, const Args&... args) {
    using Context = basic_format_context<
          Out, format_args_t<Out, std::wstring_view::value_type>>;
    return vformat_to(out, fmt, {make_format_args<Context>(args...)});
}

inline std::string vformat(std::string_view fmt, format_args args) {
    return detail::vformat_impl(fmt, args);
}
inline std::wstring vformat(std::wstring_view fmt, wformat_args args) {
    return detail::vformat_impl(fmt, args);
}

template <class... Args>
std::string format(std::string_view fmt, const Args&... args) {
    return vformat(fmt, {make_format_args(args...)});
}

template <class... Args>
std::string format(std::wstring_view fmt, const Args&... args) {
    return vformat(fmt, {make_wformat_args(args...)});
}

}  // namespace lrstd

#endif