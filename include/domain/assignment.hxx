#pragma once

#include <ctime>
#include <odb/core.hxx>
#include <odb/nullable.hxx>

#pragma db object table("assignments")
class Assignment {
public:
    long id() const {
        return id_;
    }
    long assetId() const {
        return assetId_;
    }
    long employeeId() const {
        return employeeId_;
    }
    std::time_t issuedAt() const {
        return issuedAt_;
    }
    std::time_t returnedAt() const {
        return returnedAt_.null() ? 0 : returnedAt_.get();
    }
    bool isActive() const {
        return returnedAt_.null();
    }

    void setAssetId(long value) {
        assetId_ = value;
    }
    void setEmployeeId(long value) {
        employeeId_ = value;
    }
    void setIssuedAt(std::time_t value) {
        issuedAt_ = value;
    }
    void setReturnedAt(std::time_t value) {
        returnedAt_ = value;
    }

private:
    friend class odb::access;

#pragma db id auto
    long id_ = 0;

#pragma db not_null column("asset_id")
    long assetId_ = 0;

#pragma db not_null column("employee_id")
    long employeeId_ = 0;

#pragma db not_null column("issued_at")
    std::time_t issuedAt_ = 0;

#pragma db null column("returned_at")
    odb::nullable<std::time_t> returnedAt_;
};
