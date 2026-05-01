// ============================================================================
// Template 08 — Exercises: CRTP & Static Polymorphism
// ============================================================================
#include <cassert>
#include <cmath>
#include <iostream>
#include <string>

// ──────────────────────────────────────────────────────────────────────────
// Exercise 1: Write a CRTP base `Stringify<Derived>` that provides:
//   - operator<<(ostream&, Derived) calling Derived::to_string()
//   - std::string str() const — returns to_string()
// Then create a `Color` class with r, g, b that uses it.
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename Derived> struct Stringify { ... };
// TODO: struct Color : Stringify<Color> { ... };

void exercise_stringify()
{
    // Color c{255, 128, 0};
    // assert(c.str() == "rgb(255, 128, 0)");
    // std::cout << c << "\n";
    std::cout << "  Exercise 1: Stringify — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 2: Write a CRTP `Singleton<T>` mixin that:
//   - Deletes copy/move constructors
//   - Provides static T& instance()
//   - Allows T to have a private constructor
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename T> class Singleton { ... };

void exercise_singleton()
{
    // struct AppConfig : Singleton<AppConfig> {
    //     friend class Singleton<AppConfig>;
    //     int value = 42;
    // private:
    //     AppConfig() = default;
    // };
    // assert(AppConfig::instance().value == 42);
    // assert(&AppConfig::instance() == &AppConfig::instance());
    std::cout << "  Exercise 2: Singleton — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 3: Write CRTP shapes Triangle, Ellipse with a Shape<Derived>
// base. Each must provide area_impl(), perimeter_impl(), and name().
// Write a free function template `print_info(Shape<D>&)`.
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename Derived> struct Shape { ... };
// TODO: struct Triangle : Shape<Triangle> { ... };
// TODO: struct Ellipse : Shape<Ellipse> { ... };

void exercise_shapes()
{
    // Triangle t(3.0, 4.0, 5.0);
    // assert(std::abs(t.area() - 6.0) < 0.01);
    // Ellipse e(3.0, 2.0);
    // print_info(t);
    // print_info(e);
    std::cout << "  Exercise 3: Shapes — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 4: Write a CRTP `Registry<Derived>` that:
//   - Tracks all live instances in a static vector<Derived*>
//   - Provides static size_t count()
//   - Provides static void for_each(func) — calls func on each instance
//   - Auto-registers in constructor, unregisters in destructor
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename Derived> class Registry { ... };

void exercise_registry()
{
    // struct Sensor : Registry<Sensor> {
    //     std::string name;
    //     Sensor(std::string n) : name(std::move(n)) {}
    // };
    // { Sensor s1("temp"), s2("humidity");
    //   assert(Sensor::count() == 2);
    //   Sensor::for_each([](Sensor* s) { std::cout << s->name; });
    // }
    // assert(Sensor::count() == 0);
    std::cout << "  Exercise 4: Registry — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 5 (Challenge): Write a CRTP `Builder<Derived, Product>`:
//   - Each setter returns Derived& for method chaining
//   - build() returns Product
// Use it to create a PersonBuilder → Person.
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename Derived, typename Product>
//       class Builder { ... };

void exercise_builder()
{
    // auto person = PersonBuilder()
    //     .name("Alice")
    //     .age(30)
    //     .email("alice@example.com")
    //     .build();
    // assert(person.name == "Alice");
    // assert(person.age == 30);
    std::cout << "  Exercise 5: Builder — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Template 08 — Exercises: CRTP                   ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    exercise_stringify();
    exercise_singleton();
    exercise_shapes();
    exercise_registry();
    exercise_builder();

    std::cout << "\nAll exercises done!\n";
    return 0;
}
