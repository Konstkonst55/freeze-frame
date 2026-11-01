#pragma once

#include "domain.hxx"

#include <odb/pgsql/database.hxx>

#include <string>
#include <vector>

struct AssetRow {
    Asset asset;
    std::string holder;
};

struct PagedAssets {
    std::vector<AssetRow> items;
    bool hasNext = false;
};

struct HistoryEntry {
    Assignment assignment;
    std::string employeeName;
};

class Service {
public:
    explicit Service(odb::pgsql::database& db) : db_(db) {
    }

    std::vector<Employee> employees();
    void addEmployee(const std::string& name, const std::string& email);
    void updateEmployee(long id, const std::string& name, const std::string& email, bool active);

    std::vector<AssetRow> assets();
    void addAsset(const std::string& inv, const std::string& model, const std::string& category);

    void issue(long assetId, long employeeId);
    void giveBack(long assetId);

    std::vector<Employee> activeEmployees();
    std::vector<Asset> freeAssets();

    void duplicateAsset(long sourceId, const std::string& newInv);
    void issueMultiple(const std::vector<long>& assetIds, long employeeId);
    std::vector<Asset> searchAssets(const std::string& category, bool onlyFree, const std::string& text);
    PagedAssets listAssetsPaged(int page, int pageSize, const std::string& statusFilter = "");
    std::vector<HistoryEntry> assetHistory(long assetId);
    std::vector<Asset> employeeAssets(long employeeId);
    void reassignAsset(long assetId, long newEmployeeId);
    void deleteAssetCascade(long assetId);

private:
    odb::pgsql::database& db_;
};
