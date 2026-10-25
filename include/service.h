#pragma once
#include "domain.hxx"
#include <odb/pgsql/database.hxx>
#include <vector>
#include <string>

struct AssetRow {
    Asset asset;
    std::string holder;
};

class Service {
public:
    explicit Service(odb::pgsql::database& db) : db_(db) {}

    std::vector<Employee> employees();
    void addEmployee(const std::string& name, const std::string& email);
    void setEmployeeActive(long id, bool active);
    void updateEmployee(long id, const std::string& name,
        const std::string& email, bool active);

    std::vector<AssetRow> assets();
    void addAsset(const std::string& inv, const std::string& model,
        const std::string& category);

    void issue(long assetId, long employeeId);
    void giveBack(long assetId);

    std::vector<Employee> activeEmployees();
    std::vector<Asset>    freeAssets();

private:
    odb::pgsql::database& db_;
};
