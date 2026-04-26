//information system for an automobile insurance company
#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
using namespace std;

int MAX_LEN = 256;

enum ClaimStatus { pending, approved, rejected };

/*
entities
	- customers
	- vehicles
	- insurance details
	- claims
	- inspections
	- staff (salesmen, surveyors, manager)
	- workshops
transactions
	- customer registration
	- vehicle registration
	- issue insurance policy
	- submit claim
	- inspection by surveyor
	- claim approval by manager
	- vehicle repair at registered workshop
*/

/*------------------------------------------------------------------------------------------------------------*/
// Forward declarations — needed so classes can reference each other via pointers
/*------------------------------------------------------------------------------------------------------------*/

class Company;
class Workshop;
class Staff;
class Salesman;
class Surveyor;
class Manager;
class Customer;
class Vehicle;
class InsuranceDetails;
class Claim;
class Inspection;

/*------------------------------------------------------------------------------------------------------------*/
//business layer - classes
/*------------------------------------------------------------------------------------------------------------*/

// Abstract class
// Represents a staff member (Manager, Salesman, Surveyor) in the insurance system
class Staff {
private:
	int staffID;
	char* staffName;
public:
	Staff(int id, char* name) {
		staffID = id;
		staffName = new char[strlen(name) + 1];
		strcpy(staffName, name);
	}
	void display() {
		cout << "staff id : " << staffID << "\n" << "staff name : " << staffName << "\n";
	}
	char* getName() const { return staffName; }
	int getStaffID() const { return staffID; }
};

// Represents a manager in the insurance system.
// Stores personal details and responsibilities.
class Manager : public Staff {
private:
	char* designation;
public:
	Manager(int id, char* name, char* des) : Staff(id, name) {
		designation = new char[strlen(des) + 1];
		strcpy(designation, des);
	}
	void approveClaim(Claim* claim); //implemented after Claim is fully defined
	void displayReport() {}          //requests from Company
	void reviewInspectionReport(Inspection*) {}
};


// Represents a salesman in the insurance system.
// Stores personal details and responsibilities.
class Salesman : public Staff {
private:
	Manager* manager;
public:
	Salesman(int id, char* name, Manager* manager) : Staff(id, name) {
		this->manager = manager;
	}
};


// Represents a surveyor in the insurance system.
// Stores personal details and responsibilities.
class Surveyor : public Staff {
private:
	Manager* manager;
public:
	Surveyor(int id, char* name, Manager* manager) : Staff(id, name) {
		this->manager = manager;
	}
};


// Represents an insurance policy in the insurance system.
// Stores policy details.
class InsuranceDetails {
private:
	int policyNumber;
	char* validUntil;
public:
	InsuranceDetails(int policyNum, char* exp) {
		policyNumber = policyNum;
		validUntil = new char[strlen(exp) + 1];
		strcpy(validUntil, exp);
	}
	int getPolicy() const { return policyNumber; }
	char* getValidUntil() const { return validUntil; }
};

// Represents a customer in the insurance system.
// Stores personal details and links to claims.
class Customer { //owns vehicle, has insurance policies
private:
	int customerID;
	char* name;
	char* address;
	char* phoneNumber;
	vector<Vehicle*> vehicle; //customer can have multiple vehicles
public:
	Customer(int, char*, char*, char*, vector<Vehicle*>);
	void display();
	char* getName() const;
	int getCustomerID() const;
	vector<Vehicle*>& getVehicles();
	char* getAddress();
	char* getPhoneNumber();
	~Customer();
};

// Represents a workshop in the insurance system.
// Stores staff and vehicles.
class Workshop { //contains staff
private:
	char* workshopID;
	char* address;
	vector<Staff*> staffList;
	vector<Vehicle*> vehicleList;
public:
	Workshop(char*, char*, vector<Staff*>, vector<Vehicle*>);
	void assignVehicleWorkshop(Vehicle*);
	char* getID() const;
	~Workshop();
};

// Represents a vehicle in the insurance system.
// Linked to customer.
// CHANGE: removed Workshop* parameter to reduce coupling (IF4 fix)
class Vehicle { //linked to customer
private:
	int vehicleID;
	int year;
	Customer* owner;
	InsuranceDetails* insurancePolicy;
public:
	Vehicle(int, int, Customer*, InsuranceDetails*);
	void issueInsurancePolicy();
	void setInsurancePolicy(InsuranceDetails*);
	void viewInsuranceDetails();
	void display();
	int getVehicleID() const;
	int getYear() const;
	Customer* getOwner() const;
};

// Represents a claim in the insurance system.
// Linked to a vehicle.
class Claim { //raised when vehicle is damaged
private:
	int claimID;
	ClaimStatus status;
	char* date;
	Vehicle* vehicle;
public:
	Claim(int, ClaimStatus, char*, Vehicle*);
	void setStatus(ClaimStatus);
	ClaimStatus getStatus();
	void display();
	Customer* getBelongsTo() const;
	int getClaimID() const;
	char* getDate() const;
};

// Represents an inspection in the insurance system.
// Linked to a surveyor and a claim.
class Inspection { //performed by surveyor, linked to claim
private:
	int inspectionID;
	Surveyor* inspector;
	char* date;
	char* findings;
	Claim* claim;
public:
	Inspection(int, Surveyor*, char*, char*, Claim*);
	void performInspection();
	char* generateInspectionReport(); //returns a cstring
};

/*------------------------------------------------------------------------------------------------------------*/
// persistence layer - data access
// isolate file handling into helper classes instead of putting file I/O inside Customer and Vehicle
/*------------------------------------------------------------------------------------------------------------*/

// Repository for managing Customer objects.
// Provides add, remove, and search functionality.
class CustomerRepository {
private:
	// CHANGE: helper to parse one line from customers.dat (reduces MVG of loadAll)
	Customer* parseLine(char* line) {
		char* next = nullptr;
		char* id = strtok_s(line, "|", &next);
		char* name = strtok_s(nullptr, "|", &next);
		char* addr = strtok_s(nullptr, "|", &next);
		char* phone = strtok_s(nullptr, "|", &next);
		if (id && name && addr && phone) {
			vector<Vehicle*> v;
			return new Customer(atoi(id), name, addr, phone, v);
		}
		return nullptr;
	}
public:
	// Append one customer record to customers.dat
	void save(Customer* c) {
		ofstream out("customers.dat", ios::app);
		if (out) {
			out << c->getCustomerID() << "|"
				<< c->getName() << "|"
				<< c->getAddress() << "|"
				<< c->getPhoneNumber() << "\n";
		}
		else {
			cerr << "cannot open customers.dat\n";
		}
	}

	// Load all customers from customers.dat
	// CHANGE: simplified by delegating line parsing to parseLine()
	vector<Customer*> loadAll() {
		vector<Customer*> customers;
		ifstream in("customers.dat");
		if (!in) { cerr << "cannot open customers.dat\n"; return customers; }
		char line[256];
		while (in.getline(line, 256)) {
			Customer* c = parseLine(line);
			if (c) customers.push_back(c);
		}
		return customers;
	}

	// Find a single customer by ID (scans file)
	Customer* findByID(int targetID) {
		vector<Customer*> all = loadAll();
		for (Customer* c : all)
			if (c->getCustomerID() == targetID) return c;
		return nullptr;
	}

	// List customers whose record line contains the given month string
	void listByMonth(char* month) {
		ifstream in("customers.dat");
		if (!in) { cerr << "cannot open customers.dat\n"; return; }
		char line[256];
		bool found = false;
		while (in.getline(line, 256)) {
			if (strstr(line, month)) {
				cout << line << "\n";
				found = true;
			}
		}
		if (!found) cout << "no customers found for month : " << month << "\n";
	}
};

// Repository for managing Vehicle objects.
// Provides add, remove, and search functionality.
class VehicleRepository {
public:
	// Append one vehicle record to vehicles.dat
	void save(Vehicle* v) {
		ofstream out("vehicles.dat", ios::app);
		if (out) {
			int ownerID = v->getOwner() ? v->getOwner()->getCustomerID() : -1;
			out << v->getVehicleID() << "|"
				<< v->getYear() << "|"
				<< ownerID << "\n";
		}
		else {
			cerr << "cannot open vehicles.dat\n";
		}
	}

	// load all vehicle records (owner pointer is nullptr, caller resolves)
	vector<Vehicle*> loadAll() {
		vector<Vehicle*> vehicles;
		ifstream in("vehicles.dat");
		if (!in) { cerr << "cannot open vehicles.dat\n"; return vehicles; }
		char line[256];
		while (in.getline(line, 256)) {
			char* next = nullptr;
			char* id = strtok_s(line, "|", &next);
			char* year = strtok_s(nullptr, "|", &next);
			if (id && year)
				vehicles.push_back(new Vehicle(atoi(id), atoi(year), nullptr, nullptr));
		}
		return vehicles;
	}
};

// Repository for managing Claim objects.
// Provides add, remove, and search functionality.
class ClaimRepository {
public:
	// Append one claim record to claims.dat
	void save(Claim* c) {
		ofstream out("claims.dat", ios::app);
		if (out) {
			int ownerID = (c->getBelongsTo()) ? c->getBelongsTo()->getCustomerID() : -1;
			out << c->getClaimID() << "|"
				<< c->getStatus() << "|"
				<< c->getDate() << "|"
				<< ownerID << "\n";
		}
		else {
			cerr << "cannot open claims.dat\n";
		}
	}

	// Load all claim records from claims.dat
	vector<Claim*> loadAll() {
		vector<Claim*> claims;
		ifstream in("claims.dat");
		if (!in) { cerr << "cannot open claims.dat\n"; return claims; }
		char line[256];
		while (in.getline(line, 256)) {
			char* next = nullptr;
			char* id = strtok_s(line, "|", &next);
			char* status = strtok_s(nullptr, "|", &next);
			char* date = strtok_s(nullptr, "|", &next);
			if (id && status && date)
				claims.push_back(new Claim(atoi(id), (ClaimStatus)atoi(status), date, nullptr));
		}
		return claims;
	}
};

// Repository for managing Inspection objects.
// Provides save and display functionality.
class InspectionRepository {
public:
	// Append one inspection report to inspections.dat
	void save(Inspection* insp) {
		char* report = insp->generateInspectionReport();
		ofstream out("inspections.dat", ios::app);
		if (out)
			out << report << "\n";
		else
			cerr << "cannot open inspections.dat\n";
		delete[] report;
	}

	// Display all inspection reports from inspections.dat
	void displayAll() {
		ifstream in("inspections.dat");
		if (!in) { cerr << "cannot open inspections.dat\n"; return; }
		char line[256];
		bool found = false;
		while (in.getline(line, 256)) {
			cout << line << "\n";
			found = true;
		}
		if (!found) cout << "no inspection reports found\n";
	}
};

// Repository for managing Company data.
// Provides save and load functionality for company.dat
class CompanyRepository {
public:
	// saves company name and address to company.dat
	void save(char* name, char* address) {
		ofstream out("company.dat");
		if (out)
			out << name << "|" << address << "\n";
		else
			cerr << "cannot open company.dat\n";
	}

	// loads company name and address from company.dat
	void load(char*& name, char*& address) {
		ifstream in("company.dat");
		if (!in) { cerr << "cannot open company.dat\n"; return; }
		char line[256];
		in.getline(line, 256);
		char* next = nullptr;
		char* token = strtok_s(line, "|", &next);
		if (token) { name = new char[strlen(token) + 1]; strcpy(name, token); }
		token = strtok_s(nullptr, "|", &next);
		if (token) { address = new char[strlen(token) + 1]; strcpy(address, token); }
		cout << "company data loaded\n";
	}
};

// storage layer - flat files

/*------------------------------------------------------------------------------------------------------------*/
// COMPANY
/*------------------------------------------------------------------------------------------------------------*/

// Represents the company in the insurance system.
// Stores company details and links to the business classes.
class Company { //contains workshops
private:
	char* companyName;
	char* addressHQ;
	vector<Customer*> customers;
	vector<Workshop*> workshops;
	vector<Claim*> claims;
	int nextVehicleID;
	// CHANGE: repositories removed as members, now created locally inside each method (reduces IF4/coupling)

public:
	Company(char*, char*, vector<Customer*>, vector<Workshop*>, vector<Claim*>);
	void saveCompany();
	void loadCompany();
	void createClaim();

	void displayNewCustomersWon();
	void displayPendingClaims();
	void displayInspectionReports();
	void displayCustomerClaimHistory(Customer*);
	void saveInspection(Inspection*);
	~Company();
	void addCustomer(char*, char*, char*);
	void registerVehicle(Customer*, int, InsuranceDetails*);

	void displayCustomerDetails();
	void listRegisteredCustomers(char*);
	Customer* findCustomerByID(int);
	vector<Customer*> getCustomers() const;
	Vehicle* findVehicleByID(int id);

	vector<Claim*> getClaims() const;
	vector<Workshop*> getWorkshops() const;

	void registerWorkshop(char* workshopID, char* address, vector<Staff*> staff);
};

/*------------------------------------------------------------------------------------------------------------*/
// COMPANY
/*------------------------------------------------------------------------------------------------------------*/

Company::Company(char* name, char* addr, vector<Customer*> c, vector<Workshop*> w, vector<Claim*> cl) {
	cout << "Company Created" << endl;
	companyName = new char[strlen(name) + 1];
	strcpy(companyName, name);

	addressHQ = new char[strlen(addr) + 1];
	strcpy(addressHQ, addr);

	customers = c;
	workshops = w;
	claims = cl;

	nextVehicleID = 1;
}

// CHANGE: repo created locally instead of as member
void Company::saveCompany() {
	CompanyRepository repo;
	repo.save(companyName, addressHQ);
}

// CHANGE: repo created locally instead of as member
void Company::loadCompany() {
	CompanyRepository repo;
	repo.load(companyName, addressHQ);
}

// CHANGE: repo created locally instead of as member
void Company::addCustomer(char* name, char* address, char* phoneNumber) {
	int id = customers.size() + 1;
	vector<Vehicle*> v;
	Customer* c = new Customer(id, name, address, phoneNumber, v);
	customers.push_back(c);
	CustomerRepository repo;
	repo.save(c);
	cout << "customer added successfully with id : " << id << "\n";
}

// CHANGE: removed Workshop* parameter, repo created locally
void Company::registerVehicle(Customer* owner, int year, InsuranceDetails* insurance) {
	int vehicleID = nextVehicleID++;
	Vehicle* v = new Vehicle(vehicleID, year, owner, insurance);
	owner->getVehicles().push_back(v);
	VehicleRepository repo;
	repo.save(v);
	cout << "Vehicle registered with global ID " << vehicleID << "\n";
}

Vehicle* Company::findVehicleByID(int id) {
	for (Customer* cust : customers) {
		for (Vehicle* v : cust->getVehicles()) {
			if (v->getVehicleID() == id) return v;
		}
	}
	return nullptr;
}

void Company::displayCustomerDetails() {
	if (customers.size() == 0) {
		cout << "no customers registered\n";
	}
	else {
		for (int i = 0; i < customers.size(); i++) {
			customers[i]->display();
		}
	}
}

// CHANGE: repo created locally instead of as member
void Company::listRegisteredCustomers(char* month) {
	CustomerRepository repo;
	repo.listByMonth(month);
}

// CHANGE: repo created locally instead of as member
void Company::createClaim() {
	int id = claims.size() + 1;
	char date[50];
	cout << "enter date (dd/mm/yyyy) : ";
	cin >> date;
	cout << "enter vehicle id : ";
	int vid;
	cin >> vid;

	Vehicle* v = nullptr;
	for (int i = 0; i < customers.size(); i++) {
		vector<Vehicle*> vehicles = customers[i]->getVehicles();
		for (int j = 0; j < vehicles.size(); j++) {
			if (vehicles[j]->getVehicleID() == vid) {
				v = vehicles[j];
			}
		}
	}

	if (v) {
		Claim* c = new Claim(id, pending, date, v);
		claims.push_back(c);
		ClaimRepository repo;
		repo.save(c);
		cout << "claim created with id : " << id << "\n";
	}
	else {
		cout << "vehicle not found\n";
	}
}

void Company::displayPendingClaims() {
	bool found = false;
	for (int i = 0; i < claims.size(); i++) {
		if (claims[i]->getStatus() == pending) {
			claims[i]->display();
			found = true;
		}
	}
	if (!found) {
		cout << "no pending claims\n";
	}
}

void Company::displayNewCustomersWon() {
	cout << "total customers registered : " << customers.size() << "\n";
	for (int i = 0; i < customers.size(); i++) {
		customers[i]->display();
	}
}

// CHANGE: repo created locally instead of as member
void Company::displayInspectionReports() {
	InspectionRepository repo;
	repo.displayAll();
}

// CHANGE: repo created locally instead of as member
void Company::saveInspection(Inspection* insp) {
	InspectionRepository repo;
	repo.save(insp);
}

void Company::displayCustomerClaimHistory(Customer* c) {
	if (!c) {
		cout << "invalid customer\n";
	}
	else {
		bool found = false;
		for (int i = 0; i < claims.size(); i++) {
			if (claims[i]->getBelongsTo() == c) {
				claims[i]->display();
				found = true;
			}
		}
		if (!found) {
			cout << "no claims found for customer\n";
		}
	}
}

void Company::registerWorkshop(char* workshopID, char* address, vector<Staff*> staff) {
	vector<Vehicle*> empty;
	Workshop* w = new Workshop(workshopID, address, staff, empty);
	workshops.push_back(w);
	cout << "workshop registered with id : " << workshopID << "\n";
}

Customer* Company::findCustomerByID(int id) {
	for (int i = 0; i < customers.size(); i++) {
		if (customers[i]->getCustomerID() == id) {
			return customers[i];
		}
	}
	return nullptr;
}

vector<Customer*> Company::getCustomers() const { return customers; }
vector<Claim*> Company::getClaims() const { return claims; }
vector<Workshop*> Company::getWorkshops() const { return workshops; }

/*------------------------------------------------------------------------------------------------------------*/
// MANAGER
/*------------------------------------------------------------------------------------------------------------*/

void Manager::approveClaim(Claim* claim) {
	claim->setStatus(approved);
	cout << "Claim approved by manager.\n";
}

/*------------------------------------------------------------------------------------------------------------*/
// CUSTOMER
/*------------------------------------------------------------------------------------------------------------*/

Customer::Customer(int id, char* name, char* address, char* number, vector<Vehicle*> vehicle) {
	customerID = id;

	this->name = new char[strlen(name) + 1];
	strcpy(this->name, name);

	this->address = new char[strlen(address) + 1];
	strcpy(this->address, address);

	this->phoneNumber = new char[strlen(number) + 1];
	strcpy(this->phoneNumber, number);

	this->vehicle = vehicle;
}

// displays customer details to console
void Customer::display() {
	cout << "customer id : " << customerID << "\n"
		<< "name        : " << name << "\n"
		<< "address     : " << address << "\n"
		<< "phone       : " << phoneNumber << "\n";
}

// returns customer address
char* Customer::getAddress() { return this->address; }

// returns customer phone number
char* Customer::getPhoneNumber() { return this->phoneNumber; }

// returns customer name
char* Customer::getName() const { return name; }

// returns customer ID
int Customer::getCustomerID() const { return customerID; }

// returns list of vehicles owned by this customer
vector<Vehicle*>& Customer::getVehicles() { return vehicle; }

/*------------------------------------------------------------------------------------------------------------*/
// VEHICLE
// CHANGE: Workshop* removed from constructor to reduce coupling (IF4 fix)
/*------------------------------------------------------------------------------------------------------------*/

Vehicle::Vehicle(int id, int year, Customer* owner, InsuranceDetails* insurance) {
	vehicleID = id;
	this->year = year;
	this->owner = owner;
	this->insurancePolicy = insurance;
}

void Vehicle::issueInsurancePolicy() {
	if (insurancePolicy) {
		cout << "insurance policy issued for vehicle " << vehicleID << "\n";
	}
	else {
		cout << "no insurance policy linked to vehicle " << vehicleID << "\n";
	}
}

void Vehicle::setInsurancePolicy(InsuranceDetails* policy) { insurancePolicy = policy; }

void Vehicle::viewInsuranceDetails() {
	if (insurancePolicy) {
		cout << "Policy #" << insurancePolicy->getPolicy()
			<< " valid until: " << insurancePolicy->getValidUntil() << "\n";
	}
	else {
		cout << "no policy on record for vehicle " << vehicleID << "\n";
	}
}

void Vehicle::display() {
	if (owner) {
		cout << "vehicle id  : " << vehicleID << "\n"
			<< "year        : " << year << "\n"
			<< "owner       : " << owner->getName() << "\n";
	}
	else {
		cout << "vehicle id  : " << vehicleID << "\n"
			<< "year        : " << year << "\n"
			<< "owner       : n/a\n";
	}
}

int Vehicle::getVehicleID() const { return vehicleID; }
int Vehicle::getYear() const { return year; }
Customer* Vehicle::getOwner() const { return owner; }

/*------------------------------------------------------------------------------------------------------------*/
// CLAIM
/*------------------------------------------------------------------------------------------------------------*/

Claim::Claim(int id, ClaimStatus status, char* date, Vehicle* vehicle) {
	claimID = id;
	this->status = status;
	this->date = new char[strlen(date) + 1];
	strcpy(this->date, date);
	this->vehicle = vehicle;
}

void Claim::setStatus(ClaimStatus s) { status = s; }
ClaimStatus Claim::getStatus() { return status; }
char* Claim::getDate() const { return date; }
int Claim::getClaimID() const { return claimID; }

void Claim::display() {
	cout << "ID: " << claimID << "\nDate: " << date << endl;
}

Customer* Claim::getBelongsTo() const {
	return (vehicle && vehicle->getOwner()) ? vehicle->getOwner() : nullptr;
}

/*------------------------------------------------------------------------------------------------------------*/
// INSPECTION
/*------------------------------------------------------------------------------------------------------------*/

Inspection::Inspection(int id, Surveyor* name, char* date, char* findings, Claim* claim) {
	inspectionID = id;
	inspector = name;
	this->date = new char[strlen(date) + 1];
	strcpy(this->date, date);
	this->findings = new char[strlen(findings) + 1];
	strcpy(this->findings, findings);
	this->claim = claim;
}

void Inspection::performInspection() {
	cout << inspector->getName() << " is performing the inspection..." << endl;
}

char* Inspection::generateInspectionReport() {
	char tmp[512];
	sprintf(tmp, "Inspection ID: %d | Date: %s | Findings: %s",
		inspectionID, date, findings);
	char* report = new char[strlen(tmp) + 1];
	strcpy(report, tmp);
	return report;
}

/*------------------------------------------------------------------------------------------------------------*/
// WORKSHOP
/*------------------------------------------------------------------------------------------------------------*/

Workshop::Workshop(char* id, char* addr, vector<Staff*> staff, vector<Vehicle*> vehicles) {
	workshopID = new char[strlen(id) + 1];
	strcpy(workshopID, id);
	address = new char[strlen(addr) + 1];
	strcpy(address, addr);
	staffList = staff;
	vehicleList = vehicles;
}

void Workshop::assignVehicleWorkshop(Vehicle* v) {
	vehicleList.push_back(v);
}

char* Workshop::getID() const { return workshopID; }

/*------------------------------------------------------------------------------------------------------------*/
// DESTRUCTORS
/*------------------------------------------------------------------------------------------------------------*/

// Clean up dynamically allocated objects to prevent memory leaks
Company::~Company() {
	for (Customer* c : customers) delete c;
	for (Workshop* w : workshops) delete w;
	for (Claim* cl : claims) delete cl;
}

// Clean up dynamically allocated objects to prevent memory leaks
Customer::~Customer() {
	for (Vehicle* v : vehicle) delete v;
}

// Clean up dynamically allocated objects to prevent memory leaks
Workshop::~Workshop() {
	for (Staff* s : staffList) delete s;
	for (Vehicle* v : vehicleList) delete v;
}

/*------------------------------------------------------------------------------------------------------------*/
// application layer
/*------------------------------------------------------------------------------------------------------------*/

// reads user input from menu()
// calls the right company or customer methods
// handles flow

// Initializes the insurance system controller.
// Creates a Company object with provided customers, workshops, and claims.
// CHANGE: Manager* added as parameter so it is not created inside methods (reduces IF4)
class InsuranceSystemController {
private:
	Company* c;
	Manager* mgr;  // CHANGE: manager stored as member, passed from menu
public:
	InsuranceSystemController(char*, char*, vector<Customer*>, vector<Workshop*>, vector<Claim*>, Manager*);
	void registerCustomer(char*, char*, char*);
	void registerVehicle(int, int);
	void issueInsurancePolicy();
	void submitClaim();
	void inspectionBySurveyor();
	void claimApprovalByManager();
	void vehicleRepairAtRegisteredWorkshop();
	void registerWorkshop(char*, char*, vector<Staff*>);
	void displayNewCustomersWon();
	void displayPendingClaims();
	void displayInspectionReports();
	void displayCustomerClaimHistory(int);
	void saveInspection(Inspection*);
};

// CHANGE: Manager* added as parameter
InsuranceSystemController::InsuranceSystemController(char* name, char* addr,
	vector<Customer*> customer, vector<Workshop*> workshop, vector<Claim*> claim, Manager* manager) {
	c = new Company(name, addr, customer, workshop, claim);
	mgr = manager;
}

// Handles customer registration workflow.
void InsuranceSystemController::registerCustomer(char* name, char* addr, char* number) {
	c->addCustomer(name, addr, number);
}

// Handles vehicle registration workflow.
void InsuranceSystemController::registerVehicle(int customerID, int year) {
	Customer* owner = c->findCustomerByID(customerID);
	if (!owner) {
		cout << "Customer not found with ID " << customerID << endl;
		return;
	}
	// CHANGE: no Workshop* passed, removed from registerVehicle
	c->registerVehicle(owner, year, nullptr);
}

// Handles insurance policy issuance workflow.
void InsuranceSystemController::issueInsurancePolicy() {
	int vehicleID;
	cout << "Enter vehicle ID: ";
	cin >> vehicleID;

	Vehicle* v = c->findVehicleByID(vehicleID);
	if (!v) {
		cout << "Vehicle not found.\n";
		return;
	}

	int policyNum;
	char* expiry = new char[MAX_LEN];
	cout << "Enter policy number: ";
	cin >> policyNum;
	cout << "Enter expiry date: ";
	cin.ignore();
	cin.getline(expiry, MAX_LEN);

	InsuranceDetails* policy = new InsuranceDetails(policyNum, expiry);
	v->setInsurancePolicy(policy);
	v->issueInsurancePolicy();
}

// Handles claim submission workflow.
void InsuranceSystemController::submitClaim() {
	c->createClaim();
}

// Handles inspection workflow.
void InsuranceSystemController::inspectionBySurveyor() {
	int claimID;
	cout << "Enter claim ID: ";
	cin >> claimID;

	Claim* cl = nullptr;
	for (Claim* claim : c->getClaims()) {
		if (claim->getClaimID() == claimID) {
			cl = claim;
			break;
		}
	}

	if (!cl) {
		cout << "Claim not found.\n";
		return;
	}

	char* findings = new char[MAX_LEN];
	cout << "Enter inspection findings: ";
	cin.ignore();
	cin.getline(findings, MAX_LEN);

	Surveyor* surveyor = new Surveyor(1, (char*)"Default Surveyor", nullptr);
	Inspection* insp = new Inspection(1, surveyor, (char*)"today", findings, cl);
	insp->performInspection();
	c->saveInspection(insp);

	char* report = insp->generateInspectionReport();
	cout << report << endl;
	delete[] report;
}

// Handles claim approval workflow.
// CHANGE: uses mgr member instead of creating new Manager inside method
void InsuranceSystemController::claimApprovalByManager() {
	int claimID;
	cout << "Enter claim ID: ";
	cin >> claimID;

	Claim* cl = nullptr;
	for (Claim* claim : c->getClaims()) {
		if (claim->getClaimID() == claimID) {
			cl = claim;
			break;
		}
	}

	if (!cl) {
		cout << "Claim not found.\n";
		return;
	}

	int choice;
	cout << "Approve (1) or Reject (2): ";
	cin >> choice;

	if (choice == 1) {
		mgr->approveClaim(cl);
	}
	else {
		cl->setStatus(rejected);
		cout << "Claim rejected.\n";
	}
}

// Handles vehicle repair workflow.
void InsuranceSystemController::vehicleRepairAtRegisteredWorkshop() {
	int vehicleID;
	cout << "Enter vehicle ID: ";
	cin >> vehicleID;

	Vehicle* v = nullptr;
	for (Customer* cust : c->getCustomers()) {
		for (Vehicle* veh : cust->getVehicles()) {
			if (veh->getVehicleID() == vehicleID) {
				v = veh;
				break;
			}
		}
		if (v) break;
	}

	if (!v) {
		cout << "Vehicle not found.\n";
		return;
	}

	char* workshopID = new char[MAX_LEN];
	cout << "Enter workshop ID: ";
	cin.ignore();
	cin.getline(workshopID, MAX_LEN);

	Workshop* w = nullptr;
	for (Workshop* ws : c->getWorkshops()) {
		if (strcmp(ws->getID(), workshopID) == 0) {
			w = ws;
			break;
		}
	}

	if (!w) {
		cout << "Workshop not found.\n";
		return;
	}

	w->assignVehicleWorkshop(v);
	cout << "Vehicle assigned to workshop.\n";
}

// Handles workshop registration workflow.
void InsuranceSystemController::registerWorkshop(char* id, char* addr, vector<Staff*> staff) {
	c->registerWorkshop(id, addr, staff);
}

// Displays newly registered customers.
void InsuranceSystemController::displayNewCustomersWon() { c->displayNewCustomersWon(); }

// Displays all pending claims.
void InsuranceSystemController::displayPendingClaims() { c->displayPendingClaims(); }

// Displays all inspection reports.
void InsuranceSystemController::displayInspectionReports() { c->displayInspectionReports(); }

// Displays claim history for a given customer.
void InsuranceSystemController::displayCustomerClaimHistory(int id) {
	Customer* cust = c->findCustomerByID(id);
	if (!cust) {
		cout << "Customer not found with ID " << id << endl;
		return;
	}
	c->displayCustomerClaimHistory(cust);
}

/*------------------------------------------------------------------------------------------------------------*/
// presentation layer - user interface
/*------------------------------------------------------------------------------------------------------------*/

void menu() {

	//get company details from user
	int userInput;
	char* name = new char[MAX_LEN];
	cout << "Enter Company Name: ";
	cin.getline(name, MAX_LEN);

	char* addr = new char[MAX_LEN];
	cout << "Enter Address: ";
	cin.getline(addr, MAX_LEN);

	//  Manager created once here and passed to controller
	char mname[] = "default manager";
	char mdes[] = "claims manager";
	Manager manager(1, mname, mdes);

	vector<Customer*> customer;
	vector<Workshop*> workshop;
	vector<Claim*> claim;
	//intitialize controller
	InsuranceSystemController i(name, addr, customer, workshop, claim, &manager);
	cout << endl;
	//run main menu loop
	do {
		cout <<
			"1. Register Customer\n2. Register Vehicle\n3. Issue Insurance Policy\n4. Submit Claim\n5. Inspection By Surveyor\n" <<
			"6. Claim Approval By Manager\n7. Register Workshop\n8. Vehicle Repair At Registered Workshop\n9. Display New Customers Won\n"
			<< "10. Display Pending Claims\n11. Display Inspection Reports\n12. Display Customer Claim History\n13. Exit" << endl;

		cout << "Enter Input: ";
		cin >> userInput;
		cout << endl;

		if (userInput == 1) {
			cin.ignore();
			cout << "Enter customer name: ";
			char* name = new char[MAX_LEN];
			cin.getline(name, MAX_LEN);

			cout << "Enter customer address: ";
			char* addr = new char[MAX_LEN];
			cin.getline(addr, MAX_LEN);

			cout << "Enter customer phone number: ";
			char* number = new char[MAX_LEN];
			cin.getline(number, MAX_LEN);

			i.registerCustomer(name, addr, number);
		}
		else if (userInput == 2) {
			int customerID, year;
			cout << "Enter customer ID: ";
			cin >> customerID;
			cout << "Enter year: ";
			cin >> year;
			i.registerVehicle(customerID, year);
		}
		else if (userInput == 3) {
			i.issueInsurancePolicy();
		}
		else if (userInput == 4) {
			i.submitClaim();
		}
		else if (userInput == 5) {
			i.inspectionBySurveyor();
		}
		else if (userInput == 6) {
			i.claimApprovalByManager();
		}
		else if (userInput == 7) {
			cin.ignore();
			char* workshopID = new char[MAX_LEN];
			char* addr = new char[MAX_LEN];
			cout << "Enter workshop ID: ";
			cin.getline(workshopID, MAX_LEN);
			cout << "Enter workshop address: ";
			cin.getline(addr, MAX_LEN);
			vector<Staff*> staff;
			i.registerWorkshop(workshopID, addr, staff);
		}
		else if (userInput == 8) {
			i.vehicleRepairAtRegisteredWorkshop();
		}
		else if (userInput == 9) {
			i.displayNewCustomersWon();
		}
		else if (userInput == 10) {
			i.displayPendingClaims();
		}
		else if (userInput == 11) {
			i.displayInspectionReports();
		}
		else if (userInput == 12) {
			int customerID;
			cout << "Enter customer ID: ";
			cin >> customerID;
			i.displayCustomerClaimHistory(customerID);
		}
		else if (userInput == 13) {
			cout << "Program Terminated" << endl;
		}
		else {
			cout << "Invalid Input" << endl;
		}
		cout << endl;
	} while (userInput != 13);
}

/*------------------------------------------------------------------------------------------------------------*/

int main() {
	menu();
	return 0;
}

