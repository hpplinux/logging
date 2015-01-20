#ifndef LOGGING_H
#define LOGGING_H

#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <list>
using namespace std;

typedef map<string, string>     Record;

typedef enum _Level {
    DEBUG       = 0x00,
    INFO        = 0x01,
    WARNING     = 0x02,
    _ERROR       = 0x03,
    CRITICAL    = 0x04
} Level;

// Форматтер
class Formatter {
    public:
        Formatter(string);

        string get_string(Record &);
        string get_time();
        void set_time_format(string);

    private:
        string          format;
        string          time_format;
};

// Обработчик
class BaseHandler {
    public:
        BaseHandler(string = string());
        virtual ~BaseHandler();

        void set_formatter(Formatter *);
        virtual void log(Level, Record &);

    protected:
        virtual void fflush(Level, string &) = 0;

    protected:
        Formatter       *formatter;
        string          name;
};

class Logger;
// Менеджер
class Log {
    public:
        Log(Level, stringstream &, Logger *);
        ~Log();

        template<typename T>
        Log& operator<<(T v) {
            buffer << v;
            fflush();
            return *this;
        }

        void fflush();

    private:
        stringstream    &buffer;
        Level           level;
        Logger          *logger;
};

class Logger {
    public:
        Logger(string);
        ~Logger();

        void add_handler(BaseHandler*);
        void fflush(Level, Record &);

    public:
        Log& debug();
        Log& info();
        Log& warning();
        Log& error();
        Log& critical();

        template <typename T>
        void set_param(string pname, T pvalue) {
            stringstream ss;
            ss << pvalue;
            last_param_name = pname;
            last_param_value = append_rec[pname];
            append_rec[pname] = ss.str();
        }
        template <typename T>
        void set_single_param(string pname, T pvalue) {
            set_param(pname, pvalue);
            is_single_param = true;
        }
        void restore_param() {
            append_rec[last_param_name] = last_param_value;
            is_single_param = false;
        }

        inline void remove_param(string pname) {
            append_rec.erase(pname);
        }

    private:
        bool                is_single_param;
        string              last_param_name;
        string              last_param_value;
        string              name;
        stringstream        *stream;
        list<BaseHandler*>  handlers;
        Log                 *_debug;
        Log                 *_info;
        Log                 *_warning;
        Log                 *_error;
        Log                 *_critical;
        Record              append_rec;
};

// Базовый класс для работы с системой логирования
class LogManager {
    public:
        static LogManager* get_logging();
        void add_handler(BaseHandler *);
        static void set_level(Level);
        static Level get_level();
        Logger* get_logger(string = "ROOT");


    private:
        LogManager();
        ~LogManager();

    private:
        static LogManager       *base;
        list<BaseHandler*>      handlers;
        static Level            custom_level;
        map<string, Logger*>    loggers;
};

// Хендлеры
// Консольный
class ConsoleHandler : public BaseHandler {
    public:
        ConsoleHandler();
        virtual ~ConsoleHandler();

    protected:
        virtual void fflush(Level, string &);
};

// Файловый
class FileHandler : public BaseHandler {
    public:
        FileHandler(string);
        virtual ~FileHandler();

    protected:
        virtual void fflush(Level, string &);

    private:
        string      file_path;
        fstream     fs;
};

#endif // LOGGING_H
