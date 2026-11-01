#pragma once

#include <odb/core.hxx>

#include <string>

#pragma db object table("employees")
class Employee {
public:
    long id() const {
        return id_;
    }

    const std::string& fullName() const {
        return fullName_;
    }

    const std::string& email() const {
        return email_;
    }

    bool isActive() const {
        return active_;
    }

    void setFullName(const std::string& value) {
        fullName_ = value;
    }

    void setEmail(const std::string& value) {
        email_ = value;
    }

    void setActive(bool value) {
        active_ = value;
    }

private:
    friend class odb::access;

#pragma db id auto
    long id_ = 0;

#pragma db not_null column("full_name")
    std::string fullName_;

#pragma db not_null unique column("email")
    std::string email_;

#pragma db not_null column("active")
    bool active_ = true;
};
