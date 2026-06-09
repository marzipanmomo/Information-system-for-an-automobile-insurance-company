Automobile Insurance Information System

A console-based information system for an automobile insurance company, built in C++. The system manages customers, vehicles, insurance policies, claims, inspections, workshops, and staff through a layered architecture with file-based data persistence.


 Features

- Register customers and their vehicles
- Issue insurance policies to vehicles
- Submit and track claims
- Conduct inspections by surveyors
- Approve or reject claims by manager
- Assign damaged vehicles to registered workshops for repair
- View pending claims, inspection reports, and customer claim history
- Data persisted to flat files — no database required



## Architecture

The system is divided into three layers:

```
Presentation Layer      →   menu() function — handles user input only
Application Layer       →   InsuranceSystemController — coordinates all use cases
Business Layer          →   Company, Customer, Vehicle, Claim, Inspection, Staff, Workshop
Persistence Layer       →   Repository classes — all file I/O isolated here
```

### Business Layer Classes

| Class | Responsibility |
|-------|---------------|
| `Company` | Central class — manages customers, workshops, and claims |
| `Customer` | Stores personal details and owns vehicles |
| `Vehicle` | Linked to a customer and an insurance policy |
| `InsuranceDetails` | Stores policy number and expiry date |
| `Claim` | Raised when a vehicle is damaged |
| `Inspection` | Performed by a surveyor, linked to a claim |
| `Workshop` | Registered workshop where vehicles are repaired |
| `Staff` | Abstract base class for all staff |
| `Manager` | Approves or rejects claims |
| `Salesman` | Registers new customers |
| `Surveyor` | Performs inspections and submits reports |

### Persistence Layer Classes

| Class | File |
|-------|------|
| `CustomerRepository` | `customers.dat` |
| `VehicleRepository` | `vehicles.dat` |
| `ClaimRepository` | `claims.dat` |
| `InspectionRepository` | `inspections.dat` |
| `CompanyRepository` | `company.dat` |

---

## Getting Started

### Requirements

- Windows — Visual Studio 2019 or later (uses `strcpy_s`, `strtok_s`)
- Linux/Mac — GCC with the provided standard version (uses `strcpy`, `strtok_r`)

### Build and Run

**Visual Studio:**
1. Create a new empty C++ project
2. Add `auto_system.cpp` to the project
3. Press `Ctrl+F5` to build and run

**GCC (Linux/Mac):**
```bash
g++ auto_system.cpp -o auto_system
./auto_system
```

---

## Usage

On startup the program asks for the company name and address, then shows the main menu:

```
1.  Register Customer
2.  Register Vehicle
3.  Issue Insurance Policy
4.  Submit Claim
5.  Inspection By Surveyor
6.  Claim Approval By Manager
7.  Register Workshop
8.  Vehicle Repair At Registered Workshop
9.  Display New Customers Won
10. Display Pending Claims
11. Display Inspection Reports
12. Display Customer Claim History
13. Exit
```

### Typical workflow

```
1  → Register a customer          (gets a customer ID)
2  → Register a vehicle           (uses the customer ID)
3  → Issue an insurance policy    (uses the vehicle ID)
4  → Submit a claim               (uses the vehicle ID)
5  → Inspect by surveyor          (uses the claim ID)
6  → Manager approves/rejects     (uses the claim ID)
7  → Register a workshop          (gets a workshop ID)
8  → Assign vehicle to workshop   (uses vehicle ID + workshop ID)
```

---

## Data Files

All data is saved automatically to flat text files in the same directory as the executable:

| File | Contents |
|------|----------|
| `customers.dat` | `customerID|name|address|phone` |
| `vehicles.dat` | `vehicleID|year|ownerID` |
| `claims.dat` | `claimID|status|date|ownerID` |
| `inspections.dat` | `inspectionID|date|findings|claimID` |
| `company.dat` | `companyName|address` |

---

## Code Quality

Code quality was measured using [CCCC (C and C++ Code Counter)](https://sourceforge.net/projects/cccc/).

| Metric | Value | Meaning |
|--------|-------|---------|
| LOC | 867 | Lines of code |
| COM | 166 | Lines of comments |
| L_C | 5.22 | Lines of code per comment (lower is better) |
| M_C | 0.89 | Complexity per comment (below 1 is good) |
| MVG | 148 | Total cyclomatic complexity |
| IF4 | 219 | Information flow / coupling |

Key improvements made after CCCC analysis:
- Repository classes introduced to isolate all file I/O from business classes
- Comment density improved significantly (L_C reduced from 7.7 to 5.2)
- Coupling reduced (IF4 reduced from 323 to 219)
- `parseLine()` helper added to `CustomerRepository` to reduce function complexity
- Manager object created once in `menu()` instead of repeatedly inside controller methods

---

## Project Structure

```
auto_system.cpp        main source file (single file project)
customers.dat          generated at runtime
vehicles.dat           generated at runtime
claims.dat             generated at runtime
inspections.dat        generated at runtime
company.dat            generated at runtime
class_diagram.drawio   UML class diagram (open in draw.io)
README.md              this file
```

---

## Class Diagram

The UML class diagram is included as `class_diagram.drawio`. Open it at [app.diagrams.net](https://app.diagrams.net) via **File → Open From → Device**.

---

## Limitations

- Data is stored in plain text files — no encryption or access control
- No update or delete operations on existing records
- IDs are assigned sequentially at runtime and reset on each run
- Single-user system — no concurrent access handling

---

## Course

Software Design and Architecture — Spring 2026  
Department of Computer Science
