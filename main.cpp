#include "logging.h"

class A {
    public:
        A() {
            log = LogManager::get_logging() -> get_logger("AClass");
            log -> debug() << "Hello A: " << 3.14 << '\n';
            log -> info() << "Hello A: " << 3.14 << '\n';
            log -> warning() << "Hello A: " << 3.14 << '\n';
            log -> error() << "Hello A: " << 3.14 << '\n';
            log -> critical() << "Hello A: " << 3.14 << '\n';
        }

    private:
        Logger      *log;
};

class B {
    public:
        B() {
            log = LogManager::get_logging() -> get_logger("BClass");
            log -> set_param("SESSION", "b_session");

            log -> debug() << "Hello B: " << 3.14 << '\n';
            log -> info() << "Hello B: " << 3.14 << '\n';
            log -> warning() << "Hello B: " << 3.14 << '\n';
            log -> error() << "Hello B: " << 3.14 << '\n';
            log -> critical() << "Hello B: " << 3.14 << '\n';
        }

    private:
        Logger      *log;
};

int main(int c, char **v) {

    // logging init
    LogManager      *lg = LogManager::get_logging();
    Formatter       *f = new Formatter("[TIME] [LEVEL:10] [NAME:15] [SESSION:10]    [MESSAGE]");
    ConsoleHandler  *ch = new ConsoleHandler();
    FileHandler     *fh = new FileHandler("test.log");

    f -> set_time_format("%Y-%m-%d %H:%M:%S,");
    ch -> set_formatter(f);
    fh -> set_formatter(f);
    lg -> add_handler(ch);
    lg -> add_handler(fh);

    // Test
    A   a;
    B   b;

    return 0;
}
