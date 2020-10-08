#ifndef ERROR_H
#define ERROR_H

#include "types.h"

#include <memory>
#include <ostream>

namespace toggl {

    class ErrorBase {
    public:
        virtual std::string Class() const = 0;
        virtual int Type() const { return 0; }
        virtual bool IsError() const { return true; };
        virtual std::string LogMessage() const = 0;
        virtual std::string UserMessage() const { return {}; }

        bool operator==(const ErrorBase &o) const {
            return IsError() == o.IsError() &&
                    UserMessage() == o.UserMessage();
        }
    };

    class NoError final : public ErrorBase {
    public:
        std::string Class() const override { return "NoError"; }
        bool IsError() const override { return false; }
        std::string LogMessage() const override { return {}; }
        std::string UserMessage() const override { return {}; }
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
        template<class T> std::shared_ptr<T> promote() {
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
