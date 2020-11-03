#ifndef ERROR_H
#define ERROR_H

#include "types.h"

#include <map>
#include <memory>
#include <ostream>

namespace toggl {

    class ErrorBase {
    public:
        enum ReservedEnumValues {
            _NO_TYPE = 0,
            _CUSTOM_MESSAGE,
            FIRST_AVAILABLE_ENUM,
        };
        virtual std::string Class() const = 0;
        virtual int Type() const { return 0; }
        virtual bool IsError() const { return true; };
        virtual std::string LogMessage() const = 0;
        virtual std::string UserMessage() const { return {}; }

        bool operator==(const ErrorBase &o) const {
            if (!IsError() && !o.IsError())
                return true;
            return IsError() == o.IsError() &&
                    UserMessage() == o.UserMessage() &&
                    Class() == o.Class() &&
                    Type() == o.Type();
        }
    };

    inline static const std::multimap<int, std::string> EnumBasedErrorNoRegexes;
    template <class Enum, const std::map<int, std::string> &Messages, const std::multimap<int, std::string> &Regexes = EnumBasedErrorNoRegexes>
    class EnumBasedError : public ErrorBase {
    public:
        EnumBasedError(const std::string &custom_message = {})
            : ErrorBase()
        {
            bool found = false;
            for (auto &i : Messages) {
                if (i.second == custom_message) {
                    found = true;
                    type_ = i.first;
                }
            }
            if (!found && !custom_message.empty()) {
                type_ = _CUSTOM_MESSAGE;
                custom_message_ = custom_message;
            }
        }
        explicit EnumBasedError(Enum type)
            : ErrorBase()
            , type_(type)
        {

        }
        explicit EnumBasedError(const EnumBasedError &o) = default;
        explicit EnumBasedError(EnumBasedError &&o) = default;
        EnumBasedError &operator=(const EnumBasedError &o) = default;

        int Type() const override { return type_; }
        bool IsError() const override { return type_ != _NO_TYPE; }
        std::string LogMessage() const override { return UserMessage(); }
        std::string UserMessage() const override {
            if (Type() == _CUSTOM_MESSAGE)
                return custom_message_;
            if (Type() != _NO_TYPE) {
                if (Messages.find(type_) != Messages.end()) {
                    return Messages.at(type_);
                }
                else
                    return "Unexpected error";
            }
            return {};
        }

        bool Clear() {
            if (custom_message_.empty()) {
                if (type_ == _NO_TYPE)
                    return false;
            }
            type_ = _NO_TYPE;
            custom_message_.clear();
            return true;
        }
    protected:
        int type_ { _NO_TYPE };
        std::string custom_message_ {};
    };

    class NoError final : public ErrorBase {
    public:
        std::string Class() const override { return "NoError"; }
        bool IsError() const override { return false; }
        std::string LogMessage() const override { return {}; }
    };

    class GenericError : public ErrorBase {
    public:
        GenericError(const std::string &log_message, const std::string &user_message)
            : ErrorBase()
            , log_message_(log_message)
            , user_message_(user_message)
        {}
        virtual ~GenericError() {}
        std::string Class() const override { return "GenericError"; }
        bool IsError() const override { return true; }
        std::string LogMessage() const override { return log_message_; }
        std::string UserMessage() const override { return user_message_; }
    protected:
        std::string log_message_;
        std::string user_message_;
    };

    class Error {
    public:
        template<class T> Error(T &&e) : data_(std::make_shared<T>(std::move(e))) { }
        template<class T> Error(const T &e) : data_(std::make_shared<T>(e)) { }
        Error(Error &&o) : data_(std::move(o.data_)) { }
        Error(const Error &o) : data_(o.data_) { }
        Error &operator=(const Error &o) {
            data_ = o.data_;
            return *this;
        }
        ErrorBase *operator->() { return data_.get(); }
        const ErrorBase *operator->() const { return data_.get(); }
        ErrorBase &operator*() { return *data_.get(); }
        const ErrorBase &operator*() const { return *data_.get(); }
        bool operator==(const Error &o) const {
            return *data_ == *o.data_;
        }
        operator std::string() const {
            return data_->LogMessage();
        }
        /**
         * This method is to be used to access the actual derived type,
         * not just the stuff that's exposed by the ErrorBase interface
         *
         * @return a shared pointer to a derived error type
         */
        template<class T> std::shared_ptr<T> promote() const {
            return std::dynamic_pointer_cast<T>(data_);
        }
    private:
        std::shared_ptr<ErrorBase> data_;
    };

    inline std::ostream &operator<<(std::ostream &out, const Error &err) {
        out << err->UserMessage();
        out << std::string(" ");
        out << err->LogMessage();
        return out;
    }

    /* TODO WARNING
     * These operators exist only to provide backward compatibility with the string-based error class
     * They should be removed once all of string-based error is gone
     */
    inline bool operator==(const Error &l, const std::string &r) {
        return l->LogMessage() == r;
    }
    inline bool operator!=(const Error &l, const std::string &r) {
        return l->LogMessage() != r;
    }
    inline bool operator==(const std::string &l, const Error &r) {
        return l == r->LogMessage();
    }
    inline bool operator!=(const std::string &l, const Error &r) {
        return l != r->LogMessage();
    }
}

#endif // ERROR_H
