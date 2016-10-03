#include "logging.h"

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

    Logger *log = LogManager::get_logging() -> get_logger("Test");

    for(unsigned long long i = 0; i < 350000; ++i) {
        log -> debug() << "aaaaaaaaaaaaaaaaaaaaaaaaa" << 234 << "333333333" << 456465 << "\n";
        log -> info() << "aaaaaaaaaaaaaaaaaaaaaaaaa" << 234 << "333333333" << 456465 << "\n";
        log -> warning() << "aaaaaaaaaaaaaaaaaaaaaaaaa" << 234 << "333333333" << 456465 << "\n";
        log -> error() << "aaaaaaaaaaaaaaaaaaaaaaaaa" << 234 << "333333333" << 456465 << "\n";
        log -> critical() << "aaaaaaaaaaaaaaaaaaaaaaaaa" << 234 << "333333333" << 456465 << "\n";
    }

    return 0;
}
