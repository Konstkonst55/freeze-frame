#include "service.h"

#include "employee-odb.hxx"
#include "asset-odb.hxx"
#include "assignment-odb.hxx"

#include <odb/query.hxx>
#include <odb/transaction.hxx>

#include <ctime>
#include <memory>
#include <string>
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

void Service::updateEmployee(long id,
                             const std::string& name,
                             const std::string& email,
                             bool active) {
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
        odb::result<Assignment> assignments =
            db_.query<Assignment>(QAsg::employeeId == id);

        for (auto it = assignments.begin(); it != assignments.end(); ++it) {
            if ((*it).isActive()) {
                throw DomainError("У фотографа на руках техника — сначала примите возврат");
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
        odb::result<Assignment> assignments =
            db_.query<Assignment>(QAsg::assetId == asset.id());

        for (auto assignmentIt = assignments.begin();
             assignmentIt != assignments.end();
             ++assignmentIt) {
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

void Service::addAsset(const std::string& inventoryNumber,
                       const std::string& model,
                       const std::string& category) {
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
    odb::result<Assignment> assignments =
        db_.query<Assignment>(QAsg::assetId == assetId);
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
