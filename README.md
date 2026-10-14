# SmartFare — Smart Vehicle Fare System

A C++17 console-based vehicle booking and fare management system with multiple vehicle categories, configurable fare rules, customer wallets, loyalty rewards, coupons, persistent trip records, driver dispatch, trip lifecycle management, and an administrative transaction ledger.

> **Project type:** Educational console application  
> **Language:** C++17  
> **External libraries:** None required

## Table of Contents

- [Overview](#overview)
- [Key Features](#key-features)
- [Supported Vehicles](#supported-vehicles)
- [Fare Calculation](#fare-calculation)
- [Trip Lifecycle](#trip-lifecycle)
- [Main Menu](#main-menu)
- [Admin Panel](#admin-panel)
- [Project Structure](#project-structure)
- [Architecture](#architecture)
- [Requirements](#requirements)
- [Build and Run](#build-and-run)
- [Run Self-Tests](#run-self-tests)
- [Persistent Data Files](#persistent-data-files)
- [Default Coupons and Admin Access](#default-coupons-and-admin-access)
- [Limitations](#limitations)
- [Future Enhancements](#future-enhancements)
- [Troubleshooting](#troubleshooting)

## Overview

SmartFare simulates the core workflow of a vehicle booking service. Customers can compare vehicle fares, request rides, manage a wallet, use coupons and loyalty points, review trip history, and view vehicle specifications. Administrators can manage vehicle availability, configure surge pricing and coupons, review performance reports, dispatch requested rides, advance trips through lifecycle states, and inspect or export transaction records.

The application saves important records to local files, allowing trips, customer profiles, coupons, and transaction history to be loaded again when the program starts.

## Key Features

- **Multiple vehicle categories:** Car, Bus, AC Bus, Bike, Auto, Luxury Car, SUV, MiniVan, Truck, Bicycle, and Electric Car.
- **Fare estimates:** Vehicle-specific base fares, distance-based pricing, round-trip discount, optional night and peak charges, waiting charges, luggage charges, baggage fees, and comfort surcharges.
- **Dynamic surge pricing:** An administrator can set a surge multiplier from greater than `0` up to `5x`.
- **Customer profiles:** Customer type, phone number, preferred payment method, wallet balance, loyalty points, applied coupon, and trip history.
- **Wallet and payment options:** Cash, Card, UPI, and Wallet selection. Wallet top-ups and wallet-related events can be recorded in the transaction ledger.
- **Coupons and rewards:** Built-in coupons, configurable coupon limits, one-use-per-customer options, student/senior/member discounts, loyalty tiers, and point redemption.
- **Smart vehicle suggestions:** Filters by passenger capacity, availability, AC requirement, and budget; shows fare and estimated travel time.
- **Driver dispatch:** Assigns the designated vehicle driver to a requested booking and tracks driver availability.
- **Trip lifecycle control:** Validates transitions before a trip can be assigned, started, completed, or cancelled.
- **Cancellation handling:** Calculates a cancellation fee and records eligible wallet refunds.
- **Operating analytics:** Revenue and estimated operating-cost reports, driver leaderboard, vehicle ratings, top customers, and trip summaries.
- **Persistent storage:** Saves trips, profiles, coupons, and transactions to local files.
- **CSV exports:** Export trip and transaction records for external analysis.
- **Input validation and exceptions:** Handles invalid distances, malformed numeric input, invalid coupons, unavailable vehicles, baggage limits, and insufficient wallet balance.
- **Self-test mode:** Runs a lightweight set of checks without opening the interactive menus.

## Supported Vehicles

| Vehicle | Example characteristics |
|---|---|
| Car | Standard passenger car with AC |
| Bus | High-capacity non-AC bus |
| AC Bus | High-capacity air-conditioned bus |
| Bike | Single-passenger option |
| Auto | Three-passenger auto-rickshaw |
| Luxury Car | Premium car with higher base pricing |
| SUV | Larger passenger capacity |
| MiniVan | Group travel option |
| Truck | Small goods-transport model |
| Bicycle | Low-cost, eco-friendly option |
| Electric Car | Electric vehicle model with a configured range limit |

Vehicle rates and specifications are configured in the source code and can be adjusted for demonstration purposes.

## Fare Calculation

The estimate can include several components depending on the selected vehicle and customer choices:

1. **Vehicle base fare:** Calculated according to the vehicle's fare model and distance.
2. **Round-trip discount:** A 10% discount on the base fare when round trip is selected.
3. **Surge multiplier:** Configurable by the administrator.
4. **Time-based charges:** Optional night charge of 20% and peak-hour charge of 15%; the system can also detect these periods from the local system clock.
5. **Waiting charge:** Rs. 2 per waiting minute.
6. **Luggage charge:** Rs. 10 per kilogram when luggage weight is entered.
7. **Baggage fees:** Small, medium, and large bag categories use separate fees; eligible vehicle types may include one free bag.
8. **Comfort surcharges:** Optional AC and music-system surcharges when the selected vehicle supports the requested feature.
9. **Discounts:** Customer category discount, loyalty-tier bonus, valid coupon, and eligible redeemed loyalty points.

The final amount is an estimate produced by the program's configured rules. It is not connected to a real route or live transport tariff service.

## Trip Lifecycle

Bookings use explicit status transitions to prevent invalid jumps between states:

```text
Requested ──> Assigned ──> In Progress ──> Completed
     │             │
     └─────────────┴──────────────> Cancelled
```

The program validates supported transitions. Completed trips cannot be cancelled, and a requested trip cannot skip directly to completion. A vehicle/driver marked busy by an active trip should not be assigned to another trip until released.

## Main Menu

The interactive customer menu offers:

1. Register or switch customer
2. Book a ride
3. View available vehicles
4. View personal trip history
5. Compare two vehicle fares
6. View trip analytics
7. Merge two customer histories for a combined report
8. View full vehicle specifications
9. Manage wallet, loyalty points, and coupons
10. Get smart vehicle suggestions
11. Cancel the latest eligible ride
12. Open the admin panel
13. Exit

## Admin Panel

The admin panel contains tools for:

- Fleet/customer totals, bookings, earnings, and current surge multiplier
- Viewing the saved trip log
- Updating the surge multiplier
- Reviewing vehicle ratings
- Viewing the driver leaderboard
- Reviewing revenue, estimated fuel/operating costs, and margins
- Viewing top customers by completed-trip spending
- Adding, enabling, and disabling coupons
- Toggling vehicle maintenance mode
- Exporting trips to `trips_report.csv`
- Changing the admin password for the current application session
- Managing the trip dispatch and lifecycle workflow
- Viewing and exporting the transaction ledger

## Project Structure

The repository is a single-source-file C++ project. Data and report files are created in the working directory when the application uses those features.

```text
SmartFare/
├── README.md
├── smart_vehicle_fare_advanced.cpp
├── trips.dat                  # generated: saved trip records
├── customers.dat              # generated: customer profiles and wallet details
├── coupons.dat                # generated: coupon configuration and usage
├── transactions.dat           # generated: transaction ledger
├── trip_history.txt           # generated: readable trip log
├── trips_report.csv           # generated when trip export is selected
└── transactions_report.csv    # generated when transaction export is selected
```

Do not create the generated data files manually unless you understand the record formats. Keep backups of the files if you need to preserve application history.

## Architecture

The program is organized into classes and helper functions within `smart_vehicle_fare_advanced.cpp`:

| Component | Responsibility |
|---|---|
| `util` namespace | Input handling, string cleanup, formatting, and time helpers |
| Fare exception classes | Represent invalid inputs, distance errors, unavailable vehicles, coupon errors, and wallet errors |
| `Vehicle` and derived classes | Vehicle specifications, fare calculation, capacity, estimated time, and operating-cost estimates |
| `Customer` | Customer profile, trip history, wallet, coupons, loyalty points, and tier calculation |
| `CouponManager` | Coupon definitions, validation, usage limits, activation, and persistence |
| `FareCalculator` | Surge, night/peak, waiting, luggage, baggage, comfort charges, and discounts |
| `FileHandler` | Saving/loading trip data and customer profiles; writing trip logs and CSV exports |
| `TransactionLedger` | Recording, displaying, loading, and exporting transaction records |
| `ReceiptGenerator` | Printing a booking receipt |
| `AdminPanel` | Administrative reports, coupon and maintenance controls, dispatch, trip lifecycle, and ledger access |
| `SmartFareApp` | Application initialization, customer menu, booking flow, and shutdown persistence |
| `runSelfTests()` | Lightweight programmatic checks for selected rules and serialization flows |

## Requirements

- A C++ compiler with C++17 support, such as GCC or Clang
- Terminal or command-line access
- Write permission in the directory where the program runs, because it creates local data and report files

No external packages or libraries are required.

## Build and Run

Place `smart_vehicle_fare_advanced.cpp` in your project directory and open Terminal in that directory.

### macOS or Linux

Compile with warnings enabled:

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic smart_vehicle_fare_advanced.cpp -o smartfare
```

Run the application:

```bash
./smartfare
```

### Windows (MinGW-w64)

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic smart_vehicle_fare_advanced.cpp -o smartfare.exe
smartfare.exe
```

## Run Self-Tests

Run the built-in tests without opening the customer/admin menus:

```bash
./smartfare --self-test
```

On Windows, run:

```bash
smartfare.exe --self-test
```

The checks cover selected trip-state transitions, cancellation-fee cases, trip record serialization, backward compatibility with the older 16-field trip record, transaction serialization, a sample fare calculation, negative-distance rejection, and customer profile serialization/restoration.

The included self-tests are useful smoke tests, but they are not a substitute for comprehensive automated testing of all booking, payment, refund, file-system, and edge-case scenarios.

## Persistent Data Files

| File | Purpose |
|---|---|
| `trips.dat` | Structured trip records, including booking status, fare, vehicle, customer, and route fields |
| `customers.dat` | Customer details, wallet balance, points, preferred payment method, and used coupons |
| `coupons.dat` | Coupon settings and usage counters |
| `transactions.dat` | Transaction entries such as wallet top-ups, payments, refunds, and cancellation fees |
| `trip_history.txt` | Human-readable trip log |
| `trips_report.csv` | Trip export generated through the admin panel |
| `transactions_report.csv` | Transaction export generated through the transaction ledger menu |

These are local flat files rather than a database. The program should be run from a directory where it can read and write these files consistently.

## Default Coupons and Admin Access

The source initializes these coupons when no saved coupon configuration overrides them:

| Coupon | Discount | Usage rule |
|---|---:|---|
| `SAVE10` | 10% | Repeatable |
| `FIRST50` | 50% | Once per customer |
| `VIT25` | 25% | Repeatable |
| `WELCOME15` | 15% | Up to 100 uses, once per customer |

The source code sets the initial admin password to `admin123`. Change it before demonstrating the application to others. The current password-change feature applies to the running session; it is not a production authentication system and the password is not securely hashed or managed through a user database.

## Limitations

- This is a console application, not a web or mobile app.
- Pickup and destination are stored as text; road distance is entered by the user.
- ETA is an estimate based on configured average vehicle speed, not live traffic.
- Driver assignment uses the predefined vehicle/driver records; no live GPS-based nearest-driver service is connected.
- Card and UPI are selectable payment labels only. The application does not contact a bank, payment gateway, or UPI provider.
- Data is stored in local text files, not a transactional database. Back up these files and avoid editing them while the application is running.
- Admin authentication is suitable only for a local educational demonstration, not a public deployment.
- Operating costs and profitability are simplified estimates based on configured mileage and fuel-price assumptions.

## Future Enhancements

- Add a REST API and a responsive web frontend for customers, drivers, and administrators.
- Integrate a maps/geocoding service for route distance, toll estimates, alternate routes, and map-based pickup selection.
- Add real GPS tracking and driver-location updates through a secure service.
- Implement a stronger dispatch engine based on distance, driver availability, vehicle capacity, ratings, and estimated arrival time.
- Replace flat-file persistence with SQLite or PostgreSQL and use database transactions for bookings, wallets, refunds, and state changes.
- Add secure authentication with password hashing, role-based permissions, session expiry, and audit controls.
- Integrate a payment gateway in a sandbox before considering production payment workflows.
- Add scheduled bookings, trip sharing, emergency workflows, lost-and-found reports, and accessibility requirements.
- Add unit/integration tests for all fare rules, coupon limits, refunds, persistence failures, and concurrent booking scenarios.
- Add CI builds, static analysis, code formatting, and automated regression tests.

## Troubleshooting

**`g++: command not found`**  
Install a C++ compiler toolchain, then confirm it is available with `g++ --version` or `clang++ --version`.

**The application cannot save records**  
Run it from a writable folder and check file permissions. The application uses relative paths, so data files are created in the current working directory.

**A trip cannot be assigned or started**  
Check its current status in the dispatch menu. Only valid lifecycle transitions are accepted, and a busy or maintained vehicle/driver cannot be assigned to a new ride.

**A wallet payment falls back to Cash**  
The current wallet balance may be lower than the estimated final fare. Top up the wallet before booking with Wallet as the preferred payment method.

---

Built as an educational C++17 project to demonstrate object-oriented programming, inheritance, polymorphism, operator overloading, templates, lambdas, exception handling, file persistence, fare modelling, and administrative analytics.
