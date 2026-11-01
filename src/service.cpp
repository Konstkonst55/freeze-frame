#include "service.h"

#include "asset-odb.hxx"
#include "assignment-odb.hxx"
#include "employee-odb.hxx"

#include <odb/query.hxx>
#include <odb/transaction.hxx>

#include <algorithm>
#include <ctime>
#include <memory>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

using QEmp = odb::query<Employee>;
using QAst = odb::query<Asset>;
using QAsg = odb::query<Assignment>;

std::vector<Employee> Service::employees() {
    odb::transaction transaction(db_.begin());
    std::vector<Employee> employees;
    odb::result<Employee> result = db_.query<Employee>();

    for (auto it = result.begin(); it != result.end(); ++it) {
        employees.push_back(*it);
    }

    transaction.commit();
    return employees;
}

void Service::addEmployee(const std::string& name, const std::string& email) {
    odb::transaction transaction(db_.begin());

    if (!db_.query<Employee>(QEmp::email == email).empty()) {
        throw DomainError("Фотограф с email " + email + " уже существует");
    }

    Employee employee;
    employee.setFullName(name);
    employee.setEmail(email);
    employee.setActive(true);
    db_.persist(employee);

    transaction.commit();
}

void Service::updateEmployee(long id, const std::string& name, const std::string& email, bool active) {
    odb::transaction transaction(db_.begin());
    std::unique_ptr<Employee> employee(db_.load<Employee>(id));

    if (employee->email() != email) {
        odb::result<Employee> matches = db_.query<Employee>(QEmp::email == email);

        for (auto it = matches.begin(); it != matches.end(); ++it) {
            if ((*it).id() != id) {
                throw DomainError("Фотограф с email " + email + " уже существует");
            }
        }
    }

    if (!active) {
        odb::result<Assignment> assignments = db_.query<Assignment>(QAsg::employeeId == id);

        for (auto it = assignments.begin(); it != assignments.end(); ++it) {
            if ((*it).isActive()) {
                throw DomainError(
                        "У фотографа на руках техника — "
                        "сначала примите возврат");
            }
        }
    }

    employee->setFullName(name);
    employee->setEmail(email);
    employee->setActive(active);
    db_.update(*employee);

    transaction.commit();
}

std::vector<AssetRow> Service::assets() {
    odb::transaction transaction(db_.begin());
    std::vector<AssetRow> rows;
    odb::result<Asset> assets = db_.query<Asset>();

    for (auto it = assets.begin(); it != assets.end(); ++it) {
        Asset asset = *it;
        std::string holder;
        odb::result<Assignment> assignments = db_.query<Assignment>(QAsg::assetId == asset.id());

        for (auto assignmentIt = assignments.begin(); assignmentIt != assignments.end(); ++assignmentIt) {
            Assignment assignment = *assignmentIt;

            if (assignment.isActive()) {
                std::unique_ptr<Employee> employee(db_.load<Employee>(assignment.employeeId()));
                holder = employee->fullName();
                break;
            }
        }

        rows.push_back({asset, holder});
    }

    transaction.commit();
    return rows;
}

void Service::addAsset(const std::string& inventoryNumber, const std::string& model, const std::string& category) {
    odb::transaction transaction(db_.begin());

    if (!db_.query<Asset>(QAst::inventoryNumber == inventoryNumber).empty()) {
        throw DomainError("Инвентарный номер " + inventoryNumber + " уже существует");
    }

    Asset asset;
    asset.setInventoryNumber(inventoryNumber);
    asset.setModel(model);
    asset.setCategory(category);
    asset.setAssigned(false);
    db_.persist(asset);

    transaction.commit();
}

void Service::issue(long assetId, long employeeId) {
    odb::transaction transaction(db_.begin());
    std::unique_ptr<Asset> asset(db_.load<Asset>(assetId));
    std::unique_ptr<Employee> employee(db_.load<Employee>(employeeId));

    if (asset->isAssigned()) {
        throw DomainError("Техника уже выдана");
    }

    if (!employee->isActive()) {
        throw DomainError("Нельзя выдать уволенному фотографу");
    }

    asset->setAssigned(true);
    db_.update(*asset);

    Assignment assignment;
    assignment.setAssetId(assetId);
    assignment.setEmployeeId(employeeId);
    assignment.setIssuedAt(std::time(nullptr));
    db_.persist(assignment);

    transaction.commit();
}

void Service::giveBack(long assetId) {
    odb::transaction transaction(db_.begin());
    odb::result<Assignment> assignments = db_.query<Assignment>(QAsg::assetId == assetId);
    bool found = false;
    Assignment activeAssignment;

    for (auto it = assignments.begin(); it != assignments.end(); ++it) {
        Assignment assignment = *it;

        if (assignment.isActive()) {
            activeAssignment = assignment;
            found = true;
            break;
        }
    }

    if (!found) {
        throw DomainError("Нет активной выдачи для этой техники");
    }

    activeAssignment.setReturnedAt(std::time(nullptr));
    db_.update(activeAssignment);

    std::unique_ptr<Asset> asset(db_.load<Asset>(assetId));
    asset->setAssigned(false);
    db_.update(*asset);

    transaction.commit();
}

std::vector<Employee> Service::activeEmployees() {
    odb::transaction transaction(db_.begin());
    std::vector<Employee> employees;
    odb::result<Employee> result = db_.query<Employee>(QEmp::active == true);

    for (auto it = result.begin(); it != result.end(); ++it) {
        employees.push_back(*it);
    }

    transaction.commit();
    return employees;
}

std::vector<Asset> Service::freeAssets() {
    odb::transaction transaction(db_.begin());
    std::vector<Asset> assets;
    odb::result<Asset> result = db_.query<Asset>(QAst::assigned == false);

    for (auto it = result.begin(); it != result.end(); ++it) {
        assets.push_back(*it);
    }

    transaction.commit();
    return assets;
}

void Service::duplicateAsset(long sourceId, const std::string& newInv) {
    odb::transaction transaction(db_.begin());

    if (!db_.query<Asset>(QAst::inventoryNumber == newInv).empty()) {
        throw DomainError("Инвентарный номер " + newInv + " уже существует");
    }

    std::unique_ptr<Asset> source(db_.load<Asset>(sourceId));

    Asset copy;
    copy.setInventoryNumber(newInv);
    copy.setModel(source->model());
    copy.setCategory(source->category());
    copy.setAssigned(false);
    db_.persist(copy);

    transaction.commit();
}

void Service::issueMultiple(const std::vector<long>& assetIds, long employeeId) {
    if (assetIds.empty()) {
        throw DomainError("Не выбрано ни одной единицы техники");
    }

    odb::transaction transaction(db_.begin());
    std::unique_ptr<Employee> employee(db_.load<Employee>(employeeId));

    if (!employee->isActive()) {
        throw DomainError("Нельзя выдать уволенному фотографу");
    }

    std::vector<std::unique_ptr<Asset>> assets;
    assets.reserve(assetIds.size());
    std::unordered_set<long> processedIds;

    for (long assetId : assetIds) {
        if (!processedIds.insert(assetId).second) {
            continue;
        }

        std::unique_ptr<Asset> asset(db_.load<Asset>(assetId));

        if (asset->isAssigned()) {
            throw DomainError("Техника " + asset->inventoryNumber() + " уже выдана");
        }

        assets.push_back(std::move(asset));
    }

    const std::time_t issuedAt = std::time(nullptr);

    for (auto& asset : assets) {
        asset->setAssigned(true);
        db_.update(*asset);

        Assignment assignment;
        assignment.setAssetId(asset->id());
        assignment.setEmployeeId(employeeId);
        assignment.setIssuedAt(issuedAt);
        db_.persist(assignment);
    }

    transaction.commit();
}

std::vector<Asset> Service::searchAssets(const std::string& category, bool onlyFree, const std::string& text) {
    odb::transaction transaction(db_.begin());
    odb::query<Asset> query = odb::query<Asset>::true_expr;

    if (!category.empty()) {
        query = query && (QAst::category == category);
    }

    if (onlyFree) {
        query = query && (QAst::assigned == false);
    }

    if (!text.empty()) {
        const std::string pattern = "%" + text + "%";
        query = query && (QAst::model.like(pattern) || QAst::inventoryNumber.like(pattern));
    }

    std::vector<Asset> assets;
    odb::result<Asset> result = db_.query<Asset>(query);

    for (auto it = result.begin(); it != result.end(); ++it) {
        assets.push_back(*it);
    }

    std::sort(
            assets.begin(), assets.end(), [](const Asset& left, const Asset& right) { return left.id() > right.id(); });

    transaction.commit();
    return assets;
}

PagedAssets Service::listAssetsPaged(int page, int pageSize, const std::string& statusFilter) {
    odb::transaction transaction(db_.begin());
    PagedAssets pagedAssets;

    odb::query<Asset> query = odb::query<Asset>::true_expr;

    if (statusFilter == "В стойке") {
        query = query && (QAst::assigned == false);
    } else if (statusFilter == "На съёмке") {
        query = query && (QAst::assigned == true);
    }

    std::vector<Asset> allAssets;
    odb::result<Asset> result = db_.query<Asset>(query);

    for (auto it = result.begin(); it != result.end(); ++it) {
        allAssets.push_back(*it);
    }

    std::sort(allAssets.begin(), allAssets.end(), [](const Asset& left, const Asset& right) {
        return left.id() > right.id();
    });

    const int offset = page * pageSize;
    const int end = std::min(offset + pageSize, static_cast<int>(allAssets.size()));

    for (int index = offset; index < end; ++index) {
        AssetRow row;
        row.asset = allAssets[static_cast<std::size_t>(index)];

        if (row.asset.isAssigned()) {
            odb::result<Assignment> assignments
                    = db_.query<Assignment>(QAsg::assetId == row.asset.id() && QAsg::returnedAt.is_null());

            if (!assignments.empty()) {
                const Assignment assignment = *assignments.begin();
                std::unique_ptr<Employee> employee(db_.load<Employee>(assignment.employeeId()));
                row.holder = employee->fullName();
            }
        }

        pagedAssets.items.push_back(row);
    }

    pagedAssets.hasNext = offset + pageSize < static_cast<int>(allAssets.size());

    transaction.commit();
    return pagedAssets;
}

std::vector<HistoryEntry> Service::assetHistory(long assetId) {
    odb::transaction transaction(db_.begin());
    std::vector<HistoryEntry> history;
    odb::result<Assignment> result = db_.query<Assignment>(QAsg::assetId == assetId);

    for (auto it = result.begin(); it != result.end(); ++it) {
        HistoryEntry entry;
        entry.assignment = *it;

        std::unique_ptr<Employee> employee(db_.load<Employee>(entry.assignment.employeeId()));
        entry.employeeName = employee->fullName();
        history.push_back(entry);
    }

    std::sort(history.begin(), history.end(), [](const HistoryEntry& left, const HistoryEntry& right) {
        return left.assignment.issuedAt() > right.assignment.issuedAt();
    });

    transaction.commit();
    return history;
}

std::vector<Asset> Service::employeeAssets(long employeeId) {
    odb::transaction transaction(db_.begin());
    std::vector<Asset> assets;
    odb::result<Assignment> result
            = db_.query<Assignment>(QAsg::employeeId == employeeId && QAsg::returnedAt.is_null());

    for (auto it = result.begin(); it != result.end(); ++it) {
        std::unique_ptr<Asset> asset(db_.load<Asset>(it->assetId()));
        assets.push_back(*asset);
    }

    std::sort(
            assets.begin(), assets.end(), [](const Asset& left, const Asset& right) { return left.id() > right.id(); });

    transaction.commit();
    return assets;
}

void Service::reassignAsset(long assetId, long newEmployeeId) {
    odb::transaction transaction(db_.begin());
    std::unique_ptr<Asset> asset(db_.load<Asset>(assetId));

    if (!asset->isAssigned()) {
        throw DomainError("Техника не выдана — используйте «Выдать»");
    }

    odb::result<Assignment> assignments = db_.query<Assignment>(QAsg::assetId == assetId && QAsg::returnedAt.is_null());

    if (assignments.empty()) {
        throw DomainError("Нет активной выдачи");
    }

    Assignment currentAssignment = *assignments.begin();
    std::unique_ptr<Employee> newEmployee(db_.load<Employee>(newEmployeeId));

    if (!newEmployee->isActive()) {
        throw DomainError("Нельзя переназначить уволенному фотографу");
    }

    if (currentAssignment.employeeId() == newEmployeeId) {
        throw DomainError("Техника уже у этого фотографа");
    }

    const std::time_t now = std::time(nullptr);
    currentAssignment.setReturnedAt(now);
    db_.update(currentAssignment);

    Assignment newAssignment;
    newAssignment.setAssetId(assetId);
    newAssignment.setEmployeeId(newEmployeeId);
    newAssignment.setIssuedAt(now);
    db_.persist(newAssignment);

    transaction.commit();
}

void Service::deleteAssetCascade(long assetId) {
    odb::transaction transaction(db_.begin());
    std::unique_ptr<Asset> asset(db_.load<Asset>(assetId));

    if (asset->isAssigned()) {
        throw DomainError(
                "Нельзя удалить выданную технику — "
                "сначала примите возврат");
    }

    odb::result<Assignment> activeAssignments
            = db_.query<Assignment>(QAsg::assetId == assetId && QAsg::returnedAt.is_null());

    if (!activeAssignments.empty()) {
        throw DomainError(
                "Нельзя удалить выданную технику — "
                "сначала примите возврат");
    }

    db_.erase_query<Assignment>(QAsg::assetId == assetId);
    db_.erase<Asset>(assetId);

    transaction.commit();
}
