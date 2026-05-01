// ============================================================================
// Template 10 — Demo: Expression Templates & Tag Dispatch
// ============================================================================
#include <array>
#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// 1. Expression Templates — Mini Vector Library
// ──────────────────────────────────────────────────────────────────────────

// Base expression class
template <typename E>
struct VecExpr
{
    double operator[](size_t i) const
    {
        return static_cast<const E&>(*this)[i];
    }
    size_t size() const
    {
        return static_cast<const E&>(*this).size();
    }
};

// Concrete vector
class Vec : public VecExpr<Vec>
{
    std::vector<double> data_;

public:
    explicit Vec(size_t n)
        : data_(n, 0.0)
    {
    }
    Vec(std::initializer_list<double> il)
        : data_(il)
    {
    }

    // Assign from any expression
    template <typename E>
    Vec& operator=(const VecExpr<E>& expr)
    {
        for (size_t i = 0; i < data_.size(); ++i)
            data_[i] = expr[i];
        return *this;
    }

    template <typename E>
    Vec(const VecExpr<E>& expr, size_t n)
        : data_(n)
    {
        for (size_t i = 0; i < n; ++i)
            data_[i] = expr[i];
    }

    double operator[](size_t i) const
    {
        return data_[i];
    }
    double& operator[](size_t i)
    {
        return data_[i];
    }
    size_t size() const
    {
        return data_.size();
    }
};

// Add expression
template <typename L, typename R>
struct VecAdd : VecExpr<VecAdd<L, R>>
{
    const L& lhs;
    const R& rhs;
    VecAdd(const L& l, const R& r)
        : lhs(l)
        , rhs(r)
    {
    }
    double operator[](size_t i) const
    {
        return lhs[i] + rhs[i];
    }
    size_t size() const
    {
        return lhs.size();
    }
};

// Scalar multiply expression
template <typename E>
struct VecScale : VecExpr<VecScale<E>>
{
    double   scalar;
    const E& expr;
    VecScale(double s, const E& e)
        : scalar(s)
        , expr(e)
    {
    }
    double operator[](size_t i) const
    {
        return scalar * expr[i];
    }
    size_t size() const
    {
        return expr.size();
    }
};

template <typename L, typename R>
VecAdd<L, R> operator+(const VecExpr<L>& l, const VecExpr<R>& r)
{
    return VecAdd<L, R>(static_cast<const L&>(l), static_cast<const R&>(r));
}

template <typename E>
VecScale<E> operator*(double s, const VecExpr<E>& e)
{
    return VecScale<E>(s, static_cast<const E&>(e));
}

void demo_expression_templates()
{
    std::cout << "=== 1. Expression Templates ===\n";

    Vec a{1.0, 2.0, 3.0};
    Vec b{4.0, 5.0, 6.0};
    Vec c{0.1, 0.2, 0.3};

    // This creates NO temporaries — evaluated lazily at assignment
    Vec result(3);
    result = a + b + 2.0 * c;

    std::cout << "  a + b + 2*c = [";
    for (size_t i = 0; i < result.size(); ++i)
    {
        if (i)
            std::cout << ", ";
        std::cout << result[i];
    }
    std::cout << "]\n"; // [5.2, 7.4, 9.6]
    std::cout << "  (No temporaries created!)\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 2. Tag Dispatch
// ──────────────────────────────────────────────────────────────────────────

// Tag types
struct fast_tag
{
};
struct safe_tag
{
};
struct debug_tag
{
};

// Implementation selected by tag
template <typename T>
T divide_impl(T a, T b, fast_tag)
{
    return a / b; // no checks
}

template <typename T>
T divide_impl(T a, T b, safe_tag)
{
    if (b == T{})
        return T{};
    return a / b;
}

template <typename T>
T divide_impl(T a, T b, debug_tag)
{
    std::cout << "  [debug] dividing " << a << " / " << b << "\n";
    if (b == T{})
    {
        std::cout << "  [debug] division by zero!\n";
        return T{};
    }
    return a / b;
}

// User-facing function with tag parameter
template <typename Tag = safe_tag, typename T>
T divide(T a, T b)
{
    return divide_impl(a, b, Tag{});
}

// Iterator category-based dispatch
template <typename It>
void advance_impl(It& it, int n, std::random_access_iterator_tag)
{
    std::cout << "  (random access: O(1) advance)\n";
    it += n;
}

template <typename It>
void advance_impl(It& it, int n, std::input_iterator_tag)
{
    std::cout << "  (input iterator: O(n) advance)\n";
    while (n-- > 0)
        ++it;
}

template <typename It>
void my_advance(It& it, int n)
{
    advance_impl(it, n, typename std::iterator_traits<It>::iterator_category{});
}

void demo_tag_dispatch()
{
    std::cout << "=== 2. Tag Dispatch ===\n";
    std::cout << "  fast: 10/3 = " << divide<fast_tag>(10, 3) << "\n";
    std::cout << "  safe: 10/0 = " << divide<safe_tag>(10, 0) << "\n";
    divide<debug_tag>(10, 3);

    std::vector<int> v{1, 2, 3, 4, 5};
    auto             it = v.begin();
    my_advance(it, 3);
    std::cout << "  advanced to: " << *it << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 3. Policy-Based Design
// ──────────────────────────────────────────────────────────────────────────

// Storage policies
template <typename T>
struct VectorStorage
{
    std::vector<T> data;
    void           add(const T& val)
    {
        data.push_back(val);
    }
    size_t size() const
    {
        return data.size();
    }
    const T& get(size_t i) const
    {
        return data[i];
    }
};

template <typename T>
struct FixedStorage
{
    std::array<T, 100> data{};
    size_t             count = 0;
    void               add(const T& val)
    {
        if (count < 100)
            data[count++] = val;
    }
    size_t size() const
    {
        return count;
    }
    const T& get(size_t i) const
    {
        return data[i];
    }
};

// Logging policies
struct NoLog
{
    void log(const std::string&) const
    {
    }
};

struct ConsoleLog
{
    void log(const std::string& msg) const
    {
        std::cout << "  [LOG] " << msg << "\n";
    }
};

// Policy-based container
template <typename T, template <typename> class Storage = VectorStorage, typename LogPolicy = NoLog>
class PolicyContainer : private LogPolicy
{
    Storage<T> storage_;

public:
    void add(const T& val)
    {
        this->log("Adding element");
        storage_.add(val);
    }
    size_t size() const
    {
        return storage_.size();
    }
    const T& get(size_t i) const
    {
        return storage_.get(i);
    }
};

void demo_policy_design()
{
    std::cout << "=== 3. Policy-Based Design ===\n";

    // Default: vector storage, no logging
    PolicyContainer<int> basic;
    basic.add(1);
    basic.add(2);
    basic.add(3);
    std::cout << "  basic size = " << basic.size() << "\n";

    // With logging
    PolicyContainer<int, VectorStorage, ConsoleLog> logged;
    logged.add(42);

    // Fixed storage, no logging
    PolicyContainer<int, FixedStorage> fixed;
    fixed.add(10);
    fixed.add(20);
    std::cout << "  fixed size = " << fixed.size() << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 4. The Detecting Idiom
// ──────────────────────────────────────────────────────────────────────────

// General-purpose detector
template <typename, template <typename...> class Op, typename... Args>
struct is_detected_impl : std::false_type
{
};

template <template <typename...> class Op, typename... Args>
struct is_detected_impl<std::void_t<Op<Args...>>, Op, Args...> : std::true_type
{
};

template <template <typename...> class Op, typename... Args>
constexpr bool is_detected_v = is_detected_impl<void, Op, Args...>::value;

// Detectors
template <typename T>
using has_push_back_t = decltype(std::declval<T>().push_back(std::declval<typename T::value_type>()));

template <typename T>
using has_size_t = decltype(std::declval<T>().size());

template <typename T>
using has_to_string_t = decltype(std::to_string(std::declval<T>()));

void demo_detecting_idiom()
{
    std::cout << "=== 4. The Detecting Idiom ===\n";

    static_assert(is_detected_v<has_push_back_t, std::vector<int>>);
    static_assert(!is_detected_v<has_push_back_t, std::array<int, 5>>);
    std::cout << "  vector has push_back: true ✓\n";
    std::cout << "  array has push_back: false ✓\n";

    static_assert(is_detected_v<has_size_t, std::vector<int>>);
    static_assert(is_detected_v<has_size_t, std::string>);
    static_assert(!is_detected_v<has_size_t, int>);
    std::cout << "  string has size: true ✓\n";
    std::cout << "  int has size: false ✓\n";

    static_assert(is_detected_v<has_to_string_t, int>);
    static_assert(!is_detected_v<has_to_string_t, std::string>);
    std::cout << "  int has to_string: true ✓\n";
    std::cout << "  string has to_string: false ✓\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Template 10 — Expression Templates & Tag Disp.  ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    demo_expression_templates();
    demo_tag_dispatch();
    demo_policy_design();
    demo_detecting_idiom();

    std::cout << "All demos complete.\n";
    return 0;
}
