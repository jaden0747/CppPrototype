// ============================================================================
// Template 12 — Demo: Library Design Patterns
// ============================================================================
#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// 1. Type Erasure
// ──────────────────────────────────────────────────────────────────────────

// A simplified std::function<int(int)>
class AnyCallable
{
    struct Concept
    {
        virtual ~Concept()                               = default;
        virtual int                      call(int) const = 0;
        virtual std::unique_ptr<Concept> clone() const   = 0;
    };

    template <typename F>
    struct Model : Concept
    {
        F func;
        Model(F f)
            : func(std::move(f))
        {
        }
        int call(int x) const override
        {
            return func(x);
        }
        std::unique_ptr<Concept> clone() const override
        {
            return std::make_unique<Model>(func);
        }
    };

    std::unique_ptr<Concept> impl_;

public:
    AnyCallable() = default;

    template <typename F>
    AnyCallable(F f)
        : impl_(std::make_unique<Model<F>>(std::move(f)))
    {
    }

    AnyCallable(const AnyCallable& other)
        : impl_(other.impl_ ? other.impl_->clone() : nullptr)
    {
    }

    AnyCallable& operator=(const AnyCallable& other)
    {
        impl_ = other.impl_ ? other.impl_->clone() : nullptr;
        return *this;
    }

    int operator()(int x) const
    {
        return impl_->call(x);
    }
    explicit operator bool() const
    {
        return impl_ != nullptr;
    }
};

// Type erasure for any "drawable"
class Drawable
{
    struct Concept
    {
        virtual ~Concept()                             = default;
        virtual void                     draw() const  = 0;
        virtual std::unique_ptr<Concept> clone() const = 0;
    };

    template <typename T>
    struct Model : Concept
    {
        T obj;
        Model(T o)
            : obj(std::move(o))
        {
        }
        void draw() const override
        {
            obj.draw();
        }
        std::unique_ptr<Concept> clone() const override
        {
            return std::make_unique<Model>(obj);
        }
    };

    std::unique_ptr<Concept> impl_;

public:
    template <typename T>
    Drawable(T obj)
        : impl_(std::make_unique<Model<T>>(std::move(obj)))
    {
    }

    Drawable(const Drawable& o)
        : impl_(o.impl_->clone())
    {
    }
    Drawable& operator=(const Drawable& o)
    {
        impl_ = o.impl_->clone();
        return *this;
    }

    void draw() const
    {
        impl_->draw();
    }
};

struct Circle
{
    double radius;
    void   draw() const
    {
        std::cout << "  Drawing circle r=" << radius << "\n";
    }
};

struct Square
{
    double side;
    void   draw() const
    {
        std::cout << "  Drawing square s=" << side << "\n";
    }
};

void demo_type_erasure()
{
    std::cout << "=== 1. Type Erasure ===\n";

    // AnyCallable — like std::function
    AnyCallable f1 = [](int x) { return x * 2; };
    AnyCallable f2 = [](int x) { return x + 10; };
    std::cout << "  f1(5) = " << f1(5) << "\n";
    std::cout << "  f2(5) = " << f2(5) << "\n";

    // Drawable — heterogeneous container without inheritance
    std::vector<Drawable> shapes;
    shapes.push_back(Circle{3.0});
    shapes.push_back(Square{4.0});
    shapes.push_back(Circle{1.5});

    for (const auto& s : shapes)
        s.draw();
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 2. Compile-Time String Processing
// ──────────────────────────────────────────────────────────────────────────

template <size_t N>
struct FixedString
{
    char data[N]{};
    constexpr FixedString(const char (&str)[N])
    {
        for (size_t i = 0; i < N; ++i)
            data[i] = str[i];
    }
    constexpr size_t size() const
    {
        return N - 1;
    }
    constexpr char operator[](size_t i) const
    {
        return data[i];
    }
};

// Use FixedString as NTTP (C++20)
template <FixedString S>
struct Tag
{
    static constexpr auto name = S;
    static void           print()
    {
        std::cout << "  Tag<\"" << S.data << "\"> size=" << S.size() << "\n";
    }
};

// Compile-time string concatenation
template <FixedString A, FixedString B>
constexpr auto concat_strings()
{
    char result[A.size() + B.size() + 1]{};
    for (size_t i = 0; i < A.size(); ++i)
        result[i] = A[i];
    for (size_t i = 0; i < B.size(); ++i)
        result[A.size() + i] = B[i];
    return FixedString<A.size() + B.size() + 1>(result);
}

void demo_compile_time_strings()
{
    std::cout << "=== 2. Compile-Time Strings ===\n";

    Tag<"hello">::print();
    Tag<"world">::print();

    constexpr auto combined = concat_strings<"hello", " world">();
    std::cout << "  concat: " << combined.data << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 3. Template-Based Serialization
// ──────────────────────────────────────────────────────────────────────────

template <typename T, typename Enable = void>
struct Serializer;

// Arithmetic types
template <typename T>
struct Serializer<T, std::enable_if_t<std::is_arithmetic_v<T>>>
{
    static void write(std::ostream& os, T val)
    {
        os << val;
    }
    static T read(std::istream& is)
    {
        T v;
        is >> v;
        return v;
    }
};

// String
template <>
struct Serializer<std::string>
{
    static void write(std::ostream& os, const std::string& val)
    {
        os << val.size() << " " << val;
    }
    static std::string read(std::istream& is)
    {
        size_t n;
        is >> n;
        std::string s(n, '\0');
        is.ignore(1);
        is.read(s.data(), n);
        return s;
    }
};

// Vectors
template <typename T>
struct Serializer<std::vector<T>>
{
    static void write(std::ostream& os, const std::vector<T>& v)
    {
        os << v.size();
        for (const auto& elem : v)
        {
            os << " ";
            Serializer<T>::write(os, elem);
        }
    }
    static std::vector<T> read(std::istream& is)
    {
        size_t n;
        is >> n;
        std::vector<T> v;
        v.reserve(n);
        for (size_t i = 0; i < n; ++i)
            v.push_back(Serializer<T>::read(is));
        return v;
    }
};

// Convenience functions
template <typename T>
std::string serialize(const T& val)
{
    std::ostringstream os;
    Serializer<T>::write(os, val);
    return os.str();
}

template <typename T>
T deserialize(const std::string& data)
{
    std::istringstream is(data);
    return Serializer<T>::read(is);
}

void demo_serialization()
{
    std::cout << "=== 3. Template-Based Serialization ===\n";

    auto s1 = serialize(42);
    std::cout << "  serialize(42) = \"" << s1 << "\"\n";
    assert(deserialize<int>(s1) == 42);

    auto s2 = serialize(std::string("hello"));
    std::cout << "  serialize(\"hello\") = \"" << s2 << "\"\n";
    assert(deserialize<std::string>(s2) == "hello");

    auto s3 = serialize(std::vector<int>{1, 2, 3});
    std::cout << "  serialize({1,2,3}) = \"" << s3 << "\"\n";
    auto v = deserialize<std::vector<int>>(s3);
    assert(v == (std::vector<int>{1, 2, 3}));
    std::cout << "  Round-trip OK ✓\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 4. Range-Like View Adaptor (Simplified)
// ──────────────────────────────────────────────────────────────────────────

// Simplified transform_view
template <typename Range, typename Func>
class TransformView
{
    Range range_;
    Func  func_;

public:
    TransformView(Range r, Func f)
        : range_(std::move(r))
        , func_(std::move(f))
    {
    }

    struct Iterator
    {
        using inner_iter = decltype(std::declval<const Range>().begin());
        inner_iter  it;
        const Func* func;

        auto operator*() const
        {
            return (*func)(*it);
        }
        Iterator& operator++()
        {
            ++it;
            return *this;
        }
        bool operator!=(const Iterator& o) const
        {
            return it != o.it;
        }
    };

    Iterator begin() const
    {
        return {range_.begin(), &func_};
    }
    Iterator end() const
    {
        return {range_.end(), &func_};
    }
};

// Pipe operator adaptor
template <typename Func>
struct TransformAdaptor
{
    Func func;
};

template <typename Func>
TransformAdaptor<Func> transform(Func f)
{
    return {std::move(f)};
}

template <typename Range, typename Func>
auto operator|(Range&& r, TransformAdaptor<Func> a)
{
    return TransformView<std::decay_t<Range>, Func>(std::forward<Range>(r), std::move(a.func));
}

void demo_range_internals()
{
    std::cout << "=== 4. Range-Like View Adaptor ===\n";

    std::vector<int> v{1, 2, 3, 4, 5};

    // Pipe syntax — like std::views::transform
    auto doubled = v | transform([](int x) { return x * 2; });

    std::cout << "  {1,2,3,4,5} | transform(*2) = [";
    bool first = true;
    for (auto val : doubled)
    {
        if (!first)
            std::cout << ", ";
        std::cout << val;
        first = false;
    }
    std::cout << "]\n";

    // Chain two transforms
    auto result = v | transform([](int x) { return x * 2; }) | transform([](int x) { return x + 1; });

    std::cout << "  | transform(*2) | transform(+1) = [";
    first = true;
    for (auto val : result)
    {
        if (!first)
            std::cout << ", ";
        std::cout << val;
        first = false;
    }
    std::cout << "]\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Template 12 — Library Design Patterns           ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    demo_type_erasure();
    demo_compile_time_strings();
    demo_serialization();
    demo_range_internals();

    std::cout << "All demos complete.\n";
    return 0;
}
