#include <functional>
#include <iostream>
#include <source_location>
#include <type_traits>
#include <sstream>
#include <system_error>

void panic(const char* message, const std::source_location location = std::source_location::current())
{
    std::cerr << location.file_name() << '('
              << location.line() << ':'
              << location.column() << ") `"
              << location.function_name() << "`: "
              << message << std::endl;
    exit(static_cast<int>(std::errc::invalid_argument));
}

template<typename T>
class Owned
{
public:
    Owned() = default;
    virtual ~Owned() = default;
    virtual T& Owner() = 0;
};

class Engine;

template <typename Class, typename Result, typename... Args>
auto result_type(Result (Class::*)(Args...)) -> Result;

template <typename T>
concept IsObject = requires () {
    // Function exists
    std::is_member_pointer_v<decltype(&T::Create)>;
    std::is_member_pointer_v<decltype(&T::Destroy)>;
    std::is_member_pointer_v<decltype(&T::Recreate)>;
    // Function return type
    requires std::same_as<decltype(result_type(&T::Create)), void>;
    requires std::same_as<decltype(result_type(&T::Destroy)), void>;
    requires std::same_as<decltype(result_type(&T::Recreate)), void>;
    // Member variables
    // { &T::Stump } -> std::same_as<int T::*>;
    requires std::is_base_of_v<Owned<Engine>, T>;
};

class Window : public virtual Owned<Engine>
{
public:
    Window() = default;
    void Create() {
        printf("Window::Ctor\n");
    }
    void Destroy() {}
    void Recreate() {}
    void Greet()
    {
        printf("Window: Hello world!\n");
    }
};

class Device : public virtual Owned<Engine>
{
public:
    Device() = default;
    void Create(int x);
    void Destroy() {}
    void Recreate() {}
    void Greet()
    {
        printf("Device: Hello world!\n");
    }
};

template<IsObject T>
class Handler : public T
{
    std::function<Engine&()> fn;
    std::function<Engine&()> RefToLambda(Engine& engine) {
        return [&engine] -> Engine& {
            return engine;
        };
    }
public:
    explicit Handler(Engine& engine)
        : fn(RefToLambda(engine))
    {}
    Engine& Owner() override
    {
        return fn();
    }
};

class Engine
{
public:
    Handler<Window> window;
    Handler<Device> device;
    Engine()
    : window(*this)
    , device(*this)
    {
        printf("Engine::Ctor\n");
        device.Greet();
        device.Create(1);
    }
};

void Device::Create(int x)
{
    printf("Device::Ctor(x = %i)\n", x);
    Owner().window.Greet();
}



int main()
{
    Engine e;
    return 0;
}