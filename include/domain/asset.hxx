#pragma once

#include <odb/core.hxx>
#include <string>

#pragma db object table("assets")
class Asset {
public:
    long id() const {
        return id_;
    }
    const std::string& inventoryNumber() const {
        return inventoryNumber_;
    }
    const std::string& model() const {
        return model_;
    }
    const std::string& category() const {
        return category_;
    }
    bool isAssigned() const {
        return assigned_;
    }

    void setInventoryNumber(const std::string& value) {
        inventoryNumber_ = value;
    }
    void setModel(const std::string& value) {
        model_ = value;
    }
    void setCategory(const std::string& value) {
        category_ = value;
    }
    void setAssigned(bool value) {
        assigned_ = value;
    }

private:
    friend class odb::access;

#pragma db id auto
    long id_ = 0;

#pragma db not_null unique column("inventory_number")
    std::string inventoryNumber_;

#pragma db not_null column("model")
    std::string model_;

#pragma db not_null column("category")
    std::string category_;

#pragma db not_null column("assigned")
    bool assigned_ = false;
};
