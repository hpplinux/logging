#include <iostream>
#include <ratio>
#include <chrono>
#include <stdexcept>
using namespace std;
using namespace std::chrono;

#include "logging.h"

static map<Level, string>   level_info = {
    {DEBUG, "DEBUG" },
    {INFO, "INFO" },
    {WARNING, "WARNING" },
    {_ERROR, "ERROR" },
    {CRITICAL, "CRITICAL" },
};


/********************************************/
/*               Formatter                  */
/********************************************/
Formatter::Formatter(string f)
    : format(f) {
    set_time_format("%Y-%m-%d %H:%M:%S,");
}

void Formatter::set_time_format(string f) {
    time_format = f;
}

string Formatter::get_string(Record &rec) {
    stringstream    result;
    string          unit;
    string          buf = format;
    size_t          start_index;
    size_t          stop_index;
    int             width;

    while(!buf.empty()) {
        start_index = buf.find("[");
        stop_index = buf.find("]");

        if(start_index == string::npos || stop_index == string::npos) {
            result << buf;
            return result.str();
        }
        unit = buf.substr(start_index + 1, stop_index - start_index - 1);
        result << buf.substr(0, start_index);

        start_index = unit.find(":");
        if(start_index != string::npos) {
            try {
                width = stoi(unit.substr(start_index + 1));
                unit = unit.substr(0, start_index);
            } catch(const std::invalid_argument&) {
                width = 0;
            }
        }
        else {
            width = 0;
        }
        result.width(width);

        if(unit == "TIME") {
            rec[unit] = get_time();
        }

        if(!rec[unit].empty()) {
            result << rec[unit];
        }
        else {
            result << "[" + unit + "]";
        }
        buf.erase(0, stop_index + 1);
    }
    return result.str();
}

string Formatter::get_time() {
    stringstream    ss;
    auto tp = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
    size_t modulo = ms.count() % 1000;
    time_t seconds = std::chrono::duration_cast<std::chrono::seconds>( ms ).count();

#if HAS_STD_PUT_TIME
    ss << std::put_time(localtime(&seconds ), time_format.data() );
#else
    char buf[30];

    if(strftime(buf, sizeof(buf), time_format.data(), localtime(&seconds))) {
        ss << buf;
    }
#endif // HAS_STD_PUT_TIME

    ss.fill( '0' );
    ss.width( 3 );
    ss << modulo;

    return ss.str();
}

/********************************************/
/*             BaseHandler                  */
/********************************************/
BaseHandler::BaseHandler(string str) {
    if(str.empty()) {
        name = "BaseHandler";
    }
    else {
        name = str;
    }
}
BaseHandler::~BaseHandler() {
    if(formatter) {
        delete formatter;
        formatter = 0;
    }
}

void BaseHandler::set_formatter(Formatter *f) {
    formatter = f;
}

void BaseHandler::log(Level l, Record &rec) {
    rec["LEVEL"] = level_info[l];
    string  s = formatter -> get_string(rec);

    try {
        fflush(l, s);
    } catch (exception &e) {
        cerr << "Failed to fflush record in handler <" << name << ">" << endl;
        cerr << "\tdescription: " << e.what() << endl;
        cerr << "\tstring: " << s << endl;
    }
}

/********************************************/
/*                  Logger                  */
/********************************************/
Logger::Logger(string n)
    : is_single_param(false), name(n) {
    stream = new stringstream(ios::in | ios::out | ios::ate);
    _debug = new Log(DEBUG, *stream, this);
    _info = new Log(INFO, *stream, this);
    _warning = new Log(WARNING, *stream, this);
    _error = new Log(_ERROR, *stream, this);
    _critical = new Log(CRITICAL, *stream, this);
}

Logger::~Logger() {
    if(_debug) {
        delete _debug;
        _debug = 0;
    }
    if(_info) {
        delete _info;
        _info = 0;
    }
    if(_warning) {
        delete _warning;
        _warning = 0;
    }
    if(_error) {
        delete _error;
        _error = 0;
    }
    if(_critical) {
        delete _critical;
        _critical = 0;
    }
}

void Logger::add_handler(BaseHandler *hd) {
    handlers.push_back(hd);
}

void Logger::fflush(Level l, Record &rec) {
    rec.insert(append_rec.begin(), append_rec.end());
    rec["NAME"] = name;
    for(auto it = handlers.begin(); it != handlers.end(); ++it) {
        (*it) -> log(l, rec);
    }
    if(is_single_param) {
        restore_param();
    }
}

Log& Logger::debug() {
    return *_debug;
}

Log& Logger::info() {
    return *_info;
}

Log& Logger::warning() {
    return *_warning;
}

Log& Logger::error() {
    return *_error;
}

Log& Logger::critical() {
    return *_critical;
}

/********************************************/
/*                      Log                 */
/********************************************/
Log::Log(Level l, stringstream &ss, Logger *log)
    : buffer(ss), level(l), logger(log) {
}

Log::~Log() {
    if(!buffer.str().empty() && buffer.str().back() != '\n') {
        buffer << '\n';
        fflush();
    }
}

void Log::fflush() {
    if(level < LogManager::get_level()) {
        return;
    }
    Record  rec;
    string  current_str = buffer.str();
    size_t  endl_index = current_str.find('\n');
    while(endl_index != string::npos) {
        rec["MESSAGE"] = current_str.substr(0, endl_index + 1);
        logger -> fflush(level, rec);
        current_str = current_str.substr(endl_index + 1);
        endl_index = current_str.find('\n');
    }
    buffer.str(current_str);
}

/********************************************/
/*                  Хендлеры                */
/********************************************/
// Консольный
ConsoleHandler::ConsoleHandler():
    BaseHandler("ConsoleHandler") {
}

ConsoleHandler::~ConsoleHandler() {
}

void ConsoleHandler::fflush(Level l, string &str) {
    switch(l) {
        case DEBUG:
            str = "\033[32m" + str + "\033[0m";             // Зеленый
            break;

        case INFO:
            str = "\033[36m" + str + "\033[0m";           // Cyan
            break;

        case WARNING:
            str = "\033[33;1m" + str + "\033[0m";           // Желтый
            break;

        case _ERROR:
            str = "\033[31;1m" + str + "\033[0m";           // Красный
            break;

        case CRITICAL:
            str = "\033[35;1m" + str + "\033[0m";           // Magenta
            break;
    }
    cout << str;
}

// Файловый
FileHandler::FileHandler(string f)
    : BaseHandler("FileHandler"), file_path(f) {
    fs.open(f, fstream::in | fstream::out | fstream::app);
}

FileHandler::~FileHandler() {
    fs.close();
}

void FileHandler::fflush(Level, string &str) {
    fs << str;
    fs.flush();
}

/********************************************/
/*                  Logging                 */
/********************************************/
LogManager* LogManager::base = 0;
Level       LogManager::custom_level = DEBUG;

LogManager* LogManager::get_logging() {
    if(base == 0) {
        base = new LogManager();
    }
    return base;
}

LogManager::LogManager() {
}

LogManager::~LogManager() {
    for(auto it = handlers.begin(); it != handlers.end(); ++it) {
        delete *it;
    }
    if(base) {
        delete base;
        base = 0;
    }
}

void LogManager::add_handler(BaseHandler *hd) {
    handlers.push_back(hd);
}

void LogManager::set_level(Level l) {
    custom_level = l;
}

Level LogManager::get_level() {
    return custom_level;
}

Logger* LogManager::get_logger(string name) {
    try {
        Logger  *lg = loggers[name];
        if(!lg) {
            throw out_of_range("Key \"" + name + "\" not found");
        }
        return lg;
    }
    catch (out_of_range &) {
        Logger  *lg = new Logger(name);
        loggers[name] = lg;
        for(auto it = handlers.begin(); it != handlers.end(); ++it) {
            lg -> add_handler(*it);
        }
        return lg;
    }
}
